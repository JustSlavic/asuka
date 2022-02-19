/*

   GLOBAL TODO:

- Make testing framework + write simple tests for parsing
- Parse function calls
+ Parse expressions (a + b * c)
- Scope checking
- Type checking
- AST generation
- C code generation
- Compiling C code
- [optionally] Interpreter

Types:
- Type parsing
- Type representation
- Struct type
- Pointer type
- Tuple type (multiplication of types)
- Unit type (empty tuple)
- Enum type
- Discriminated union type (sum of types)

EXPERIMENTAL:
- Make space to be valid in name context ?
  e.g.:

        name of something := call something ();
        why can I use this : because everything is = separated ( by=punctuation );

  equivalent to:

        name_of_something := call_something();
        why_can_I_use_this : because_everything_is = separated(by=punctuation);
*/

#include <stdio.h>
#include <stdlib.h>

#include <string.hpp>
#include <memory_arena.hpp>
#include "parser.hpp"
#include "typecheck.hpp"

#include "token.hpp"


using asuka::string;

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

        result.data = (char *) memory;
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
    parser.arena = &arena;

    rei::print_settings settings;
    settings.indentation = 4;

    rei::AST__scope *global_scope = parser.parse_scope();

    auto *expression = global_scope->expressions;
    while (expression) {
        bool32 ok = check_types(&arena, global_scope, expression);
        printf("Types are %sOK!!\n", ok ? "" : "_NOT_ ");
        if (ok) print_ast(expression, settings, 0);

        expression = expression->next;
    }

    return 0;
}
