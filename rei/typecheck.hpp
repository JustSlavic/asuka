#pragma once

#include <defines.hpp>
#include "ast.hpp"


namespace rei {


AST__type *get_type(MemoryArena *arena, AST__scope *scope, AST__expression *expression)
{
    ASSERT(expression);

    if (expression->type)
    {
        return expression->type;
    }

    switch (expression->tag) {
        case AST_EXPRESSION_LITERAL:
        {
            AST__literal *literal = &expression->literal;
            if (literal->value.type == TOKEN_INT_LITERAL)
            {
                AST__type *type = allocate_struct(arena, AST__type);
                type->name = string::from_cstr("int");

                expression->type = type;
            }
        }
        break;

        case AST_EXPRESSION_VARIABLE:
        {
            AST__variable *variable = &expression->variable;
            AST__expression *declaration = find_symbol_in_scope(scope, variable->name);
            if (declaration)
            {
                ASSERT(declaration->type);
                expression->type = declaration->type;
            }
            else
            {
                printf("Could not find declaration of variable '%.*s'!\n", PRINT_SPAN(variable->name));
                return NULL;
            }
        }
        break;

        case AST_EXPRESSION_OPERATOR_CALL:
        {
            AST__operator_call *op_call = &expression->operator_call;
            auto *left_type = get_type(arena, scope, op_call->left_operand);
            auto *right_type = get_type(arena, scope, op_call->right_operand);

            if (left_type && right_type)
            {
                if (is_same_type(left_type, right_type))
                {
                    expression->type = left_type;
                }
                else
                {
                    // @todo: report error: Type checking phase! Types do not match!
                    printf("error: Type checking phase! Types do not match!\n");
                    printf("       Left   '%.*s'\n"
                           "       Right  '%.*s'\n",
                           PRINT_SPAN(left_type->name), PRINT_SPAN(right_type->name));
                }
            }
        }
        break;
    }

    return expression->type;
}

bool32 check_types(MemoryArena *arena, AST__scope *scope, AST__expression *expression)
{
    ASSERT(expression);

    if (expression->tag == AST_EXPRESSION_VARIABLE_DECLARATION)
    {
        AST__variable_declaration *declaration = &expression->variable_declaration;

        if (expression->type == NULL)
        {
            // Type deduction
            if (declaration->initialization == NULL)
            {
                // @todo: report error: Could not deduce type of a variable! Initialization should be provided!
                printf("error: Could not deduce type of a variable! Initialization should be provided!\n");
                return FAILURE;
            }

            AST__type *type = get_type(arena, scope, declaration->initialization);
            if (type == NULL)
            {
                // @todo: report error: Could not get type of a initialization!
                printf("error: Could not get type of a initialization!\n");
                return FAILURE;
            }

            expression->type = type;
        }
        else
        {
            // Check types
            if (declaration->initialization == NULL)
            {
                return SUCCESS; // Initialization is not provided, bute type is => ok.
            }

            AST__type *type = get_type(arena, scope, declaration->initialization);
            if (type == NULL)
            {
                // @todo: report error: Could not get type of a initialization!
                printf("error: Could not get type of a initialization!\n");
                return 0;
            }

            bool32 ok = is_same_type(expression->type, type);
            return ok;
        }
    }

    if (expression->tag == AST_EXPRESSION_FUNCTION_DECLARATION)
    {
        AST__function_declaration *declaration = &expression->function_declaration;
        if (declaration->body)
        {
            AST__scope *body_scope = declaration->body->scope;
            for (AST__expression *expr = body_scope->expressions; expr; expr = expr->next)
            {
                b32 ok = check_types(arena, body_scope, expr);
                if (!ok) return FAILURE;
            }
        }
    }

    return SUCCESS;
}


} // namespace rei
