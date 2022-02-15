#pragma once

#include <defines.hpp>

#include <memory_arena.hpp>
#include "token.hpp"
#include "ast.hpp"


namespace rei {

// Characters allowed to form operators: + - / % < > = | ~ !
const u32 allowed_as_operator_count = 11;
char allowed_as_operator[allowed_as_operator_count] { '+', '-', '*', '/', '%', '<', '>', '=', '|', '~', '!' };


bool is_whitespace(char c) { return c == ' '; }
bool is_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool is_newline(char c) { return c == '\n'; }
bool is_alpha(char c) { return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; }
bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_valid_identifier_head(char c) { return c == '_' || is_alpha(c); }
bool is_valid_identifier_body(char c) { return c == '_' || is_alpha(c) || is_digit(c) || c == '\''; }

using namespace asuka;

INLINE
TokenType get_identifier_type(string id) {
    if (equals_to_cstr(id, "return")) {
        return TOKEN_KW_RETURN;
    }

    return TOKEN_IDENTIFIER;
}


struct Lexer {
    string buffer;
    u64 index;

    i32 line;
    i32 column;

    Token next_token;
    bool  next_token_valid;

    u32   keyword_count;
    char *keywords[32];
};


char *get_char_pointer(Lexer *lexer) {
    ASSERT(lexer->index < lexer->buffer.size);

    char *result = lexer->buffer.data + lexer->index;
    return result;
}


char get_char(Lexer *lexer) {
    char result = 0;

    if (lexer->index < lexer->buffer.size) {
        result = lexer->buffer.data[lexer->index];
    }

    return result;
}


char eat_char(Lexer *lexer) {
    char result = get_char(lexer);
    lexer->index += 1;

    if (result == '\n') {
        lexer->line += 1;
        lexer->column = 0;
    }

    lexer->column += 1;

    return result;
}


void consume_whitespaces(Lexer *lexer) {
    char c = get_char(lexer);
    while (is_space(c)) {
        eat_char(lexer);
        c = get_char(lexer);
    }
}


bool char_in_array(char c, char *array, u32 array_count) {
    for (u32 i = 0; i < array_count; i++) {
        if (array[i] == c) return true;
    }

    return false;
}


string consume_operator(Lexer *lexer, char *allowed_characters, u32 allowed_characters_count) {
    string result{};
    result.data = get_char_pointer(lexer);

    i32 n = 0;
    char c = get_char(lexer);
    while (char_in_array(c, allowed_characters, allowed_characters_count)) {
        eat_char(lexer);
        c = get_char(lexer);
        n += 1;
    }

    result.size = n;
    return result;
}


Token get_token(Lexer *lexer) {
    if (!lexer->next_token_valid) {
        Token t{};

        consume_whitespaces(lexer);

        char c = get_char(lexer);
        if (c == 0) {
            t.type = TOKEN_EOF;
            t.line = lexer->line;
            t.column = lexer->column;
        } else if (is_valid_identifier_head(c)) {
            t.type = TOKEN_IDENTIFIER;
            t.line = lexer->line;
            t.column = lexer->column;
            t.span.data = get_char_pointer(lexer);

            i32 n = 0;
            while(is_valid_identifier_body(c)) {
                eat_char(lexer);
                c = get_char(lexer);
                n += 1;
            }

            t.span.size = n;

            t.type = get_identifier_type(t.span);
        } else if (is_digit(c)) {
            t.type = TOKEN_INT_LITERAL;
            t.line = lexer->line;
            t.column = lexer->column;
            t.span.data = get_char_pointer(lexer);

            i64 integer = 0;

            c = get_char(lexer);
            while (is_digit(c)) {
                integer *= 10;
                integer += (c - '0');

                eat_char(lexer);
                c = get_char(lexer);
                t.span.size += 1;
            }

            t.integer = integer;
        } else {
            t.line = lexer->line;
            t.column = lexer->column;

            auto eq_cstr = [](char *s1, const char *s2) -> bool32 {
                while (*s1 && *s2) {
                    if (*s1 != *s2) return false;
                    s1 += 1;
                    s2 += 1;
                }

                return true;
            };

            // Compute the string where you'd store operator
            // string op = consume_operator(lexer, allowed, allowed_count);

            char *op_pointer = get_char_pointer(lexer);
            if (eq_cstr(op_pointer, "::")) {
                t.type = TOKEN_DOUBLE_COLON;
                t.span.data = op_pointer;
                t.span.size = 2;

                eat_char(lexer);
                eat_char(lexer);
            } else if (eq_cstr(op_pointer, "->")) {
                t.type = TOKEN_RIGHT_ARROW;
                t.span.data = op_pointer;
                t.span.size = 2;

                eat_char(lexer);
                eat_char(lexer);
            } else {
                string op = consume_operator(lexer, allowed_as_operator, allowed_as_operator_count);
                if (op.size == 0 || op.size == 1) {
                    t.type = (TokenType)(*op.data); // One-character operators are just that
                    t.span.data = op.data;
                    t.span.size = 1;

                    if (op.size == 0) {
                        eat_char(lexer);
                    }
                } else {
                    // If none found, this should be used-defined operator
                    t.type = TOKEN_USED_DEFINED_OPERATOR;
                    t.span = op;
                }
            }
        }

        lexer->next_token = t;
        lexer->next_token_valid = true;
    }

    return lexer->next_token;
}


Token eat_token(Lexer *lexer) {
    Token result = get_token(lexer);
    lexer->next_token_valid = false;
    return result;
}


void print_token(Token t) {
    if (t.type == TOKEN_EOF) {
        // printf("TOKEN:%d:%d{ EOF };\n", t.line, t.column);
    } else {
        // printf("TOKEN:%d:%d{ %.*s };\n", t.line, t.column, PRINT_SPAN(t.span));
    }
}


struct Parser {
    Lexer lexer;
    MemoryArena *arena;

    // List<Ast_FunctionDeclaration *> declared_functions;
    // List<Ast_OperatorDeclaration *> declared_operators;

    AST__literal *parse_literal();
    // Ast_FunctionCall *parse_function_call();
    AST__expression *parse_expression(int precedence);
    AST__expression *parse_expression_operand();
    AST__type *parse_type();
    AST__variable_declaration *parse_variable_declaration();
    AST__function_declaration *parse_function_declaration();
    AST__block *parse_block();
    // bool32 parse_argument_list(Ast_SequenceEntry **result);
    // Ast_ReturnStatement *parse_return_statement();
    // bool32 parse_statement_list(Ast_SequenceEntry **result);
    // Ast_OperatorDeclaration *parse_operator_declaration();
};


AST__literal *Parser::parse_literal() {
    Token t = get_token(&lexer);

    if ((t.type == TOKEN_INT_LITERAL) ||
        (t.type == TOKEN_FLOAT_LITERAL) ||
        (t.type == TOKEN_STRING_LITERAL))
    {
        eat_token(&lexer);
        AST__literal *literal = push_struct(arena, AST__literal);

        literal->value = t;
        return literal;
    }

    return nullptr;
}


// Ast_FunctionCall *Parser::parse_function_call() {
//     Token function_name = eat_token(&lexer);
//     if (function_name.type != TOKEN_IDENTIFIER) {
//         // @error: function call should start with the name of a function
//         return nullptr;
//     }

//     Token open_paren = eat_token(&lexer);
//     if (open_paren.type != TOKEN_OPEN_PAREN) {
//         // @error:
//         return nullptr;
//     }

//     Ast_ArgumentListEntry *argument_head = nullptr;
//     Ast_ArgumentListEntry *argument_tail = nullptr;

//     // @todo: parse expression list here
//     Token next_token = get_token(&lexer);
//     while (next_token.type != TOKEN_CLOSE_PAREN) {
//         Lexer saved = lexer;

//         Ast_Expression *operand = parse_expression(0);
//         if (operand) {
//             Ast_ArgumentListEntry *argument_entry = push_struct(&arena, Ast_ArgumentListEntry);
//             argument_entry->expression = operand;
//             argument_entry->next = nullptr;

//             if (argument_head == nullptr && argument_tail == nullptr) {
//                 argument_head = argument_tail = argument_entry;
//             } else {
//                 argument_tail->next = argument_entry;
//                 argument_tail = argument_entry;
//             }
//         } else {
//             break;
//         }

//         next_token = get_token(&lexer);
//         if (next_token.type == TOKEN_COMMA) {
//             eat_token(&lexer);
//         }
//     }


//     Token close_paren = eat_token(&lexer);
//     if (close_paren.type != TOKEN_CLOSE_PAREN) {
//         // @error:
//         return nullptr;
//     }

//     Ast_FunctionCall *function_call = push_struct(&arena, Ast_FunctionCall);
//     function_call->tag = AST_FUNCTION_CALL;
//     function_call->function_name = function_name;
//     function_call->argument_list = argument_head;

//     return function_call;
// }


AST__expression *Parser::parse_expression_operand() {
    Lexer saved = lexer;

    AST__expression *result = NULL;

    // AST__expression *function_call = parse_function_call();
    // if (function_call) {
    //     return function_call;
    // }

    lexer = saved; // Restore lexer state to another try

    AST__literal *literal = parse_literal();
    if (literal) {
        result = allocate_struct(arena, AST__expression);
        result->tag = AST_EXPRESSION_LITERAL;
        result->literal = literal;
        return result;
    }

    lexer = saved; // Restore lexer state to another try

    Token variable = get_token(&lexer);
    if (variable.type == TOKEN_IDENTIFIER) {
        eat_token(&lexer);
        AST__variable *var = allocate_struct(arena, AST__variable);
        var->name = variable.span;

        result = allocate_struct(arena, AST__expression);
        result->tag = AST_EXPRESSION_VARIABLE;
        result->variable = var;

        return result;
    }

    return result;
}


int get_operator_precedence(Parser *parser, Token op) {
    if (op.type == '+' || op.type == '-') {
        return 1;
    }

    if (op.type == '*' || op.type == '/') {
        return 2;
    }

    // if (op.type == TOKEN_USED_DEFINED_OPERATOR) {
    //     while (entry) {
    //         if (is_equal(entry->value->operator_name.span, op.span)) {
    //             return entry->value->precedence;
    //         }

    //         entry = entry->next;
    //     }
    // }

    return 0;
}


AST__expression *Parser::parse_expression(int precedence) {
    AST__expression *left_expr = nullptr;

    Token open_paren = get_token(&lexer);
    if (open_paren.type == TOKEN_OPEN_PAREN) {
        // ( <expression> )
        eat_token(&lexer); // Eat open parenthesis.

        left_expr = parse_expression(0);
        if (!left_expr) {
            return nullptr;
        }

        Token close_paren = eat_token(&lexer);
        if (close_paren.type != TOKEN_CLOSE_PAREN) {
            // @error: parenthesis do not match
            return nullptr;
        }
    } else {
        // <variable>|<literal>|<function_call>
        // <prefix_operator> <expression>
        // <expression> <postfix_operator>
        // <>
        Lexer saved = lexer;

        left_expr = parse_expression_operand();
        if (!left_expr) {
            return nullptr;
        }
    }

    // @todo: move this push into parse_expression_operand function
    // Ast_Literal *p_left_operand = push_struct(arena, Ast_Literal);
    // p_left_operand->tag = AST_LITERAL; // Or function call or variable? Do I need to know what kind of literal is this?

    // AST__expression *left_expr = (AST__expression *) p_left_operand;

    while (true) {
        Token op = get_token(&lexer);
        if (!(op.type == '+' || op.type == '-' || op.type == '/' || op.type == '*' || op.type == TOKEN_USED_DEFINED_OPERATOR)
            || (get_operator_precedence(this, op) < precedence))
        {
            break;
        }

        eat_token(&lexer);

        Lexer saved = lexer;
        AST__expression *right_expr = parse_expression(get_operator_precedence(this, op) + 1); // +1 for left associativity (+0 for right)
        if (!right_expr) {
            return nullptr;
        }

        AST__operator_call *operator_call = allocate_struct(arena, AST__operator_call);
        operator_call->left_operand = left_expr;
        operator_call->right_operand = right_expr;
        operator_call->op = op;

        AST__expression *result = allocate_struct(arena, AST__expression);
        result->tag = AST_EXPRESSION_OPERATOR_CALL;
        result->operator_call = operator_call;

        left_expr = result;
    }

    return left_expr;
}


AST__type *Parser::parse_type() {
    Token type_name = eat_token(&lexer);
    if (type_name.type != TOKEN_IDENTIFIER) {
        // @todo more types
        return nullptr;
    }

    AST__type *type = allocate_struct(arena, AST__type);
    type->name = type_name.span;
    return type;
}


AST__variable_declaration *Parser::parse_variable_declaration() {
//     // x : int
//     // x : int = 0
//     // x : = 0

//     // <var-name> : (<type>|[<type>] = <expression>);

    Token name = get_token(&lexer);

    if (name.type != TOKEN_IDENTIFIER) {
        // @error: variable declaration should start with a name.
        return nullptr;
    }
    eat_token(&lexer);

    Token colon = eat_token(&lexer);
    if (colon.type != TOKEN_COLON) {
        // @error: colon is required
        return nullptr;
    }

    Lexer saved = lexer;
    AST__type *type = parse_type();
    if (type) {
        Token equals = get_token(&lexer);
        AST__expression *initialization = NULL;

        if (equals.type == TOKEN_EQUALS) {
            eat_token(&lexer);

            saved = lexer;

            initialization = parse_expression(0);
            if (!initialization) {
                lexer = saved;
                // @error: after equal sign there should be expression
                return nullptr;
            }
        }

        AST__variable_declaration *declaration = allocate_struct(arena, AST__variable_declaration);
        declaration->name = name.span;
        declaration->type = type;
        declaration->initialization = initialization;

        return declaration;
    } else {
        lexer = saved; // Restore lexer state but continue, because type is optional.

        Token equals = eat_token(&lexer);
        if (equals.type != TOKEN_EQUALS) {
            // @error: cannot parse initialization of a variable
            return nullptr;
        }

        saved = lexer;

        AST__expression *expression = parse_expression(0);
        if (!expression) {
            lexer = saved;
            return nullptr;
        }

        AST__variable_declaration *declaration = allocate_struct(arena, AST__variable_declaration);
        declaration->name = name.span;
        declaration->type = NULL; // @note: Type will be deduced later.
        declaration->initialization = expression;

        return declaration;
    }
}


// bool32 Parser::parse_argument_list(Ast_SequenceEntry **result) {
//     Ast_SequenceEntry *head = *result;
//     Ast_SequenceEntry *tail = nullptr;

//     {
//         Lexer saved = lexer;

//         Ast_VariableDeclaration *declaration = parse_variable_declaration();
//         if (!declaration) {
//             lexer = saved;
//             return false;
//         }

//         Ast_SequenceEntry *entry = push_struct(arena, Ast_SequenceEntry);
//         entry->value = (Ast *) declaration;

//         if (head == nullptr && tail == nullptr) {
//             head = tail = entry;
//         } else {
//             tail->next = entry;
//             tail = entry;
//         }
//     }


//     while (true) {
//         Token comma = get_token(&lexer);
//         if (comma.type == TOKEN_COMMA) {
//             eat_token(&lexer);
//         } else {
//             break; // End of the list
//         }

//         Ast_VariableDeclaration *declaration = parse_variable_declaration();
//         if (!declaration) {
//             break; // Trailing comma
//         }

//         Ast_SequenceEntry *entry = push_struct(arena, Ast_SequenceEntry);
//         entry->value = (Ast *) declaration;

//         if (head == nullptr && tail == nullptr) {
//             head = tail = entry;
//         } else {
//             tail->next = entry;
//             tail = entry;
//         }
//     }

//     *result = head;
//     return true;
// }


// Ast_ReturnStatement *Parser::parse_return_statement() {
//     Token return_keyword = eat_token(&lexer);
//     if (return_keyword.type != TOKEN_KW_RETURN) {
//         return false;
//     }

//     Ast_Expression *p_expr = parse_expression(0);
//     if (!p_expr) {
//         return false;
//     }

//     auto return_statement = push_struct(arena, Ast_ReturnStatement);
//     return_statement->tag = AST_RETURN_STATEMENT;
//     return_statement->return_expression = p_expr;

//     return return_statement;
// }


// bool32 Parser::parse_statement_list(Ast_SequenceEntry **result) {
//     // (<statement>;|<expression>;)*
//     Lexer saved = lexer;

//     Ast_SequenceEntry *head = nullptr;
//     Ast_SequenceEntry *tail = nullptr;

//     while (true) {
//         Ast_ReturnStatement *return_statement = parse_return_statement();
//         if (return_statement) {
//             Token semicolon = eat_token(&lexer);
//             if (semicolon.type != TOKEN_SEMICOLON) {
//                 // @error: return statement should end with semicolon
//                 return false;
//             }

//             Ast_SequenceEntry *entry = push_struct(arena, Ast_SequenceEntry);
//             entry->value = (Ast *) return_statement;

//             if (head == nullptr && tail == nullptr) {
//                 head = tail = entry;
//             } else {
//                 tail->next = entry;
//                 tail = entry;
//             }

//             saved = lexer;
//             continue;
//         } else {
//             lexer = saved;
//         }

//         Ast_VariableDeclaration *declaration = parse_variable_declaration();
//         if (declaration) {
//             Token semicolon = eat_token(&lexer);
//             if (semicolon.type != TOKEN_SEMICOLON) {
//                 // @error: expression in block have to end with a semicolon.
//                 return false;
//             }

//             Ast_SequenceEntry *entry = push_struct(arena, Ast_SequenceEntry);
//             entry->value = (Ast *) declaration;

//             if (head == nullptr && tail == nullptr) {
//                 head = tail = entry;
//             } else {
//                 tail->next = entry;
//                 tail = entry;
//             }

//             saved = lexer;
//             continue;
//         } else {
//             lexer = saved;
//         }

//         Ast_Expression *p_expr = parse_expression(0);
//         if (p_expr) {
//             Token semicolon = eat_token(&lexer);
//             if (semicolon.type != TOKEN_SEMICOLON) {
//                 // @error: expression in block have to end with a semicolon.
//                 return false;
//             }

//             Ast_SequenceEntry *entry = push_struct(arena, Ast_SequenceEntry);
//             entry->value = (Ast *) p_expr;

//             if (head == nullptr && tail == nullptr) {
//                 head = tail = entry;
//             } else {
//                 tail->next = entry;
//                 tail = entry;
//             }

//             saved = lexer;
//             continue;
//         } else {
//             lexer = saved;
//         }

//         break;
//     }

//     *result = head;

//     return true;
// }


AST__block *Parser::parse_block() {
    // { <statement-list> }
    Lexer saved = lexer;

    Token open_brace = eat_token(&lexer);
    if (open_brace.type != TOKEN_OPEN_BRACE) {
        // @error:
        return false;
    }

    List<AST__expression *> *expressions_head = NULL;
    List<AST__expression *> *expressions_last = NULL;

    {
        while (true) {

            Token t = get_token(&lexer);
            if (t.type == TOKEN_KW_RETURN) {
                eat_token(&lexer);

                AST__expression *return_expression = parse_expression(0);
                if (return_expression) {
                    t = eat_token(&lexer);
                    if (t.type != TOKEN_SEMICOLON) {
                        lexer = saved;
                        printf("Expected ';' but found %.*s", PRINT_SPAN(t.span));
                        return NULL;
                    }

                    if (expressions_head == NULL && expressions_last == NULL) {
                        expressions_head = expressions_last = allocate_struct(arena, List<AST__expression *>);
                    } else if (expressions_head != NULL && expressions_last != NULL) {
                        expressions_last->next = allocate_struct(arena, List<AST__expression *>);
                        expressions_last = expressions_last->next;
                    } else {
                        INVALID_CODE_PATH();
                    }

                    AST__return_statement *return_statement = allocate_struct(arena, AST__return_statement);
                    return_statement->expr = return_expression;

                    AST__expression *return_statement_wrapper_expression = allocate_struct(arena, AST__expression);
                    return_statement_wrapper_expression->tag = AST_EXPRESSION_RETURN;
                    return_statement_wrapper_expression->return_statement = return_statement;

                    expressions_last->value = return_statement_wrapper_expression;

                    continue;
                }
            }

            AST__variable_declaration *var_decl = parse_variable_declaration();
            if (var_decl) {
                t = eat_token(&lexer);
                if (t.type != TOKEN_SEMICOLON) {
                    lexer = saved;
                    printf("Expected ';' but found %.*s", PRINT_SPAN(t.span));
                    return NULL;
                }

                if (expressions_head == NULL && expressions_last == NULL) {
                    expressions_head = expressions_last = allocate_struct(arena, List<AST__expression *>);
                } else if (expressions_head != NULL && expressions_last != NULL) {
                    expressions_last->next = allocate_struct(arena, List<AST__expression *>);
                    expressions_last = expressions_last->next;
                } else {
                    INVALID_CODE_PATH();
                }

                AST__expression *expr = allocate_struct(arena, AST__expression);
                expr->tag = AST_EXPRESSION_VARIABLE_DECLARATION;
                expr->variable_declaration = var_decl;

                expressions_last->value = expr;

                continue;
            }

            break;
        }
    }

    Token close_brace = eat_token(&lexer);
    if (close_brace.type != TOKEN_CLOSE_BRACE) {
        fprintf(stderr, "Expected close brace '}'. Got '%.*s'.\n", PRINT_SPAN(close_brace.span));
        return false;
    }


    AST__block *result = allocate_struct(arena, AST__block);
    result->expressions = expressions_head;

    return result;
}


AST__function_declaration *Parser::parse_function_declaration() {
    Lexer saved = lexer;
    // <identifier> :: (<argument-list>) [-> <return-type>] <block>
    Token name = eat_token(&lexer);
    if (name.type != TOKEN_IDENTIFIER) {
        // @error: function declaration should start from identifier
        return nullptr;
    }

    Token double_colon = eat_token(&lexer);
    if (double_colon.type != TOKEN_DOUBLE_COLON) {
        // fprintf(stderr, "Expected double colon '::'. Got '%.*s'.\n", PRINT_SPAN(double_colon.span));
        return nullptr;
    }

    Token open_paren = eat_token(&lexer);
    if (open_paren.type != TOKEN_OPEN_PAREN) {
        // fprintf(stderr, "Expected open parenthesis '('. Got '%.*s'.\n", PRINT_SPAN(open_paren.span));
        return nullptr;
    }

    List<AST__variable_declaration *> *arguments_head = NULL;
    List<AST__variable_declaration *> *arguments_last = NULL;
    Token close_paren = get_token(&lexer);
    if (close_paren.type == TOKEN_CLOSE_PAREN) {
        eat_token(&lexer); // eat close paren
    } else {
        while (true) {
            saved = lexer;
            AST__variable_declaration *arg = parse_variable_declaration();
            if (arg) {
                if (arguments_head == NULL && arguments_last == NULL) {
                    arguments_head = arguments_last = allocate_struct(arena, List<AST__variable_declaration *>);
                } else if (arguments_head != NULL && arguments_last != NULL) {
                    arguments_last->next = allocate_struct(arena, List<AST__variable_declaration *>);
                    arguments_last = arguments_last->next;
                } else {
                    INVALID_CODE_PATH();
                }

                arguments_last->value = arg;
            }
        }

        close_paren = eat_token(&lexer);
        if (close_paren.type != TOKEN_CLOSE_PAREN) {
            // @error: closing parenthesis should close argument list
            lexer = saved;
            return nullptr;
        }
    }

    AST__type *return_type = nullptr;
    Token right_arrow = get_token(&lexer);
    if (right_arrow.type == TOKEN_RIGHT_ARROW) {
        eat_token(&lexer); // eat right arrow

        saved = lexer;
        return_type = parse_type();
        if (!return_type) {
            // @error: there's right arrow but can't parse return type
            lexer = saved; // Restore lexer state after failure.
            return nullptr;
        }
    }

    AST__block *body = NULL;
    {
        saved = lexer;

        body = parse_block();
        if (!body) {
            // @error: failed to parse block
            lexer = saved; // Restore lexer state after failure.
            return NULL;
        }
    }

    AST__function_declaration *declaration = allocate_struct(arena, AST__function_declaration);
    declaration->name = name.span;
    declaration->argument_list = arguments_head;
    declaration->return_type = return_type;
    declaration->body = body;

    return declaration;
}


// Ast_OperatorDeclaration *Parser::parse_operator_declaration() {
//     // operator <symbols> [prefix|infix|postfix] <integer> :: (<argument-list>) [-> <type>] { <statement-list> }

//     Token kw_operator = eat_token(&lexer);
//     if (kw_operator.type != TOKEN_KW_OPERATOR) {
//         return nullptr;
//     }

//     Token op = eat_token(&lexer);
//     if (op.type != '+' &&
//         op.type != '-' &&
//         op.type != '*' &&
//         op.type != '/' &&
//         op.type != '%' &&
//         op.type != '<' &&
//         op.type != '>' &&
//         op.type != '=' &&
//         op.type != '|' &&
//         op.type != '~' &&
//         op.type != '!' &&
//         op.type != TOKEN_USED_DEFINED_OPERATOR)
//     {
//         fprintf(stderr, "Expected sequence of symbols, allowed to form an operator (+-*/%%<>=|~!).\nGot '%.*s'.\n", PRINT_SPAN(op.span));
//         return nullptr;
//     }

//     Token afix = eat_token(&lexer);
//     if (afix.type != TOKEN_KW_PREFIX &&
//         afix.type != TOKEN_KW_INFIX &&
//         afix.type != TOKEN_KW_POSTFIX)
//     {
//         fprintf(stderr, "Expected keyword [prefix|infix|postfix]. Got '%.*s'.\n", PRINT_SPAN(afix.span));
//         return nullptr;
//     }

//     Token precedence = eat_token(&lexer);
//     if (precedence.type != TOKEN_INT_LITERAL)
//     {
//         fprintf(stderr, "Expected integer precedence. Got '%.*s'.\n", PRINT_SPAN(precedence.span));
//         return nullptr;
//     }

//     Token double_colon = eat_token(&lexer);
//     if (double_colon.type != TOKEN_DOUBLE_COLON)
//     {
//         fprintf(stderr, "Expected double colon '::'. Got '%.*s'.\n", PRINT_SPAN(double_colon.span));
//         return nullptr;
//     }

//     Token open_paren = eat_token(&lexer);
//     if (open_paren.type != TOKEN_OPEN_PAREN)
//     {
//         fprintf(stderr, "Expected open parenthesis '('. Got '%.*s'.\n", PRINT_SPAN(open_paren.span));
//         return nullptr;
//     }

//     Ast_SequenceEntry *arguments = nullptr;
//     Ast_SequenceEntry *function_body = nullptr;

//     Token close_paren = get_token(&lexer);
//     if (close_paren.type == TOKEN_CLOSE_PAREN) {
//         eat_token(&lexer);
//     } else {
//         Lexer saved = lexer;

//         bool32 success_parse = parse_argument_list(&arguments);
//         if (success_parse) {
//             // Argument list parsed
//         } else {
//             // @error: cannot parse argument list
//             lexer = saved; // Restore lexer state after failure.
//             return nullptr;
//         }

//         close_paren = eat_token(&lexer);
//         if (close_paren.type != TOKEN_CLOSE_PAREN) {
//             // @error: closing parenthesis should close argument list
//             return nullptr;
//         }
//     }

//     Token right_arrow = get_token(&lexer);
//     if (right_arrow.type == TOKEN_RIGHT_ARROW) {
//         eat_token(&lexer); // eat right arrow

//         Lexer saved = lexer;

//         Ast_Type *type = parse_type();
//         if (!type) {
//             // @error: there's right arrow but can't parse return type
//             lexer = saved; // Restore lexer state after failure.
//             return nullptr;
//         }
//     }

//     {
//         Lexer saved = lexer;

//         bool32 success_parse = parse_block(&function_body);
//         if (!success_parse) {
//             // @error: failed to parse block
//             lexer = saved; // Restore lexer state after failure.
//             return nullptr;
//         }
//     }

//     auto declaration = push_struct(arena, Ast_OperatorDeclaration);
//     declaration->tag = AST_OPERATOR_DECLARATION;
//     declaration->operator_name = op;
//     declaration->type = OPERATOR_INFIX;
//     declaration->precedence = (int)precedence.integer;
//     declaration->argument_list = arguments;
//     declaration->entry_list = function_body;

//     return declaration;
// }


} // namespace rei
