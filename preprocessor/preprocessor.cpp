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

    TOKEN_ID = 256,
    TOKEN_KEYWORD,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_COMMENT,

    // TOKEN_LEFT_ARROW,  // <-
    TOKEN_RIGHT_ARROW, // ->
    TOKEN_DOUBLE_AMPERSAND, // &&
    TOKEN_DOUBLE_VERTICAL_LINE, // ||

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
        case TOKEN_BACKSLASH: return "BACKSLASH";
        case TOKEN_DOT: return "DOT";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_HASH: return "HASH";

        case TOKEN_AMPERSAND: return "AMPERSAND";
        case TOKEN_VERTICAL_LINE: return "VERTICAL_LINE";
        case TOKEN_CARET: return "CARET";
        case TOKEN_TILDA: return "TILDA";

        case TOKEN_ID: return "ID";
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_CHAR: return "CHAR";
        case TOKEN_STRING: return "STRING";
        case TOKEN_COMMENT: return "COMMENT";

        case TOKEN_RIGHT_ARROW: return "RIGHT_ARROW";
        case TOKEN_BANG: return "BANG";
        case TOKEN_DOUBLE_AMPERSAND: return "AND";
        case TOKEN_DOUBLE_VERTICAL_LINE: return "OR";

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
    uint32 current;

    uint32 line;
    uint32 column;

    token  next_token;
    bool32 next_token_ready;

    token_type keyword_to_token_type[64];
    string keywords[64];
    uint32 keyword_count;
};


struct attribute {
    string name;
    string category;
};


inline
void add_keyword(lexer *lex, string kw, token_type type) {
    ASSERT(keyword_count < ARRAY_COUNT(lex->keywords))
    lex->keywords[lex->keyword_count] = kw;
    lex->keyword_to_token_type[lex->keyword_count] = type;

    lex->keyword_count += 1;
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
    char next_c = get_char(lex);

    if (c == '\n') {
        lex->line += 1;
        lex->column = 0;
    }

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
void consume_whitespaces(lexer *lex) {
    char c = get_char(lex);
    while (is_space(c)) {
        eat_char(lex);
        char next_c = get_char(lex);
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

            for (uint32 keyword_index = 0; keyword_index < lex->keyword_count; keyword_index++) {
                if (compare(lex->keywords[keyword_index], result.span) == 0) {
                    result.type = lex->keyword_to_token_type[keyword_index];
                }
            }
        } else if (c == '"') {
            result.type = TOKEN_STRING;
            result.line = lex->line;
            result.column = lex->column;
            result.span.memory = lex->buffer.memory + lex->current;
            result.span.size = lex->current;

            eat_char(lex);
            while ((c = get_char(lex)) > 0) {
                if (c == '\\') {
                    eat_char(lex); // eat backslash
                    eat_char(lex); // next character
                } else if (c == '"') {
                    eat_char(lex);
                    break;
                } else if (is_newline(c)) {
                    // error
                    // !!!
                    fprintf(stderr, "Error: newline inside string\n");
                    return {};
                } else {
                    eat_char(lex);
                }
            }

            if (c == 0) {
                // error
                // !!!
                fprintf(stderr, "Error: unexpected end of buffer\n");
                return {};
            }

            result.span.size = lex->current - result.span.size;

            // @todo: allocate new buffer for strings to copy with quotes removed and escape sequences replaced
        } else if (c == '-') {
            result.line = lex->line;
            result.column = lex->column;
            result.span.memory = lex->buffer.memory + lex->current;
            result.span.size = lex->current;

            eat_char(lex);
            c = get_char(lex);

            if (c == '>') {
                eat_char(lex);
                result.type = TOKEN_RIGHT_ARROW;
            } else {
                result.type = TOKEN_MINUS;
            }

            result.span.size = lex->current - result.span.size;
        } else if (c == '/') {
            result.line = lex->line;
            result.column = lex->column;
            result.span.memory = lex->buffer.memory + lex->current;
            result.span.size = lex->current;

            eat_char(lex);
            c = get_char(lex);
            if (c == '/') {
                result.type = TOKEN_COMMENT;

                while (!is_newline(c = get_char(lex))) {
                    eat_char(lex);
                }
            } else if (c == '*') {
                result.type = TOKEN_COMMENT;

                while (!(c == '*' && get_char(lex) == '/')) {
                    c = eat_char(lex);
                }

                eat_char(lex); // eat '/'

            } else {
                // This is just a slash (division or something)
                result.type = TOKEN_SLASH;
            }

            result.span.size = lex->current - result.span.size;
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
bool32 parse_preprocessor_directive(lexer *lex) {
    string kw_include;
    kw_include.memory = "include";
    kw_include.size = c_string_size(kw_include.memory);

    string kw_define;
    kw_define.memory = "define";
    kw_define.size = c_string_size(kw_define.memory);

    token pound = get_token(lex);
    if (pound.type == TOKEN_HASH) {
        eat_token(lex);
        token directive = eat_token(lex);
        if (directive.type == TOKEN_ID) {
            if (compare(directive.span, kw_include) == 0) {
                token open_angle_or_string = eat_token(lex);
                if (open_angle_or_string.type == TOKEN_STRING) {
                    printf("import (header=%.*s)\n",
                        (int)open_angle_or_string.span.size, open_angle_or_string.span.memory);
                } else if (open_angle_or_string.type == TOKEN_OPEN_ANGLE) {
                    string include_path {};
                    include_path.memory = lex->buffer.memory + lex->current;
                    include_path.size = lex->current;

                    token t = get_token(lex);
                    while (t.type != TOKEN_CLOSE_ANGLE) {
                        if (t.type == TOKEN_SLASH ||
                            t.type == TOKEN_BACKSLASH ||
                            t.type == TOKEN_ID ||
                            t.type == TOKEN_DOT)
                        {
                            eat_token(lex);
                            t = get_token(lex);
                        }
                        else
                        {
                            // Error!
                            fprintf(stderr, "Error: unrecognized token while tokenizing include directive\n"
                                "%s:%d:%d %.*s\n", "filename", t.line, t.column, (int)t.span.size, t.span.memory);
                        }
                    }

                    // (- 1) because at that point we got '>' but we don't want it in the include_path
                    include_path.size = lex->current - include_path.size - 1;
                    // Eat '>'
                    eat_token(lex);

                    printf("import (header=\"%.*s\")\n",
                        (int)include_path.size, include_path.memory);
                } else {
                    // Error!
                    return false;
                }
            // } else if (compare(directive.span, kw_define) == 0) {
            //     token macro_name = eat_token(lex);
            //     token open_paren = get_token(lex);
            //     if (open_paren.type == TOKEN_OPEN_PAREN) {
            //         eat_token(lex); // (
            //         eat_token(lex); // CATEGORY
            //         eat_token(lex); // )
            //     }
            //     printf("define (macro=\"%.*s\")\n",
            //         (int)macro_name.span.size, macro_name.span.memory);
            //     return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }

    return true;
}


static
bool32 parse_introspect(lexer *lex, attribute *attr) {
    string kw_introspect;
    kw_introspect.memory = "introspect";
    kw_introspect.size = c_string_size(kw_introspect.memory);

    token t = get_token(lex);
    if (t.type == TOKEN_ID) {
        if (compare(t.span, kw_introspect) == 0) {
            eat_token(lex);
            token open_paren = eat_token(lex);
            token property = eat_token(lex);
            token equals = eat_token(lex);
            token value = eat_token(lex);
            token close_paren = eat_token(lex);

            printf("introspect (property=\"%.*s\"; value=\"%.*s\")\n",
                (int)property.span.size, property.span.memory,
                (int)value.span.size, value.span.memory);

            if (attr) {
                attr->name = kw_introspect;
                attr->category = value.span;
            }

            return true;
        }
    } else {
        // error
        return false;
    }

    return false;
}


static
bool32 parse_struct_members(lexer *lex) {
    token t = get_token(lex);

    if (t.type == TOKEN_ID) {
        eat_token(lex);
        token var = eat_token(lex);
        token punctuation = eat_token(lex);

        printf("    TypeInfo_%.*s %.*s;\n",
                (int)t.span.size, t.span.memory,
                (int)var.span.size, var.span.memory);

        while (punctuation.type == TOKEN_COMMA) {
            var = eat_token(lex);
            punctuation = eat_token(lex);

            printf("    TypeInfo_%.*s %.*s;\n",
                (int)t.span.size, t.span.memory,
                (int)var.span.size, var.span.memory);
        }

        if (punctuation.type != TOKEN_SEMICOLON) {
            return false;
        }
    } else {
        return false;
    }

    return true;
}


static
bool32 parse_struct(lexer *lex, attribute *attr = NULL) {
    token t = get_token(lex);

    if (t.type == TOKEN_KW_STRUCT) {
        eat_token(lex); // struct
        token id = eat_token(lex);
        if (id.type == TOKEN_ID) {
            eat_token(lex); // {
        }

        if (attr) {
            printf(
                "struct TypeInfo_%.*s {\n",
                (int)id.span.size, id.span.memory);
        }

        t = get_token(lex);
        while(true) {
            if (!parse_struct_members(lex)) {
                break;
            }
        }

        eat_token(lex); // }
        eat_token(lex); // ;

        if (attr) {
            printf("};\n");
        }
    } else {
        return false;
    }

    return true;
}


static
void parse(string source_code) {
    lexer lex_ = {};
    lex_.buffer = source_code;

    string kw_struct;
    kw_struct.memory = "struct";
    kw_struct.size = c_string_size(kw_struct.memory);

    lexer *lex = &lex_;
    add_keyword(lex, kw_struct, TOKEN_KW_STRUCT);

    if (lex->buffer.size > 0) {
        token t = get_token(lex);
        attribute attr {};
        do {
            if (parse_preprocessor_directive(lex)) {
            } else if (parse_introspect(lex, &attr)) {
                parse_struct(lex, &attr);
            } else {
                t = get_token(lex);
                if (t.type == TOKEN_EOF) {
                    printf("EOF\n");
                } else if (t.type == TOKEN_ID) {
                    printf("ID (%d:%d) %.*s\n", t.line, t.column, (int)t.span.size, t.span.memory);
                } else if (t.type == TOKEN_STRING) {
                    printf("STRING (%d:%d) %.*s\n", t.line, t.column, (int)t.span.size, t.span.memory);
                } else if (t.type == TOKEN_COMMENT) {
                    printf("COMMENT (%d:%d) %.*s\n", t.line, t.column, (int)t.span.size, t.span.memory);
                } else if (t.type == TOKEN_RIGHT_ARROW) {
                    printf("RIGHT_ARROW (%d:%d)\n", t.line, t.column);
                } else {
                    printf("%s (%d:%d) ('%c')\n", to_string(t.type), t.line, t.column, t.type);
                }
                eat_token(lex);
            }
        } while (t.type != TOKEN_EOF && t.type != TOKEN_INVALID);
    }
}


} // preprocessor


int main() {
    // char* filename = "../src/win32_main.cpp";
    char *filename = "example.cpp";

    preprocessor::string source_code = preprocessor::load_entire_file(filename);
    preprocessor::parse(source_code);

    return 0;
}
