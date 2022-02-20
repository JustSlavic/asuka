#pragma once

#include <string.hpp>
#include <stdio.h>


namespace rei {

using asuka::string;

template<typename T>
struct List {
    T value;
    List<T> *next;
};


struct AST__expression;
struct AST__function_declaration;
struct AST__variable_declaration;
struct AST__block;
struct AST__type;
struct AST__literal;
struct AST__variable;
struct AST__operator_call;
struct AST__function_call;
struct AST__return_statement;
struct AST__scope;


enum AST__expression_tag {
    AST_EXPRESSION_INVALID = 0,
    AST_EXPRESSION_LITERAL,
    AST_EXPRESSION_VARIABLE,
    AST_EXPRESSION_VARIABLE_DECLARATION,
    AST_EXPRESSION_FUNCTION_DECLARATION,
    AST_EXPRESSION_OPERATOR_CALL,
    AST_EXPRESSION_FUNCTION_CALL,
    AST_EXPRESSION_RETURN,
};

struct AST__function_declaration {
    string name;
    AST__block *body;
};

struct AST__variable_declaration {
    string name;
    AST__expression *initialization;
};

struct AST__block {
    AST__scope *scope;
};

struct AST__type {
    string name;
};

struct AST__variable {
    string name;
};

struct AST__literal {
    Token value;
};

struct AST__operator_call {
    AST__expression *left_operand;
    AST__expression *right_operand;
    Token op;
};

struct AST__function_call {

};

struct AST__return_statement {
    AST__expression *returned_expression;
};


struct AST__expression {
    AST__expression_tag tag;
    AST__type *type;

    union {
        AST__literal               literal;
        AST__variable              variable;
        AST__variable_declaration  variable_declaration;
        AST__function_declaration  function_declaration;
        AST__function_call         function_call;
        AST__operator_call         operator_call;
        AST__return_statement      return_statement;
    };

    AST__expression *next;
};


struct AST__scope {
    AST__expression *expressions;
    AST__expression *last_expression;

    AST__scope *parent_scope;
};


void push_expression(AST__scope *scope, AST__expression *expr)
{
    if (scope->expressions == NULL && scope->last_expression == NULL)
    {
        scope->expressions = expr;
        scope->last_expression = expr;
    }
    else
    {
        ASSERT(scope->expressions);
        ASSERT(scope->last_expression);

        scope->last_expression->next = expr;
        scope->last_expression = expr;
    }
}


bool32 is_same_type(AST__type *lhs, AST__type *rhs)
{
    return lhs->name == rhs->name;
}


// @note: This function should be called AFTER type deduction phase!
//        Before type deduction, types can be NULL.
AST__expression *find_symbol_in_scope(AST__scope *scope, string symbol)
{
    AST__expression *result = NULL;

    while (scope && (result == NULL))
    {
        for (AST__expression *expr = scope->expressions; expr; expr = expr->next)
        {
            if (expr->tag == AST_EXPRESSION_VARIABLE_DECLARATION)
            {
                if (expr->variable_declaration.name == symbol)
                {
                    result = expr;
                    break;
                }
            }
            else if (expr->tag == AST_EXPRESSION_FUNCTION_DECLARATION)
            {
                if (expr->function_declaration.name == symbol)
                {
                    result = expr;
                    break;
                }
            }
        }

        scope = scope->parent_scope;
    }

    return result;
}


AST__expression *make_literal(memory::arena_allocator *arena, Token value)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_LITERAL;
    result->literal.value = value;

    return result;
}


AST__expression *make_variable(memory::arena_allocator *arena, string name)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_LITERAL;
    result->variable.name = name;

    return result;
}


AST__expression *make_variable_declaration(memory::arena_allocator *arena, string name, AST__type *type, AST__expression *init)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_VARIABLE_DECLARATION;
    result->type = type;
    result->variable_declaration.name = name;
    result->variable_declaration.initialization = init;

    return result;
}


AST__expression *make_function_declaration(memory::arena_allocator *arena, string name, AST__expression *arguments, AST__type *type, AST__block *body)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_FUNCTION_DECLARATION;
    result->type = type;
    result->function_declaration.name = name;
    result->function_declaration.body = body;

    return result;
}

AST__expression *make_function_call(memory::arena_allocator *arena)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_FUNCTION_CALL;

    return result;
}


AST__expression *make_operator_call(memory::arena_allocator *arena, AST__expression *lhs, AST__expression *rhs, Token op)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_OPERATOR_CALL;
    result->operator_call.left_operand  = lhs;
    result->operator_call.right_operand = rhs;
    result->operator_call.op = op;

    return result;
}


AST__expression *make_return_statement(memory::arena_allocator *arena, AST__expression *returned_expression)
{
    auto *result = allocate_struct(arena, AST__expression);
    result->tag = AST_EXPRESSION_RETURN;
    result->return_statement.returned_expression = returned_expression;

    return result;
}



char spaces[] = "                                                  ";
void indent(int count) {
    printf("%.*s", count, spaces);
}

struct print_settings {
    int indentation = 4;
};

void print_ast(AST__expression *expression, print_settings settings, int depth = 0);


void print_ast(AST__type *type, print_settings settings, int depth = 0)
{
    printf("%.*s", PRINT_SPAN(type->name));
}


void print_ast(AST__operator_call *op_call, print_settings settings, int depth = 0)
{
    if (op_call->left_operand)
    {
        printf("(");
        print_ast(op_call->left_operand, settings, depth);
        printf(")");
    }
    else
    {
        printf("<none>");
    }

    switch (op_call->op.type)
    {
        case TOKEN_MINUS: printf("-"); break;
        case TOKEN_PLUS: printf("+"); break;
        case TOKEN_ASTERICKS: printf("*"); break;
        case TOKEN_SLASH: printf("/"); break;
    }

    if (op_call->right_operand)
    {
        printf("(");
        print_ast(op_call->right_operand, settings, depth);
        printf(")");
    }
    else
    {
        printf("<none>");
    }
}


void print_ast(AST__expression *expression, print_settings settings, int depth)
{
    // indent(settings.indentation * depth);

    switch (expression->tag)
    {
        case AST_EXPRESSION_INVALID:
        {
            printf("AST_EXPRESSION_INVALID");
        }
        break;

        case AST_EXPRESSION_LITERAL:
        {
            AST__literal *literal = &expression->literal;
            if (literal->value.type == TOKEN_INT_LITERAL) {
                printf("<LITERAL: %lld>", literal->value.integer);
            } else {
                INVALID_CODE_PATH();
            }
        }
        break;

        case AST_EXPRESSION_VARIABLE_DECLARATION:
        {
            AST__variable_declaration *declaration = &expression->variable_declaration;

            indent(settings.indentation * depth);

            printf("%.*s : ", PRINT_SPAN(declaration->name));

            if (expression->type)
            {
                printf("%.*s", PRINT_SPAN(expression->type->name));
            }
            else
            {
                printf("<none>");
            }

            printf(" = ");

            if (declaration->initialization)
            {
                print_ast(declaration->initialization, settings, depth);
            }
            else
            {
                printf("<none>");
            }

            printf(";\n");
        }
        break;

        case AST_EXPRESSION_OPERATOR_CALL:
        {
            AST__operator_call *operator_call = &expression->operator_call;
            print_ast(operator_call, settings, depth);
        }
        break;

        case AST_EXPRESSION_RETURN:
        {
            AST__return_statement *return_statement = &expression->return_statement;
            indent(settings.indentation * depth);
            printf("return "); print_ast(return_statement->returned_expression, settings, 0);
            printf(";\n");
        }
        break;

        case AST_EXPRESSION_VARIABLE:
        {
            AST__variable *variable = &expression->variable;
            printf("<VAR '%.*s'>", PRINT_SPAN(variable->name));
        }
        break;

        case AST_EXPRESSION_FUNCTION_DECLARATION:
        {
            AST__function_declaration *declaration = &expression->function_declaration;
            printf("%.*s :: (<none>) -> <none>\n", PRINT_SPAN(declaration->name));

            if (declaration->body) {
                indent(settings.indentation * depth); printf("{\n");

                AST__expression *expr = declaration->body->scope->expressions;
                while (expr) {
                    print_ast(expr, settings, depth + 1);
                    expr = expr->next;
                }

                indent(settings.indentation * depth); printf("}\n");
            }
        }
        break;

        default:
        {
            INVALID_CODE_PATH();
        }
    }
}

} // namespace rei
