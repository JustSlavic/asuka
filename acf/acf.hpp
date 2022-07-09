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


struct acf
{

};


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

    keyword_null = 300,
    keyword_true,
    keyword_false,

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
            osOutputDebugString("token: parentheses_open");
        }
        break;

        case acf_token_type::parentheses_close:
        {
            osOutputDebugString("token: parentheses_close");
        }
        break;

        case acf_token_type::brace_open:
        {
            osOutputDebugString("token: brace_open");
        }
        break;

        case acf_token_type::brace_close:
        {
            osOutputDebugString("token: brace_close");
        }
        break;

        case acf_token_type::bracket_open:
        {
            osOutputDebugString("token: bracket_open");
        }
        break;

        case acf_token_type::bracket_close:
        {
            osOutputDebugString("token: bracket_close");
        }
        break;

        case acf_token_type::equals:
        {
            osOutputDebugString("token: equals");
        }
        break;

        case acf_token_type::semicolon:
        {
            osOutputDebugString("token: semicolon");
        }
        break;

        case acf_token_type::comma:
        {
            osOutputDebugString("token: comma");
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

        case acf_token_type::identifier:
        {
            osOutputDebugString("%.*s", (int) token.span.size, token.span.data);
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
            osOutputDebugString("%.*s", (int) token.span.size, token.span.data);
        }
        break;

        case acf_token_type::comment:
        {
            osOutputDebugString("%.*s", (int) token.span.size, token.span.data);
        }
        break;

        case acf_token_type::end_of_file:
        {
            osOutputDebugString("token: end_of_file");
        }
        break;
    }
}


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
};


void register_acf_keyword(acf_lexer *lexer, char const *keyword, acf_token_type type)
{
    lexer->keywords[lexer->keyword_count] = Asuka::from_cstr(keyword);
    lexer->keyword_types[lexer->keyword_count] = type;

    lexer->keyword_count += 1;
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

    register_acf_keyword(lexer, "null", acf_token_type::keyword_null);
    register_acf_keyword(lexer, "true", acf_token_type::keyword_true);
    register_acf_keyword(lexer, "false", acf_token_type::keyword_false);
}


char *get_char_pointer(acf_lexer *lexer)
{
    ASSERT(lexer->index < lexer->buffer.size);

    char *result = lexer->buffer.data + lexer->index;
    return result;
}


char get_char(acf_lexer *lexer)
{
    char result = 0;

    if (lexer->index < lexer->buffer.size)
    {
        result = lexer->buffer.data[lexer->index];
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

                token.span.data = get_char_pointer(lexer);

                eat_char(lexer);
                c = eat_char(lexer);

                consume_while(lexer, [](char c) { return c != '\n' && c != '\r'; } );

                uint64 size = ((uint64)get_char_pointer(lexer) - (uint64)token.span.data);
                token.span.size = size;

                token.type = acf_token_type::comment;
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

            token.span.data = get_char_pointer(lexer);

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

            uint64 size = ((uint64)get_char_pointer(lexer) - (uint64)token.span.data);
            token.span.size = size;

            token.type = acf_token_type::string;
        }
        else if (is_digit(c) || c == '-' || c == '+')
        {
            token.line = lexer->line;
            token.column = lexer->column;

            token.span.data = get_char_pointer(lexer);

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

            uint64 size = ((uint64)get_char_pointer(lexer) - (uint64)token.span.data);
            token.span.size = size;

            token.integer_value = sign * integer_value;
            token.type = acf_token_type::integer;
        }
        else if (is_valid_identifier_head(c))
        {
            token.line = lexer->line;
            token.column = lexer->column;

            token.span.data = get_char_pointer(lexer);

            consume_while(lexer, is_valid_identifier_body);

            uint64 size = ((uint64)get_char_pointer(lexer) - (uint64)token.span.data);
            token.span.size = size;

            // It can be one of the keywords or just an identifier.
            token.type = get_identifier_type(lexer, token.span);
        }
        else
        {
            token.type = (acf_token_type) c;
            token.line = lexer->line;
            token.column = lexer->column;

            token.span.data = get_char_pointer(lexer);
            token.span.size = 1;

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


bool parse_key_value_pair(acf_lexer *lexer, acf_token *key, acf_token *value);
bool parse_array(acf_lexer *lexer, bool brackets_optional = false);
bool parse_object(acf_lexer *lexer, bool braces_optional = false);


bool parse_key_value_pair(acf_lexer *lexer, acf_token *key, acf_token *value)
{
    acf_lexer checkpoint = *lexer;

    acf_token ident_token = eat_token(lexer);
    if (ident_token.type != acf_token_type::identifier)
    {
        *lexer = checkpoint;
        return false;
    }

    print_acf_token(ident_token);

    acf_token equals_token = eat_token(lexer);
    if (equals_token.type != acf_token_type::equals)
    {
        *lexer = checkpoint;
        return false;
    }

    osOutputDebugString(" = ");

    acf_token value_token = get_token(lexer);
    switch (value_token.type)
    {
        case acf_token_type::keyword_null:
        {
            print_acf_token(value_token);
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_true:
        {
            print_acf_token(value_token);
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_false:
        {
            print_acf_token(value_token);
            eat_token(lexer);
        }
        break;

        case acf_token_type::identifier:
        {
            print_acf_token(value_token);
            eat_token(lexer);
        }
        break;

        case acf_token_type::integer:
        {
            print_acf_token(value_token);
            eat_token(lexer);
        }
        break;

        case acf_token_type::string:
        {
            print_acf_token(value_token);
            eat_token(lexer);
        }
        break;

        case acf_token_type::brace_open:
        {
            bool success = parse_object(lexer);
            if (!success)
            {
                *lexer = checkpoint;
                return false;
            }
        }
        break;

        case acf_token_type::bracket_open:
        {
            bool success = parse_array(lexer);
            if (!success)
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

    *key = ident_token;
    *value = value_token;

    return true;
}


bool parse_object(acf_lexer *lexer, bool braces_optional)
{
    acf_lexer checkpoint = *lexer;

    acf_token open_brace_token = get_token(lexer);
    if (open_brace_token.type == acf_token_type::brace_open)
    {
        eat_token(lexer);
        osOutputDebugString("{");

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

    bool should_stop = false;
    int32 iterations = 0;
    while (!should_stop)
    {
        acf_token t = get_token(lexer);
        switch (t.type)
        {
            case acf_token_type::identifier:
            {
                acf_token key, value;
                bool success = parse_key_value_pair(lexer, &key, &value);
                if (!success)
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
            osOutputDebugString(";");
        }

        iterations += 1;
    }

    acf_token close_brace_token = get_token(lexer);
    if (close_brace_token.type == acf_token_type::brace_close)
    {
        eat_token(lexer);
        osOutputDebugString("}");
    }
    else
    {
        if (!braces_optional)
        {
            *lexer = checkpoint;
            return false;
        }
    }

    return true;
}


bool parse_array(acf_lexer *lexer, bool brackets_optional)
{
    acf_lexer checkpoint = *lexer;

    acf_token open_bracket_token = get_token(lexer);
    if (open_bracket_token.type == acf_token_type::bracket_open)
    {
        eat_token(lexer);
        osOutputDebugString("[");

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

    bool should_stop = false;
    int32 iterations = 0;
    while (!should_stop)
    {
        acf_token t = get_token(lexer);
        switch (t.type)
        {
            case acf_token_type::keyword_null:
            {
                print_acf_token(t);
                eat_token(lexer);
            }
            break;

            case acf_token_type::keyword_true:
            {
                print_acf_token(t);
                eat_token(lexer);
            }
            break;

            case acf_token_type::keyword_false:
            {
                print_acf_token(t);
                eat_token(lexer);
            }
            break;

            case acf_token_type::integer:
            {
                print_acf_token(t);
                eat_token(lexer);
            }
            break;

            case acf_token_type::string:
            {
                print_acf_token(t);
                eat_token(lexer);
            }
            break;

            case acf_token_type::brace_open:
            {
                bool success = parse_object(lexer);
                if (!success)
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
            osOutputDebugString(", ");
        }

        iterations += 1;
    }

    acf_token close_bracket_token = get_token(lexer);
    if (close_bracket_token.type == acf_token_type::bracket_close)
    {
        eat_token(lexer);
        osOutputDebugString("]");
    }
    else
    {
        if (!brackets_optional)
        {
            *lexer = checkpoint;
            return false;
        }
    }

    return true;
}


acf parse_acf(string buffer)
{
    acf result = {};

    acf_lexer lexer;
    initialize_acf_lexer(&lexer, buffer);

    osOutputDebugString("\n");
    bool success = parse_object(&lexer, true);
    if (success)
    {
        acf_token t = get_token(&lexer);
        if (t.type != acf_token_type::end_of_file)
        {
            bool sup_array_success = parse_array(&lexer, true);
            if (sup_array_success)
            {
                t = get_token(&lexer);
                if (t.type != acf_token_type::end_of_file)
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
        success = parse_array(&lexer, true);
        if (success)
        {
            acf_token t = get_token(&lexer);
            if (t.type != acf_token_type::end_of_file)
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

#endif // ACF_LIB_IMPLEMENTATION

#endif // ACF_LIB_HPP
