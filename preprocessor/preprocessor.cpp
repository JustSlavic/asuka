#include <defines.hpp>
#include <math.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>


namespace preprocessor {

struct string {
    char*  memory;
    uint32 size;
};


enum token_type {
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
    TOKEN_PLUS = '+',
    TOKEN_MINUS = '-',
    TOKEN_SLASH = '/',
    TOKEN_DOT = '.',
    TOKEN_COMMA = ',',
    TOKEN_HASH = '#',

    TOKEN_ID = 256,
    TOKEN_KEYWORD,
    TOKEN_CHAR,
    TOKEN_STRING,

    TOKEN_RIGHT_ARROW,

    TOKEN_KW_RETURN = 300,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_WHILE,
    TOKEN_KW_FOR,
    TOKEN_KW_DO,
    TOKEN_KW_BREAK,
    TOKEN_KW_CONTINUE,
    TOKEN_KW_STRUCT,
    TOKEN_KW_ENUM,

    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,

    TOKEN_EOF,
};


inline
char *to_string(token_type type) {
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
        case TOKEN_DOT: return "DOT";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_HASH: return "HASH";

        case TOKEN_ID: return "ID";
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_STRING: return "STRING";

        case TOKEN_RIGHT_ARROW: return "RIGHT_ARROW";

        case TOKEN_KW_RETURN: return "KW_RETURN";
        case TOKEN_KW_IF: return "KW_IF";
        case TOKEN_KW_ELSE: return "KW_ELSE";
        case TOKEN_KW_WHILE: return "KW_WHILE";
        case TOKEN_KW_FOR: return "KW_FOR";
        case TOKEN_KW_DO: return "KW_DO";
        case TOKEN_KW_BREAK: return "KW_BREAK";
        case TOKEN_KW_CONTINUE: return "KW_CONTINUE";
        case TOKEN_KW_STRUCT: return "KW_STRUCT";
        case TOKEN_KW_ENUM: return "KW_ENUM";

        case TOKEN_KW_TRUE: return "KW_TRUE";
        case TOKEN_KW_FALSE: return "KW_FALSE";

        case TOKEN_EOF: return "EOF";
    }

    return "INVALID";
}


struct token {
    token_type type;

    int32 line;
    int32 column;

    string span;
};


struct lexer {
    string buffer;
    size_t current;

    uint32 line;
    uint32 column;

    token next_token;
    bool32 next_token_ready;
};


inline
char get_char(lexer *lex) {
    char c = 0;

    if (lex->current < lex->buffer.size) {
        c = lex->buffer.memory[lex->current];
    }

    return c;
}


inline
char eat_char(lexer *lex) {
    char c = get_char(lex);
    lex->current++;
    lex->column++;
    return c;
}


inline
void eat_chars(lexer *lex, uint32 count) {
    ASSERT(lex->current + count < lex->buffer.size);

    lex->current += count;
    lex->column += count;
}


inline
bool32 is_substring(char *str1, char *str2, size_t size) {
    while (*str1 != 0 && *str2 != 0 && size > 0) {
        if (*str1 != *str2) return false;

        str1++;
        str2++;
        size--;
    }

    return true;
}


inline
bool32 on_substring(lexer *lex, string str) {
    ASSERT(lex->current + str.size < lex->buffer.size);

    bool32 result = is_substring(lex->buffer.memory + lex->current, str.memory, str.size);
    return result;
}



static
int32 compare(string str1, string str2) {
    int32 result = 0;
    if (str1.size != str2.size) {
        result = math::sign((int32)str1.size - (int32)str2.size);
        return result;
    }

    for (uint32 idx = 0; idx < str1.size; idx++) {
        char c1 = str1.memory[idx];
        char c2 = str2.memory[idx];
        if (c1 != c2) {
            result = math::sign((int32)c1 - (int32)c2);
            return result;
        }
    }

    return result;
}


static
size_t get_filesize(char *filename) {
    size_t filesize = 0;

    struct stat st;
    if (stat(filename, &st) == 0) {
        filesize = st.st_size;
    }

    return filesize;
}


static
string load_entire_file(char *filename) {
    string result {};

    size_t filesize = get_filesize(filename);
    if (filesize > 0) {
        FILE* f = fopen(filename, "rb");
        if (f) {
            result.memory = (char *)calloc(filesize, sizeof(char));
            result.size = (uint32)fread(result.memory, sizeof(char), filesize, f);
        }
    }

    return result;
}


inline
int32 c_string_size(char *str) {
    ASSERT(str);

    int32 size = 0;
    while (*str++) {
        size += 1;
    }

    return size;
}


inline
bool32 is_newline(char c) {
    bool32 result = (c == '\r') || (c == '\n');
    return result;
}

inline
bool32 is_space(char c) {
    bool32 result = (c == ' ') || (c == '\t') || is_newline(c);
    return result;
}


inline
bool32 is_alphabetical(char c) {
    bool32 result = ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'));
    return result;
}


inline
bool32 is_digit(char c) {
    bool32 result = (c >= '0') && (c <= '9');
    return result;
}


inline
bool32 is_valid_identifier_head(char c) {
    bool32 result = (c == '_') || is_alphabetical(c);
    return result;
}


inline
bool32 is_valid_identifier_body(char c) {
    bool32 result = (c == '_') || is_alphabetical(c) || is_digit(c);
    return result;
}


inline
void consume_whitespaces(lexer *lex) {
    char c = get_char(lex);
    while (is_space(c)) {
        eat_char(lex);
        char next_c = get_char(lex);

        if (is_newline(c) && !is_newline(next_c)) {
            lex->line += 1;
            lex->column = 1;
        }

        c = next_c;
    }
}


static
token get_token(lexer *lex) {
    if (!lex->next_token_ready) {
        token result {};

        consume_whitespaces(lex);

        char c = get_char(lex);
        if (c == 0) {
            result.type = TOKEN_EOF;
            result.line = lex->line;
            result.column = lex->column;
        } else if (is_valid_identifier_head(c)) {
            result.type = TOKEN_ID;
            result.line = lex->line;
            result.column = lex->column;
            result.span.memory = lex->buffer.memory + lex->current;
            result.span.size = 0;

            while (is_valid_identifier_body(c)) {
                eat_char(lex);
                result.span.size += 1;
                c = get_char(lex);
            }
        } else {
            result.type = (token_type)c;
            result.line = lex->line;
            result.column = lex->column;
            result.span.memory = lex->buffer.memory + lex->current;
            result.span.size = 1;
            eat_char(lex);
        }

        lex->next_token = result;
        lex->next_token_ready = true;
    }

    return lex->next_token;
}


static token eat_token(lexer *lex) {
    token result {};

    if (!lex->next_token_ready) {
        result = get_token(lex);
    }

    lex->next_token_ready = false;
    return result;
}


static
bool32 parse_introspect(lexer *lex) {
    string introspect_keyword;
    introspect_keyword.memory = "introspect";
    introspect_keyword.size = c_string_size(introspect_keyword.memory);

    token t = get_token(lex);
    if (t.type == TOKEN_ID) {
        if (compare(t.span, introspect_keyword) == 0) {
            eat_token(lex);
            token open_paren = eat_token(lex);
            token property = eat_token(lex);
            token equals = eat_token(lex);
            token value = eat_token(lex);
            token close_paren = eat_token(lex);

            printf("                introspect (property=\"%.*s\"; value=\"%.*s\")\n",
                (int)property.span.size, property.span.memory,
                (int)value.span.size, value.span.memory);

            return true;
        }
    } else {
        // error
        return false;
    }

    return false;
}


static
void parse(char *filename) {
    lexer lex = {};
    lex.buffer = load_entire_file(filename);

    if (lex.buffer.size > 0) {
        token t {};
        do {
            bool32 success = parse_introspect(&lex);
            if (success) {
                printf("SUCCESS!!!\n");
            } else {
                t = get_token(&lex);
                if (t.type == TOKEN_EOF) {
                    printf("EOF\n");
                } else if (t.type == TOKEN_ID) {
                    printf("ID (%d:%d) %.*s\n", t.line, t.column, (int)t.span.size, t.span.memory);
                } else {
                    printf("%s('%c')\n", to_string(t.type), t.type);
                }
                eat_token(&lex);
            }
        } while (t.type != TOKEN_EOF && t.type != TOKEN_INVALID);
    }
}


} // preprocessor


int main() {
    char* filename = "example.cpp";

    preprocessor::string category_keyword;
    category_keyword.memory = "category";
    category_keyword.size = preprocessor::c_string_size(category_keyword.memory);

    preprocessor::parse(filename);

    return 0;
}
