#pragma once

#include <os.hpp>

/*

    ASUKA SERIALIZED OBJECT NOTATION

    Features:
        - comment until the eol with //;
        - keys are identifiers, not strings;
        - key-value pairs use equal sign as separator;
        - key-value paris separated by semicolon;
        - values in lists separated by comma;
        - trailing comma;
        - top-level braces and brackets are optional;
        - semicolons are optional;
        - commas are optional;

    @todo:
        - you can register your own types and constructors for these types:

            // macro or template?
            son::register_constructor<math::vector2>("vector2", math::v2::from);
            son::register_constructor<math::vector2>("v2", math::v2::from);

            son::register_constructor<math::color24>("color24", math::color32::from_rgb);
            son::register_constructor<math::color24>("rgb", math::color32::from_rgb);
            son::register_constructor<math::color24>("hsv", math::color32::from_hsv);

            son::deserialize_from_file("example.son");

            {
                size = v2(10, 10); // should produce vector2
                color = hsv(100, 100, 24); // should produce color24
            }

        - skippable suffixes for readability!

            {
                width = 10px    // px is skipped!
                height = 10px   // px is skipped!
                duration = 10s  //  s is skipped!
            }
*/


namespace asuka
{


struct son
{
    enum class type_t {
        null = 0,
        boolean,
        integer,
        floating,
        string,
        object,
        array,
        // @todo Custom value type.
    };

private:
    using boolean_t = bool;
    using integer_t = i64;
    using floating_t = f64;
    using string_t = asuka::string;
    // using object_t = std::vector<std::pair<std::string, son>>;
    // using array_t = std::vector<son>;

    type_t m_type;
    union {
        boolean_t  boolean_;
        integer_t  integer_;
        floating_t floating_;
        string_t   string_;
    } m_value;

public:
    ~son();

    son(); // Contructor for null object.
    son(std::nullptr_t); // Also null object.
    son(type_t t); // Default value of that type.
    son(boolean_t v);
    son(integer_t v);
    son(int v) : son(static_cast<integer_t>(v)) {}
    son(floating_t v);
    // son(const char* s);
    // son(string_t s);
    // son(std::initializer_list<son>);

    void swap(son& other);

    son(const son& other);
    son(son&& other);

    son& operator=(const son& other);
    son& operator=(son&& other);

    son &operator=(bool value);

    type_t get_type() const { return m_type; }

    bool is_null()     const { return m_type == type_t::null; }
    bool is_boolean()  const { return m_type == type_t::boolean; }
    bool is_integer()  const { return m_type == type_t::integer; }
    bool is_floating() const { return m_type == type_t::floating; }
    bool is_string()   const { return m_type == type_t::string; }
    bool is_object()   const { return m_type == type_t::object; }
    bool is_array()    const { return m_type == type_t::array; }

    bool get_boolean() const { ASSERT(is_boolean()); return m_value.boolean_; }
    integer_t get_integer() const { ASSERT(is_integer()); return m_value.integer_; }
    floating_t get_floating() const { ASSERT(is_floating()); return m_value.floating_; }
    // string_t get_string() const { ASSERT(is_string()); return *(string_t*)m_value.storage_; }
};


son::~son() {}
son::son() {}
son::son(std::nullptr_t) {}

son::son(son::boolean_t value)
{
    m_type = type_t::boolean;
    m_value.boolean_ = value;
}

son::son(son::integer_t value)
{
    m_type = type_t::integer;
    m_value.integer_ = value;
}

son::son(son::floating_t value)
{
    m_type = type_t::floating;
    m_value.floating_ = value;
}

void son::swap(son& other)
{
    {
        auto tmp_type = m_type;
        m_type = other.m_type;
        other.m_type = tmp_type;
    }

    {
        auto value = m_value;
        m_value = other.m_value;
        other.m_value = value;
    }
}


son::son(const son& other)
{
    m_type = other.m_type;

    switch (m_type) {
        case type_t::null:
        break;

        case type_t::boolean:
        {
            m_value.boolean_ = other.m_value.boolean_;
        }
        break;

        case type_t::integer:
        {
            m_value.integer_ = other.m_value.integer_;
        }
        break;

        case type_t::floating:
        {
            m_value.floating_ = other.m_value.floating_;
        }
        break;

        case type_t::string:
        {
            // string_t* copy = new string_t(*(string_t*)other.m_value.storage);
            // m_value.storage = copy;
        }
        break;

        case type_t::object:
        {
            // object_t* copy = new object_t(*(object_t*)other.m_value.storage);
            // m_value.storage = copy;
        }
        break;

        case type_t::array:
        {
            // array_t* copy = new array_t(*(array_t*)other.m_value.storage);
            // m_value.storage = copy;
        }
        break;
    }
}


son::son(son&& other) {
    this->swap(other);
}


son& son::operator=(const son& other) {
    son(other).swap(*this);
    return *this;
}


son& son::operator=(son&& other) {
    other.swap(*this);
    return *this;
}


son &son::operator=(bool value)
{
    *this = son(value);
    return *this;
}


namespace internal
{


static usize       constructor_count;
static void       *constructor_callbacks[32];
static char const *constructor_names[32];


bool is_whitespace(char c) { return c == ' '; }
bool is_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool is_newline(char c) { return c == '\n'; }
bool is_alpha(char c) { return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; }
bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_valid_identifier_head(char c) { return c == '_' || is_alpha(c); }
bool is_valid_identifier_body(char c) { return c == '_' || is_alpha(c) || is_digit(c); }


enum TokenType
{
    TOKEN_INVALID = 0,

    TOKEN_EQUALS = '=',
    TOKEN_SEMICOLON = ';',
    TOKEN_COMMA = ',',

    TOKEN_BRACE_OPEN = '{',
    TOKEN_BRACE_CLOSE = '}',

    TOKEN_PAREN_OPEN = '(',
    TOKEN_PAREN_CLOSE = ')',

    TOKEN_BRACKET_OPEN = '[',
    TOKEN_BRACKET_CLOSE = ']',

    TOKEN_KW_NULL = 256,
    TOKEN_KW_TRUE,
    TOKEN_KW_FALSE,

    TOKEN_IDENTIFIER,
    TOKEN_INTEGER,
    TOKEN_FLOATING,
    TOKEN_STRING,

    TOKEN_DOUBLE_SLASH,

    TOKEN_EOF,
};


struct Token
{
    TokenType type;

    i32 line;
    i32 column;

    union
    {
        i64 integer;
        f64 floating;
    };

    asuka::string span;
};


struct Lexer
{
    string filename;
    string buffer;
    u64 index;

    i32 line;
    i32 column;

    Token next_token;
    bool  next_token_valid;
};


void
print_token(Lexer *lexer, Token t)
{
    printf("%.*s:%d:%d ", PRINT_SPAN(lexer->filename), t.line, t.column);
    switch (t.type)
    {
        case TOKEN_INVALID: printf("INVALID"); break;
        case TOKEN_EQUALS: printf("EQUAL_SIGN"); break;
        case TOKEN_SEMICOLON: printf("SEMICOLON"); break;
        case TOKEN_COMMA: printf("COMMA"); break;
        case TOKEN_BRACE_OPEN: printf("BRACE_OPEN"); break;
        case TOKEN_BRACE_CLOSE: printf("BRACE_CLOSE"); break;
        case TOKEN_PAREN_OPEN: printf("PAREN_OPEN"); break;
        case TOKEN_PAREN_CLOSE: printf("PAREN_CLOSE"); break;
        case TOKEN_BRACKET_OPEN: printf("BRACKET_OPEN"); break;
        case TOKEN_BRACKET_CLOSE: printf("BRACKET_CLOSE"); break;
        case TOKEN_KW_NULL: printf("KW_NULL"); break;
        case TOKEN_KW_TRUE: printf("KW_TRUE"); break;
        case TOKEN_KW_FALSE: printf("KW_FALSE"); break;
        case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;
        case TOKEN_INTEGER: printf("INTEGER"); break;
        case TOKEN_FLOATING: printf("FLOATING"); break;
        case TOKEN_STRING: printf("STRING \"%.*s\"", PRINT_SPAN(t.span)); break;
        case TOKEN_DOUBLE_SLASH: printf("DOUBLE_SLASH"); break;
        case TOKEN_EOF: printf("EOF"); break;
    }

    printf(";\n");
}


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


INLINE
TokenType get_identifier_type(string id) {
    if (equals_to_cstr(id, "true")) {
        return TOKEN_KW_TRUE;
    }
    if (equals_to_cstr(id, "false")) {
        return TOKEN_KW_FALSE;
    }
    if (equals_to_cstr(id, "null")) {
        return TOKEN_KW_NULL;
    }

    return TOKEN_IDENTIFIER;
}


Token get_token(Lexer *lexer) {
    if (!lexer->next_token_valid) {
        Token t{};

        consume_whitespaces(lexer);

        char c = get_char(lexer);
        if (c == 0)
        {
            t.type = TOKEN_EOF;
            t.line = lexer->line;
            t.column = lexer->column;
        }
        else if (is_valid_identifier_head(c))
        {
            t.type = TOKEN_IDENTIFIER;
            t.line = lexer->line;
            t.column = lexer->column;
            t.span.data = get_char_pointer(lexer);

            i32 n = 0;
            while(is_valid_identifier_body(c))
            {
                eat_char(lexer);
                c = get_char(lexer);
                n += 1;
            }

            t.span.size = n;
            t.type = get_identifier_type(t.span);
        }
        else if (is_digit(c))
        {
            t.type = TOKEN_INTEGER;
            t.line = lexer->line;
            t.column = lexer->column;
            t.span.data = get_char_pointer(lexer);

            i64 integer = 0;

            c = get_char(lexer);
            while (is_digit(c))
            {
                integer *= 10;
                integer += (c - '0');

                eat_char(lexer);
                c = get_char(lexer);
                t.span.size += 1;
            }

            t.integer = integer;
        }
        else if (c == '"')
        {
            t.type = TOKEN_STRING;
            t.line = lexer->line;
            t.column = lexer->column;

            eat_char(lexer);

            t.span.data = get_char_pointer(lexer);
            t.span.size = 0;

            while (true) {
                c = eat_char(lexer);
                if (c == '"') break;
                t.span.size += 1;
            }
        }
        else
        {
            t.type = (TokenType) c;
            t.line = lexer->line;
            t.column = lexer->column;

            t.span.data = get_char_pointer(lexer);
            t.span.size = 1;

            eat_char(lexer);
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


struct Parser
{
    Lexer *lexer;
    memory::arena_allocator *arena;

    b32 parse_kv_pair(string *key, son *value);
    son parse_object(b32 top_level);
    b32 parse_son(son *value, b32 top_level);
};


b32 Parser::parse_son(son *value, b32 top_level)
{
    Token t = get_token(lexer);
    switch (t.type)
    {
        case TOKEN_KW_NULL:
        {
            eat_token(lexer);
            *value = son();
        }
        break;

        case TOKEN_KW_TRUE:
        {
            eat_token(lexer);
            *value = true;
        }
        break;

        case TOKEN_KW_FALSE:
        {
            eat_token(lexer);
            *value = false;
        }
        break;

        case TOKEN_INTEGER:
        {
            return false;
        }
        break;

        case TOKEN_FLOATING:
        {
            return false;
        }
        break;

        case TOKEN_STRING:
        {
            eat_token(lexer);
            *value = "<string>";
        }
        break;

        case TOKEN_IDENTIFIER:
        {
            if (!top_level) return false;
        }
        // @note: fall through on top-level objects, because we can omit top-level braces

        case TOKEN_BRACE_OPEN:
        {
            b32 open_brace_consumed = false;
            if (t.type == TOKEN_BRACE_OPEN) // @note: it is not guaranteed that token type is "TOKEN_BRACE_OPEN", because of fallthrough.
            {
                eat_token(lexer); // {
                open_brace_consumed = true;
            }

            while (true)
            {
                string k;
                son v;
                b32 ok = parse_kv_pair(&k, &v);
                if (!ok) {
                    break;
                }

                printf("kv pair: %.*s -> ", PRINT_SPAN(k));
                if (v.get_type() == son::type_t::boolean)
                {
                    printf("%s\n", v.get_boolean() ? "true" : "false");
                }
            }

            if (open_brace_consumed)
            {
                Token closing_brace = get_token(lexer);
                if (closing_brace.type != TOKEN_BRACE_CLOSE)
                {
                    // @error: consumed open brace but closed brace is not present.
                    return false;
                }

                eat_token(lexer); // }
            }
        }
        break;

        case TOKEN_BRACKET_OPEN:
        {
            return false;
        }
        break;

        default:
        {
            // @error
            return false;
        }
    }

    return true;
}


b32 Parser::parse_kv_pair(string *key, son *value)
{
    Token id = eat_token(lexer);
    if (id.type != TOKEN_IDENTIFIER) {
        return false;
    }

    Token equals = eat_token(lexer);
    if (equals.type != TOKEN_EQUALS) {
        return false;
    }

    Lexer saved = *lexer;
    b32 ok = parse_son(value, false);

    if (ok)
    {
        *key = id.span;
    }

    Token semicolon = get_token(lexer);
    if (semicolon.type == TOKEN_SEMICOLON) {
        eat_token(lexer);
    }

    return ok;
}


son Parser::parse_object(b32 top_level)
{
    son result;
    return result;
}


} // namespace internal


struct print_options {
    enum class multiline_t {
        disabled,
        enabled,
        smart,
    };

    FILE* output = stdout;
    b32 print_semicolons = true;
    b32 print_commas = true;
    i32 indent = 2;
    multiline_t multiline = multiline_t::smart;
};


static const char* spaces = "                                                  ";

i32 pretty_print_impl(son *value, const print_options& options, i32 depth) {
    // @Fix: The reason that value is non-constant type is that I didn't make pairs work with constant iterators.
    //       To fix this, make const_object_iterator, const_iterator_proxy, and make pairs which will work with constant types.
    switch (value->get_type()) {
    case son::type_t::null: fprintf(options.output, "null"); break;
    case son::type_t::boolean: fprintf(options.output, "%s", value->get_boolean() ? "true" : "false"); break;
    case son::type_t::integer: fprintf(options.output, "%lld", value->get_integer()); break;
    case son::type_t::floating: fprintf(options.output, "%lf", value->get_floating()); break;
    // case son::type_t::string: fprintf(options.output, "\"%s\"", value->get_string().c_str()); break;
    // case son::type_t::object: {
    //     bool in_one_line = (options.multiline == print_options::multiline_t::smart && value->deep_size() <= 6)
    //         || options.multiline == print_options::multiline_t::disabled;

    //     fprintf(options.output, "{%s", in_one_line ? " " : "\n");
    //     depth += 1;

    //     for (auto p : value->pairs()) {
    //         auto& k = p.first;
    //         auto& v = p.second;

    //         if (!in_one_line) { fprintf(options.output, "%.*s", options.indent * depth, spaces); }
    //         fprintf(options.output, "%s = ", k.c_str());

    //         pretty_print_impl(v, options, depth);

    //         fprintf(options.output, "%s%s",
    //             options.print_semicolons ? ";" : "",
    //             in_one_line ? " " : "\n"
    //         );
    //     }

    //     depth -= 1;
    //     fprintf(options.output, "%.*s}", in_one_line ? 0 : options.indent * depth, spaces);
    //     break;
    // }
    // case son::type_t::array: {
    //     bool in_one_line = (options.multiline == print_options::multiline_t::smart && value->deep_size() <= 6)
    //         || options.multiline == print_options::multiline_t::disabled;

    //     fprintf(options.output, "[%s", in_one_line ? value->size() > 0 ? " " : "" : "\n");
    //     depth += 1;

    //     for (size_t i = 0; i < value.size(); i++) {
    //         auto& v = value[i];

    //         if (!in_one_line) { fprintf(options.output, "%.*s", options.indent * depth, spaces); }

    //         pretty_print_impl(v, options, depth);

    //         fprintf(options.output, "%s%s",
    //             options.print_commas && i + 1 < value.size() ? "," : "",
    //             in_one_line ? " " : "\n"
    //         );
    //     }

    //     depth -= 1;
    //     fprintf(options.output, "%.*s]", in_one_line ? 0 : options.indent * depth, spaces);
    //     break;
    // }
    }

    return 0;
}


i32 pretty_print(son *value, const print_options& options = print_options())
{
    if (options.output == nullptr) { return -1; }
    return pretty_print_impl(value, options, 0);
}


son load_from_file(char const *filename)
{

    internal::Lexer lexer {};
    lexer.buffer = os::load_entire_file(filename);
    lexer.line = 1;
    lexer.column = 1;

    memory::arena_allocator arena{};
    arena.size   = 1 << 16;
    arena.memory = calloc(sizeof(uint8), arena.size);

    internal::Parser parser {};
    parser.arena = &arena;
    parser.lexer = &lexer;

    son result;
    b32 ok = parser.parse_son(&result, true);
    if (!ok) {
        return son();
    }

    pretty_print(&result);

    return result;
}


} // namespace asuka

