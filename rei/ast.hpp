#pragma once

#include <string.hpp>
#include <stdio.h>


namespace rei {

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
struct AST__operator_call;
struct AST__function_call;
struct AST__return_statement;


enum AST__expression_type {
    AST_EXPRESSION_INVALID = 0,
    AST_EXPRESSION_LITERAL,
    AST_EXPRESSION_VARIABLE_DECLARATION,
    AST_EXPRESSION_OPERATOR_CALL,
    AST_EXPRESSION_RETURN,
};

struct AST__expression {
    AST__expression_type type;

    union {
        AST__literal *literal;
        AST__function_declaration *function_declaration;
        AST__operator_call *operator_call;
        AST__return_statement *return_statement;
    };
};

struct AST__function_declaration {
    string name;
    // Arguments: name-type pairs
    List<AST__variable_declaration *> *argument_list;
    AST__type *return_type;
    AST__block *body;
};

struct AST__variable_declaration {
    string name;
    AST__type *type;
    AST__expression *initialization;
};

struct AST__block {
    List<AST__expression *> *expressions;
};

struct AST__type {
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
    AST__expression *expr;
};


void print_ast(AST__expression *expression);

void print_ast(AST__literal *literal) {
    if (literal->value.type == TOKEN_INT_LITERAL) {
        printf("<LITERAL: %lld>", literal->value.integer);
    } else {
        INVALID_CODE_PATH();
    }
}

void print_ast(AST__type *type) {
    printf("%.*s", PRINT_SPAN(type->name));
}

void print_ast(AST__variable_declaration *declaration) {
    printf("%.*s : ", PRINT_SPAN(declaration->name));
    if (declaration->type) print_ast(declaration->type);
    printf(" = ");
    if (declaration->initialization) print_ast(declaration->initialization);
    printf(";\n");
}

void print_ast(AST__block *block) {
    printf("{");
    List<AST__expression *> *expr = block->expressions;
    while (expr) {
        print_ast(expr->value);
        expr = expr->next;
    }
    printf("}");
}

void print_ast(AST__function_declaration *declaration) {
    printf("%.*s :: (", PRINT_SPAN(declaration->name));
    auto *arg = declaration->argument_list;
    while (arg) {
        print_ast(arg->value);
        arg = arg->next;
    }
    printf(")");
    if (declaration->return_type) {
        printf(" -> ");
        print_ast(declaration->return_type);
    }
    print_ast(declaration->body);
}

void print_ast(AST__operator_call *op_call) {
    if (op_call->left_operand) print_ast(op_call->left_operand);
    switch (op_call->op.type) {
        case TOKEN_MINUS: printf("-"); break;
    }
    if (op_call->right_operand) print_ast(op_call->right_operand);

}

void print_ast(AST__expression *expression) {
    switch (expression->type) {
        case AST_EXPRESSION_INVALID: printf("AST_EXPRESSION_INVALID"); break;
        case AST_EXPRESSION_LITERAL:
        {
            AST__literal *literal = expression->literal;
            print_ast(literal);
        }
        break;
        case AST_EXPRESSION_VARIABLE_DECLARATION: printf("AST_EXPRESSION_VARIABLE_DECLARATION"); break;
        case AST_EXPRESSION_OPERATOR_CALL:
        {
            AST__operator_call *operator_call = expression->operator_call;
            print_ast(operator_call);
        }
        break;
        case AST_EXPRESSION_RETURN:
        {
            AST__return_statement *return_statement = expression->return_statement;
            printf("return "); print_ast(return_statement->expr); printf(";");
        }
        break;
        default:
            INVALID_CODE_PATH();
    }

}

} // namespace rei
