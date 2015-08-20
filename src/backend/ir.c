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

    /* Boolean strings for printf */
    LLVMValueRef bool_strings;

    /* Maps literal strings to LLVMValueRef */
    TableRef strings;

    /* Current function */
    LLVMValueRef function;
} IRState;

/* Pair with basic block and value, used as return value */
typedef struct IRBlockValue {
    LLVMBasicBlockRef block;
    LLVMValueRef value;
} IRBlockValue;
 
/* Max number of parameters for in a function call */
const int MAX_N_PARAMETERS = 64;

/* Max number of elements printed with print statement */
const int MAX_FORMAT = 1024;

/* Verifies if the LLVM module is correct */
static void verifyModule(LLVMModuleRef module);

/* Creates the initial state */
static IRState* createState(LLVMModuleRef module);

/* Destroys the state */
static void destroyState(IRState* state);

/* Creates the equivalent llvm type */
static LLVMTypeRef createType(Type type);

/* Creates the equivalent llvm function type */
static LLVMTypeRef createFunctionType(AstDeclaration* function);

/* Creates the printf prototype */
static LLVMValueRef createPrintfPrototype(LLVMModuleRef module);

/* Creates an array with {"false", "true"} */
static LLVMValueRef createBooleanStrings(LLVMModuleRef module);

/* Creates the format function based on a list of expressions */
static LLVMValueRef createPrintfFormat(AstExpression* expressions,
        IRState* state);

/* Remove declarations from the table */
static void removeDeclarations(AstDeclaration* variables,
        TableRef declarations);

/* Create hidden global variables for each literal string */
static void compileStrings(IRState* state);

/* Create global variables */
static void compileGlobalVariables(AstDeclaration* tree,
        TableRef declarations, IRState* state);

/* Compiles functions declarations */
static void compileFunctionsDeclarations(AstDeclaration* tree,
        TableRef declarations, IRState* state);

/* Compiles functions parameters references */
static void compileParameters(AstDeclaration* parameters, TableRef declarations,
        IRState* state);

/* Compiles the variables by initializing them with empty values */
static void compileLocalVariables(AstDeclaration* variables,
        TableRef declarations);

/* Compiles the statements
 * Receiveis the input basic block and returns the output basic block */
static LLVMBasicBlockRef compileStatements(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementBlock(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementIf(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementWhile(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementAssign(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementDelete(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementPrint(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMBasicBlockRef compileStatementReturn(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

/* Merges two blocks into one block */
static void mergeBlocks(LLVMBasicBlockRef left_block,
        TableRef left_declarations, LLVMBasicBlockRef right_block,
        TableRef right_declarations, LLVMBasicBlockRef out_block,
        TableRef declarations, IRState* state);

/* Returns the local declarations */
static TablePair* getLocalDeclarations(TableRef declarations, int* n);

/* Assign all other declarations to declarations */
static void updateDeclarations(TableRef declarations, TableRef other);

/* Links the phis variables */
static void linkPhis(TablePair* locals, int n_locals, LLVMValueRef* phis,
        TableRef left_declarations, Vector* left_arrives,
        TableRef right_declarations, Vector* right_arrives);

/* Compiles expressions */
static IRBlockValue compileExpression(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMValueRef compileExpressionString(AstExpression* expression,
        LLVMBasicBlockRef block, IRState* state);

static IRBlockValue compileExpressionCall(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static IRBlockValue compileExpressionVariable(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static IRBlockValue compileExpressionNew(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static IRBlockValue compileExpressionUnary(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static IRBlockValue compileExpressionBinary(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static IRBlockValue compileLogicalBinaryExpression(AstBinaryOperator operator,
        AstExpression* left_expression, AstExpression* right_expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

static LLVMValueRef compileExpressionBinaryInt(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);

static LLVMValueRef compileExpressionBinaryFloat(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);

static LLVMValueRef compileExpressionBinaryIntCmp(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);

static LLVMValueRef compileExpressionBinaryFloatCmp(AstBinaryOperator operator,
        LLVMValueRef lhs, LLVMValueRef rhs, IRState* state);

static IRBlockValue compileExpressionCast(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

/* Compiles the expression and jumps to the corresponding block.
 * Arrive_at_* vectors are filled with the blocks that arrive at 
 * the corresponding block. Those vectors can be NULL */
static void compileJump(AstExpression* expression,
        LLVMBasicBlockRef in_block, LLVMBasicBlockRef true_block,
        LLVMBasicBlockRef false_block, TableRef declarations, IRState* state,
        Vector* arrive_at_true, Vector* arrive_at_false);

static void compileJumpExpression(AstExpression* expression,
        LLVMBasicBlockRef in_block, LLVMBasicBlockRef true_block,
        LLVMBasicBlockRef false_block, TableRef declarations, IRState* state,
        Vector* arrive_at_true, Vector* arrive_at_false);


static void compileJumpBinary(AstExpression* expression,
        LLVMBasicBlockRef in_block, LLVMBasicBlockRef true_block,
        LLVMBasicBlockRef false_block, TableRef declarations, IRState* state,
        Vector* arrive_at_true, Vector* arrive_at_false);


/* Returns the pointer to the array's element */
static IRBlockValue compileVariableArray(AstVariable* variable,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state);

/* 
 * SECTION: Implementation
 */

LLVMModuleRef IRCompileModule(AstDeclaration* tree)
{
    LLVMModuleRef module = LLVMModuleCreateWithName("monga-executable");

    TableRef declarations = TableCreateDummy();
    IRState* state = createState(module);

    compileStrings(state);
    compileGlobalVariables(tree, declarations, state);
    compileFunctionsDeclarations(tree, declarations, state);

    TableDestroy(declarations);
    destroyState(state);

    verifyModule(module);
    return module;
}

static void verifyModule(LLVMModuleRef module)
{
    char *error = NULL;
    LLVMVerifyModule(module, LLVMPrintMessageAction, &error);
    LLVMDisposeMessage(error);
}

static IRState* createState(LLVMModuleRef module)
{
    IRState* state = NEW(IRState);
    state->module = module;
    state->builder = LLVMCreateBuilder();
    state->printf = createPrintfPrototype(module);
    state->bool_strings = createBooleanStrings(module);
    state->strings = TableCreateDummy();
    state->function = NULL;
    return state;
}

static void destroyState(IRState* state)
{
    LLVMDisposeBuilder(state->builder);
    TableDestroy(state->strings);
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

static LLVMValueRef createBooleanStrings(LLVMModuleRef module)
{
    LLVMTypeRef str_type = LLVMPointerType(LLVMInt8Type(), 0);

    LLVMValueRef false_string = LLVMAddGlobal(module,
            LLVMArrayType(LLVMInt8Type(), 6), ".false");
    LLVMSetInitializer(false_string, LLVMConstString("false", 6, true));
    LLVMValueRef false_ptr = LLVMConstPointerCast(false_string, str_type);

    LLVMValueRef true_string = LLVMAddGlobal(module,
            LLVMArrayType(LLVMInt8Type(), 5), ".true");
    LLVMSetInitializer(true_string, LLVMConstString("true", 5, true));
    LLVMValueRef true_ptr = LLVMConstPointerCast(true_string, str_type);

    LLVMValueRef strings = LLVMAddGlobal(module,
            LLVMArrayType(str_type, 2), ".boolean");
    LLVMValueRef array_values[] = {false_ptr, true_ptr};
    LLVMValueRef array = LLVMConstArray(str_type, array_values, 2);
    LLVMSetInitializer(strings, array);

    return strings;
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
            strcat(format, "%s");
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

static void removeDeclarations(AstDeclaration* variables, TableRef declarations)
{
    AST_FOREACH(AstDeclaration, variable, variables) {
        TableErase(declarations, variable);
    }
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

static void compileGlobalVariables(AstDeclaration* tree,
        TableRef declarations, IRState* state)
{
    AST_FOREACH(AstDeclaration, variable, tree) {
        if (variable->tag != AST_DECLARATION_VARIABLE)
            continue;

        LLVMTypeRef type = createType(variable->type);
        LLVMValueRef llvm_variable = LLVMAddGlobal(state->module, type,
                variable->identifier);
        LLVMSetInitializer(llvm_variable, LLVMConstNull(type));
        TableInsert(declarations, variable, llvm_variable);
    }
}

static void compileFunctionsDeclarations(AstDeclaration* tree,
        TableRef declarations, IRState* state)
{
    AST_FOREACH(AstDeclaration, function, tree) {
        if (function->tag != AST_DECLARATION_FUNCTION)
            continue;

        LLVMTypeRef type = createFunctionType(function);
        state->function = LLVMAddFunction(state->module, function->identifier,
                type);
        TableInsert(declarations, function, state->function);

        AstDeclaration* parameters = function->u.function_.parameters;
        compileParameters(parameters, declarations, state);

        AstStatement* block = function->u.function_.block;
        LLVMBasicBlockRef entry_block =
                LLVMAppendBasicBlock(state->function, "entry");
        compileStatements(block, entry_block, declarations, state);

        removeDeclarations(parameters, declarations);
    }
}

static void compileParameters(AstDeclaration* parameters, TableRef declarations,
        IRState* state)
{
    int i = 0;
    AST_FOREACH(AstDeclaration, parameter, parameters) {
        LLVMValueRef llvm_parameter = LLVMGetParam(state->function, i++);
        LLVMSetValueName(llvm_parameter, parameter->identifier);
        TableInsert(declarations, parameter, llvm_parameter);
    }
}

static void compileLocalVariables(AstDeclaration* variables,
        TableRef declarations)
{
    AST_FOREACH(AstDeclaration, variable, variables) {
        LLVMTypeRef type = createType(variable->type);
        LLVMValueRef empty_const = LLVMConstNull(type);
        TableInsert(declarations, variable, empty_const);
    }
}

static LLVMBasicBlockRef compileStatements(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    if (statement == NULL)
        return in_block;

    LLVMBasicBlockRef out_block = NULL;
    
    switch (statement->tag) {
    case AST_STATEMENT_BLOCK:
        out_block = compileStatementBlock(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_IF:
        out_block = compileStatementIf(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_WHILE:
        out_block = compileStatementWhile(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_ASSIGN:
        out_block = compileStatementAssign(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_DELETE:
        out_block = compileStatementDelete(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_PRINT:
        out_block = compileStatementPrint(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_RETURN:
        out_block = compileStatementReturn(statement, in_block, declarations,
                state);
        break;
    case AST_STATEMENT_CALL:
        out_block = compileExpressionCall(statement->u.call_, in_block, 
                declarations, state).block;
        break;
    }

    return compileStatements(statement->next, out_block, declarations, state);
}

static LLVMBasicBlockRef compileStatementBlock(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstDeclaration* variables = statement->u.block_.variables;
    compileLocalVariables(variables, declarations);

    AstStatement* statements = statement->u.block_.statements;
    LLVMBasicBlockRef out_block =
        compileStatements(statements, in_block, declarations, state);

    removeDeclarations(variables, declarations);
    return out_block;
}

static LLVMBasicBlockRef compileStatementIf(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    // Creates the then and else blocks and build the jump
    LLVMBasicBlockRef then_in_block =
            LLVMAppendBasicBlock(state->function, "then");
    LLVMBasicBlockRef else_in_block =
            LLVMAppendBasicBlock(state->function, "else");
    AstExpression* expression = statement->u.if_.expression;
    compileJump(expression, in_block, then_in_block, else_in_block,
            declarations, state, NULL, NULL);

    // Compiles the then statement
    AstStatement* then_statement = statement->u.if_.then_statement;
    TableRef then_declarations = TableClone(declarations);
    LLVMBasicBlockRef then_out_block = compileStatements(then_statement,
            then_in_block, then_declarations, state);

    // Compiles the else statement
    AstStatement* else_statement = statement->u.if_.else_statement;
    if (else_statement == NULL) {
        else_statement = AstStatementBlock(NULL, NULL, -1);
        statement->u.if_.else_statement = else_statement;
    }
    TableRef else_declarations = TableClone(declarations);
    LLVMBasicBlockRef else_out_block = compileStatements(else_statement,
            else_in_block, else_declarations, state);

    // Defines the out block
    LLVMBasicBlockRef out_block = NULL;
    if (then_statement->returned || else_statement->returned) {
        if (!then_statement->returned) {
            updateDeclarations(declarations, then_declarations);
            out_block = then_out_block;
        } else if (!else_statement->returned) {
            updateDeclarations(declarations, else_declarations);
            out_block = else_out_block;
        }
    } else {
        out_block = LLVMAppendBasicBlock(state->function, "out");
        mergeBlocks(then_out_block, then_declarations, else_out_block,
                else_declarations, out_block, declarations, state);
    }

    TableDestroy(then_declarations);
    TableDestroy(else_declarations);
    return out_block;
}

static LLVMBasicBlockRef compileStatementWhile(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    // TODO: Refactoring to remove duplicate code

    // Compiles a while statement with:
    // if (condition) {
    //   do {
    //     statement;
    //   while (condition);
    // }

    // Creates the while's blocks
    LLVMBasicBlockRef loop_in_block =
            LLVMAppendBasicBlock(state->function, "loop_in");
    LLVMBasicBlockRef end_block =
            LLVMAppendBasicBlock(state->function, "loop_end");

    // Evaluates the expression before the loop
    AstExpression* expression = statement->u.while_.expression;
    Vector* arrive_at_loop_from_in = VectorCreate();
    Vector* arrive_at_end_from_in = VectorCreate();
    compileJump(expression, in_block, loop_in_block, end_block,
            declarations, state, arrive_at_loop_from_in,
            arrive_at_end_from_in);

    // Create the loop's phis variables
    int n_locals;
    TablePair* locals = getLocalDeclarations(declarations, &n_locals);
    LLVMValueRef loop_phis[n_locals];
    TableRef loop_declarations = TableClone(declarations);
    LLVMPositionBuilderAtEnd(state->builder, loop_in_block);
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        LLVMTypeRef phi_type = createType(declaration->type);
        loop_phis[i] = LLVMBuildPhi(state->builder, phi_type, "");
        TableErase(loop_declarations, declaration);
        TableInsert(loop_declarations, declaration, loop_phis[i]);
    }

    // Compiles the loop statement
    AstStatement* loop_statement = statement->u.while_.statement;
    LLVMBasicBlockRef loop_out_block = compileStatements(loop_statement,
            loop_in_block, loop_declarations, state);

    // Evaluetes the expression inside the loop
    Vector* arrive_at_loop_from_loop = VectorCreate();
    Vector* arrive_at_end_from_loop = VectorCreate();
    compileJump(expression, loop_out_block, loop_in_block, end_block,
            loop_declarations, state, arrive_at_loop_from_loop,
            arrive_at_end_from_loop);

    // Creates the end block phis
    LLVMValueRef end_phis[n_locals];
    LLVMPositionBuilderAtEnd(state->builder, end_block);
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        LLVMTypeRef phi_type = createType(declaration->type);
        end_phis[i] = LLVMBuildPhi(state->builder, phi_type, "");
    }

    // Link phis
    linkPhis(locals, n_locals, loop_phis, declarations, arrive_at_loop_from_in,
            loop_declarations, arrive_at_loop_from_loop);
    linkPhis(locals, n_locals, end_phis, declarations, arrive_at_end_from_in,
            loop_declarations, arrive_at_end_from_loop);

    // Replace phis in out declarations
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        TableErase(declarations, declaration);
        TableInsert(declarations, declaration, end_phis[i]);
    }

    // Deallocation
    VectorDestroy(arrive_at_loop_from_in);
    VectorDestroy(arrive_at_end_from_in);
    VectorDestroy(arrive_at_loop_from_loop);
    VectorDestroy(arrive_at_end_from_loop);
    TableDestroy(loop_declarations);
    free(locals);
    return end_block;
}
#if 0
{
    // Creates the while's blocks
    LLVMBasicBlockRef expression_in_block =
            LLVMAppendBasicBlock(state->function, "while_expression");
    LLVMBasicBlockRef statement_in_block =
            LLVMAppendBasicBlock(state->function, "while_statement");
    LLVMBasicBlockRef out_block =
            LLVMAppendBasicBlock(state->function, "while_out");

    // Add jump from input block to while's expression block
    LLVMPositionBuilderAtEnd(state->builder, in_block);
    LLVMBuildBr(state->builder, expression_in_block);

    // Create phis variables
    int n_locals;
    TablePair* locals = getLocalDeclarations(declarations, &n_locals);
    LLVMValueRef phis[n_locals];

    // Add phis at the beginning of the expression block
    LLVMPositionBuilderAtEnd(state->builder, expression_in_block);
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        LLVMTypeRef phi_type = createType(declaration->type);
        phis[i] = LLVMBuildPhi(state->builder, phi_type, "");
        TableErase(declarations, declaration);
        TableInsert(declarations, declaration, phis[i]);
    }

    // Compile expression and add the jump to statement block or out block
    AstExpression* expression = statement->u.while_.expression;
    compileJump(expression, expression_in_block, statement_in_block, out_block,
            declarations, state, NULL, NULL);

    // Compile statement
    TableRef while_declarations = TableClone(declarations);
    AstStatement* substatment = statement->u.while_.statement;
    LLVMBasicBlockRef statement_out_block = compileStatements(substatment,
            statement_in_block, while_declarations, state);
    LLVMPositionBuilderAtEnd(state->builder, statement_out_block);
    LLVMBuildBr(state->builder, expression_in_block);

    // Add phis' incomming values
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        LLVMValueRef in_value = locals[i].data;
        LLVMValueRef statement_value =
                TableFind(while_declarations, declaration).data;
        if (statement_value == phis[i]) {
            LLVMReplaceAllUsesWith(phis[i], in_value);
            LLVMInstructionEraseFromParent(phis[i]);
            TableErase(declarations, declaration);
            TableInsert(declarations, declaration, in_value);
        } else {
            LLVMValueRef incomming_values[] = {in_value, statement_value};
            LLVMBasicBlockRef incomming_blocks[] = {in_block,
                    statement_out_block};
            LLVMAddIncoming(phis[i], incomming_values, incomming_blocks, 2);
        }
    }

    TableDestroy(while_declarations);
    free(locals);

    return out_block;
}
#endif

static LLVMBasicBlockRef compileStatementAssign(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* expression = statement->u.assign_.expression;
    IRBlockValue expression_return =
            compileExpression(expression, in_block, declarations, state);
    in_block = expression_return.block;
    LLVMValueRef value = expression_return.value;

    AstVariable* variable = statement->u.assign_.variable;
    if (TypeIsChar(variable->type)) {
        LLVMPositionBuilderAtEnd(state->builder, in_block);
        value = LLVMBuildTrunc(state->builder, value, LLVMInt8Type(), "");
    }
    LLVMBasicBlockRef out_block;

    switch (variable->tag) {
    case AST_VARIABLE_ARRAY: {
        expression_return =
                compileVariableArray(variable, in_block, declarations, state);
        LLVMValueRef array = expression_return.value;
        out_block = expression_return.block;
        LLVMPositionBuilderAtEnd(state->builder, out_block);
        LLVMBuildStore(state->builder, value, array);
        break;
    }
    case AST_VARIABLE_REFERENCE: {
        AstDeclaration* declaration = variable->u.reference_.u.declaration_;
        out_block = in_block;
        if (declaration->u.variable_.global) {
            LLVMValueRef llvm_variable =
                    TableFind(declarations, declaration).data;
            LLVMPositionBuilderAtEnd(state->builder, out_block);
            LLVMBuildStore(state->builder, value, llvm_variable);
        } else {
            TableErase(declarations, declaration);
            TableInsert(declarations, declaration, value);
        }
        break;
    }
    }

    return out_block;
}

static LLVMBasicBlockRef compileStatementDelete(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* expression = statement->u.delete_.expression;
    IRBlockValue expression_return =
            compileExpression(expression, in_block, declarations, state);
    LLVMBasicBlockRef out_block = expression_return.block;
    LLVMValueRef value = expression_return.value;
    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMBuildFree(state->builder, value);
    return out_block;
}

static LLVMBasicBlockRef compileStatementPrint(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* expressions = statement->u.print_.expressions;

    LLVMValueRef parameters[MAX_N_PARAMETERS];
    int n = 0;
    LLVMPositionBuilderAtEnd(state->builder, in_block);
    parameters[n++] = createPrintfFormat(expressions, state);

    LLVMBasicBlockRef curr_in_block = in_block;
    AST_FOREACH(AstExpression, expression, expressions) {
        Type type = expression->type;
        if (TypeIsArray(type) || TypeIsBool(type) || TypeIsInt(type) ||
            TypeIsFloat(type)) {
            IRBlockValue expression_return = compileExpression(expression,
                    curr_in_block, declarations, state);
            curr_in_block = expression_return.block;
            LLVMValueRef value = expression_return.value;

            LLVMPositionBuilderAtEnd(state->builder, curr_in_block);
            if (TypeIsFloat(type)) {
                value = LLVMBuildFPCast(state->builder, value,
                        LLVMDoubleType(), "");
            } else if (TypeIsBool(type)) {
                LLVMValueRef zero = LLVMConstInt(LLVMInt32Type(), 0, false);
                LLVMValueRef index = LLVMBuildZExt(state->builder,
                        value, LLVMInt32Type(), "");
                LLVMValueRef indices[] = {zero, index};
                LLVMValueRef location = LLVMBuildGEP(state->builder,
                        state->bool_strings, indices, 2, "");
                value = LLVMBuildLoad(state->builder, location, "");
            }

            parameters[n++] = value;
        }
    }

    LLVMBasicBlockRef out_block = curr_in_block;
    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMBuildCall(state->builder, state->printf, parameters, n, "");

    return out_block;
}

static LLVMBasicBlockRef compileStatementReturn(AstStatement* statement, 
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* expression = statement->u.return_.expression;

    LLVMBasicBlockRef out_block = in_block;
    LLVMValueRef value = NULL;
    if (expression != NULL) {
        IRBlockValue expression_return = compileExpression(expression,
                in_block, declarations, state);
        out_block = expression_return.block;
        value = expression_return.value;
    } 

    LLVMPositionBuilderAtEnd(state->builder, out_block);
    if (expression == NULL || TypeIsVoid(expression->type))
        LLVMBuildRetVoid(state->builder);
    else
        LLVMBuildRet(state->builder, value);

    return out_block;
}

static void mergeBlocks(LLVMBasicBlockRef left_block,
        TableRef left_declarations, LLVMBasicBlockRef right_block,
        TableRef right_declarations, LLVMBasicBlockRef out_block,
        TableRef declarations, IRState* state)
{
    // Add jumps from left and right to output block
    LLVMPositionBuilderAtEnd(state->builder, left_block);
    LLVMBuildBr(state->builder, out_block);
    LLVMPositionBuilderAtEnd(state->builder, right_block);
    LLVMBuildBr(state->builder, out_block);
    LLVMPositionBuilderAtEnd(state->builder, out_block);

    // Create phi variables
    int n_locals;
    TablePair* locals = getLocalDeclarations(declarations, &n_locals);
    LLVMValueRef phis[n_locals];
    LLVMValueRef left_values[n_locals];
    LLVMValueRef right_values[n_locals];

    // Add phis to the beginning of the output block
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        LLVMValueRef backup = locals[i].data;
        left_values[i] = TableFind(left_declarations, declaration).data;
        right_values[i] = TableFind(right_declarations, declaration).data;

        if (left_values[i] != backup || right_values[i] != backup) {
            LLVMTypeRef phi_type = createType(declaration->type);
            phis[i] = LLVMBuildPhi(state->builder, phi_type,
                    declaration->identifier);
            TableErase(declarations, declaration);
            TableInsert(declarations, declaration, phis[i]);
        } else {
            phis[i] = NULL;
        }
    }

    // Add phis' incomming values
    for (int i = 0; i < n_locals; ++i) {
        if (phis[i] == NULL)
            continue;

        LLVMValueRef incomming_values[] = {left_values[i], right_values[i]};
        LLVMBasicBlockRef incomming_blocks[] = {left_block, right_block};
        LLVMAddIncoming(phis[i], incomming_values, incomming_blocks, 2);
    }

    free(locals);
}

static TablePair* getLocalDeclarations(TableRef declarations, int* n)
{
    int n_declarations = TableSize(declarations);
    TablePair* declaration_pairs = TableToArray(declarations);

    TablePair* locals = NEW_ARRAY(TablePair, n_declarations);
    int n_locals = 0;

    for (int i = 0; i < n_declarations; ++i) {
        AstDeclaration* declaration = declaration_pairs[i].key;
        if (declaration->tag == AST_DECLARATION_VARIABLE &&
            !declaration->u.variable_.global)
            locals[n_locals++] = declaration_pairs[i];
    }

    free(declaration_pairs);
    *n = n_locals;
    return locals;
}

static void updateDeclarations(TableRef declarations, TableRef other)
{
    TablePair* other_pairs = TableToArray(other);
    for (int i = 0; i < TableSize(other); ++i) {
        AstDeclaration* declaration = other_pairs[i].key;
        LLVMValueRef value = other_pairs[i].data;
        TableErase(declarations, declaration);
        TableInsert(declarations, declaration, value);
    }
    free(other_pairs);
}

static void linkPhis(TablePair* locals, int n_locals, LLVMValueRef* phis,
        TableRef left_declarations, Vector* left_arrives,
        TableRef right_declarations, Vector* right_arrives)
{
    for (int i = 0; i < n_locals; ++i) {
        AstDeclaration* declaration = locals[i].key;
        LLVMValueRef left_value =
                TableFind(left_declarations, declaration).data;
        LLVMValueRef right_value =
                TableFind(right_declarations, declaration).data;
        size_t left_n_blocks = VectorSize(left_arrives);
        size_t right_n_blocks = VectorSize(right_arrives);
        size_t total_blocks = left_n_blocks + right_n_blocks;
        LLVMValueRef incomming_values[total_blocks];
        LLVMBasicBlockRef incomming_blocks[total_blocks];
        size_t curr_block = 0;

        for (size_t block = 0; block < left_n_blocks; ++block) {
            incomming_values[curr_block] = left_value;
            incomming_blocks[curr_block] = VectorGet(left_arrives, block);
            ++curr_block;
        }
        for (size_t block = 0; block < right_n_blocks; ++block) {
            incomming_values[curr_block] = right_value;
            incomming_blocks[curr_block] = VectorGet(right_arrives, block);
            ++curr_block;
        }

        LLVMAddIncoming(phis[i], incomming_values, incomming_blocks,
                total_blocks);
    }
}

static IRBlockValue compileExpression(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    IRBlockValue expression_return = {.block = in_block, .value = NULL};

    switch (expression->tag) {
    case AST_EXPRESSION_KBOOL:
        expression_return.value =
                LLVMConstInt(LLVMIntType(1), expression->u.kbool_, false);
        break;
    case AST_EXPRESSION_KINT:
        expression_return.value =
                LLVMConstInt(LLVMInt32Type(), expression->u.kint_, false);
        break;
    case AST_EXPRESSION_KFLOAT:
        expression_return.value =
                LLVMConstReal(LLVMFloatType(), expression->u.kfloat_);
        break;
    case AST_EXPRESSION_STRING:
        expression_return.value =
                compileExpressionString(expression, in_block, state);
        break;
    case AST_EXPRESSION_NULL:
        expression_return.value = LLVMConstNull(createType(expression->type));
        break;
    case AST_EXPRESSION_CALL:
        expression_return = compileExpressionCall(expression, in_block,
                declarations, state);
        break;
    case AST_EXPRESSION_VARIABLE:
        expression_return = compileExpressionVariable(expression, in_block,
                declarations, state);
        break;
    case AST_EXPRESSION_NEW:
        expression_return = compileExpressionNew(expression, in_block,
                declarations, state);
        break;
    case AST_EXPRESSION_UNARY:
        expression_return = compileExpressionUnary(expression, in_block,
                declarations, state);
        break;
    case AST_EXPRESSION_BINARY:
        expression_return = compileExpressionBinary(expression, in_block,
                declarations, state);
        break;
    case AST_EXPRESSION_CAST:
        expression_return = compileExpressionCast(expression, in_block,
                declarations, state);
        break;
    }

    return expression_return;
}

static LLVMValueRef compileExpressionString(AstExpression* expression,
        LLVMBasicBlockRef block, IRState* state)
{
    LLVMValueRef string = TableFind(state->strings, expression->u.string_).data;
    LLVMPositionBuilderAtEnd(state->builder, block);
    return LLVMBuildPointerCast(state->builder, string,
            LLVMPointerType(LLVMInt8Type(), 0), "");
}

static IRBlockValue compileExpressionCall(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstDeclaration* declaration = expression->u.call_.u.declaration_;
    LLVMValueRef function = TableFind(declarations, declaration).data;

    AstExpression* parameters = expression->u.call_.expressions;
    LLVMValueRef llvm_parameters[MAX_N_PARAMETERS];
    int n = 0;
    LLVMBasicBlockRef curr_in_block = in_block;

    AST_FOREACH(AstExpression, parameter, parameters) {
        IRBlockValue expression_return =
                compileExpression(parameter, in_block, declarations, state);
        curr_in_block = expression_return.block;
        llvm_parameters[n++] = expression_return.value;
    }

    LLVMBasicBlockRef out_block = curr_in_block;
    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMValueRef value = 
            LLVMBuildCall(state->builder, function, llvm_parameters, n, "");
    return (IRBlockValue) {.block = out_block, .value = value};
}

static IRBlockValue compileExpressionVariable(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstVariable* variable = expression->u.variable_;
    LLVMBasicBlockRef out_block = in_block;
    LLVMValueRef value = NULL;

    switch (variable->tag) {
    case AST_VARIABLE_ARRAY: {
        IRBlockValue variable_return = compileVariableArray(variable, in_block,
                declarations, state);
        out_block = variable_return.block;
        LLVMPositionBuilderAtEnd(state->builder, out_block);
        value = LLVMBuildLoad(state->builder, variable_return.value, "");
        break;
    }
    case AST_VARIABLE_REFERENCE: {
        AstDeclaration* declaration = variable->u.reference_.u.declaration_;
        LLVMValueRef llvm_variable = TableFind(declarations, declaration).data;
        if (declaration->u.variable_.global) {
            LLVMPositionBuilderAtEnd(state->builder, out_block);
            value = LLVMBuildLoad(state->builder, llvm_variable, "");
        }
        else {
            value = llvm_variable;
        }
        break;
    }
    }

    if (TypeIsChar(variable->type)) {
        LLVMPositionBuilderAtEnd(state->builder, out_block);
        value = LLVMBuildZExt(state->builder, value, LLVMInt32Type(), "");
    }

    return (IRBlockValue) {.block = out_block, .value = value};
}

static IRBlockValue compileExpressionNew(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    LLVMTypeRef type = createType(expression->u.new_.type);
    AstExpression* subexpression = expression->u.new_.expression;
    IRBlockValue expression_return = compileExpression(subexpression, in_block,
            declarations, state);
    LLVMBasicBlockRef out_block = expression_return.block;
    LLVMValueRef size = expression_return.value;
    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMValueRef value = LLVMBuildArrayMalloc(state->builder, type, size, "");
    return (IRBlockValue) {.block = out_block, .value = value};
}

static IRBlockValue compileExpressionUnary(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* subexpression = expression->u.unary_.expression;
    IRBlockValue expression_return = compileExpression(subexpression, in_block,
            declarations, state);
    LLVMBasicBlockRef out_block = expression_return.block;
    LLVMValueRef operand = expression_return.value;

    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMValueRef value = NULL;
    switch (expression->u.unary_.operator) {
    case AST_OPERATOR_NEGATE:
        if (TypeIsInt(expression->type))
            value = LLVMBuildNeg(state->builder, operand, "");
        else
            value = LLVMBuildFNeg(state->builder, operand, "");
        break;
    case AST_OPERATOR_NOT:
        value = LLVMBuildNot(state->builder, operand, "");
        break;
    }

    return (IRBlockValue) {.block = out_block, .value = value};
}

static IRBlockValue compileExpressionBinary(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstBinaryOperator operator = expression->u.binary_.operator;
    AstExpression* left_expression = expression->u.binary_.expression_left;
    AstExpression* right_expression = expression->u.binary_.expression_right;

    if (operator == AST_OPERATOR_OR || operator == AST_OPERATOR_AND) {
        return compileLogicalBinaryExpression(operator, left_expression,
                right_expression, in_block, declarations, state);
    }

    IRBlockValue left_expression_return = compileExpression(left_expression,
             in_block, declarations, state);
    in_block = left_expression_return.block;
    LLVMValueRef lhs = left_expression_return.value;

    IRBlockValue right_expression_return = compileExpression(right_expression,
            in_block, declarations, state);
    in_block = right_expression_return.block;
    LLVMValueRef rhs = right_expression_return.value;

    Type type = expression->type;
    Type subexpression_type = left_expression->type;
    LLVMPositionBuilderAtEnd(state->builder, in_block);
    LLVMValueRef value = NULL;

    if (TypeIsInt(type)) {
        value = compileExpressionBinaryInt(operator, lhs, rhs, state);
    } else if (TypeIsFloat(type)) {
        value = compileExpressionBinaryFloat(operator, lhs, rhs, state);
    } else if (TypeIsInt(subexpression_type)
            || TypeIsBool(subexpression_type)
            || TypeIsArray(subexpression_type)) {
        value = compileExpressionBinaryIntCmp(operator, lhs, rhs, state);
    } else {
        value = compileExpressionBinaryFloatCmp(operator, lhs, rhs, state);
    }

    return (IRBlockValue) {.block = in_block, .value = value};
}

static IRBlockValue compileLogicalBinaryExpression(AstBinaryOperator operator,
        AstExpression* left_expression, AstExpression* right_expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    // Compute left expression
    IRBlockValue left_expression_return = compileExpression(left_expression,
             in_block, declarations, state);
    in_block = left_expression_return.block;
    LLVMValueRef lhs = left_expression_return.value;

    // Create blocks necessary for short circuit
    LLVMBasicBlockRef rhs_in_block =
            LLVMAppendBasicBlock(state->function, "logical_compute_rhs");
    LLVMBasicBlockRef final_block =
            LLVMAppendBasicBlock(state->function, "logical_end");

    // Jump to final block in caso of short circuit
    LLVMPositionBuilderAtEnd(state->builder, in_block);
    if (operator == AST_OPERATOR_OR)
        LLVMBuildCondBr(state->builder, lhs, final_block, rhs_in_block);
    else
        LLVMBuildCondBr(state->builder, lhs, rhs_in_block, final_block);

    // Computes right expression in specific block
    IRBlockValue right_expression_return = compileExpression(right_expression,
            rhs_in_block, declarations, state);
    LLVMBasicBlockRef rhs_out_block = right_expression_return.block;
    LLVMValueRef rhs = right_expression_return.value;
    LLVMPositionBuilderAtEnd(state->builder, rhs_out_block);
    LLVMBuildBr(state->builder, final_block);

    // Add phi into final block
    LLVMPositionBuilderAtEnd(state->builder, final_block);
    LLVMValueRef phi = LLVMBuildPhi(state->builder, LLVMIntType(1), "");
    LLVMValueRef incomming_values[] = {lhs, rhs};
    LLVMBasicBlockRef incomming_blocks[] = {in_block, rhs_out_block};
    LLVMAddIncoming(phi, incomming_values, incomming_blocks, 2);

    return (IRBlockValue) {.block = final_block, .value = phi};
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

static IRBlockValue compileExpressionCast(AstExpression* expression,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* subexpression = expression->u.cast_.expression;
    IRBlockValue expression_return = compileExpression(subexpression, in_block,
            declarations, state);
    LLVMBasicBlockRef out_block = expression_return.block;
    LLVMValueRef operand = expression_return.value;

    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMValueRef value = NULL;
    switch (expression->u.cast_.tag) {
    case AST_CAST_INT_TO_FLOAT:
        value = LLVMBuildSIToFP(state->builder, operand, LLVMFloatType(), "");
        break;
    case AST_CAST_FLOAT_TO_INT:
        value = LLVMBuildFPToSI(state->builder, operand, LLVMInt32Type(), "");
        break;
    }

    return (IRBlockValue) {.block = out_block, .value = value};
}

static void compileJump(AstExpression* expression,
        LLVMBasicBlockRef in_block, LLVMBasicBlockRef true_block,
        LLVMBasicBlockRef false_block, TableRef declarations, IRState* state,
        Vector* arrive_at_true, Vector* arrive_at_false)
{
    switch (expression->tag) {
    case AST_EXPRESSION_KBOOL:
    case AST_EXPRESSION_CALL:
    case AST_EXPRESSION_VARIABLE:
        compileJumpExpression(expression, in_block, true_block, false_block,
                declarations, state, arrive_at_true, arrive_at_false);
        break;
    case AST_EXPRESSION_UNARY:
        assert(expression->u.unary_.operator == AST_OPERATOR_NOT);
        compileJump(expression, in_block, false_block, true_block, declarations,
                state, arrive_at_false, arrive_at_true);
        break;
    case AST_EXPRESSION_BINARY:
        compileJumpBinary(expression, in_block, true_block, false_block,
                declarations, state, arrive_at_true, arrive_at_false);
        break;
    case AST_EXPRESSION_NEW:
    case AST_EXPRESSION_CAST:
    case AST_EXPRESSION_KINT:
    case AST_EXPRESSION_KFLOAT:
    case AST_EXPRESSION_STRING:
    case AST_EXPRESSION_NULL:
        assert(false);
        break;
    }
}

static void compileJumpExpression(AstExpression* expression,
        LLVMBasicBlockRef in_block, LLVMBasicBlockRef true_block,
        LLVMBasicBlockRef false_block, TableRef declarations, IRState* state,
        Vector* arrive_at_true, Vector* arrive_at_false)
{
    IRBlockValue expression_return = compileExpression(expression, in_block,
            declarations, state);
    LLVMPositionBuilderAtEnd(state->builder, expression_return.block);
    LLVMBuildCondBr(state->builder, expression_return.value, true_block,
            false_block);

    if (arrive_at_true != NULL)
        VectorPush(arrive_at_true, expression_return.block);
    if (arrive_at_false != NULL)
        VectorPush(arrive_at_false, expression_return.block);
}

static void compileJumpBinary(AstExpression* expression,
        LLVMBasicBlockRef in_block, LLVMBasicBlockRef true_block,
        LLVMBasicBlockRef false_block, TableRef declarations, IRState* state,
        Vector* arrive_at_true, Vector* arrive_at_false)
{
    AstBinaryOperator operator = expression->u.binary_.operator;
    if (operator != AST_OPERATOR_OR && operator != AST_OPERATOR_AND) {
        compileJumpExpression(expression, in_block, true_block, false_block,
                declarations, state, arrive_at_true, arrive_at_false);
        return;
    }

    // Computes left hand side operand
    LLVMBasicBlockRef rhs_in_block =
            LLVMAppendBasicBlock(state->function, "compute_rhs");
    AstExpression* left_expression = expression->u.binary_.expression_left;
    if (operator == AST_OPERATOR_OR) {
        compileJump(left_expression, in_block, true_block, rhs_in_block,
                declarations, state, arrive_at_true, NULL);
    } else {
        compileJump(left_expression, in_block, rhs_in_block, false_block,
                declarations, state, NULL, arrive_at_false);
    }

    // Computes right hand side operand
    AstExpression* right_expression = expression->u.binary_.expression_right;
    compileJump(right_expression, rhs_in_block, true_block, false_block,
            declarations, state, arrive_at_true, arrive_at_false);
}

static IRBlockValue compileVariableArray(AstVariable* variable,
        LLVMBasicBlockRef in_block, TableRef declarations, IRState* state)
{
    AstExpression* location = variable->u.array_.location;
    IRBlockValue location_return = compileExpression(location, in_block,
            declarations, state);
    LLVMBasicBlockRef out_block = location_return.block;
    LLVMValueRef llvm_location = location_return.value;

    AstExpression* offset = variable->u.array_.offset;
    IRBlockValue offset_return = compileExpression(offset, out_block,
            declarations, state);
    out_block = offset_return.block;
    LLVMValueRef llvm_offset = offset_return.value;

    LLVMValueRef indices[] = {llvm_offset};
    LLVMPositionBuilderAtEnd(state->builder, out_block);
    LLVMValueRef value =
            LLVMBuildGEP(state->builder, llvm_location, indices, 1, "");

    return (IRBlockValue) {.block = out_block, .value = value};
}

