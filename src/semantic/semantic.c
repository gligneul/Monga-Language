/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * semantic.c
 */

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#include "semantic.h"

#include "ast/ast_print.h"
#include "semantic/symbols.h"
#include "util/error.h"

/* Adds declarations to the symbol table */
static void addDeclarationsToSymbolsTable(AstDeclaration* declarations);

/* Analyse a function declaration */
static void analyseFunction(AstDeclaration* declaration);

/* Analyse statements, check if all paths returned */
static bool analyseStatement(AstStatement* statement);
static bool analyseStatementBlock(AstStatement* statement);
static bool analyseStatementIf(AstStatement* statement);
static bool analyseStatementWhile(AstStatement* statement);
static bool analyseStatementAssign(AstStatement* statement);
static bool analyseStatementDelete(AstStatement* statement);
static bool analyseStatementReturn(AstStatement* statement);

/* Analyse expressions */
static void analyseExpression(AstExpression* expression);
static void analyseExpressionNew(AstExpression* expression);
static void analyseExpressionCall(AstExpression* expression);
static void analyseExpressionUnary(AstExpression* expression);
static void analyseExpressionBinary(AstExpression* expression);

/* Analyse specific binary expression, returns true if there is an error */
static bool analyseExpressionArith(AstExpression* expression,
        AstExpression* left, AstExpression* right);
static bool analyseExpressionNumberCompare(AstExpression* expression,
        AstExpression* left, AstExpression* right);
static bool analyseExpressionCompare(AstExpression* expression,
        AstExpression* left, AstExpression* right);
static bool analyseExpressionLogical(AstExpression* expression,
        AstExpression* left, AstExpression* right);

/* Analyse a variable */
static void analyseVariable(AstVariable* variable);

/* Set the expression type if the expression is null and the type an array */
static void setNullExpressionType(AstExpression* expression, Type type);

/* Add cast for assignments, if necessary */
static void insertAssignmentCast(AstExpression* expression, Type variable);

/* Add cast for float/int binary expressions, if necessary */
static void insertNumericalCast(AstExpression* left, AstExpression* right);

/* Return type of current function */
static Type return_type = {TYPE_UNDEFINED, 0};

AstDeclaration* SemanticAnalyseTree(AstDeclaration* ast)
{
    AST_FOREACH(AstDeclaration, declaration, ast) {
        SymbolsAdd(declaration->identifier, declaration, declaration->line);
        switch (declaration->tag) {
        case AST_DECLARATION_FUNCTION:
            analyseFunction(declaration);
            break;
        case AST_DECLARATION_VARIABLE:
            declaration->u.variable_.global = true;
            break;
        }
    }
	return ast;
}

static void addDeclarationsToSymbolsTable(AstDeclaration* declarations)
{
    AST_FOREACH(AstDeclaration, declaration, declarations) {
        SymbolsAdd(declaration->identifier, declaration, declaration->line);
    }
}

static void analyseFunction(AstDeclaration* function)
{
    SymbolsOpenBlock();
    addDeclarationsToSymbolsTable(function->u.function_.parameters);
    return_type = function->type;
    AstStatement* block = function->u.function_.block;
    AstDeclaration* variables = block->u.block_.variables;
    AstStatement* statements = block->u.block_.statements;
    addDeclarationsToSymbolsTable(variables);
    bool returned = analyseStatement(statements);
    SymbolsCloseBlock();

    if (!returned) {
        if (TypeIsVoid(return_type)) {
            AST_CONCAT(statements, AstStatementReturn(NULL, -1));
        } else {
            ErrorL(function->line, "there are branches of the function that "
                    "don't return");
        }
    }
}

static bool analyseStatement(AstStatement* statement)
{
    if (statement == NULL)
        return false;

    switch (statement->tag) {
    case AST_STATEMENT_BLOCK:
        analyseStatementBlock(statement);
        break;
    case AST_STATEMENT_IF:
        analyseStatementIf(statement);
        break;
    case AST_STATEMENT_WHILE:
        analyseStatementWhile(statement);
        break;
    case AST_STATEMENT_ASSIGN:
        analyseStatementAssign(statement);
        break;
    case AST_STATEMENT_DELETE:
        analyseStatementDelete(statement);
        break;
    case AST_STATEMENT_PRINT:
        analyseExpression(statement->u.print_.expressions);
        break;
    case AST_STATEMENT_RETURN:
        analyseStatementReturn(statement);
        break;
    case AST_STATEMENT_CALL:
        analyseExpression(statement->u.call_);
        break;
    }

    if (statement->returned) {
        if (statement->next != NULL)
            ErrorL(statement->next->line, "unexpected statement after return");
        return true;
    }

    return analyseStatement(statement->next);
}

static bool analyseStatementBlock(AstStatement* statement)
{
    SymbolsOpenBlock();
    addDeclarationsToSymbolsTable(statement->u.block_.variables);
    AstStatement* substatement = statement->u.block_.statements;
    bool returned = analyseStatement(substatement);
    SymbolsCloseBlock();

    statement->returned = returned;
    return returned;
}

static bool analyseStatementIf(AstStatement* statement)
{
    AstExpression* expression = statement->u.if_.expression;
    analyseExpression(expression);

    AstStatement* then_statement = statement->u.if_.then_statement;
    bool then_returned = analyseStatement(then_statement);

    AstStatement* else_statement = statement->u.if_.else_statement;
    bool else_returned = analyseStatement(else_statement);

    if (!TypeIsBool(expression->type)) {
        ErrorL(statement->line, "mismatch type in if's expression, expected 'bool', "
                "read '%s'", TypeToString(expression->type));
    }

    bool returned = then_returned && else_returned;
    statement->returned = returned;
    return returned;
}

static bool analyseStatementWhile(AstStatement* statement)
{
    AstExpression* expression = statement->u.while_.expression;
    analyseExpression(expression);

    AstStatement* substatement = statement->u.while_.statement;
    analyseStatement(substatement);

    if (!TypeIsBool(expression->type)) {
        ErrorL(statement->line, "mismatch type in while's expression, expected "
                "'bool', read '%s'", TypeToString(expression->type));
    }

    return false;
}

static bool analyseStatementAssign(AstStatement* statement)
{
    AstVariable* variable = statement->u.assign_.variable;
    analyseVariable(variable);

    AstExpression* expression = statement->u.assign_.expression;
    setNullExpressionType(expression, variable->type);
    analyseExpression(expression);

    if (TypeIsAssignable(variable->type, expression->type)) {
        insertAssignmentCast(expression, variable->type);
    } else {
        ErrorL(statement->line, "mismatch type in '%s = %s' assignment",
                TypeToString(variable->type), TypeToString(expression->type));
    }

    return false;
}

static bool analyseStatementDelete(AstStatement* statement)
{
    AstExpression* expression = statement->u.delete_.expression;
    analyseExpression(expression);
    if (!TypeIsArray(expression->type)) {
        ErrorL(statement->line, "mismatch type in delete's expression, "
                "expected an array, read '%s'", TypeToString(expression->type));
    }
    return false;
}

static bool analyseStatementReturn(AstStatement* statement)
{
    AstExpression* expression = statement->u.return_.expression;
    analyseExpression(expression);

    const char* wrong_type = NULL;
    if (expression == NULL) {
        if (!TypeIsVoid(return_type))
            wrong_type = "void";
    } else {
        setNullExpressionType(expression, return_type);
        if (TypeIsAssignable(return_type, expression->type))
            insertAssignmentCast(expression, return_type);
        else
            wrong_type = TypeToString(expression->type);
    }

    if (wrong_type != NULL) {
        ErrorL(statement->line, "mismatch type in return's expression, expected "
                "'%s', read '%s'", TypeToString(return_type), wrong_type);
    }

    statement->returned = true;
    return true;
}

static void analyseExpression(AstExpression* expression)
{
    if (!expression) return;

    switch (expression->tag) {
    case AST_EXPRESSION_KINT:
        expression->type = TypeCreate(TYPE_INT, 0);
        break;
    case AST_EXPRESSION_KFLOAT:
        expression->type = TypeCreate(TYPE_FLOAT, 0);
        break;
    case AST_EXPRESSION_STRING:
        expression->type = TypeCreate(TYPE_CHAR, 1);
        break;
    case AST_EXPRESSION_NULL:
        // The null expression depends on the other operand type
        break;
    case AST_EXPRESSION_KBOOL:
        expression->type = TypeCreate(TYPE_BOOL, 0);
        break;
    case AST_EXPRESSION_CALL:
        analyseExpressionCall(expression);
        break;
    case AST_EXPRESSION_VARIABLE:
        analyseVariable(expression->u.variable_);
        expression->type = expression->u.variable_->type;
        if (TypeIsChar(expression->type))
            expression->type = TypeCreate(TYPE_INT, 0);
        break;
    case AST_EXPRESSION_NEW:
        analyseExpressionNew(expression);
        break;
    case AST_EXPRESSION_UNARY:
        analyseExpressionUnary(expression);
        break;
    case AST_EXPRESSION_BINARY:
        analyseExpressionBinary(expression);
        break;
    case AST_EXPRESSION_CAST:
        // Unexpected case since cast will be added at this phase
        assert(false);
        break;
    }

    analyseExpression(expression->next);
}

static void analyseExpressionNew(AstExpression* expression)
{
    Type array_type = expression->u.new_.type;
    AstExpression* array_size = expression->u.new_.expression;
    analyseExpression(array_size);
    if (!TypeIsInt(array_size->type)) {
        ErrorL(expression->line, "mismatch type in new expression, expected 'int', "
                "read '%s'", TypeToString(array_size->type));
    }
    expression->type = TypeCreate(array_type.tag, array_type.pointers + 1);
}

static void analyseExpressionCall(AstExpression* expression)
{
    char* identifier = expression->u.call_.u.identifier_;
    AstDeclaration* declaration = SymbolsFind(identifier, expression->line);
    if (declaration->tag != AST_DECLARATION_FUNCTION) {
        ErrorL(expression->line, "cannot call non-function symbol '%s'", 
                identifier);
    }

    AstDeclaration* declaration_parameter = declaration->u.function_.parameters;
    AstExpression* call_parameter = expression->u.call_.expressions;
    analyseExpression(call_parameter);
    while (declaration_parameter != NULL && call_parameter != NULL) {
        Type declaration_type = declaration_parameter->type;
        setNullExpressionType(call_parameter, declaration_type);
        Type call_type = call_parameter->type;
        if (!TypeIsAssignable(declaration_type, call_type)) {
            ErrorL(expression->line, "mismatch type in parameter of '%s' "
                    "function call, cannot assign '%s' to '%s'", identifier,
                    TypeToString(call_type), TypeToString(declaration_type));
        }
        insertAssignmentCast(call_parameter, declaration_type);
        declaration_parameter = declaration_parameter->next;
        call_parameter = call_parameter->next;
    }
    if (declaration_parameter != NULL || call_parameter != NULL) {
        ErrorL(expression->line, "mismatch number of parameters in '%s' "
                "function call", identifier);
    }

    expression->type = declaration->type;
    if (TypeIsChar(expression->type))
        expression->type = TypeCreate(TYPE_INT, 0);
    expression->u.call_.is_declaration = true;
    expression->u.call_.u.declaration_ = declaration;
}

static void analyseExpressionUnary(AstExpression* expression)
{
    AstExpression* subexpression = expression->u.unary_.expression;
    analyseExpression(subexpression);
    expression->type = subexpression->type;

    AstUnaryOperator operator = expression->u.unary_.operator;
    const char* expected = NULL;
    switch (operator) {
    case AST_OPERATOR_NEGATE:
        if (!TypeIsInt(subexpression->type) && !TypeIsFloat(subexpression->type))
            expected = "int' or 'float";
        break;
    case AST_OPERATOR_NOT:
        if (!TypeIsBool(subexpression->type))
            expected = "bool";
        break;
    }

    if (expected != NULL) {
        ErrorL(expression->line, "mismatch type in '%s' unary operation, "
                "expected '%s', read '%s'", AstPrintUnaryOperator(operator),
                expected, TypeToString(subexpression->type));
    }
}

static void analyseExpressionBinary(AstExpression* expression)
{
    AstExpression* left = expression->u.binary_.expression_left;
    analyseExpression(left);
    AstExpression* right = expression->u.binary_.expression_right;
    analyseExpression(right);

    AstBinaryOperator operator = expression->u.binary_.operator;
    bool type_error = false;
    switch (operator) {
    case AST_OPERATOR_ADD:
    case AST_OPERATOR_SUB:
    case AST_OPERATOR_MUL:
    case AST_OPERATOR_DIV:
        type_error = analyseExpressionArith(expression, left, right);
        break;
    case AST_OPERATOR_LESS:
    case AST_OPERATOR_LESS_EQUALS:
    case AST_OPERATOR_GREATER:
    case AST_OPERATOR_GREATER_EQUALS:
        type_error = analyseExpressionNumberCompare(expression, left, right);
        break;
    case AST_OPERATOR_EQUALS:
    case AST_OPERATOR_NOT_EQUALS:
        type_error = analyseExpressionCompare(expression, left, right);
        break;
    case AST_OPERATOR_AND:
    case AST_OPERATOR_OR:
        type_error = analyseExpressionLogical(expression, left, right);
        break;
    }

    if (type_error) {
        ErrorL(expression->line, "mismatch type in '%s %s %s' binary operation",
                TypeToString(left->type), AstPrintBinaryOperator(operator),
                TypeToString(right->type));
    }
}

static bool analyseExpressionArith(AstExpression* expression,
        AstExpression* left, AstExpression* right)
{
    if (!TypeIsNumerical(left->type) || !TypeIsNumerical(right->type))
        return true;

    insertNumericalCast(left, right);
    expression->type = left->type;
    return false;
}

static bool analyseExpressionNumberCompare(AstExpression* expression,
        AstExpression* left, AstExpression* right)
{
    if (!TypeIsNumerical(left->type) || !TypeIsNumerical(right->type))
        return true;

    insertNumericalCast(left, right);
    expression->type = TypeCreate(TYPE_BOOL, 0);
    return false;
}

static bool analyseExpressionCompare(AstExpression* expression,
        AstExpression* left, AstExpression* right)
{
    setNullExpressionType(left, right->type);
    setNullExpressionType(right, left->type);

    bool numerical =
            TypeIsNumerical(left->type) && TypeIsNumerical(right->type);
    bool boolean = TypeIsBool(left->type) && TypeIsBool(right->type);
    bool array =
            TypeIsArray(left->type) && TypeEquals(left->type, right->type);

    if (!numerical && !boolean && !array)
        return true;

    insertNumericalCast(left, right);
    expression->type = TypeCreate(TYPE_BOOL, 0);
    return false;
}

static bool analyseExpressionLogical(AstExpression* expression,
        AstExpression* left, AstExpression* right)
{
    bool error = !TypeIsBool(left->type) || !TypeIsBool(right->type);
    if (!error)
        expression->type = TypeCreate(TYPE_BOOL, 0);
    return error;
}

static void analyseVariable(AstVariable* variable)
{
    switch (variable->tag) {
    case AST_VARIABLE_REFERENCE: {
        char* identifier = variable->u.reference_.u.identifier_;
        AstDeclaration* declaration = SymbolsFind(identifier, variable->line);
        if (declaration->tag != AST_DECLARATION_VARIABLE) {
            ErrorL(variable->line, "cannot access '%s' function's value",
                    identifier);
        }
        variable->type = declaration->type;
        variable->u.reference_.u.declaration_ = declaration;
        variable->u.reference_.is_declaration = true;
        break;
    }
    case AST_VARIABLE_ARRAY: {
        AstExpression* location = variable->u.array_.location;
        analyseExpression(location);
        if (!TypeIsArray(location->type)) {
            ErrorL(variable->line, "mismatch type in left expression of "
                    "access, expected an array, read '%s'",
                    TypeToString(location->type));
        }
        AstExpression* offset = variable->u.array_.offset;
        analyseExpression(offset);
        if (!TypeIsInt(offset->type)) {
            ErrorL(variable->line, "mismatch type in right expression of "
                    "access, expected 'int', read '%s'",
                    TypeToString(offset->type));
        }
        variable->type = TypeCreate(location->type.tag,
                location->type.pointers - 1);
        break;
    }
    }
}

static void setNullExpressionType(AstExpression* expression, Type type)
{
    if (expression->tag == AST_EXPRESSION_NULL && TypeIsArray(type))
        expression->type = type;
}

static void insertAssignmentCast(AstExpression* expression, Type variable)
{
    if (TypeIsChar(variable))
        variable = TypeCreate(TYPE_INT, 0);

    if (TypeIsInt(variable) && TypeIsFloat(expression->type))
        AstExpressionCast(expression, variable, AST_CAST_FLOAT_TO_INT);
    else if (TypeIsFloat(variable) && TypeIsInt(expression->type))
        AstExpressionCast(expression, variable, AST_CAST_INT_TO_FLOAT);
}

static void insertNumericalCast(AstExpression* left, AstExpression* right)
{
    if (!TypeEquals(left->type, right->type)) {
        if (TypeIsInt(left->type))
            AstExpressionCast(left, right->type, AST_CAST_INT_TO_FLOAT);
        else
            AstExpressionCast(right, left->type, AST_CAST_INT_TO_FLOAT);
    }
}

