#pragma once

#include <defines.hpp>
#include "ast.hpp"


namespace rei {

bool32 is_same_type(AST__type *lhs, AST__type *rhs) {
    return lhs->name == rhs->name;
}

AST__type *get_type(MemoryArena *arena, AST__expression *expression) {
    ASSERT(expression);

    if (expression->type) {
        return expression->type;
    }

    switch (expression->tag) {
        case AST_EXPRESSION_LITERAL:
        {
            AST__literal *literal = expression->literal;
            if (literal->value.type == TOKEN_INT_LITERAL) {
                AST__type *type = allocate_struct(arena, AST__type);
                type->name = string::from_cstr("int");

                expression->type = type;
            }
        }
        break;
        case AST_EXPRESSION_OPERATOR_CALL:
        {
            AST__operator_call *op_call = expression->operator_call;
            auto *left_type = get_type(arena, op_call->left_operand);
            auto *right_type = get_type(arena, op_call->right_operand);

            if (is_same_type(left_type, right_type)) {
                expression->type = left_type;
            } else {
                // @todo: report error: Type checking phase! Types do not match!
                printf("error: Type checking phase! Types do not match!\n");
                printf("       Left   '%.*s'\n"
                       "       Right  '%.*s'\n",
                       PRINT_SPAN(left_type->name), PRINT_SPAN(right_type->name));
            }
        } break;
    }

    return expression->type;
}

bool32 check_types(MemoryArena *arena, AST__expression *expression) {
    ASSERT(expression);

    if (expression->tag == AST_EXPRESSION_VARIABLE_DECLARATION) {
        AST__variable_declaration *declaration = expression->variable_declaration;
        ASSERT(declaration);

        if (declaration->type == NULL) {
            // Type deduction
            if (declaration->initialization == NULL) {
                // @todo: report error: Could not deduce type of a variable! Initialization should be provided!
                printf("error: Could not deduce type of a variable! Initialization should be provided!\n");
                return 0;
            }

            AST__type *type = get_type(arena, declaration->initialization);
            if (type == NULL) {
                // @todo: report error: Could not get type of a initialization!
                printf("error: Could not get type of a initialization!\n");
                return 0;
            }

            declaration->type = type;
        } else {
            // Check types
            if (declaration->initialization == NULL) {
                return 1; // Initialization is not provided, ok.
            }

            AST__type *type = get_type(arena, declaration->initialization);
            if (type == NULL) {
                // @todo: report error: Could not get type of a initialization!
                printf("error: Could not get type of a initialization!\n");
                return 0;
            }

            bool32 ok = is_same_type(declaration->type, type);
            return ok;
        }
    }

    return 1;
}


} // namespace rei
