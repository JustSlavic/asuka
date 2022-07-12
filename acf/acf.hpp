#ifndef ACF_LIB_HPP
#define ACF_LIB_HPP


/*
    Asuke Config File Format

    JSON-like file format, but advanced with

    Features:
        - comment until the eol with //
        - keys are identifiers, not strings
        - key-value pairs use equal sign as separator
        - key-value paris separated by semicolon
        - values in lists separated by comma
        - trailing comma
        - top-level braces and brackets are optional
        - semicolons are optional
        - commas are optional

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


enum class acf_type
{
    null,
    boolean,
    integer,
    floating,
    string,
    object,
    array,
    custom,
    // type, // @note: this is special type, for trying to make types be values
};


char const *get_acf_type_string(acf_type type)
{
    switch (type)
    {
        case acf_type::null: return "null";
        case acf_type::boolean: return "boolean";
        case acf_type::integer: return "integer";
        case acf_type::floating: return "floating";
        case acf_type::string: return "string";
        case acf_type::object: return "object";
        case acf_type::array: return "array";
        case acf_type::custom: return "custom";
    }

    return "<none>";
}


struct acf;

using acf_boolean_type  = bool;
using acf_integer_type  = int64;
using acf_floating_type = float64;
using acf_string_type   = string;
using acf_array_type    = array<acf>;


struct acf_object_type
{
    array<string> keys;
    array<acf>    values;
};


struct acf_custom_type
{
    string name;
    array<acf> arguments;
};


struct acf
{
private:
    acf_type type;
    union
    {
        acf_boolean_type  boolean_value;
        acf_integer_type  integer_value;
        acf_floating_type floating_value;
        acf_string_type   string_value;
        acf_object_type   object_value;
        acf_array_type    array_value;
        acf_custom_type   custom_value;
    };

public:
    acf() : type(acf_type::null)
    {
    }
    acf(acf_type type) : type(type)
    {
    }
    acf(bool value) : type(acf_type::boolean)
    {
        boolean_value = value;
    }
    acf(int value) : type(acf_type::integer)
    {
        integer_value = value;
    }
    acf(int64 value) : type(acf_type::integer)
    {
        integer_value = value;
    }
    acf(float value) : type(acf_type::floating)
    {
        floating_value = value;
    }
    acf(double value) : type(acf_type::floating)
    {
        floating_value = value;
    }
    acf(char const *value) : type(acf_type::string)
    {
        string_value = Asuka::from_cstr(value);
    }
    acf(string value) : type(acf_type::string)
    {
        string_value = value;
    }

    acf_type get_type() const { return type; }

    bool is_boolean  () const { return (type == acf_type::boolean);  }
    bool is_integer  () const { return (type == acf_type::integer);  }
    bool is_floating () const { return (type == acf_type::floating); }
    bool is_string   () const { return (type == acf_type::string);   }
    bool is_object   () const { return (type == acf_type::object);   }
    bool is_array    () const { return (type == acf_type::array);    }
    bool is_custom   () const { return (type == acf_type::custom);   }

    acf_boolean_type  get_boolean  () const { return boolean_value;  }
    acf_integer_type  get_integer  () const { return integer_value;  }
    acf_floating_type get_floating () const { return floating_value; }
    acf_string_type   get_string   () const { return string_value;   }
    // @todo: remove this method because it'd be better to access values just from the value itself
    // @todo: iterators for accessing values in object and array
    acf_object_type   get_object   () const { return object_value;   }
    acf_array_type    get_array    () const { return array_value;    }
    acf_custom_type   get_custom   () const { return custom_value;   }

    void set_null()
    {
        // @todo: deallocate what was there previously.
        type = acf_type::null;
    }

    void set_boolean(bool value)
    {
        // @todo: deallocate what was there previously.
        type = acf_type::boolean;
        boolean_value = value;
    }

    void set_integer(int value)
    {
        // @todo: deallocate what was there previously.
        type = acf_type::integer;
        integer_value = value;
    }

    void set_array(acf_array_type value)
    {
        // @todo: deallocate what was there previously.
        type = acf_type::array;
        array_value = value;
    }

    void set_object(acf_object_type value)
    {
        // @todo: deallocate what was there previously.
        type = acf_type::object;
        object_value = value;
    }

    void set_custom(acf_custom_type value)
    {
        // @todo: deallocate what was there previously.
        type = acf_type::custom;
        custom_value = value;
    }

    usize size() const
    {
        switch (get_type()) {
            case acf_type::null:
                return 0;
            case acf_type::boolean:
            case acf_type::integer:
            case acf_type::floating:
            case acf_type::string:
                return 1;
            case acf_type::object:
            {
                return object_value.keys.get_size();
            }
            case acf_type::array:
            {
                return array_value.get_size();
            }
            case acf_type::custom:
            {
                return 1 + custom_value.arguments.get_size();
            }
        }

        // Why gcc says that control reaches end of non-void function,
        // if there's return in every possible case above?
        return 0;
    }

    usize depth_size() const
    {
        switch (get_type())
        {
            case acf_type::null:
            case acf_type::boolean:
            case acf_type::integer:
            case acf_type::floating:
            case acf_type::string:
                return 1;
            case acf_type::object:
            {
                usize n = 0;
                for (auto& v : object_value.values)
                {
                    n += v.depth_size();
                }
                return n;
            }
            case acf_type::array:
            {
                usize n = 0;
                for (auto& v : array_value)
                {
                    n += v.depth_size();
                }
                return n;
            }
            case acf_type::custom:
            {
                usize n = 1 + custom_value.arguments.get_size();
                return n;
            }
        }

        return 0;
    }
};


struct acf_print_options
{
    enum class multiline_t
    {
        disabled,
        enabled,
        smart,
    };

    bool print_semicolons = true;
    bool print_commas = true;
    int32 indent = 2;
    multiline_t multiline = multiline_t::smart;
    int32 max_elements_in_line = 3;
};


void acf_print(acf const& value, acf_print_options options = acf_print_options());


#ifdef ACF_LIB_IMPLEMENTATION

namespace
{

bool is_whitespace(char c) { return c == ' '; }
bool is_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
bool is_newline(char c) { return c == '\n'; }
bool is_alpha(char c) { return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; }
bool is_digit(char c) { return c >= '0' && c <= '9'; }
bool is_valid_identifier_head(char c) { return c == '_' || is_alpha(c); }
bool is_valid_identifier_body(char c) { return c == '_' || is_alpha(c) || is_digit(c); }


enum class acf_token_type
{
    undefined = 0,

    parentheses_open = '(',
    parentheses_close = ')',

    brace_open = '{',
    brace_close = '}',

    bracket_open = '[',
    bracket_close = ']',

    equals = '=',
    semicolon = ';',
    comma = ',',
    pound = '#',

    keyword_null = 300,
    keyword_true,
    keyword_false,
    keyword_bool,
    keyword_int,
    keyword_float,
    keyword_string,
    keyword_object,
    keyword_array,

    identifier,
    integer,
    floating,
    string,
    comment,

    end_of_file,
};


char const *get_acf_token_type_string(acf_token_type type)
{
    switch (type)
    {
        case acf_token_type::undefined: return "undefined";
        case acf_token_type::parentheses_open: return "parentheses_open";
        case acf_token_type::parentheses_close: return "parentheses_close";
        case acf_token_type::brace_open: return "brace_open";
        case acf_token_type::brace_close: return "brace_close";
        case acf_token_type::bracket_open: return "bracket_open";
        case acf_token_type::bracket_close: return "bracket_close";
        case acf_token_type::equals: return "equals";
        case acf_token_type::semicolon: return "semicolon";
        case acf_token_type::comma: return "comma";
        case acf_token_type::keyword_null: return "keyword_null";
        case acf_token_type::keyword_true: return "keyword_true";
        case acf_token_type::keyword_false: return "keyword_false";
        case acf_token_type::keyword_bool: return "keyword_bool";
        case acf_token_type::keyword_int: return "keyword_int";
        case acf_token_type::keyword_float: return "keyword_float";
        case acf_token_type::keyword_string: return "keyword_string";
        case acf_token_type::keyword_object: return "keyword_object";
        case acf_token_type::keyword_array: return "keyword_array";
        case acf_token_type::identifier: return "identifier";
        case acf_token_type::integer: return "integer";
        case acf_token_type::floating: return "floating";
        case acf_token_type::string: return "string";
        case acf_token_type::comment: return "comment";
        case acf_token_type::end_of_file: return "end_of_file";
    }

    return "<none>";
}


struct acf_token
{
    acf_token_type type;

    union
    {
        int64   integer_value;
        float64 floating_value;
    };

    usize line;
    usize column;
    string span;
};


void print_acf_token(acf_token token)
{
    switch (token.type)
    {
        case acf_token_type::undefined:
        {
            osOutputDebugString("token: undefined");
        }
        break;

        case acf_token_type::parentheses_open:
        {
            osOutputDebugString("(");
        }
        break;

        case acf_token_type::parentheses_close:
        {
            osOutputDebugString(")");
        }
        break;

        case acf_token_type::brace_open:
        {
            osOutputDebugString("{");
        }
        break;

        case acf_token_type::brace_close:
        {
            osOutputDebugString("}");
        }
        break;

        case acf_token_type::bracket_open:
        {
            osOutputDebugString("[");
        }
        break;

        case acf_token_type::bracket_close:
        {
            osOutputDebugString("]");
        }
        break;

        case acf_token_type::equals:
        {
            osOutputDebugString("=");
        }
        break;

        case acf_token_type::semicolon:
        {
            osOutputDebugString(";");
        }
        break;

        case acf_token_type::comma:
        {
            osOutputDebugString(",");
        }
        break;

        case acf_token_type::keyword_null:
        {
            osOutputDebugString("null");
        }
        break;

        case acf_token_type::keyword_true:
        {
            osOutputDebugString("true");
        }
        break;

        case acf_token_type::keyword_false:
        {
            osOutputDebugString("false");
        }
        break;

        case acf_token_type::keyword_bool:
        {
            osOutputDebugString("bool");
        }
        break;

        case acf_token_type::keyword_int:
        {
            osOutputDebugString("int");
        }
        break;

        case acf_token_type::keyword_float:
        {
            osOutputDebugString("float");
        }
        break;

        case acf_token_type::keyword_string:
        {
            osOutputDebugString("string");
        }
        break;

        case acf_token_type::keyword_object:
        {
            osOutputDebugString("object");
        }
        break;

        case acf_token_type::keyword_array:
        {
            osOutputDebugString("array");
        }
        break;

        case acf_token_type::identifier:
        {
            osOutputDebugString("%.*s", (int) token.span.get_size(), token.span.get_data());
        }
        break;

        case acf_token_type::integer:
        {
            osOutputDebugString("%lld", token.integer_value);
        }
        break;

        case acf_token_type::floating:
        {
            osOutputDebugString("%lf", token.floating_value);
        }
        break;

        case acf_token_type::string:
        {
            osOutputDebugString("%.*s", (int) token.span.get_size(), token.span.get_data());
        }
        break;

        case acf_token_type::comment:
        {
            osOutputDebugString("%.*s", (int) token.span.get_size(), token.span.get_data());
        }
        break;

        case acf_token_type::end_of_file:
        {
            osOutputDebugString("token: end_of_file");
        }
        break;
    }
}


struct acf_constructor_arguments
{
    uint32 argument_count;
    acf_type arguments[4];
};


struct acf_lexer
{
    string buffer;
    usize index;

    int32 line;
    int32 column;

    acf_token next_token;
    bool next_token_valid;

    u32 keyword_count;
    string keywords[32];
    acf_token_type keyword_types[32];

    u32 new_type_count;
    string new_types[32];
    acf_constructor_arguments arguments[32];
};


void register_acf_keyword(acf_lexer *lexer, char const *keyword, acf_token_type type)
{
    lexer->keywords[lexer->keyword_count] = Asuka::from_cstr(keyword);
    lexer->keyword_types[lexer->keyword_count] = type;

    lexer->keyword_count += 1;
}


void register_acf_new_type(acf_lexer *lexer, string type_name, acf_constructor_arguments args)
{
    lexer->new_types[lexer->new_type_count] = type_name;
    lexer->arguments[lexer->new_type_count] = args;
    lexer->new_type_count += 1;
}


void register_acf_new_type(acf_lexer *lexer, char const *type_name, acf_constructor_arguments args)
{
    register_acf_new_type(lexer, Asuka::from_cstr(type_name), args);
}


bool is_newtype_registered(acf_lexer *lexer, string s, acf_constructor_arguments *args)
{
    for (uint32 type_name_index = 0; type_name_index < lexer->new_type_count; type_name_index++)
    {
        string type_name = lexer->new_types[type_name_index];
        if (s == type_name)
        {
            if (args)
            {
                *args = lexer->arguments[type_name_index];
            }
            return true;
        }
    }

    return false;
}


void initialize_acf_lexer(acf_lexer *lexer, string buffer)
{
    lexer->buffer = buffer;
    lexer->index  = 0;
    lexer->line   = 1;
    lexer->column = 1;
    lexer->next_token = {};
    lexer->next_token_valid = false;
    lexer->keyword_count = 0;
    memory::set(lexer->keywords, 0, sizeof(lexer->keywords));
    lexer->new_type_count = 0;
    memory::set(lexer->new_types, 0, sizeof(lexer->new_types));

    register_acf_keyword(lexer, "null", acf_token_type::keyword_null);
    register_acf_keyword(lexer, "true", acf_token_type::keyword_true);
    register_acf_keyword(lexer, "false", acf_token_type::keyword_false);
    register_acf_keyword(lexer, "bool", acf_token_type::keyword_bool);
    register_acf_keyword(lexer, "int", acf_token_type::keyword_int);
    register_acf_keyword(lexer, "float", acf_token_type::keyword_float);
}


char *get_char_pointer(acf_lexer *lexer)
{
    ASSERT(lexer->index < lexer->buffer.get_size());

    char *result = lexer->buffer.get_data() + lexer->index;
    return result;
}


char get_char(acf_lexer *lexer)
{
    char result = 0;

    if (lexer->index < lexer->buffer.get_size())
    {
        result = lexer->buffer[lexer->index];
    }

    return result;
}


char eat_char(acf_lexer *lexer)
{
    char result = get_char(lexer);
    lexer->index += 1;

    if (result == '\n')
    {
        lexer->line += 1;
        lexer->column = 0;
    }

    lexer->column += 1;
    return result;
}


void consume_while(acf_lexer *lexer, bool (*predicate)(char))
{
    while (predicate(get_char(lexer)))
    {
        eat_char(lexer);
    }
}


acf_token_type get_identifier_type(acf_lexer *lexer, string s)
{
    for (uint32 keyword_index = 0; keyword_index < lexer->keyword_count; keyword_index++)
    {
        string keyword = lexer->keywords[keyword_index];
        if (s == keyword)
        {
            return lexer->keyword_types[keyword_index];
        }
    }

    return acf_token_type::identifier;
}


acf_token get_token(acf_lexer *lexer)
{
    if (!lexer->next_token_valid)
    {
        acf_token token = {};

        consume_while(lexer, is_space);

        char c = get_char(lexer);
        if (c == 0)
        {
            token.type = acf_token_type::end_of_file;
            token.line = lexer->line;
            token.column = lexer->column;
        }
        else if (c == '/')
        {
            if (c == '/')
            {
                token.line = lexer->line;
                token.column = lexer->column;

                char *span_start = get_char_pointer(lexer);

                eat_char(lexer);
                c = eat_char(lexer);

                consume_while(lexer, [](char c) { return c != '\n' && c != '\r'; } );

                char *span_end = get_char_pointer(lexer);
                usize span_size = ((usize)span_end - (usize)span_start);

                token.type = acf_token_type::comment;
                token.span = make_string(span_start, span_size);
            }
            else
            {
                // Lexer failure!!!
                // There are no tokens starting with '/',
                // Probably wanted to make comment?
            }
        }
        else if (c == '"')
        {
            token.line = lexer->line;
            token.column = lexer->column;

            char *span_start = get_char_pointer(lexer);

            eat_char(lexer);

            bool success = true;
            loop
            {
                c = get_char(lexer);
                if (c == 0 || is_newline(c))
                {
                    success = false;
                    break;
                }

                eat_char(lexer);

                if (c == '\"')
                {
                    break;
                }
            }

            char *span_end = get_char_pointer(lexer);
            usize span_size = ((usize)span_end - (usize)span_start);

            token.type = acf_token_type::string;
            token.span = make_string(span_start, span_size);
        }
        else if (is_digit(c) || c == '-' || c == '+')
        {
            token.line = lexer->line;
            token.column = lexer->column;

            char *span_start = get_char_pointer(lexer);

            int64 sign = 1;
            int64 integer_value = 0;

            if (c == '-')
            {
                sign = -1;
                eat_char(lexer);
            }
            else if (c == '+')
            {
                eat_char(lexer);
            }

            while (is_digit(c = get_char(lexer)))
            {
                eat_char(lexer);

                integer_value *= 10;
                integer_value += (c - '0');
            }

            char *span_end = get_char_pointer(lexer);
            usize span_size = ((usize)span_end - (usize)span_start);

            token.type = acf_token_type::integer;
            token.integer_value = sign * integer_value;
            token.span = make_string(span_start, span_size);
        }
        else if (is_valid_identifier_head(c))
        {
            token.line = lexer->line;
            token.column = lexer->column;

            char *span_start = get_char_pointer(lexer);

            consume_while(lexer, is_valid_identifier_body);

            char *span_end = get_char_pointer(lexer);
            usize span_size = ((usize)span_end - (usize)span_start);

            // @note: Data dependency! Execute only in this order!
            token.span = make_string(span_start, span_size);
            token.type = get_identifier_type(lexer, token.span);
        }
        else
        {
            token.type = (acf_token_type) c;
            token.line = lexer->line;
            token.column = lexer->column;

            token.span = make_string(get_char_pointer(lexer), 1);
            eat_char(lexer);
        }

        lexer->next_token = token;
        lexer->next_token_valid = true;
    }

    // For now skip all comments!!!
    if (lexer->next_token.type == acf_token_type::comment)
    {
        // Parse token again!
        lexer->next_token_valid = false;
        get_token(lexer);
    }

    return lexer->next_token;
}


acf_token eat_token(acf_lexer *lexer)
{
    acf_token result = get_token(lexer);
    lexer->next_token_valid = false;
    return result;
}


#define ACF_GET_TOKEN_OR_FAIL(TOKEN_TYPE) \
    get_token(lexer); \
    if (get_token(lexer).type == acf_token_type::TOKEN_TYPE) { eat_token(lexer); } else { *lexer = checkpoint; return false; } \
    void(0)


template <typename Allocator>
bool parse_key_value_pair(Allocator *allocator, acf_lexer *lexer, string *key, acf *value);

template <typename Allocator>
bool parse_array(Allocator *allocator, acf_lexer *lexer, acf *result, bool brackets_optional = false);

template <typename Allocator>
bool parse_object(Allocator *allocator, acf_lexer *lexer, acf *result, bool braces_optional = false);

template <typename Allocator>
bool parse_constructor_call(Allocator *allocator, acf_lexer *lexer, acf *result);


template <typename Allocator>
bool parse_key_value_pair(Allocator *allocator, acf_lexer *lexer, string *key, acf *value)
{
    acf_lexer checkpoint = *lexer;

    auto ident_token = ACF_GET_TOKEN_OR_FAIL(identifier);
    ACF_GET_TOKEN_OR_FAIL(equals);

    *key = ident_token.span;
    acf_token t = get_token(lexer);
    switch (t.type)
    {
        case acf_token_type::keyword_null:
        {
            *value = acf();
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_true:
        {
            *value = acf(true);
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_false:
        {
            *value = acf(false);
            eat_token(lexer);
        }
        break;

        case acf_token_type::integer:
        {
            *value = acf(t.integer_value);
            eat_token(lexer);
        }
        break;

        // @todo: floating value

        case acf_token_type::identifier:
        {
            acf custom_value;
            bool success = parse_constructor_call(allocator, lexer, &custom_value);
            if (success)
            {
                // Ok.
                *value = custom_value;
            }
            else
            {
                osOutputDebugString("Could not parse constructor call '%.*s'!\n",
                    STRING_PRINT_(t.span));
                *lexer = checkpoint;
                return false;
            }
        }
        break;

        case acf_token_type::string:
        {
            // @note: This is somewhat dangerous operation, and the strings
            // should be reallocated in the allocator's memory anyway (for
            // escape symbols resolution), so I'll leave it like that for now.
            string string_value = t.span;
            string_value.data += 1;
            string_value.size -= 2;

            *value = acf(string_value);
            eat_token(lexer);
        }
        break;

        case acf_token_type::brace_open:
        {
            acf object_value;
            bool success = parse_object(allocator, lexer, &object_value);
            if (success)
            {
                *value = object_value;
            }
            else
            {
                *lexer = checkpoint;
                return false;
            }
        }
        break;

        case acf_token_type::bracket_open:
        {
            acf array_value;
            bool success = parse_array(allocator, lexer, &array_value);
            if (success)
            {
                *value = array_value;
            }
            else
            {
                *lexer = checkpoint;
                return false;
            }
        }
        break;

        default:
        {
            *lexer = checkpoint;
            return false;
        }
    }

    return true;
}


template <typename Allocator>
bool parse_object(Allocator *allocator, acf_lexer *lexer, acf *result, bool braces_optional)
{
    acf_lexer checkpoint = *lexer;

    acf_token open_brace_token = get_token(lexer);
    if (open_brace_token.type == acf_token_type::brace_open)
    {
        eat_token(lexer);

        // Ensure there is closing brace to this open brace.
        braces_optional = false;
    }
    else
    {
        if (!braces_optional)
        {
            *lexer = checkpoint;
            return false;
        }
    }

    auto keys = make_dynamic_array<string>(allocator);
    auto values = make_dynamic_array<acf>(allocator);

    bool should_stop = false;
    int32 iterations = 0;
    while (!should_stop)
    {
        acf_token t = get_token(lexer);
        switch (t.type)
        {
            case acf_token_type::identifier:
            {
                string key;
                acf value;
                bool success = parse_key_value_pair(allocator, lexer, &key, &value);
                if (success)
                {
                    bool key_already_defined = false;
                    for (auto& k : keys)
                    {
                        if (k == key) {
                            key_already_defined = true;
                            break;
                        }
                    }

                    if (key_already_defined)
                    {
                        osOutputDebugString("Key '%.*s' already defined in this object.\n", STRING_PRINT_(key));
                    }

                    keys.push(key);
                    values.push(value);
                }
                else
                {
                    *lexer = checkpoint;
                    return false;
                }
            }
            break;

            case acf_token_type::brace_close:
            {
                // End of object.
                should_stop = true;
            }
            break;

            default:
            {
                if (!braces_optional || iterations == 0)
                {
                    *lexer = checkpoint;
                    return false;
                }
                else
                {
                    // End of object, because can't parse anything further.
                    should_stop = true;
                }
            }
        }

        acf_token semicolon = get_token(lexer);
        if (semicolon.type == acf_token_type::semicolon)
        {
            // Consume optional semicolon, if present.
            eat_token(lexer);
        }

        iterations += 1;
    }

    acf_token close_brace_token = get_token(lexer);
    if (close_brace_token.type == acf_token_type::brace_close)
    {
        eat_token(lexer);
    }
    else
    {
        if (!braces_optional)
        {
            *lexer = checkpoint;
            return false;
        }
    }

    acf_object_type object_value = {};
    object_value.keys = make_array(keys);
    object_value.values = make_array(values);

    result->set_object(object_value);
    return true;
}


template <typename Allocator>
bool parse_array(Allocator *allocator, acf_lexer *lexer, acf *result, bool brackets_optional)
{
    acf_lexer checkpoint = *lexer;

    acf_token open_bracket_token = get_token(lexer);
    if (open_bracket_token.type == acf_token_type::bracket_open)
    {
        eat_token(lexer);

        // Ensure there is closing bracket to this open bracket.
        brackets_optional = false;
    }
    else
    {
        if (!brackets_optional)
        {
            *lexer = checkpoint;
            return false;
        }
    }

    auto values = make_dynamic_array<acf>(allocator);

    bool should_stop = false;
    int32 iterations = 0;
    while (!should_stop)
    {
        acf_token t = get_token(lexer);
        switch (t.type)
        {
            case acf_token_type::keyword_null:
            {
                values.push(acf());
                eat_token(lexer);
            }
            break;

            case acf_token_type::keyword_true:
            {
                values.push(acf(true));
                eat_token(lexer);
            }
            break;

            case acf_token_type::keyword_false:
            {
                values.push(acf(false));
                eat_token(lexer);
            }
            break;

            case acf_token_type::integer:
            {
                values.push(acf(t.integer_value));
                eat_token(lexer);
            }
            break;

            case acf_token_type::identifier:
            {
                acf custom_value;
                bool success = parse_constructor_call(allocator, lexer, &custom_value);
                if (success)
                {
                    // Ok.
                    values.push(custom_value);
                }
                else
                {
                    osOutputDebugString("Could not parse constructor call '%.*s'!\n",
                        STRING_PRINT_(t.span));
                    *lexer = checkpoint;
                    return false;
                }
            }
            break;

            case acf_token_type::string:
            {
                // @note: This is somewhat dangerous operation, and the strings
                // should be reallocated in the allocator's memory anyway (for
                // escape symbols resolution), so I'll leave it like that for now.
                string string_value = t.span;
                string_value.data += 1;
                string_value.size -= 2;

                values.push(acf(string_value));
                eat_token(lexer);
            }
            break;

            case acf_token_type::brace_open:
            {
                acf object_value;
                bool success = parse_object(allocator, lexer, &object_value);
                if (success)
                {
                    values.push(object_value);
                }
                else
                {
                    *lexer = checkpoint;
                    return false;
                }
            }
            break;

            case acf_token_type::bracket_close:
            {
                // End of array.
                should_stop = true;
            }
            break;

            default:
            {
                if (!brackets_optional || iterations == 0)
                {
                    *lexer = checkpoint;
                    return false;
                }
                else
                {
                    // End of array, because can't parse anything further.
                    should_stop = true;
                }
            }
        }

        acf_token comma = get_token(lexer);
        if (comma.type == acf_token_type::comma)
        {
            // Consume optional comma, if present.
            eat_token(lexer);
        }

        iterations += 1;
    }

    acf_token close_bracket_token = get_token(lexer);
    if (close_bracket_token.type == acf_token_type::bracket_close)
    {
        eat_token(lexer);
    }
    else
    {
        if (!brackets_optional)
        {
            *lexer = checkpoint;
            return false;
        }
    }

    result->set_array(make_array(values));
    return true;
}


bool parse_type(acf_lexer *lexer, acf_type *type)
{
    // @todo: make it parse already registered types
    acf_token type_name = get_token(lexer);
    switch (type_name.type)
    {
        case acf_token_type::keyword_null:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::null;
            }
            return true;
        }
        case acf_token_type::keyword_bool:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::boolean;
            }
            return true;
        }
        case acf_token_type::keyword_int:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::integer;
            }
            return true;
        }
        case acf_token_type::keyword_float:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::floating;
            }
            return true;
        }
        case acf_token_type::keyword_string:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::string;
            }
            return true;
        }
        case acf_token_type::keyword_object:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::object;
            }
            return true;
        }
        case acf_token_type::keyword_array:
        {
            eat_token(lexer);
            if (type)
            {
                *type = acf_type::array;
            }
            return true;
        }
    }

    return false;
}


bool parse_directive(acf_lexer *lexer)
{
    acf_lexer checkpoint = *lexer;

    auto pound_token = ACF_GET_TOKEN_OR_FAIL(pound);

    acf_token directive_token = get_token(lexer);
    if ((directive_token.type == acf_token_type::identifier)
        && (Asuka::equals_to_cstr(directive_token.span, "newtype")))
    {
        eat_token(lexer);
    }
    else
    {
        *lexer = checkpoint;
        return false;
    }

    auto constructor_name_token = ACF_GET_TOKEN_OR_FAIL(identifier);
    auto paren_open_token = ACF_GET_TOKEN_OR_FAIL(parentheses_open);

    acf_constructor_arguments args = {};
    bool should_stop = false;
    while (!should_stop)
    {
        acf_type arg_type;
        bool success = parse_type(lexer, &arg_type);
        if (success)
        {
            // Ok
            if (args.argument_count < ARRAY_COUNT(args.arguments))
            {
                args.arguments[args.argument_count++] = arg_type;
            }
        }
        else
        {
            // Could not parse next item
            should_stop = true;
        }

        acf_token comma = get_token(lexer);
        if (comma.type == acf_token_type::comma)
        {
            // Consume optional comma, if present.
            eat_token(lexer);
        }
    }

    auto paren_close_token = ACF_GET_TOKEN_OR_FAIL(parentheses_close);

    register_acf_new_type(lexer, constructor_name_token.span, args);

    return true;
}


template <typename Allocator>
bool parse_constructor_call(Allocator *allocator, acf_lexer *lexer, acf *result)
{
    acf_lexer checkpoint = *lexer;

    auto constructor_name_token = ACF_GET_TOKEN_OR_FAIL(identifier);

    array<acf> argument_values = allocate_array<acf>(allocator, 4);
    acf_constructor_arguments args;
    if (is_newtype_registered(lexer, constructor_name_token.span, &args))
    {
        ACF_GET_TOKEN_OR_FAIL(parentheses_open);

        for (uint32 argument_index = 0; argument_index < args.argument_count; argument_index++)
        {
            acf_type type = args.arguments[argument_index];
            acf_token t = get_token(lexer);

            if (t.type == acf_token_type::parentheses_close)
            {
                // Unexpected end of arguments.
                osOutputDebugString("Argument count mismatch!\n");
                *lexer = checkpoint;
                return false;
            }
            else if ((type == acf_type::null) && (t.type == acf_token_type::keyword_null))
            {
                argument_values[argument_index] = acf();
                eat_token(lexer);
            }
            else if ((type == acf_type::boolean) && (t.type == acf_token_type::keyword_true))
            {
                argument_values[argument_index] = acf(true);
                eat_token(lexer);
            }
            else if ((type == acf_type::boolean) && (t.type == acf_token_type::keyword_false))
            {
                argument_values[argument_index] = acf(false);
                eat_token(lexer);
            }
            else if ((type == acf_type::integer) && (t.type == acf_token_type::integer))
            {
                argument_values[argument_index] = acf(t.integer_value);
                eat_token(lexer);
            }
            else
            {
                osOutputDebugString("Constructor call expected value of type %s, but got %s.\n",
                    get_acf_type_string(type),
                    get_acf_token_type_string(t.type));
                *lexer = checkpoint;
                return false;
            }

            if (argument_index + 1 < args.argument_count) // Do not support trailing comma in function calls.
            {
                // Consume required comma, if present.
                acf_token comma = ACF_GET_TOKEN_OR_FAIL(comma);
            }
        }

        auto paren_close_token = ACF_GET_TOKEN_OR_FAIL(parentheses_close);
    }
    else
    {
        osOutputDebugString("Referencing an identifier '%.*s', which was not declared before!\n",
            STRING_PRINT_(constructor_name_token.span));
        *lexer = checkpoint;
        return false;
    }

    if (result)
    {
        acf_custom_type custom = {};
        custom.name = constructor_name_token.span;
        custom.arguments = argument_values;
        result->set_custom(custom);
    }

    return true;
}

template <typename Allocator>
acf parse_acf(Allocator *allocator, string buffer)
{
    acf result = {};

    acf_lexer lexer;
    initialize_acf_lexer(&lexer, buffer);

    bool success;
    do {
        success = parse_directive(&lexer);
    } while (success);

    success = parse_object(allocator, &lexer, &result, true);
    if (success)
    {
        acf_token t = get_token(&lexer);
        if (t.type != acf_token_type::end_of_file)
        {
            acf super_array;
            bool sup_array_success = parse_array(allocator, &lexer, &super_array, true);
            if (sup_array_success)
            {
                t = get_token(&lexer);
                if (t.type == acf_token_type::end_of_file)
                {
                    // All good.
                    // super_array.push(result);
                    // result = super_array;
                }
                else
                {
                    osOutputDebugString("Something left in the file, although should not been!\n");
                    success = false;
                }
            }
            else
            {
                success = false;
                osOutputDebugString("Something went wrong!\n");
            }
        }
    }
    else
    {
        acf array_value;
        success = parse_array(allocator, &lexer, &array_value, true);
        if (success)
        {
            acf_token t = get_token(&lexer);
            if (t.type == acf_token_type::end_of_file)
            {
                // All good.
                result = array_value;
            }
            else
            {
                osOutputDebugString("Something left in the file, although should not been!\n");
                success = false;
            }
        }
    }
    osOutputDebugString("\n\n%s!\n", success ? "success" : "failure");

    return result;
}

} // namespace


// 60 spaces should be enough
char const* spaces = "                                                            ";
void acf_print_impl(acf const& value, acf_print_options options, int32 depth = 0)
{
    switch (value.get_type())
    {
        case acf_type::null: osOutputDebugString("null"); break;
        case acf_type::boolean: osOutputDebugString("%s", value.get_boolean() ? "true" : "false"); break;
        case acf_type::integer: osOutputDebugString("%lld", value.get_integer()); break;
        case acf_type::floating: osOutputDebugString("<NOT IMPLEMENTED>"); break;
        case acf_type::string:
        {
            auto s = value.get_string();
            osOutputDebugString("\"%.*s\"", STRING_PRINT_(s));
        }
        break;

        case acf_type::object:
        {
        bool in_one_line = (options.multiline == acf_print_options::multiline_t::smart && value.depth_size() <= options.max_elements_in_line)
                || options.multiline == acf_print_options::multiline_t::disabled;

            osOutputDebugString("{%s", in_one_line ? " " : "\n");
            depth += 1;

            auto obj = value.get_object();

            // @todo: make iterators for acf structure
            //        this loop should be just
            // for (auto& [k, v] : obj.pairs()) { ... }
            for (int i = 0; i < obj.keys.get_size(); i++)
            {
                auto &s = obj.keys[i];
                auto &v = obj.values[i];

                if (!in_one_line) { osOutputDebugString("%.*s", options.indent * depth, spaces); }
                osOutputDebugString("%.*s = ", STRING_PRINT_(s));

                acf_print_impl(v, options, depth);

                osOutputDebugString("%s%s",
                    options.print_semicolons ? ";" : "",
                    in_one_line ? " " : "\n");
            }

            depth -= 1;
            osOutputDebugString("%.*s}", in_one_line ? 0 : options.indent * depth, spaces);
        }
        break;
        
        case acf_type::array:
        {
            bool in_one_line = (options.multiline == acf_print_options::multiline_t::smart && value.depth_size() <= options.max_elements_in_line)
                || options.multiline == acf_print_options::multiline_t::disabled;

            osOutputDebugString("[%s", in_one_line ? value.size() > 0 ? " " : "" : "\n");
            depth += 1;

            auto arr = value.get_array();
            for (usize i = 0; i < arr.get_size(); i++)
            {
                // @todo: make acf::operator[] to get values from an array just with [], and without value.get_array() part
                auto& v = arr[i];

                if (!in_one_line) { osOutputDebugString("%.*s", options.indent * depth, spaces); }

                acf_print_impl(v, options, depth);

                osOutputDebugString("%s%s",
                    (options.print_commas && ((i + 1) < arr.get_size())) ? "," : "",
                    in_one_line ? " " : "\n");
            }

            depth -= 1;
            osOutputDebugString("%.*s]", in_one_line ? 0 : options.indent * depth, spaces);
        }
        break;

        case acf_type::custom:
        {
            auto custom = value.get_custom();
            osOutputDebugString("%.*s(", STRING_PRINT_(custom.name));

            if (custom.arguments.get_size() > 0)
            {
                auto& arg = custom.arguments[0];

                acf_print_impl(arg, options, depth);
            }

            for (usize i = 1; i < custom.arguments.get_size(); i++)
            {
                auto& arg = custom.arguments[i];

                osOutputDebugString(", ");
                acf_print(arg);
            }

            osOutputDebugString(")");
        }
        break;

    }
}

void acf_print(acf const& value, acf_print_options options)
{
    acf_print_impl(value, options, 0);
}

#endif // ACF_LIB_IMPLEMENTATION

#endif // ACF_LIB_HPP
