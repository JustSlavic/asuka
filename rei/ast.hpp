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

struct AST__expression {
    AST__expression_tag tag;
    AST__type *type;

    union {
        AST__literal *literal;
        AST__variable *variable;
        AST__variable_declaration *variable_declaration;
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
    AST__expression *expr;
};


char spaces[] = "                                                  ";
void indent(int count) {
    printf("%.*s", count, spaces);
}

struct print_settings {
    int indentation = 4;
};

void print_ast(AST__expression *expression, print_settings settings, int depth = 0);

void print_ast(AST__literal *literal, print_settings settings, int depth = 0) {
    if (literal->value.type == TOKEN_INT_LITERAL) {
        printf("<LITERAL: %lld>", literal->value.integer);
    } else {
        INVALID_CODE_PATH();
    }
}

void print_ast(AST__type *type, print_settings settings, int depth = 0) {
    printf("%.*s", PRINT_SPAN(type->name));
}

void print_ast(AST__variable_declaration *declaration, print_settings settings, int depth = 0) {
    printf("%.*s : ", PRINT_SPAN(declaration->name));
    if (declaration->type) print_ast(declaration->type, settings); else printf("<none>");
    printf(" = ");
    if (declaration->initialization) print_ast(declaration->initialization, settings, depth); else printf("<none>");
    printf(";");
}

void print_ast(AST__block *block, print_settings settings, int depth = 0) {
    indent(depth * settings.indentation);
    printf("{\n");
    List<AST__expression *> *expr = block->expressions;
    while (expr) {
        print_ast(expr->value, settings, depth + 1);
        printf("\n");
        expr = expr->next;
    }
    indent(depth * settings.indentation);
    printf("}\n");
}

void print_ast(AST__function_declaration *declaration, print_settings settings, int depth = 0) {
    printf("%.*s :: (", PRINT_SPAN(declaration->name));
    auto *arg = declaration->argument_list;
    while (arg) {
        print_ast(arg->value, settings);
        arg = arg->next;
    }
    printf(")");
    if (declaration->return_type) {
        printf(" -> ");
        print_ast(declaration->return_type, settings, depth);
    }
    print_ast(declaration->body, settings);
}

void print_ast(AST__operator_call *op_call, print_settings settings, int depth = 0) {
    if (op_call->left_operand) {
        printf("(");
        print_ast(op_call->left_operand, settings, depth);
        printf(")");
    } else {
        printf("<none>");
    }
    switch (op_call->op.type) {
        case TOKEN_MINUS: printf("-"); break;
        case TOKEN_PLUS: printf("+"); break;
        case TOKEN_ASTERICKS: printf("*"); break;
        case TOKEN_SLASH: printf("/"); break;
    }
    if (op_call->right_operand) {
        printf("(");
        print_ast(op_call->right_operand, settings, depth);
        printf(")");
    } else {
        printf("<none>");
    }

}

void print_ast(AST__expression *expression, print_settings settings, int depth) {
    switch (expression->tag) {
        case AST_EXPRESSION_INVALID: printf("AST_EXPRESSION_INVALID"); break;
        case AST_EXPRESSION_LITERAL:
        {
            AST__literal *literal = expression->literal;
            print_ast(literal, settings, depth);
        }
        break;
        case AST_EXPRESSION_VARIABLE_DECLARATION: {
            AST__variable_declaration *declaration = expression->variable_declaration;

            indent(settings.indentation * depth);
            print_ast(declaration, settings, depth);
        }
        break;
        case AST_EXPRESSION_OPERATOR_CALL:
        {
            AST__operator_call *operator_call = expression->operator_call;
            print_ast(operator_call, settings, depth);
        }
        break;
        case AST_EXPRESSION_RETURN:
        {
            AST__return_statement *return_statement = expression->return_statement;
            indent(settings.indentation * depth);
            printf("return "); print_ast(return_statement->expr, settings, depth); printf(";");
        }
        break;
        case AST_EXPRESSION_VARIABLE:
        {
            AST__variable *variable = expression->variable;
            printf("<VAR '%.*s'>", PRINT_SPAN(variable->name));
        } break;
        default:
            INVALID_CODE_PATH();
    }

}

} // namespace rei
