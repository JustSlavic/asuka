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

#include <defines.hpp>
#include <math.hpp>

#include <stdio.h>
#include <stdlib.h>

#include <string.hpp>
#include <memory_arena.hpp>
#include "parser.hpp"
#include "typecheck.hpp"

#include "token.hpp"
#include "../son/son.hpp"


int main() {
    asuka::register_constructor("v2", make_v2);

    memory::arena_allocator arena{};
    arena.size   = 1 << 16;
    arena.memory = calloc(sizeof(uint8), arena.size);

    asuka::string file_contents = os::load_entire_file("example.rei");
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
