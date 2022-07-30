#ifndef ACF_LIB_HPP
#define ACF_LIB_HPP

#include <defines.hpp>
#include <array.hpp>
#include <windows.h>
#include <allocator.hpp>

// #include <initializer_list>


/*
    Asuka Config File Format

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

        - Implement acf's own containers to make it a good library, which is not dependend
          onto engine's code nor std's code, which is relying onto things like exceptions
          which are don't make any sense in the year 2022.
          It might be even not pointers but just the pointer-size-capacity 
*/


enum class acf_type_t
{
    null = 0,
    boolean,
    integer,
    floating,
    string,
    object,
    array,
    custom,
    type,
};

INTERNAL
char const *get_acf_type_string(acf_type_t type)
{
    switch (type)
    {
        case acf_type_t::null: return "null";
        case acf_type_t::boolean: return "boolean";
        case acf_type_t::integer: return "integer";
        case acf_type_t::floating: return "floating";
        case acf_type_t::string: return "string";
        case acf_type_t::object: return "object";
        case acf_type_t::array: return "array";
        case acf_type_t::custom: return "custom";
        case acf_type_t::type: return "type";
    }

    return "<none>";
}


struct acf
{
    using acf_allocator_t = memory::mallocator;

    using acf_boolean_t = bool;
    using acf_integer_t = int64;
    using acf_floating_t = float64;
    using acf_string_t = string; // @todo: Make strings UTF-8.
    using acf_array_t = array<acf>;

    struct acf_object_t
    {
        array<string> keys;
        array<acf> values;
    };

    struct acf_custom_t
    {
        string     newtype_name;
        array<acf> newtype_arguments;
    };

    union acf_value_t
    {
        acf_boolean_t boolean_value;
        acf_integer_t integer_value;
        acf_string_t  string_value;
        acf_array_t   array_value;
        acf_object_t  object_value;
        acf_custom_t  custom_value;
    };

    acf_type_t type;
    acf_value_t value;
    acf_allocator_t *allocator;

    acf_array_t::iterator begin()
    {
        ASSERT(type == acf_type_t::array);
        return value.array_value.begin();
    }

    acf_array_t::iterator end()
    {
        ASSERT(type == acf_type_t::array);
        return value.array_value.end();
    }

    acf_array_t::const_iterator begin() const
    {
        ASSERT(type == acf_type_t::array);
        return value.array_value.cbegin();
    }

    acf_array_t::const_iterator end() const
    {
        ASSERT(type == acf_type_t::array);
        return value.array_value.cend();
    }

    acf_array_t::const_iterator cbegin() const
    {
        ASSERT(type == acf_type_t::array);
        return value.array_value.cbegin();
    }

    acf_array_t::const_iterator cend() const
    {
        ASSERT(type == acf_type_t::array);
        return value.array_value.cend();
    }

    struct object_iterator
    {
        struct pair
        {
            string& key;
            acf& value;
        };

        acf_object_t *host;
        usize index;

        object_iterator& operator ++ () { index += 1; return *this; }
        object_iterator  operator ++ (int) { object_iterator result = *this; index += 1; return result; }
        bool operator == (object_iterator other) const { return (host == other.host) && (index == other.index); }
        bool operator != (object_iterator other) const { return !(*this == other); }
        pair operator * () const { return { host->keys[index], host->values[index] }; }
    };

    struct object_iterator_wrapper
    {
        acf_object_t *host;

        object_iterator begin()
        {
            object_iterator it = {};
            it.host = host;
            it.index = 0;

            return it;
        }

        object_iterator end()
        {
            object_iterator it = {};
            it.host = host;
            it.index = host->keys.get_size();

            return it;
        }
    };

    object_iterator_wrapper pairs()
    {
        object_iterator_wrapper it = {};
        it.host = &this->value.object_value;

        return it;
    }
};


acf acf_null()
{
    acf result = {};
    result.type = acf_type_t::null;
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

acf acf_boolean(bool value)
{
    acf result = {};
    result.type = acf_type_t::boolean;
    result.value.boolean_value = value;
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

template <typename T>
acf acf_integer(T value)
{
    acf result = {};
    result.type = acf_type_t::integer;
    result.value.integer_value = (int64) value;
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

acf acf_string(string s)
{
    acf result = {};
    result.type = acf_type_t::string;
    result.value.string_value = s;
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

acf acf_array()
{
    acf result = {};
    result.type = acf_type_t::array;
    result.value.array_value = allocate_array<acf>(&memory::global_mallocator_instance, 4);
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

acf acf_object()
{
    acf result = {};
    result.type = acf_type_t::object;
    result.value.object_value.keys = allocate_array<string>(&memory::global_mallocator_instance, 4);
    result.value.object_value.values = allocate_array<acf>(&memory::global_mallocator_instance, 4);
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

acf acf_custom_type(string name)
{
    acf result = {};
    result.type = acf_type_t::custom;
    result.value.custom_value.newtype_name = name;
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

void acf_push(acf *dest, acf value)
{
    if (dest->type == acf_type_t::null)
    {
        if (dest->allocator == NULL)
        {
            dest->allocator = &memory::global_mallocator_instance;
        }

        dest->type = acf_type_t::array;
        dest->value.array_value = allocate_array<acf>(dest->allocator, 4);
    }

    if (dest->type == acf_type_t::array)
    {
        auto& arr = dest->value.array_value;
        if (arr.size == arr.capacity)
        {
            array<acf> new_array = allocate_array<acf>(dest->allocator, (arr.capacity + 1) * 2);
            copy_array(&new_array, arr);
            deallocate_array(dest->allocator, arr);
            arr = new_array;
        }

        arr.push(value);
    }
}

void acf_push(acf *dest, string key, acf value)
{
    if (dest->type == acf_type_t::null)
    {
        if (dest->allocator == NULL)
        {
            dest->allocator = &memory::global_mallocator_instance;
        }

        dest->type = acf_type_t::object;
        dest->value.object_value.keys = allocate_array<string>(dest->allocator, 4);
        dest->value.object_value.values = allocate_array<acf>(dest->allocator, 4);
    }

    if (dest->type == acf_type_t::object)
    {
        auto& keys   = dest->value.object_value.keys;
        auto& values = dest->value.object_value.values;

        ASSERT(keys.size == values.size); // Invariant!
        ASSERT(keys.capacity == values.capacity); // Invariant!

        if (keys.size == keys.capacity)
        {
            array<string> new_keys = allocate_array<string>(dest->allocator, (keys.capacity + 1) * 2);
            copy_array(&new_keys, keys);
            deallocate_array(dest->allocator, keys);
            keys = new_keys;

            array<acf> new_values = allocate_array<acf>(dest->allocator, (keys.capacity + 1) * 2);
            copy_array(&new_values, values);
            deallocate_array(dest->allocator, values);
            values = new_values;
        }

        keys.push(key);
        values.push(value);
    }
}

void acf_push_argument(acf *dest, acf arg)
{
    if (dest->type == acf_type_t::custom)
    {
        auto& arguments = dest->value.custom_value.newtype_arguments;
        if (arguments.size == arguments.capacity)
        {
            array<acf> new_args = allocate_array<acf>(dest->allocator, (arguments.capacity + 1) * 2);
            copy_array(&new_args, arguments);
            deallocate_array(dest->allocator, arguments);
            arguments = new_args;
        }

        arguments.push(arg);
    }
}

usize acf_size(acf value)
{
    switch (value.type)
    {
        case acf_type_t::null:
            return 0;
        case acf_type_t::boolean:
        case acf_type_t::integer:
        case acf_type_t::floating:
        case acf_type_t::string:
        case acf_type_t::type:
            return 1;
        case acf_type_t::array:
            return value.value.array_value.get_size();
        case acf_type_t::object:
            return value.value.object_value.keys.get_size();
        // case acf_type_t::custom:
        //     return value.value.cust
        default:
            INVALID_CODE_PATH();
            return 0;
    }
}

usize acf_depth_size(acf value)
{
    switch (value.type)
    {
        case acf_type_t::null:
        case acf_type_t::boolean:
        case acf_type_t::integer:
        case acf_type_t::floating:
        case acf_type_t::string:
        case acf_type_t::type:
            return 1;
        case acf_type_t::array:
        {
            usize n = 0;
            for (auto& v : value.value.array_value)
            {
                n += acf_depth_size(v);
            }
            return n;
        }
        case acf_type_t::object:
        {
            usize n = 0;
            for (auto& v : value.value.object_value.values)
            {
                n += acf_depth_size(v) + 1; // @note: +1 for the key
            }
            return n;
        }
        // case acf_type_t::custom:
        //     return value.value.cust
        default:
            INVALID_CODE_PATH();
            return 0;
    }
}

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

char const* spaces = "                                                            ";
void acf_print_impl(acf value, acf_print_options options, int depth = 0)
{
    switch (value.type)
    {
        case acf_type_t::null: osOutputDebugString("null"); break;
        case acf_type_t::boolean: osOutputDebugString("%s", value.value.boolean_value ? "true" : "false"); break;
        case acf_type_t::integer: osOutputDebugString("%lld", value.value.integer_value); break;
        case acf_type_t::floating: osOutputDebugString("<FLOAT NOT IMPLEMENTED>"); break;
        case acf_type_t::string: osOutputDebugString("\"%.*s\"", STRING_PRINT_(value.value.string_value)); break;

        case acf_type_t::object:
        {
            bool in_one_line = (options.multiline == acf_print_options::multiline_t::smart && acf_depth_size(value) <= options.max_elements_in_line)
                || options.multiline == acf_print_options::multiline_t::disabled;

            osOutputDebugString("{%s", in_one_line ? " " : "\n");
            depth += 1;

            for (auto [k, v] : value.pairs())
            {
                if (!in_one_line) { osOutputDebugString("%.*s", options.indent * depth, spaces); }
                osOutputDebugString("%.*s = ", STRING_PRINT_(k));
                acf_print_impl(v, options, depth);
                osOutputDebugString("%s%s", options.print_semicolons ? ";" : "", in_one_line ? " " : "\n");
            }

            depth -= 1;
            osOutputDebugString("%.*s}", in_one_line ? 0 : options.indent * depth, spaces);
        }
        break;

        case acf_type_t::array:
        {
            bool in_one_line = (options.multiline == acf_print_options::multiline_t::smart && acf_depth_size(value) <= options.max_elements_in_line)
                || options.multiline == acf_print_options::multiline_t::disabled;

            osOutputDebugString("[%s", in_one_line ? acf_size(value) > 0 ? " " : "" : "\n");
            depth += 1;

            int i = 0;
            for (auto& v : value)
            {
                if (!in_one_line) { osOutputDebugString("%.*s", options.indent * depth, spaces); }

                acf_print_impl(v, options, depth);

                osOutputDebugString("%s%s",
                    (options.print_commas && ((i + 1) < acf_size(value))) ? "," : "",
                    in_one_line ? " " : "\n");

                i += 1;
            }

            depth -= 1;
            osOutputDebugString("%.*s]", in_one_line ? 0 : options.indent * depth, spaces);
        }
        break;

        case acf_type_t::custom:
        {
            osOutputDebugString("%.*s(", STRING_PRINT_(value.value.custom_value.newtype_name));
            for (auto& arg : value.value.custom_value.newtype_arguments)
            {
                acf_print_options sub_options = options;
                sub_options.multiline = acf_print_options::multiline_t::disabled;
                acf_print_impl(arg, sub_options, depth);
            }
            osOutputDebugString(")");
        }
        break;

        case acf_type_t::type:
        {
            osOutputDebugString("<TYPE NOT IMPLEMENTED>");
        }
        break;
    }
}

void acf_print(acf value, acf_print_options options = acf_print_options())
{
    acf_print_impl(value, options, 0);
}


//     // @note: this requires an allocation strategy figured out
//     // @todo implement initializer list constructor to be able to create acfs with {}
//     // acf::acf(std::initializer_list<acf> init_list)
//     //     : acf()
//     // {
//     //     bool is_an_object = true;
//     //     for (auto it = init_list.begin(); it != init_list.end(); ++it)
//     //     {
//     //         acf v = acf(*it);
//     //         if (!(v.is_array() && v.get_size() == 2 && v[0].is_string()))
//     //         {
//     //             is_an_object = false;
//     //             break;
//     //         }
//     //     }

//         // if (is_an_object)
//         // {
//         //     for (auto& v : init_list) {
//         //         push(v[0].get_string(), v[1]);
//         //     }
//         // }
//         // if (is_an_object) {
//         // } else {
//         //     for (auto& v : init_list) {
//         //         push(v);
//         //     }
//         // }
//     // }

//     // Copy constructor
//     acf(acf const& other)
//     {
//         type = other.type;
//         allocator = other.allocator;

//         switch (type)
//         {
//             case acf_type_t::null:
//             {
//             }
//             break;

//             case acf_type_t::boolean:
//             {
//                 value.boolean_value = other.value.boolean_value;
//             }
//             break;

//             case acf_type_t::integer:
//             {
//                 value.integer_value = other.value.integer_value;
//             }
//             break;

//             case acf_type_t::floating:
//             {
//                 value.floating_value = other.value.floating_value;
//             }
//             break;

//             case acf_type_t::string:
//             {
//                 value.string_value = make_copy(allocator, other.value.string_value);
//             }
//             break;

//             // case acf_type_t::array:
//             // {
//             //     value.array_value = make_copy(other.value.array_value, allocator);
//             // }
//             // break;

//             // case acf_type_t::object:
//             // {
//             //     value.object_value.keys = make_copy(other.value.object_value.keys, allocator);
//             //     value.object_value.values = make_copy(other.value.object_value.values, allocator);
//             // }
//             // break;

//             // case acf_type_t::custom:
//             // {

//             // }
//             // break;

//         }
//     }

//     // Move constructor
//     acf(acf&& other)
//     {
//         this->swap(other);
//     }

//     // Copy assignment operator
//     acf& operator = (acf const& other)
//     {
//         acf(other).swap(*this);
//         return *this;
//     }

//     // Move assignment operator
//     acf& operator = (acf&& other)
//     {
//         other.swap(*this);
//         return *this;
//     }

//     template <typename T>
//     STATIC void acf_swap__(T& a, T& b)
//     {
//         T tmp = a;
//         a = b;
//         b = tmp;
//     }

//     void swap(acf& other)
//     {
//         acf_swap__(type, other.type);
//         acf_swap__(value, other.value);
//         acf_swap__(allocator, other.allocator);
//     }


//     void deallocate_content()
//     {
//         switch (type)
//         {
//             case acf_type_t::string:
//             {
//                 deallocate_string(allocator, value.string_value);
//             }
//             break;

//             // case acf_type_t::object:
//             // {
//             //     deallocate_array(value.object_value.keys);
//             //     deallocate_array(value.object_value.values);
//             // }
//             // break;

//             // case acf_type_t::array:
//             // {
//             //     deallocate_array(value.array_value);
//             // }
//             // break;

//             // case acf_type_t::custom:
//             // {
//             //     // @todo: deallocate name, when I implement string copying.
//             //     deallocate_array(value.custom_value.arguments);
//             // }
//             // break;
//         }
//     }
// };


//
// ====================== IMPLEMENTATION ======================
//
#ifdef ACF_LIB_IMPLEMENTATION

INTERNAL bool is_whitespace(char c) { return c == ' '; }
INTERNAL bool is_space(char c) { return c == ' ' || c == '\t' || c == '\r' || c == '\n'; }
INTERNAL bool is_newline(char c) { return c == '\n'; }
INTERNAL bool is_alpha(char c) { return c >= 'A' && c <= 'Z' || c >= 'a' && c <= 'z'; }
INTERNAL bool is_digit(char c) { return c >= '0' && c <= '9'; }
INTERNAL bool is_valid_identifier_head(char c) { return c == '_' || is_alpha(c); }
INTERNAL bool is_valid_identifier_body(char c) { return c == '_' || is_alpha(c) || is_digit(c); }

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
    keyword_type,

    identifier,
    integer,
    floating,
    string,
    comment,

    end_of_file,
};

INTERNAL
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
        case acf_token_type::keyword_array: return "keyword_array";
        case acf_token_type::keyword_object: return "keyword_object";
        case acf_token_type::keyword_type: return "keyword_type";
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

INTERNAL
void print_acf_token(acf_token token)
{
    switch (token.type)
    {
        case acf_token_type::undefined:         { osOutputDebugString("token: undefined"); } break;
        case acf_token_type::parentheses_open:  { osOutputDebugString("("); } break;
        case acf_token_type::parentheses_close: { osOutputDebugString(")"); } break;
        case acf_token_type::brace_open:        { osOutputDebugString("{"); } break;
        case acf_token_type::brace_close:       { osOutputDebugString("}"); } break;
        case acf_token_type::bracket_open:      { osOutputDebugString("["); } break;
        case acf_token_type::bracket_close:     { osOutputDebugString("]"); } break;
        case acf_token_type::equals:            { osOutputDebugString("="); } break;
        case acf_token_type::semicolon:         { osOutputDebugString(";"); } break;
        case acf_token_type::comma:             { osOutputDebugString(","); } break;
        case acf_token_type::keyword_null:      { osOutputDebugString("null"); } break;
        case acf_token_type::keyword_true:      { osOutputDebugString("true"); } break;
        case acf_token_type::keyword_false:     { osOutputDebugString("false"); } break;
        case acf_token_type::keyword_bool:      { osOutputDebugString("bool"); } break;
        case acf_token_type::keyword_int:       { osOutputDebugString("int"); } break;
        case acf_token_type::keyword_float:     { osOutputDebugString("float"); } break;
        case acf_token_type::keyword_string:    { osOutputDebugString("string"); } break;
        case acf_token_type::keyword_object:    { osOutputDebugString("object"); } break;
        case acf_token_type::keyword_array:     { osOutputDebugString("array"); } break;
        case acf_token_type::keyword_type:      { osOutputDebugString("type"); } break;
        case acf_token_type::identifier:        { osOutputDebugString("%.*s", (int) token.span.get_size(), token.span.get_data()); } break;
        case acf_token_type::integer:           { osOutputDebugString("%lld", token.integer_value); } break;
        case acf_token_type::floating:          { osOutputDebugString("%lf", token.floating_value); } break;
        case acf_token_type::string:            { osOutputDebugString("%.*s", (int) token.span.get_size(), token.span.get_data()); } break;
        case acf_token_type::comment:           { osOutputDebugString("%.*s", (int) token.span.get_size(), token.span.get_data()); } break;
        case acf_token_type::end_of_file:       { osOutputDebugString("token: end_of_file"); } break;
    }
}


struct acf_constructor_arguments
{
    uint32 argument_count;
    acf_type_t arguments[4];
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
    string newtype_names[32];
    acf_constructor_arguments newtype_arguments[32];
};

INTERNAL
void register_acf_keyword(acf_lexer *lexer, char const *keyword, acf_token_type type)
{
    lexer->keywords[lexer->keyword_count] = cstring::make_string(keyword);
    lexer->keyword_types[lexer->keyword_count] = type;

    lexer->keyword_count += 1;
}

INTERNAL
void register_acf_new_type(acf_lexer *lexer, string type_name, acf_constructor_arguments args)
{
    lexer->newtype_names[lexer->new_type_count] = type_name;
    lexer->newtype_arguments[lexer->new_type_count] = args;
    lexer->new_type_count += 1;
}

INTERNAL
void register_acf_new_type(acf_lexer *lexer, char const *type_name, acf_constructor_arguments args)
{
    register_acf_new_type(lexer, cstring::make_string(type_name), args);
}

INTERNAL
bool is_newtype_registered(acf_lexer *lexer, string s, acf_constructor_arguments *args)
{
    for (uint32 type_name_index = 0; type_name_index < lexer->new_type_count; type_name_index++)
    {
        string type_name = lexer->newtype_names[type_name_index];
        if (s == type_name)
        {
            if (args)
            {
                *args = lexer->newtype_arguments[type_name_index];
            }
            return true;
        }
    }

    return false;
}

INTERNAL
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
    memory::set(lexer->newtype_names, 0, sizeof(lexer->newtype_names));
    memory::set(lexer->newtype_arguments, 0, sizeof(lexer->newtype_arguments));

    register_acf_keyword(lexer, "null", acf_token_type::keyword_null);
    register_acf_keyword(lexer, "true", acf_token_type::keyword_true);
    register_acf_keyword(lexer, "false", acf_token_type::keyword_false);
    register_acf_keyword(lexer, "bool", acf_token_type::keyword_bool);
    register_acf_keyword(lexer, "int", acf_token_type::keyword_int);
    register_acf_keyword(lexer, "float", acf_token_type::keyword_float);
    register_acf_keyword(lexer, "string", acf_token_type::keyword_string);
    register_acf_keyword(lexer, "array", acf_token_type::keyword_array);
    register_acf_keyword(lexer, "object", acf_token_type::keyword_object);
    register_acf_keyword(lexer, "type", acf_token_type::keyword_type);
}

INTERNAL
char *get_char_pointer(acf_lexer *lexer)
{
    ASSERT(lexer->index < lexer->buffer.get_size());

    char *result = lexer->buffer.get_data() + lexer->index;
    return result;
}

INTERNAL
char get_char(acf_lexer *lexer)
{
    char result = 0;

    if (lexer->index < lexer->buffer.get_size())
    {
        result = lexer->buffer[lexer->index];
    }

    return result;
}

INTERNAL
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

INTERNAL
void consume_while(acf_lexer *lexer, bool (*predicate)(char))
{
    while (predicate(get_char(lexer)))
    {
        eat_char(lexer);
    }
}

INTERNAL
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

INTERNAL
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

INTERNAL
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


bool parse_value(acf_lexer *lexer, acf *result);
bool parse_key_value_pair(acf_lexer *lexer, string *key, acf *value);
bool parse_array(acf_lexer *lexer, acf *result, bool brackets_optional = false);
bool parse_object(acf_lexer *lexer, acf *result, bool braces_optional = false);
bool parse_constructor_call(acf_lexer *lexer, acf *result);


bool parse_value(acf_lexer *lexer, acf *result)
{
    acf_lexer checkpoint = *lexer;

    acf_token t = get_token(lexer);
    switch (t.type)
    {
        case acf_token_type::keyword_null:
        {
            *result = acf_null();
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_true:
        {
            *result = acf_boolean(true);
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_false:
        {
            *result = acf_boolean(false);
            eat_token(lexer);
        }
        break;

        case acf_token_type::integer:
        {
            *result = acf_integer(t.integer_value);
            eat_token(lexer);
        }
        break;

        // @todo: floating value

        case acf_token_type::identifier:
        {
            acf custom_value;
            bool success = parse_constructor_call(lexer, &custom_value);
            if (success)
            {
                // Ok.
                *result = custom_value;
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
            string s = t.span;
            s.data += 1;
            s.size -= 2;

            *result = acf_string(s);
            eat_token(lexer);
        }
        break;

        case acf_token_type::brace_open:
        {
            acf object_value;
            bool success = parse_object(lexer, &object_value);
            if (success)
            {
                *result = object_value;
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
            bool success = parse_array(lexer, &array_value);
            if (success)
            {
                *result = array_value;
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


bool parse_key_value_pair(acf_lexer *lexer, string *key, acf *value)
{
    acf_lexer checkpoint = *lexer;

    auto ident_token = ACF_GET_TOKEN_OR_FAIL(identifier);
    ACF_GET_TOKEN_OR_FAIL(equals);

    *key = ident_token.span;
    bool success = parse_value(lexer, value);
    if (!success)
    {
        *lexer = checkpoint;
        return false;
    }

    return true;
}


bool parse_object(acf_lexer *lexer, acf *result, bool braces_optional)
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

    acf object_result = acf_object();

    bool should_stop = false;
    while (!should_stop)
    {
        string key;
        acf value;
        bool success = parse_key_value_pair(lexer, &key, &value);
        if (success)
        {
            bool key_already_defined = false;
            for (auto [k, v] : object_result.pairs())
            {
                if (k == key)
                {
                    key_already_defined = true;
                    break;
                }
            }

            if (key_already_defined)
            {
                osOutputDebugString("Key '%.*s' already defined in this object.\n", STRING_PRINT_(key));
            }
            else
            {
                acf_push(&object_result, key, value);
            }
        }
        else
        {
            if (braces_optional && acf_size(object_result) == 0)
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

        acf_token semicolon = get_token(lexer);
        if (semicolon.type == acf_token_type::semicolon)
        {
            // Consume optional semicolon, if present.
            eat_token(lexer);
        }
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

    *result = object_result;
    return true;
}


bool parse_array(acf_lexer *lexer, acf *result, bool brackets_optional)
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

    acf array_value = acf_array();
    int count = 0;

    bool should_stop = false;
    while (!should_stop)
    {
        acf value;
        bool success = parse_value(lexer, &value);
        if (success)
        {
            acf_push(&array_value, value);
            count += 1;
        }
        else
        {
            if (brackets_optional && count == 0) // && values.is_null())
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

        acf_token comma = get_token(lexer);
        if (comma.type == acf_token_type::comma)
        {
            // Consume optional comma, if present.
            eat_token(lexer);
        }
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

    *result = array_value;
    return true;
}

INTERNAL
bool parse_type(acf_lexer *lexer, acf_type_t *type)
{
    ASSERT(type);

    acf_lexer checkpoint = *lexer;
    acf_token type_name = eat_token(lexer);
    switch (type_name.type)
    {
        case acf_token_type::keyword_null:   { *type = acf_type_t::null; } break;
        case acf_token_type::keyword_bool:   { *type = acf_type_t::boolean; } break;
        case acf_token_type::keyword_int:    { *type = acf_type_t::integer; } break;
        case acf_token_type::keyword_float:  { *type = acf_type_t::floating; } break;
        case acf_token_type::keyword_string: { *type = acf_type_t::string; } break;
        case acf_token_type::keyword_object: { *type = acf_type_t::object; } break;
        case acf_token_type::keyword_array:  { *type = acf_type_t::array; } break;
        default:
        {
            // @todo: make it parse already registered types
            *lexer = checkpoint;
            return false;
        }
    }

    return true;
}

INTERNAL
bool parse_directive(acf_lexer *lexer)
{
    acf_lexer checkpoint = *lexer;

    auto pound_token = ACF_GET_TOKEN_OR_FAIL(pound);

    acf_token directive_token = get_token(lexer);
    if ((directive_token.type == acf_token_type::identifier)
        && (cstring::equals_to_cstr(directive_token.span, "newtype")))
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
        acf_type_t arg_type;
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
            acf_token close_paren = get_token(lexer);
            if (close_paren.type != acf_token_type::parentheses_close)
            {
                *lexer = checkpoint;
                return false;
            }

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


bool parse_constructor_call(acf_lexer *lexer, acf *result)
{
    acf_lexer checkpoint = *lexer;

    auto constructor_name_token = ACF_GET_TOKEN_OR_FAIL(identifier);

    acf custom_value_result = acf_custom_type(constructor_name_token.span);
    acf_constructor_arguments args;
    if (is_newtype_registered(lexer, constructor_name_token.span, &args))
    {
        ACF_GET_TOKEN_OR_FAIL(parentheses_open);

        for (uint32 argument_index = 0; argument_index < args.argument_count; argument_index++)
        {
            acf_type_t type = args.arguments[argument_index];
            acf_token t = get_token(lexer);

            if (t.type == acf_token_type::parentheses_close)
            {
                // Unexpected end of arguments.
                osOutputDebugString("Argument count mismatch!\n");
                *lexer = checkpoint;
                return false;
            }
            else if ((type == acf_type_t::null) && (t.type == acf_token_type::keyword_null))
            {
                acf_push_argument(&custom_value_result, acf_null());
                eat_token(lexer);
            }
            else if ((type == acf_type_t::boolean) && (t.type == acf_token_type::keyword_true))
            {
                acf_push_argument(&custom_value_result, acf_boolean(true));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::boolean) && (t.type == acf_token_type::keyword_false))
            {
                acf_push_argument(&custom_value_result, acf_boolean(false));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::integer) && (t.type == acf_token_type::integer))
            {
                acf_push_argument(&custom_value_result, acf_boolean(t.integer_value));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::string) && (t.type == acf_token_type::string))
            {
                string s = t.span;
                s.data += 1;
                s.size -= 2;
                acf_push_argument(&custom_value_result, acf_string(s));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::array) && (t.type == acf_token_type::bracket_open))
            {
                acf array_argument;
                bool success = parse_array(lexer, &array_argument, false);
                if (success)
                {
                    acf_push_argument(&custom_value_result, array_argument);
                }
                else
                {
                    osOutputDebugString("Cannot parse array argument!\n");
                    *lexer = checkpoint;
                    return false;
                }
            }
            else if ((type == acf_type_t::object) && (t.type == acf_token_type::brace_open))
            {
                acf object_argument;
                bool success = parse_object(lexer, &object_argument, false);
                if (success)
                {
                    acf_push_argument(&custom_value_result, object_argument);
                }
                else
                {
                    osOutputDebugString("Cannot parse object argument!\n");
                    *lexer = checkpoint;
                    return false;
                }
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

    *result = custom_value_result;

    return true;
}

bool parse_acf(string buffer, acf *result)
{
    acf_lexer lexer;
    initialize_acf_lexer(&lexer, buffer);

    bool success;
    do {
        success = parse_directive(&lexer);
    } while (success);

    success = parse_object(&lexer, result, true);
    return success;
}


// template <typename Allocator>
// acf create_scheme_from_acf_impl(acf const& value, Allocator *allocator, string *key = nullptr)
// {
//     switch (value.get_type())
//     {
//         case acf_type_t::null:
//         case acf_type_t::boolean:
//         case acf_type_t::integer:
//         case acf_type_t::floating:
//         case acf_type_t::string:
//         {
//             acf result;
//             {
//                 auto keys = make_dynamic_array<string>(allocator);
//                 auto values = make_dynamic_array<acf>(allocator);

//                 keys.reserve(3);
//                 values.reserve(3);

//                 if (key)
//                 {
//                     keys.push(Asuka::from_cstr("key"));
//                     values.push(*key);
//                 }

//                 keys.push(Asuka::from_cstr("type"));
//                 values.push(value.get_type_name());

//                 acf_object_type object_value = {};
//                 object_value.keys = make_array(keys);
//                 object_value.values = make_array(values);

//                 result.set_object(object_value);
//             }

//             return result;
//             // auto arr = allocate_array_<acf>(allocator, )
//             // return { {"key", key ? *key : son()}, {"type", value.type_name()}, {"values", {}} };
//         }
// //             return { {"key", key ? *key : son()}, {"type", value.type_name()}, {"values", {}} };
//         case acf_type_t::object:
//         {
//             acf result;
//             {
//                 auto keys = make_dynamic_array<string>(allocator);
//                 auto values = make_dynamic_array<acf>(allocator);

//                 keys.reserve(3);
//                 values.reserve(3);

//                 if (key)
//                 {
//                     keys.push(Asuka::from_cstr("key"));
//                     values.push(*key);
//                 }

//                 keys.push(Asuka::from_cstr("type"));
//                 values.push(value.get_type_name());

//                 auto object_value = value.get_object();

//                 auto values_array = make_dynamic_array<acf>(allocator);
//                 for (usize i = 0; i < object_value.keys.get_size(); i++)
//                 {
//                     auto& k = object_value.keys[i];
//                     auto& v = object_value.values[i];
//                     values_array.push(create_scheme_from_acf_impl(v, allocator, &k));
//                 }
//                 acf values_acf_array;
//                 values_acf_array.set_array(make_array(values_array));

//                 keys.push(Asuka::from_cstr("values"));
//                 values.push(values_acf_array);

//                 acf_object_type object_value__ = {};
//                 object_value__.keys = make_array(keys);
//                 object_value__.values = make_array(values);

//                 result.set_object(object_value__);
//             }

//             return result;
// //             son result = { {"key", key ? *key : son()}, {"type", value.type_name()} };
// //             son values;

// //             for (auto [k, v] : value.pairs()) {
// //                 values.push(create_scheme_from_son(v, &k));
// //             }

// //             result.push("values", values);
// //             return result;
//         }
//         case acf_type_t::array: {
//             acf result;
//             {
//                 auto keys = make_dynamic_array<string>(allocator);
//                 auto values = make_dynamic_array<acf>(allocator);

//                 keys.reserve(3);
//                 values.reserve(3);

//                 if (key)
//                 {
//                     keys.push(Asuka::from_cstr("key"));
//                     values.push(*key);
//                 }

//                 keys.push(Asuka::from_cstr("type"));
//                 values.push(value.get_type_name());

//                 auto array_value = value.get_array();

//                 auto values_array = make_dynamic_array<acf>(allocator);
//                 for (usize i = 0; i < array_value.get_size(); i++)
//                 {
//                     auto& v = array_value[i];
//                     values_array.push(create_scheme_from_acf_impl(v, allocator));
//                 }
//                 acf values_acf_array;
//                 values_acf_array.set_array(make_array(values_array));

//                 keys.push(Asuka::from_cstr("values"));
//                 values.push(values_acf_array);

//                 acf_object_type object_value__ = {};
//                 object_value__.keys = make_array(keys);
//                 object_value__.values = make_array(values);

//                 result.set_object(object_value__);
//             }
//             return result;
//             // return { {"key", key ? *key : son()}, {"type", value.type_name()}, {"values", {}} };
//         }
//         case acf_type_t::custom:
//         {
//             acf result;
//             {
//                 auto keys = make_dynamic_array<string>(allocator);
//                 auto values = make_dynamic_array<acf>(allocator);

//                 keys.reserve(3);
//                 values.reserve(3);

//                 if (key)
//                 {
//                     keys.push(Asuka::from_cstr("key"));
//                     values.push(*key);
//                 }

//                 auto custom_value = value.get_custom();

//                 keys.push(Asuka::from_cstr("type"));
//                 values.push(custom_value.name);

//                 acf_object_type object_value = {};
//                 object_value.keys = make_array(keys);
//                 object_value.values = make_array(values);

//                 auto values_array = make_dynamic_array<acf>(allocator);
//                 for (usize i = 0; i < custom_value.arguments.get_size(); i++)
//                 {
//                     auto& arg = custom_value.arguments[i];
//                     values_array.push(create_scheme_from_acf_impl(arg, allocator));
//                 }
//                 acf object_array__;
//                 object_array__.set_array(make_array(values_array));

//                 keys.push(Asuka::from_cstr("values"));
//                 values.push(object_array__);

//                 acf_object_type object_value__ = {};
//                 object_value__.keys = make_array(keys);
//                 object_value__.values = make_array(values);

//                 result.set_object(object_value__);
//             }
//             return result;
//         }
//     }

//     return acf();
// }


#endif // ACF_LIB_IMPLEMENTATION

#endif // ACF_LIB_HPP
