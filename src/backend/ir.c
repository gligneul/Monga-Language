/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ir.c
 */

#include <assert.h>
#include <string.h>

#include <llvm-c/Analysis.h>

#include "ir.h"

#include "scanner/scanner.h"
#include "util/new.h"
#include "util/table.h"

/* 
 * SECTION: Declarations
 */

/* State passed throughout the compile functions */
typedef struct IRState {
    /* The module that contains the compiled program */
    LLVMModuleRef module;

    /* LLVM Builder */
    LLVMBuilderRef builder;

    /* Printf prototype reference */
    LLVMValueRef printf;

    /* Maps literal strings to LLVMValueRef */
    TableRef strings;

    /* Map AstDeclaration to LLVMValueRef */
    TableRef declarations;

    /* Current function */
    LLVMValueRef function;
} IRState;

const int MAX_N_PARAMETERS = 64;
const int MAX_FORMAT = 1024;

/* Verifies if the LLVM module is correct */
static void verifyModule(LLVMModuleRef module);

/* Creates the initial state */
static IRState* createState(LLVMModuleRef module);

/* Destroys the state */
static void destroyState(IRState* state);

/* Creates the equivalent llvm type */
static LLVMTypeRef createType(Type type);

/* Creates the equivalent llvm function type for the input ast declaration */
static LLVMTypeRef createFunctionType(AstDeclaration* function);

/* Creates the printf prototype */
static LLVMValueRef createPrintfPrototype(LLVMModuleRef module);

/* Creates the format function based on a list of expressions */
static LLVMValueRef createPrintfFormat(AstExpression* expressions,
        IRState* state);

/* Create hidden global variables for each literal string */
static void compileStrings(IRState* state);

/* Create global variables */
static void compileGlobalVariables(AstDeclaration* tree, IRState* state);

/* Compiles functions declarations */
static void compileFunctionsDeclarations(AstDeclaration* tree, IRState* state);

/* Compiles functions parameters references */
static void compileFunctionParetersReferences(AstDeclaration* function,
        IRState* state);

/* Compiles the variables by initializing them with empty values */
static void compileLocalVariables(AstDeclaration* variables,
        IRState* state);

/* Compiles the statements, returns the output basic block */
static LLVMBasicBlockRef compileStatements(AstStatement* statement,
        LLVMBasicBlockRef in_block, IRState* state);
static LLVMBasicBlockRef compileStatementIf(AstStatement* statement,
        LLVMBasicBlockRef in_block, IRState* state);
static LLVMBasicBlockRef compileStatementWhile(AstStatement* statement,
        LLVMBasicBlockRef in_block, IRState* state);
static void compileStatementAssign(AstStatement* statement, IRState* state);
static void compileStatementDelete(AstStatement* statement, IRState* state);
static void compileStatementPrint(AstStatement* statement, IRState* state);
static void compileStatementReturn(AstStatement* statement, IRState* state);

/* Compiles expressions */
static LLVMValueRef compileExpression(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionString(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionCall(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionVariable(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionNew(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionUnary(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionBinary(AstExpression* expression,
        IRState* state);
static LLVMValueRef compileExpressionBinaryInt(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);
static LLVMValueRef compileExpressionBinaryFloat(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);
static LLVMValueRef compileExpressionBinaryIntCmp(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);
static LLVMValueRef compileExpressionBinaryFloatCmp(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);
static LLVMValueRef compileExpressionCast(AstExpression* expression,
        IRState* state);

/* Returns the pointer to the array's element */
static LLVMValueRef compileVariableArray(AstVariable* variable, IRState* state);

/* 
 * SECTION: Implementation
 */

LLVMModuleRef IRCompileModule(AstDeclaration* tree)
{
    LLVMModuleRef module = LLVMModuleCreateWithName("monga-executable");
    IRState* state = createState(module);
    compileGlobalVariables(tree, state);
    compileStrings(state);
    compileFunctionsDeclarations(tree, state);
    destroyState(state);
    verifyModule(module);
    return module;
}

static void verifyModule(LLVMModuleRef module)
{
    char *error = NULL;
    LLVMVerifyModule(module, LLVMAbortProcessAction, &error);
    LLVMDisposeMessage(error);
}

static IRState* createState(LLVMModuleRef module)
{
    IRState* state = NEW(IRState);
    state->module = module;
    state->builder = LLVMCreateBuilder();
    state->printf = createPrintfPrototype(module);
    state->strings = TableCreate(TableDummyDestroy, TableDummyDestroy,
            TableDummyCopy, TableDummyCopy, TableDummyLess);
    state->declarations = TableCreate(TableDummyDestroy, TableDummyDestroy,
            TableDummyCopy, TableDummyCopy, TableDummyLess);
    return state;
}

static void destroyState(IRState* state)
{
    LLVMDisposeBuilder(state->builder);
    TableDestroy(state->strings);
    TableDestroy(state->declarations);
    free(state);
}

static LLVMTypeRef createType(Type type)
{
    LLVMTypeRef llvm_type;
    switch (type.tag) {
        case TYPE_VOID:
            llvm_type = LLVMVoidType();
            break;
        case TYPE_BOOL:
            llvm_type = LLVMIntType(1);
            break;
        case TYPE_CHAR:
            llvm_type = LLVMInt8Type();
            break;
        case TYPE_INT:
            llvm_type = LLVMInt32Type();
            break;
        case TYPE_FLOAT:
            llvm_type = LLVMFloatType();
            break;
        case TYPE_UNDEFINED:
            // Unexpected case
            assert(false);
    }

    for (int i = 0; i < type.pointers; ++i) {
        llvm_type = LLVMPointerType(llvm_type, 0);
    }
    return llvm_type;
}

static LLVMTypeRef createFunctionType(AstDeclaration* function)
{
    int n_parameters = function->u.function_.n_parameters;
    LLVMTypeRef parameters_types[n_parameters];
    AstDeclaration* parameter = function->u.function_.parameters;
    for (int i = 0; i < n_parameters; ++i) {
        parameters_types[i] = createType(parameter->type);
        parameter = parameter->next;
    }
    LLVMTypeRef return_type = createType(function->type);
    return LLVMFunctionType(return_type, parameters_types, n_parameters, false);
}

static LLVMValueRef createPrintfPrototype(LLVMModuleRef module)
{
    LLVMTypeRef param_types[] = {LLVMPointerType(LLVMInt8Type(), 0)};
    LLVMTypeRef type = LLVMFunctionType(LLVMInt32Type(), param_types, 1,
            true);
    LLVMValueRef function = LLVMAddFunction(module, "printf", type);
    return function;
}

static LLVMValueRef createPrintfFormat(AstExpression* expressions,
        IRState* state)
{
    char format[MAX_FORMAT];
    format[0] = '\0';
    Type last_type = TypeCreate(TYPE_UNDEFINED, 0);
    AST_FOREACH(AstExpression, expression, expressions) {
        Type type = expression->type;
        if (TypeIsString(type)) {
            strcat(format, "%s");
        } else if (TypeIsArray(type)) {
            strcat(format, "<pointer> (0x %p)");
        } else if (TypeIsBool(type)) {
            // TODO alterar para bool ? "true" : "false"
            strcat(format, "%d");
        } else if (TypeIsInt(type)) {
            strcat(format, "%d");
        } else if (TypeIsFloat(type)) {
            strcat(format, "%f");
        } else if (TypeIsVoid(type)) {
            strcat(format, "<void>");
        } else {
            assert(false);
        }
        last_type = type;
    }
    if (!TypeIsString(last_type))
    {
        strcat(format, "\n");
    }
    LLVMValueRef llvm_format = 
            LLVMBuildGlobalString(state->builder, format, "");
    return LLVMBuildPointerCast(state->builder, llvm_format,
            LLVMPointerType(LLVMInt8Type(), 0), "");
}

static void compileStrings(IRState* state)
{
    Vector* strings = ScannerGetStrings();
    if (strings == NULL)
        return;

    for (size_t i = 0; i < VectorSize(strings); ++i) {
        char* string = (char*)VectorGet(strings, i);
        size_t len = strlen(string);
        LLVMTypeRef type = LLVMArrayType(LLVMInt8Type(), len + 1);
        LLVMValueRef llvm_string = LLVMAddGlobal(state->module, type, "");
        LLVMSetInitializer(llvm_string, LLVMConstString(string, len, false));
        LLVMSetVisibility(llvm_string, LLVMHiddenVisibility);
        TableInsert(state->strings, string, llvm_string);
    }
}

static void compileGlobalVariables(AstDeclaration* tree, IRState* state)
{
    AST_FOREACH(AstDeclaration, variable, tree) {
        if (variable->tag != AST_DECLARATION_VARIABLE)
            continue;

        LLVMTypeRef type = createType(variable->type);
        LLVMValueRef llvm_variable = LLVMAddGlobal(state->module, type,
                variable->identifier);
        LLVMSetInitializer(llvm_variable, LLVMConstNull(type));
        TableInsert(state->declarations, variable, llvm_variable);
    }
}

static void compileFunctionsDeclarations(AstDeclaration* tree, IRState* state)
{
    AST_FOREACH(AstDeclaration, function, tree) {
        if (function->tag != AST_DECLARATION_FUNCTION)
            continue;

        LLVMTypeRef type = createFunctionType(function);
        state->function = LLVMAddFunction(state->module, function->identifier,
                type);
        TableInsert(state->declarations, function, state->function);
        compileFunctionParetersReferences(function, state);
        AstStatement* block = function->u.function_.block;
        LLVMBasicBlockRef entry_block =
                LLVMAppendBasicBlock(state->function, "entry");
        compileStatements(block, entry_block, state);
    }
}

static void compileFunctionParetersReferences(AstDeclaration* function,
        IRState* state)
{
    int n_parameters = function->u.function_.n_parameters;
    AstDeclaration* parameter = function->u.function_.parameters;
    for (int i = 0; i < n_parameters; ++i) {
        LLVMValueRef llvm_parameter = LLVMGetParam(state->function, i);
        TableInsert(state->declarations, parameter, llvm_parameter);
        parameter = parameter->next;
    }
}

static void compileLocalVariables(AstDeclaration* variables,
        IRState* state)
{
    AST_FOREACH(AstDeclaration, variable, variables) {
        LLVMTypeRef type = createType(variable->type);
        LLVMValueRef empty_const = LLVMConstNull(type);
        TableInsert(state->declarations, variable, empty_const);
    }
}

static LLVMBasicBlockRef compileStatements(AstStatement* statement,
        LLVMBasicBlockRef in_block, IRState* state)
{
    if (statement == NULL)
        return in_block;

    LLVMBasicBlockRef out_block = in_block;
    LLVMPositionBuilderAtEnd(state->builder, in_block);

    switch (statement->tag) {
    case AST_STATEMENT_BLOCK:
        compileLocalVariables(statement->u.block_.variables, state);
        out_block = compileStatements(statement->u.block_.statements,
                in_block, state);
        break;
    case AST_STATEMENT_IF:
        out_block = compileStatementIf(statement, in_block, state);
        break;
    case AST_STATEMENT_WHILE:
        out_block = compileStatementWhile(statement, in_block, state);
        break;
    case AST_STATEMENT_ASSIGN:
        compileStatementAssign(statement, state);
        break;
    case AST_STATEMENT_DELETE:
        compileStatementDelete(statement, state);
        break;
    case AST_STATEMENT_PRINT:
        compileStatementPrint(statement, state);
        break;
    case AST_STATEMENT_RETURN:
        compileStatementReturn(statement, state);
        break;
    case AST_STATEMENT_CALL:
        compileExpressionCall(statement->u.call_, state);
        break;
    }

    if (statement->returned)
        return out_block;
    return compileStatements(statement->next, out_block, state);
}

static LLVMBasicBlockRef compileStatementIf(AstStatement* statement,
        LLVMBasicBlockRef in_block, IRState* state)
{
    (void)statement;
    (void)state;
    return in_block;
}

static LLVMBasicBlockRef compileStatementWhile(AstStatement* statement,
        LLVMBasicBlockRef in_block, IRState* state)
{
    (void)statement;
    (void)state;
    return in_block;
}

static void compileStatementAssign(AstStatement* statement, IRState* state)
{
    AstExpression* expression = statement->u.assign_.expression;
    LLVMValueRef value = compileExpression(expression, state);
    AstVariable* variable = statement->u.assign_.variable;

    switch (variable->tag) {
    case AST_VARIABLE_ARRAY: {
        LLVMValueRef array = compileVariableArray(variable, state);
        LLVMBuildStore(state->builder, value, array);
    }
    case AST_VARIABLE_REFERENCE: {
        AstDeclaration* declaration = variable->u.reference_.u.declaration_;
        if (declaration->u.variable_.global) {
            LLVMValueRef llvm_variable =
                    (LLVMValueRef)TableFind(state->declarations, 
                    declaration).data;
            LLVMBuildStore(state->builder, value, llvm_variable);
        } else {
            TableErase(state->declarations, declaration);
            TableInsert(state->declarations, declaration, value);
        }
    }
    }
}

static void compileStatementDelete(AstStatement* statement, IRState* state)
{
    AstExpression* expression = statement->u.delete_.expression;
    LLVMValueRef value = compileExpression(expression, state);
    LLVMBuildFree(state->builder, value);
}

static void compileStatementPrint(AstStatement* statement, IRState* state)
{
    AstExpression* expressions = statement->u.print_.expressions;
    LLVMValueRef parameters[MAX_N_PARAMETERS];
    int n = 0;
    parameters[n++] = createPrintfFormat(expressions, state);
    AST_FOREACH(AstExpression, expression, expressions) {
        Type type = expression->type;
        if (TypeIsArray(type) || TypeIsBool(type) || TypeIsInt(type) ||
            TypeIsFloat(type)) {
            LLVMValueRef value = compileExpression(expression, state);
            if (TypeIsFloat(type))
                value = LLVMBuildFPCast(state->builder, value,
                        LLVMDoubleType(), "");
            parameters[n++] = value;
        }
    }
    LLVMBuildCall(state->builder, state->printf, parameters, n, "");
}

static void compileStatementReturn(AstStatement* statement, IRState* state)
{
    AstExpression* expression = statement->u.return_.expression;

    if (expression != NULL) {
        LLVMValueRef value = compileExpression(expression, state);
        if (TypeIsVoid(expression->type))
            LLVMBuildRetVoid(state->builder);
        else
            LLVMBuildRet(state->builder, value);
    } else {
            LLVMBuildRetVoid(state->builder);
    }
}

static LLVMValueRef compileExpression(AstExpression* expression,
        IRState* state)
{
    switch (expression->tag) {
    case AST_EXPRESSION_KBOOL:
        return LLVMConstInt(LLVMIntType(1), expression->u.kbool_, false);
    case AST_EXPRESSION_KINT:
        return LLVMConstInt(LLVMInt32Type(), expression->u.kint_, false);
    case AST_EXPRESSION_KFLOAT:
        return LLVMConstReal(LLVMFloatType(), expression->u.kfloat_);
    case AST_EXPRESSION_STRING:
        return compileExpressionString(expression, state);
    case AST_EXPRESSION_NULL:
        return LLVMConstNull(createType(expression->type));
    case AST_EXPRESSION_CALL:
        return compileExpressionCall(expression, state);
    case AST_EXPRESSION_VARIABLE:
        return compileExpressionVariable(expression, state);
    case AST_EXPRESSION_NEW:
        return compileExpressionNew(expression, state);
    case AST_EXPRESSION_UNARY:
        return compileExpressionUnary(expression, state);
    case AST_EXPRESSION_BINARY:
        return compileExpressionBinary(expression, state);
    case AST_EXPRESSION_CAST:
        return compileExpressionCast(expression, state);
    }
}

static LLVMValueRef compileExpressionString(AstExpression* expression,
        IRState* state)
{
    LLVMValueRef string =
            TableFind(state->strings, expression->u.string_).data;
    return LLVMBuildPointerCast(state->builder, string,
            LLVMPointerType(LLVMInt8Type(), 0), "");
}

static LLVMValueRef compileExpressionCall(AstExpression* expression,
        IRState* state)
{
    AstDeclaration* declaration = expression->u.call_.u.declaration_;
    LLVMValueRef function = TableFind(state->declarations, declaration).data;
    AstExpression* parameters = expression->u.call_.expressions;
    LLVMValueRef llvm_parameters[MAX_N_PARAMETERS];
    int n = 0;
    AST_FOREACH(AstExpression, parameter, parameters)
        llvm_parameters[n++] = compileExpression(parameter, state);
    return LLVMBuildCall(state->builder, function, llvm_parameters, n, "");
}

static LLVMValueRef compileExpressionVariable(AstExpression* expression,
        IRState* state)
{
    AstVariable* variable = expression->u.variable_;
    switch (variable->tag) {
    case AST_VARIABLE_ARRAY: {
        LLVMValueRef array = compileVariableArray(variable, state);
        return LLVMBuildLoad(state->builder, array, "");
    }
    case AST_VARIABLE_REFERENCE: {
        AstDeclaration* declaration = variable->u.reference_.u.declaration_;
        LLVMValueRef llvm_variable =
                (LLVMValueRef)TableFind(state->declarations, declaration).data;
        if (declaration->u.variable_.global)
            return LLVMBuildLoad(state->builder, llvm_variable, "");
        else
            return llvm_variable;
    }
    }
}

static LLVMValueRef compileExpressionNew(AstExpression* expression,
        IRState* state)
{
    LLVMTypeRef type = createType(expression->u.new_.type);
    LLVMValueRef size = compileExpression(expression->u.new_.expression, state);
    return LLVMBuildArrayMalloc(state->builder, type, size, "");
}

static LLVMValueRef compileExpressionUnary(AstExpression* expression,
        IRState* state)
{
    LLVMValueRef operand =
            compileExpression(expression->u.unary_.expression, state);
    switch (expression->u.unary_.operator) {
    case AST_OPERATOR_NEGATE:
        if (TypeIsInt(expression->type))
            return LLVMBuildNeg(state->builder, operand, "");
        else // if (TypeIsFloat(expression->type))
            return LLVMBuildFNeg(state->builder, operand, "");
    case AST_OPERATOR_NOT:
        return LLVMBuildNot(state->builder, operand, "");
    }
}

static LLVMValueRef compileExpressionBinary(AstExpression* expression,
        IRState* state)
{
    Type type = expression->type;
    AstBinaryOperator operator = expression->u.binary_.operator;
    LLVMValueRef lhs =
            compileExpression(expression->u.binary_.expression_left, state);
    LLVMValueRef rhs =
            compileExpression(expression->u.binary_.expression_right, state);

    if (TypeIsInt(type))
        return compileExpressionBinaryInt(operator, lhs, rhs, state);

    if (TypeIsFloat(type))
        return compileExpressionBinaryFloat(operator, lhs, rhs, state);

    Type sub_expression_type = expression->u.binary_.expression_left->type;
    if (TypeIsInt(sub_expression_type) || TypeIsBool(sub_expression_type) ||
        TypeIsArray(sub_expression_type))
        return compileExpressionBinaryIntCmp(operator, lhs, rhs, state);

    if (TypeIsFloat(sub_expression_type))
        return compileExpressionBinaryFloatCmp(operator, lhs, rhs, state);

    assert(false);
    return NULL;
}

static LLVMValueRef compileExpressionBinaryInt(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state)
{
    switch (operator) {
    case AST_OPERATOR_ADD:
        return LLVMBuildAdd(state->builder, lhs, rhs, "");
    case AST_OPERATOR_SUB:
        return LLVMBuildSub(state->builder, lhs, rhs, "");
    case AST_OPERATOR_MUL:
        return LLVMBuildMul(state->builder, lhs, rhs, "");
    case AST_OPERATOR_DIV:
        return LLVMBuildSDiv(state->builder, lhs, rhs, "");
    default:
        assert(false);
        return NULL;
    }
}

static LLVMValueRef compileExpressionBinaryFloat(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state)
{
    switch (operator) {
    case AST_OPERATOR_ADD:
        return LLVMBuildFAdd(state->builder, lhs, rhs, "");
    case AST_OPERATOR_SUB:
        return LLVMBuildFSub(state->builder, lhs, rhs, "");
    case AST_OPERATOR_MUL:
        return LLVMBuildFMul(state->builder, lhs, rhs, "");
    case AST_OPERATOR_DIV:
        return LLVMBuildFDiv(state->builder, lhs, rhs, "");
    default:
        assert(false);
        return NULL;
    }
}

static LLVMValueRef compileExpressionBinaryIntCmp(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state)
{
    switch (operator) {
    case AST_OPERATOR_EQUALS:
        return LLVMBuildICmp(state->builder, LLVMIntEQ, lhs, rhs, "");
    case AST_OPERATOR_NOT_EQUALS:
        return LLVMBuildICmp(state->builder, LLVMIntNE, lhs, rhs, "");
    case AST_OPERATOR_LESS:
        return LLVMBuildICmp(state->builder, LLVMIntSLT, lhs, rhs, "");
    case AST_OPERATOR_LESS_EQUALS:
        return LLVMBuildICmp(state->builder, LLVMIntSLE, lhs, rhs, "");
    case AST_OPERATOR_GREATER:
        return LLVMBuildICmp(state->builder, LLVMIntSGT, lhs, rhs, "");
    case AST_OPERATOR_GREATER_EQUALS:
        return LLVMBuildICmp(state->builder, LLVMIntSGE, lhs, rhs, "");
    case AST_OPERATOR_AND:
        return LLVMBuildAnd(state->builder, lhs, rhs, "");
    case AST_OPERATOR_OR:
        return LLVMBuildOr(state->builder, lhs, rhs, "");
    default:
        assert(false);
        return NULL;
    }
}

static LLVMValueRef compileExpressionBinaryFloatCmp(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state)
{
    switch (operator) {
    case AST_OPERATOR_EQUALS:
        return LLVMBuildFCmp(state->builder, LLVMRealOEQ, lhs, rhs, "");
    case AST_OPERATOR_NOT_EQUALS:
        return LLVMBuildFCmp(state->builder, LLVMRealONE, lhs, rhs, "");
    case AST_OPERATOR_LESS:
        return LLVMBuildFCmp(state->builder, LLVMRealOLT, lhs, rhs, "");
    case AST_OPERATOR_LESS_EQUALS:
        return LLVMBuildFCmp(state->builder, LLVMRealOLE, lhs, rhs, "");
    case AST_OPERATOR_GREATER:
        return LLVMBuildFCmp(state->builder, LLVMRealOGT, lhs, rhs, "");
    case AST_OPERATOR_GREATER_EQUALS:
        return LLVMBuildFCmp(state->builder, LLVMRealOGE, lhs, rhs, "");
    default:
        assert(false);
        return NULL;
    }
}

static LLVMValueRef compileExpressionCast(AstExpression* expression,
        IRState* state)
{
    LLVMValueRef operand =
            compileExpression(expression->u.cast_.expression, state);
    switch (expression->u.cast_.tag) {
    case AST_CAST_INT_TO_FLOAT:
        return LLVMBuildSIToFP(state->builder, operand, LLVMFloatType(), "");
    case AST_CAST_FLOAT_TO_INT:
        return LLVMBuildFPToSI(state->builder, operand, LLVMInt32Type(), "");
    }
}

static LLVMValueRef compileVariableArray(AstVariable* variable, IRState* state)
{
    LLVMValueRef pointer = compileExpression(variable->u.array_.location, state);
    LLVMValueRef offset = compileExpression(variable->u.array_.offset, state);
    LLVMValueRef indices[] = {offset};
    return LLVMBuildGEP(state->builder, pointer, indices, 1, "");
}

