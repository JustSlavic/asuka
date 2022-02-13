// /*

// GLOBAL TODO:

// - Make testing framework + write simple tests for parsing
// - Parse function calls
// - Parse expressions (a + b * c)
// - Scope checking
// - Type checking
// - AST generation
// - C code generation
// - Compiling C code
// - [optionally] Interpreter

// */


#include <stdio.h>
#include <stdlib.h>

#include <string.hpp>
#include <memory_arena.hpp>
#include "parser.hpp"

#include "token.hpp"


string load_entire_file(char* filename) {
    string result{};

    FILE *f = fopen(filename, "rb");

    if (f) {
        defer { fclose(f); };

        fseek(f, 0, SEEK_END);
        size_t size = ftell(f);
        fseek(f, 0, SEEK_SET);

        char *memory = (char *)calloc(size, sizeof(char));
        size_t read_bytes = fread(memory, sizeof(char), size, f);
        if (read_bytes < size) {
            // @warning
            size = read_bytes;
        }

        result.data = (uint8 *) memory;
        result.size = size;
    }

    return result;
}


int main() {
    MemoryArena arena{};
    arena.size   = 1 << 16;
    arena.memory = calloc(sizeof(uint8), arena.size);

    string file_contents = load_entire_file("example.rei");
    if (file_contents.data == 0) {
        printf("Could not load file 'example.rei'\n");
        return 1;
    }

    rei::Lexer lexer{};
    lexer.buffer = file_contents;
    lexer.line = 1;
    lexer.column = 1;

    rei::Parser parser{};
    parser.lexer = lexer;
    parser.arena = arena;

    bool32 running = true;

    while (running) {
        rei::Token t = rei::get_token(&parser.lexer);
        if (t.type == rei::TOKEN_EOF) break;

        rei::Lexer saved = parser.lexer;

        // @todo:
        // 1. Global variables
        // 2. Main function
        // 3. Struct definition
        // 3. Function declaration

        // Global variable
        rei::AST__variable_declaration *global_var_decl = parser.parse_variable_declaration();
        if (global_var_decl) {
            rei::Token semicolon = rei::eat_token(&parser.lexer);
            if (semicolon.type != rei::TOKEN_SEMICOLON) {
                printf("Expected semicolon at the end of variable declaration!\n");
                break;
            }

            rei::print_ast(global_var_decl);
            continue;
        }

        parser.lexer = saved;

        rei::AST__function_declaration *function_declaration = parser.parse_function_declaration();
        if (function_declaration) {
            rei::print_ast(function_declaration);
            continue;
        }

        // Struct declaration (mere existance)
        // Struct definition  (with memory layout)

        // Function declaration (pre declaration)
        // Function definition  (with body)


    //     parser.lexer = saved;

    //     Ast_OperatorDeclaration *operator_declaration = parser.parse_operator_declaration();
    //     if (operator_declaration) {
    //         print_ast((Ast *) operator_declaration);
    //         auto entry = push_struct(&parser.arena, ListEntry<Ast_OperatorDeclaration *>);
    //         entry->value = operator_declaration;

    //         push_back<Ast_OperatorDeclaration *>(&parser.declared_operators, entry);
    //         continue;
    //     }

    //     parser.lexer = saved;

        running = false;
    }

    // while (true) {
    //     Token t = eat_token(&lexer);
    //     print_token(t);

    //     if (t.type == TOKEN_EOF) break;
    // }

    return 0;
}
