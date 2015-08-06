/*
 * Monga Language
 * Author: Gabriel de Quadros Ligneul
 *
 * ast.c
 */

#include "ast.h"

#include "util/new.h"

AstDeclaration* AstDeclarationVariable(Type type, char* identifier, int line)
{
    AstDeclaration* node = NEW(AstDeclaration);
    node->tag = AST_DECLARATION_VARIABLE;
    node->type = type;
    node->identifier = identifier;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.variable_.global = false;
    node->u.variable_.offset = 0;
    return node;
}

AstDeclaration* AstDeclarationFunction(Type type, char* identifier, int line,
        AstDeclaration* parameters, AstStatement* block)
{
    AstDeclaration* node = NEW(AstDeclaration);
    node->tag = AST_DECLARATION_FUNCTION;
    node->type = type;
    node->identifier = identifier;
    node->line = line;
    node->next = NULL;
    node->last = node;
    node->u.function_.parameters = parameters;
    node->u.function_.n_parameters = 0;
    while (parameters != NULL) {
        node->u.function_.n_parameters++;
        parameters = parameters->next;
    }
    node->u.function_.block = block;
    node->u.function_.space = 0;
    return node;
}

AstStatement* AstStatementBlock(AstDeclaration* variables,
        AstStatement* statements, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_BLOCK;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.block_.variables = variables;
    node->u.block_.statements = statements;
    return node;
}

AstStatement* AstStatementIf(AstExpression* expression,
        AstStatement* then_statement, AstStatement* else_statement, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_IF;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.if_.expression = expression;
    node->u.if_.then_statement = then_statement;
    node->u.if_.else_statement = else_statement;
    return node;
}

AstStatement* AstStatementWhile(AstExpression* expression,
        AstStatement* statement, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_WHILE;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.while_.expression = expression;
    node->u.while_.statement = statement;
    return node;
}

AstStatement* AstStatementAssign(AstVariable* variable,
        AstExpression* expression, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_ASSIGN;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.assign_.variable = variable;
    node->u.assign_.expression = expression;
    return node;
}

AstStatement* AstStatementDelete(AstExpression* expression, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_DELETE;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.delete_.expression = expression;
    return node;
}

AstStatement* AstStatementPrint(AstExpression* expressions, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_PRINT;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.print_.expressions = expressions;
    return node;
}

AstStatement* AstStatementReturn(AstExpression* expression, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_RETURN;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.return_.expression = expression;
    return node;
}

AstStatement* AstStatementCall(AstExpression* call, int line)
{
    AstStatement* node = NEW(AstStatement);
    node->tag = AST_STATEMENT_CALL;
    node->line = line;
    node->returned = false;
    node->next = NULL;
    node->last = node;
    node->u.call_ = call;
    return node;
}

AstExpression* AstExpressionKBool(bool value)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_KBOOL;
    node->line = 0;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.kbool_ = value;
    return node;
}

AstExpression* AstExpressionKInt(int value)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_KINT;
    node->line = 0;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.kint_ = value;
    return node;
}

AstExpression* AstExpressionKFloat(float value)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_KFLOAT;
    node->line = 0;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.kfloat_ = value;
    return node;
}

AstExpression* AstExpressionString(char* string)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_STRING;
    node->line = 0;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.string_ = string;
    return node;
}

AstExpression* AstExpressionNull()
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_NULL;
    node->line = 0;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    return node;
}

AstExpression* AstExpressionCall(char* identifier, AstExpression* expressions,
        int line)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_CALL;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.call_.is_declaration = false;
    node->u.call_.u.identifier_ = identifier;
    node->u.call_.expressions = expressions;
    return node;
}

AstExpression* AstExpressionVariable(AstVariable* variable, int line)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_VARIABLE;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.variable_ = variable;
    return node;
}

AstExpression* AstExpressionNew(Type type, AstExpression* expression, int line)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_NEW;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.new_.type = type;
    node->u.new_.expression = expression;
    return node;
}

AstExpression* AstExpressionUnary(AstUnaryOperator operator,
        AstExpression* expression, int line)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_UNARY;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.unary_.operator = operator;
    node->u.unary_.expression = expression;
    return node;
}

AstExpression* AstExpressionBinary(AstBinaryOperator operator,
        AstExpression* expression_left, AstExpression* expression_right,
        int line)
{
    AstExpression* node = NEW(AstExpression);
    node->tag = AST_EXPRESSION_BINARY;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->next = NULL;
    node->last = node;
    node->u.binary_.operator = operator;
    node->u.binary_.expression_left = expression_left;
    node->u.binary_.expression_right = expression_right;
    return node;
}

void AstExpressionCast(AstExpression* expression, Type goal_type,
        AstCastTag cast_tag)
{
    AstExpression* subexpression = NEW(AstExpression);
    *subexpression = *expression;
    subexpression->next = NULL;
    expression->tag = AST_EXPRESSION_CAST;
    expression->type = goal_type;
    expression->u.cast_.tag = cast_tag;
    expression->u.cast_.expression = subexpression;
}

AstVariable* AstVariableReference(char* identifier, int line)
{
    AstVariable* node = NEW(AstVariable);
    node->tag = AST_VARIABLE_REFERENCE;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->u.reference_.is_declaration = false;
    node->u.reference_.u.identifier_ = identifier;
    return node;
}

AstVariable* AstVariableArray(AstExpression* location, AstExpression* offset,
        int line)
{
    AstVariable* node = NEW(AstVariable);
    node->tag = AST_VARIABLE_ARRAY;
    node->line = line;
    node->type = TypeCreate(TYPE_UNDEFINED, 0);
    node->u.array_.location = location;
    node->u.array_.offset = offset;
    return node;
}

