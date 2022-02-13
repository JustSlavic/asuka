#pragma once

#include <defines.hpp>
#include <string.hpp>

#define PRINT_SPAN(SPAN) (int)SPAN.size, SPAN.data


namespace rei {


enum TokenType {
    TOKEN_INVALID = 0,

    TOKEN_OPEN_PAREN = '(',
    TOKEN_CLOSE_PAREN = ')',

    TOKEN_OPEN_BRACE = '{',
    TOKEN_CLOSE_BRACE = '}',

    TOKEN_OPEN_BRACKET = '[',
    TOKEN_CLOSE_BRACKET = ']',

    TOKEN_OPEN_ANGLE = '<',
    TOKEN_CLOSE_ANGLE = '>',

    TOKEN_EQUALS = '=',
    TOKEN_COLON = ':',
    TOKEN_SEMICOLON = ';',
    TOKEN_ASTERICKS = '*',
    TOKEN_AMPERSAND = '&',
    TOKEN_VERTICAL_LINE = '|',
    TOKEN_CARET = '^',
    TOKEN_TILDA = '~',
    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_SLASH = '/',
    TOKEN_BACKSLASH = '\\',
    TOKEN_DOT = '.',
    TOKEN_COMMA = ',',
    TOKEN_HASH = '#',
    TOKEN_BANG = '!',

    TOKEN_IDENTIFIER = 256,
    // TOKEN_KEYWORD,
    // TOKEN_CHAR,
    TOKEN_INT_LITERAL,
    TOKEN_FLOAT_LITERAL,
    TOKEN_STRING_LITERAL,
    // TOKEN_COMMENT,

    TOKEN_DOUBLE_COLON, // ::
    // TOKEN_LEFT_ARROW,  // <-
    TOKEN_RIGHT_ARROW, // ->
    // TOKEN_DOUBLE_AMPERSAND, // &&
    // TOKEN_DOUBLE_VERTICAL_LINE, // ||

    TOKEN_USED_DEFINED_OPERATOR,

    TOKEN_KW_RETURN = 300,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_WHILE,
    // TOKEN_KW_FOR,
    // TOKEN_KW_BREAK,
    // TOKEN_KW_CONTINUE,
    // TOKEN_KW_STRUCT,
    // TOKEN_KW_ENUM,
    TOKEN_KW_OPERATOR,
    TOKEN_KW_INFIX,
    TOKEN_KW_PREFIX,
    TOKEN_KW_POSTFIX,

    // TOKEN_KW_TRUE,
    // TOKEN_KW_FALSE,

    TOKEN_EOF,
};


inline
char *to_string(enum TokenType type) {
    switch (type) {
        case TOKEN_OPEN_PAREN: return "OPEN_PAREN";
        case TOKEN_CLOSE_PAREN: return "CLOSE_PAREN";

        case TOKEN_OPEN_BRACE: return "OPEN_BRACE";
        case TOKEN_CLOSE_BRACE: return "CLOSE_BRACE";

        case TOKEN_OPEN_BRACKET: return "OPEN_BRACKET";
        case TOKEN_CLOSE_BRACKET: return "CLOSE_BRACKET";

        case TOKEN_OPEN_ANGLE: return "OPEN_ANGLE";
        case TOKEN_CLOSE_ANGLE: return "CLOSE_ANGLE";

        case TOKEN_EQUALS: return "EQUALS";
        case TOKEN_COLON: return "COLON";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_ASTERICKS: return "ASTERICKS";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_BACKSLASH: return "BACKSLASH";
        case TOKEN_DOT: return "DOT";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_HASH: return "HASH";

        case TOKEN_AMPERSAND: return "AMPERSAND";
        case TOKEN_VERTICAL_LINE: return "VERTICAL_LINE";
        case TOKEN_CARET: return "CARET";
        case TOKEN_TILDA: return "TILDA";

        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        // case TOKEN_KEYWORD: return "KEYWORD";
        // case TOKEN_CHAR: return "CHAR";
        case TOKEN_INT_LITERAL: return "INT";
        case TOKEN_FLOAT_LITERAL: return "FLOAT";
        case TOKEN_STRING_LITERAL: return "STRING";
        // case TOKEN_COMMENT: return "COMMENT";

        case TOKEN_DOUBLE_COLON: return "DOUBLE_COLON";
        case TOKEN_RIGHT_ARROW: return "RIGHT_ARROW";
        // case TOKEN_BANG: return "BANG";
        // case TOKEN_DOUBLE_AMPERSAND: return "AND";
        // case TOKEN_DOUBLE_VERTICAL_LINE: return "OR";

        case TOKEN_KW_RETURN: return "KW_RETURN";
        case TOKEN_KW_IF: return "KW_IF";
        case TOKEN_KW_ELSE: return "KW_ELSE";
        case TOKEN_KW_WHILE: return "KW_WHILE";
        // case TOKEN_KW_FOR: return "KW_FOR";
        // case TOKEN_KW_DO: return "KW_DO";
        // case TOKEN_KW_BREAK: return "KW_BREAK";
        // case TOKEN_KW_CONTINUE: return "KW_CONTINUE";
        // case TOKEN_KW_STRUCT: return "KW_STRUCT";
        // case TOKEN_KW_ENUM: return "KW_ENUM";
        case TOKEN_KW_OPERATOR: return "KW_OPERATOR";
        case TOKEN_KW_INFIX: return "KW_INFIX";
        case TOKEN_KW_PREFIX: return "KW_PREFIX";
        case TOKEN_KW_POSTFIX: return "KW_POSTFIX";

        // case TOKEN_KW_TRUE: return "KW_TRUE";
        // case TOKEN_KW_FALSE: return "KW_FALSE";

        case TOKEN_EOF: return "EOF";
    }

    return "INVALID";
}

TokenType keyword_to_token_type() {
    char *keywords[] = {
        "return",
        "if",
        "else",
        "while",
        "operator",
        "infix",
        "prefix",
        "postfix",
    };

    TokenType token_types[] = {
        TOKEN_KW_RETURN,
        TOKEN_KW_IF,
        TOKEN_KW_ELSE,
        TOKEN_KW_WHILE,
        TOKEN_KW_OPERATOR,
        TOKEN_KW_INFIX,
        TOKEN_KW_PREFIX,
        TOKEN_KW_POSTFIX,
    };

    return token_types[0];
}

struct Token {
    TokenType type;

    i32 line;
    i32 column;

    union {
        i64 integer;
        f64 floating;
    };

    string span;
};


} // namespace rei
