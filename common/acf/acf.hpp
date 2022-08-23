#ifndef ACF_LIB_HPP
#define ACF_LIB_HPP

#include <defines.hpp>
#include <array.hpp>
#include <os/file.hpp>
#include <allocator.hpp>
#include <tprint.hpp>

#include <initializer_list>


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


GLOBAL memory::mallocator acf_mallocator = { "acf_alloc" };

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
        case acf_type_t::boolean: return "bool";
        case acf_type_t::integer: return "int";
        case acf_type_t::floating: return "float";
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

    using acf_type_value_t = acf_type_t;

    union acf_value_t
    {
        acf_boolean_t    boolean_value;
        acf_integer_t    integer_value;
        acf_floating_t   floating_value;
        acf_string_t     string_value;
        acf_array_t      array_value;
        acf_object_t     object_value;
        acf_custom_t     custom_value;
        acf_type_value_t type_value;
    };

    acf_type_t type;
    acf_value_t value;

    bool is_null() const noexcept { return (type == acf_type_t::null); }
    bool is_boolean() const noexcept { return (type == acf_type_t::boolean); }
    bool is_integer() const noexcept { return (type == acf_type_t::integer); }
    bool is_floating() const noexcept { return (type == acf_type_t::floating); }
    bool is_string() const noexcept { return (type == acf_type_t::string); }
    bool is_array() const noexcept { return (type == acf_type_t::array); }
    bool is_object() const noexcept { return (type == acf_type_t::object); }
    bool is_custom() const noexcept { return (type == acf_type_t::custom); }
    bool is_type() const noexcept { return (type == acf_type_t::type); }

    acf_boolean_t get_bool() const { ASSERT(is_boolean()); return value.boolean_value; }
    acf_integer_t get_int() const { ASSERT(is_integer()); return value.integer_value; }
    acf_floating_t get_float() const { ASSERT(is_floating()); return value.floating_value; }
    acf_string_t get_string() const { ASSERT(is_string()); return value.string_value; }
    acf_string_t get_string_or(acf_string_t fallback) const { if (is_string()) { return value.string_value; } else { return fallback; } }

    string get_custom_type_name() const
    {
        string result = {};
        if (is_custom())
        {
            result = value.custom_value.newtype_name;
        }
        return result;
    }

    STATIC
    acf null()
    {
        acf result = {};
        result.type = acf_type_t::null;

        return result;
    }

    STATIC
    acf custom(string name)
    {
        acf result = {};
        result.type = acf_type_t::custom;
        // @nocommit
        // result.value.custom_value.newtype_name = array<acf>::copy_from(&acf_mallocator, name);

        return result;
    }

    STATIC
    acf from()
    {
        return null();
    }

    STATIC
    acf from(bool value)
    {
        acf result = {};
        result.type = acf_type_t::boolean;
        result.value.boolean_value = value;

        return result;
    }

    template <typename T> STATIC
    acf from(T value)
    {
        acf result = {};

        if constexpr (type::is_integral<T>::value)
        {
            result.type = acf_type_t::integer;
            result.value.integer_value = (acf_integer_t) value;
        }
        else if constexpr (type::is_floating<T>::value)
        {
            result.type = acf_type_t::floating;
            result.value.floating_value = (acf_floating_t) value;
        }
        else
        {
            static_assert(false, "The value have to be of integral or floating type!");
        }

        return result;
    }

    STATIC
    acf from(char const* str)
    {
        acf result = {};
        result.type = acf_type_t::string;
        // @nocommit
        // result.value.string_value = copy_array(&acf_mallocator, string::from(str));

        return result;
    }

    STATIC
    acf from(string str)
    {
        acf result = {};
        result.type = acf_type_t::string;

        // @nocommit
        // result.value.string_value = copy_array(&acf_mallocator, str);

        return result;
    }

    STATIC
    acf from(acf_type_t value)
    {
        acf result = {};
        result.type = acf_type_t::type;
        result.value.type_value = value;

        return result;
    }

    // @note: this requires an allocation strategy figured out
    // @todo implement initializer list constructor to be able to create acfs with {}
    STATIC
    acf from(std::initializer_list<acf> const& init_list)
    {
        acf result = acf::null();

        bool is_an_object = true;
        for (auto it = init_list.begin(); it != init_list.end(); ++it)
        {
            acf v = *it;
            if (!(v.is_array() && v.size() == 2 && v[0].is_string()))
            {
                is_an_object = false;
                break;
            }
        }

        if (is_an_object)
        {
            for (auto it = init_list.begin(); it != init_list.end(); ++it)
            {
                acf const& v = *it;
                result.push(v[0].get_string(), v[1]);
            }
        }
        else
        {
            for (auto const& v : init_list) {
                result.push(v);
            }
        }

        return result;
    }

    acf operator [] (int32 index) const
    {
        if (is_array())
        {
            if (index < value.array_value.get_size())
            {
                return value.array_value[index];
            }
        }
        if (is_custom())
        {
            if (index < value.custom_value.newtype_arguments.get_size())
            {
                return value.custom_value.newtype_arguments[index];
            }
        }

        return acf::null();
    }

    acf operator [] (char const *key) const
    {
        if (is_object())
        {
            for (usize i = 0; i < value.object_value.keys.get_size(); i++)
            {
                auto& k = value.object_value.keys[i];
                if (k == key)
                {
                    return value.object_value.values[i];
                }
            }
        }

        return acf::null();
    }

    void push(acf v)
    {
        if (this->type == acf_type_t::null)
        {
            this->type = acf_type_t::array;
            this->value.array_value = allocate_array<acf>(&acf_mallocator, 4);
        }

        if (this->type == acf_type_t::array)
        {
            auto& arr = this->value.array_value;
            if (arr.size == arr.capacity)
            {
                array<acf> new_array = allocate_array<acf>(&acf_mallocator, (arr.capacity + 1) * 2);
                for (auto& t : arr)
                {
                    new_array.push(t);
                }
                deallocate_array(&acf_mallocator, arr);
                arr = new_array;
            }

            arr.push(v);
        }
    }

    void push(char const *k, acf v)
    {
        // @nocommit
        // push(string::from(k), v);
    }

    void push(string k, acf v)
    {
        if (this->type == acf_type_t::null)
        {
            this->type = acf_type_t::object;
            this->value.object_value.keys = allocate_array<string>(&acf_mallocator, 4);
            this->value.object_value.values = allocate_array<acf>(&acf_mallocator, 4);
        }

        if (this->type == acf_type_t::object)
        {
            auto& keys   = this->value.object_value.keys;
            auto& values = this->value.object_value.values;

            ASSERT(keys.size == values.size); // Invariant!
            ASSERT(keys.capacity == values.capacity); // Invariant!

            if (keys.size == keys.capacity)
            {
                array<string> new_keys = allocate_array<string>(&acf_mallocator, (keys.capacity + 1) * 2);
                for (auto& a : keys)
                {
                    new_keys.push(a);
                }
                if (!keys.is_empty())
                {
                    deallocate_array(&acf_mallocator, keys);
                }
                keys = new_keys;

                array<acf> new_values = allocate_array<acf>(&acf_mallocator, (values.capacity + 1) * 2);
                for (auto& t : values)
                {
                    new_values.push(t);
                }
                if (!values.is_empty())
                {
                    deallocate_array(&acf_mallocator, values);
                }
                values = new_values;
            }

            keys.push(copy_string(&acf_mallocator, k));
            values.push(v);
        }
    }

    void push_argument(acf arg)
    {
        if (this->type == acf_type_t::custom)
        {
            auto& arguments = this->value.custom_value.newtype_arguments;
            if (arguments.size == arguments.capacity)
            {
                array<acf> new_args = allocate_array<acf>(&acf_mallocator, (arguments.capacity + 1) * 2);
                for (auto& a : arguments)
                {
                    new_args.push(a);
                }
                if (!arguments.is_empty())
                {
                    deallocate_array(&acf_mallocator, arguments);
                }
                arguments = new_args;
            }

            arguments.push(arg);
        }
    }

    void dispose()
    {
        switch (type)
        {
            case acf_type_t::string:
            {
                if (!value.string_value.is_empty())
                {
                    // @nocommit
                    // deallocate_array(&acf_mallocator, value.string_value);
                }
            }
            break;

            case acf_type_t::array:
            {
                for (usize i = 0; i < value.array_value.get_size(); i++)
                {
                    acf& v = value.array_value[i];
                    v.dispose();
                }
                if (!value.array_value.is_empty())
                {
                    deallocate_array(&acf_mallocator, value.array_value);
                }
            }
            break;

            case acf_type_t::object:
            {
                for (usize i = 0; i < value.object_value.keys.get_size(); i++)
                {
                    auto& k = value.object_value.keys[i];
                    if (!k.is_empty())
                    {
                        // @nocommit
                        // deallocate_array(&acf_mallocator, k);
                    }
                }
                if (!value.object_value.keys.is_empty())
                {
                    deallocate_array(&acf_mallocator, value.object_value.keys);
                }

                for (usize i = 0; i < value.object_value.values.get_size(); i++)
                {
                    auto& v = value.object_value.values[i];
                    v.dispose();
                }
                if (!value.object_value.values.is_empty())
                {
                    deallocate_array(&acf_mallocator, value.object_value.values);
                }
            }
            break;

            case acf_type_t::custom:
            {
                if (!value.custom_value.newtype_name.is_empty())
                {
                    // @nocommit
                    // deallocate_array(&acf_mallocator, value.custom_value.newtype_name);
                }
                for (usize i = 0; i < value.custom_value.newtype_arguments.get_size(); i++)
                {
                    auto& v = value.custom_value.newtype_arguments[i];
                    v.dispose();
                }
                if (!value.custom_value.newtype_arguments.is_empty())
                {
                    deallocate_array(&acf_mallocator, value.custom_value.newtype_arguments);
                }
            }
            break;
        }

        type = acf_type_t::null;
        value = {};
    }

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

private:
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

public:
    object_iterator_wrapper pairs()
    {
        object_iterator_wrapper it = {};
        it.host = &this->value.object_value;

        return it;
    }

    usize size() const
    {
        switch (type)
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
                return value.array_value.get_size();
            case acf_type_t::object:
                return value.object_value.keys.get_size();
            case acf_type_t::custom:
                return value.custom_value.newtype_arguments.get_size();
            default:
                INVALID_CODE_PATH();
                return 0;
        }
    }

    usize depth_size() const
    {
        switch (type)
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
                for (auto& v : value.array_value)
                {
                    n += v.depth_size();
                }
                return n;
            }
            case acf_type_t::object:
            {
                usize n = 0;
                for (auto& v : value.object_value.values)
                {
                    n += v.depth_size() + 1; // @note: +1 for the key
                }
                return n;
            }
            case acf_type_t::custom:
                return value.custom_value.newtype_arguments.get_size() + 1;
            default:
                INVALID_CODE_PATH();
                return 0;
        }
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

char const* spaces = "                                                            ";
char const *carets = "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^";

void acf_print_impl(acf value, acf_print_options options, int depth = 0)
{
    switch (value.type)
    {
        case acf_type_t::null: osOutputDebugString("null"); break;
        case acf_type_t::boolean: osOutputDebugString("%s", value.value.boolean_value ? "true" : "false"); break;
        case acf_type_t::integer: osOutputDebugString("%lld", value.value.integer_value); break;
        case acf_type_t::floating: osOutputDebugString("%lf", value.value.floating_value); break;
        case acf_type_t::string: osOutputDebugString("\"%.*s\"", STRING_PRINT_(value.value.string_value)); break;

        case acf_type_t::object:
        {
            bool in_one_line = (options.multiline == acf_print_options::multiline_t::smart && value.depth_size() <= options.max_elements_in_line)
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
            bool in_one_line = (options.multiline == acf_print_options::multiline_t::smart && value.depth_size() <= options.max_elements_in_line)
                || options.multiline == acf_print_options::multiline_t::disabled;

            osOutputDebugString("[%s", in_one_line ? value.size() > 0 ? " " : "" : "\n");
            depth += 1;

            int i = 0;
            for (auto& v : value)
            {
                if (!in_one_line) { osOutputDebugString("%.*s", options.indent * depth, spaces); }

                acf_print_impl(v, options, depth);

                osOutputDebugString("%s%s",
                    (options.print_commas && ((i + 1) < value.size())) ? "," : "",
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
            usize i = 0;
            usize n = value.value.custom_value.newtype_arguments.get_size();
            for (auto& arg : value.value.custom_value.newtype_arguments)
            {
                acf_print_options sub_options = options;
                sub_options.multiline = acf_print_options::multiline_t::disabled;
                acf_print_impl(arg, sub_options, depth);

                if ((i + 1) < n) { osOutputDebugString(", "); }
                i += 1;
            }
            osOutputDebugString(")");
        }
        break;

        case acf_type_t::type:
        {
            osOutputDebugString("%s", get_acf_type_string(value.value.type_value));
        }
        break;
    }
}

void acf_print(acf value, acf_print_options options = acf_print_options())
{
    acf_print_impl(value, options, 0);
}

template <>
void tprint_helper<acf>(acf value)
{
    acf_print(value);
}

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
    int32 index;

    int32 line;
    int32 column;

    acf_token next_token;
    bool next_token_valid;

    u32 keyword_count;
    string keywords[32];
    acf_token_type keyword_types[32];

    u32 newtype_count;
    string newtype_names[32];
    acf_constructor_arguments newtype_arguments[32];

    int32 error_buffer_size;
    char error_buffer[2048];

    struct parse_line
    {
        int32 start_index;
        int32 line_number;
        int32 length;
    };

    parse_line current_line;
};

INTERNAL
void register_acf_keyword(acf_lexer *lexer, char const *keyword, acf_token_type type)
{
    // @nocommit
    // lexer->keywords[lexer->keyword_count] = string::from(keyword);
    lexer->keyword_types[lexer->keyword_count] = type;

    lexer->keyword_count += 1;
}

INTERNAL
void register_acf_new_type(acf_lexer *lexer, string type_name, acf_constructor_arguments args)
{
    lexer->newtype_names[lexer->newtype_count] = type_name;
    lexer->newtype_arguments[lexer->newtype_count] = args;
    lexer->newtype_count += 1;
}

INTERNAL
void register_acf_new_type(acf_lexer *lexer, char const *type_name, acf_constructor_arguments args)
{
    // @nocommit
    // register_acf_new_type(lexer, string::from(type_name), args);
}

INTERNAL
bool is_newtype_registered(acf_lexer *lexer, string s, acf_constructor_arguments *args)
{
    for (uint32 newtype_index = 0; newtype_index < lexer->newtype_count; newtype_index++)
    {
        string type_name = lexer->newtype_names[newtype_index];
        if (s == type_name)
        {
            if (args)
            {
                *args = lexer->newtype_arguments[newtype_index];
            }
            return true;
        }
    }

    return false;
}

INTERNAL
acf_lexer::parse_line lexer_start_line(acf_lexer *lexer)
{
    acf_lexer::parse_line result = {};
    result.start_index = lexer->index;
    result.line_number = lexer->line;

    // Count the length of the line
    int32 index = lexer->index;
    while (index < lexer->buffer.get_size() && lexer->buffer[index] != '\n' && lexer->buffer[index] != 0)
    {
        result.length += 1;
        index += 1;
    }

    return result;
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
    lexer->newtype_count = 0;
    memory::set(lexer->newtype_names, 0, sizeof(lexer->newtype_names));
    memory::set(lexer->newtype_arguments, 0, sizeof(lexer->newtype_arguments));
    lexer->error_buffer_size = 0;
    memory::set(lexer->error_buffer, 0, sizeof(lexer->error_buffer));

    lexer->current_line = lexer_start_line(lexer);

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

        lexer->current_line = lexer_start_line(lexer);
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

            i64 sign = 1;
            i64 integral_part = 0;
            f64 fractional_part = 0;

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

                integral_part *= 10;
                integral_part += (c - '0');
            }

            if (c == '.')
            {
                eat_char(lexer);

                f64 multiplier = 1;
                while(is_digit(c = get_char(lexer)))
                {
                    eat_char(lexer);
                    multiplier *= 0.1;
                    fractional_part += (multiplier * (c - '0'));
                }

                char *span_end = get_char_pointer(lexer);
                usize span_size = ((usize)span_end - (usize)span_start);

                token.type = acf_token_type::floating;
                token.floating_value = sign * ((float64)integral_part + fractional_part);
                token.span = make_string(span_start, span_size);
            }
            else
            {
                char *span_end = get_char_pointer(lexer);
                usize span_size = ((usize)span_end - (usize)span_start);

                token.type = acf_token_type::integer;
                token.integer_value = sign * integral_part;
                token.span = make_string(span_start, span_size);
            }
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


void acf_parser_report_error(acf_lexer *lexer, char const* message, ...)
{
    va_list args;
    va_start(args, message);
    int32 count = vsprintf(lexer->error_buffer + lexer->error_buffer_size, message, args);
    if (count > 0)
    {
        lexer->error_buffer_size += count;
    }
    va_end(args);
}


void acf_parser_highlight_token(acf_lexer *lexer, acf_token t)
{
    int32 count = snprintf(lexer->error_buffer + lexer->error_buffer_size,
        ARRAY_COUNT(lexer->error_buffer) - lexer->error_buffer_size,
        "\n"
        "%.*s\n"
        "%.*s%.*s\n",
        (int) lexer->current_line.length, lexer->buffer.get_data() + lexer->current_line.start_index,
        (int) t.column - 1, spaces,
        (int) t.span.get_size(), carets
    );
}


bool parse_value(acf_lexer *lexer, acf *result);
bool parse_key_value_pair(acf_lexer *lexer, string *key, acf *value);
bool parse_array(acf_lexer *lexer, acf *result);
bool parse_object(acf_lexer *lexer, acf *result, bool braces_optional = false);
bool parse_constructor_call(acf_lexer *lexer, acf *result);


bool parse_constructor_call(acf_lexer *lexer, acf *result)
{
    auto name_token = eat_token(lexer);
    if (name_token.type != acf_token_type::identifier)
    {
        acf_parser_report_error(lexer, "Constructor call should start with the registered type name.\n");
        acf_parser_highlight_token(lexer, name_token);
        return false;
    }

    acf custom_value_result = acf::custom(name_token.span);
    acf_constructor_arguments args;
    if (is_newtype_registered(lexer, name_token.span, &args))
    {
        auto paren_open_token = eat_token(lexer);
        if (paren_open_token.type != acf_token_type::parentheses_open)
        {
            acf_parser_report_error(lexer, "Constructor call have to have parentheses around arguments, even if number of them is 0.\n");
            acf_parser_highlight_token(lexer, paren_open_token);
            return false;
        }

        for (uint32 argument_index = 0; argument_index < args.argument_count; argument_index++)
        {
            acf_type_t type = args.arguments[argument_index];
            acf_token t = get_token(lexer);

            if (t.type == acf_token_type::parentheses_close)
            {
                // Unexpected end of arguments.
                acf_parser_report_error(lexer, "Argument count mismatch! Expected %d arguments, got %d.\n", args.argument_count, argument_index);
                acf_parser_highlight_token(lexer, t);
                return false;
            }
            else if ((type == acf_type_t::null) && (t.type == acf_token_type::keyword_null))
            {
                custom_value_result.push_argument(acf::null());
                eat_token(lexer);
            }
            else if ((type == acf_type_t::boolean) && (t.type == acf_token_type::keyword_true))
            {
                custom_value_result.push_argument(acf::from(true));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::boolean) && (t.type == acf_token_type::keyword_false))
            {
                custom_value_result.push_argument(acf::from(false));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::integer) && (t.type == acf_token_type::integer))
            {
                custom_value_result.push_argument(acf::from(t.integer_value));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::floating) && (t.type == acf_token_type::floating))
            {
                custom_value_result.push_argument(acf::from(t.floating_value));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::floating) && (t.type == acf_token_type::integer))
            {
                custom_value_result.push_argument(acf::from((f64) t.integer_value));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::string) && (t.type == acf_token_type::string))
            {
                string s = t.span;
                s.data += 1;
                s.size -= 2;
                custom_value_result.push_argument(acf::from(s));
                eat_token(lexer);
            }
            else if ((type == acf_type_t::array) && (t.type == acf_token_type::bracket_open))
            {
                acf array_argument;
                bool success = parse_array(lexer, &array_argument);
                if (success)
                {
                    custom_value_result.push_argument(array_argument);
                }
                else
                {
                    return false;
                }
            }
            else if ((type == acf_type_t::object) && (t.type == acf_token_type::brace_open))
            {
                acf object_argument;
                bool success = parse_object(lexer, &object_argument, false);
                if (success)
                {
                    custom_value_result.push_argument(object_argument);
                }
                else
                {
                    return false;
                }
            }
            else if (type == acf_type_t::type)
            {
                if (t.type == acf_token_type::keyword_null)   { custom_value_result.push_argument(acf::from(acf_type_t::null)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_bool)   { custom_value_result.push_argument(acf::from(acf_type_t::boolean)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_int)    { custom_value_result.push_argument(acf::from(acf_type_t::integer)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_float)  { custom_value_result.push_argument(acf::from(acf_type_t::floating)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_string) { custom_value_result.push_argument(acf::from(acf_type_t::string)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_array)  { custom_value_result.push_argument(acf::from(acf_type_t::array)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_object) { custom_value_result.push_argument(acf::from(acf_type_t::object)); eat_token(lexer); }
                if (t.type == acf_token_type::keyword_type)   { custom_value_result.push_argument(acf::from(acf_type_t::type)); eat_token(lexer); }
            }
            else
            {
                acf_parser_report_error(lexer, "Argument type mismatch! Constructor call expected value of type %s, but got %s.\n",
                    get_acf_type_string(type),
                    get_acf_token_type_string(t.type));
                acf_parser_highlight_token(lexer, t);
                return false;
            }

            if (argument_index + 1 < args.argument_count) // Do not support trailing comma in function calls.
            {
                // Consume required comma, if present.
                auto comma_token = eat_token(lexer);
                if (comma_token.type != acf_token_type::comma)
                {
                    acf_parser_report_error(lexer, "Expected ','\n");
                    acf_parser_highlight_token(lexer, comma_token);
                    return false;
                }
            }
        }

        auto paren_close_token = eat_token(lexer);
        if (paren_close_token.type != acf_token_type::parentheses_close)
        {
            acf_parser_report_error(lexer, "Expected ')'\n");
            acf_parser_highlight_token(lexer, paren_close_token);
            return false;
        }
    }
    else
    {
        acf_parser_report_error(lexer, "Referencing an identifier '%.*s', which was not declared before!\n", STRING_PRINT_(name_token.span));
        acf_parser_highlight_token(lexer, name_token);
        return false;
    }

    *result = custom_value_result;
    return true;
}


bool parse_array(acf_lexer *lexer, acf *result)
{
    auto open_bracket_token = eat_token(lexer);
    if (open_bracket_token.type != acf_token_type::bracket_open)
    {
        acf_parser_report_error(lexer, "Array should always start with the '['\n");
        return false;
    }

    acf array_value = acf::null();
    while (true)
    {
        auto t = get_token(lexer);
        if (t.type == acf_token_type::bracket_close)
        {
            break;
        }

        acf value;
        bool success = parse_value(lexer, &value);
        if (success)
        {
            array_value.push(value);
        }
        else
        {
            // The error is contained from the 'parse_value' call.
            return false;
        }

        auto comma = get_token(lexer);
        if (comma.type == acf_token_type::comma)
        {
            // Consume optional comma, if present, including trailing comma.
            eat_token(lexer);
        }
    }

    auto close_bracket_token = eat_token(lexer);
    if (close_bracket_token.type != acf_token_type::bracket_close)
    {
        acf_parser_report_error(lexer, "Expected ']'\n");
        acf_parser_highlight_token(lexer, close_bracket_token);
        return false;
    }

    *result = array_value;
    return true;
}


bool parse_value(acf_lexer *lexer, acf *result)
{
    acf_token t = get_token(lexer);
    switch (t.type)
    {
        case acf_token_type::keyword_null:
        {
            *result = acf::null();
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_true:
        {
            *result = acf::from(true);
            eat_token(lexer);
        }
        break;

        case acf_token_type::keyword_false:
        {
            *result = acf::from(false);
            eat_token(lexer);
        }
        break;

        case acf_token_type::integer:
        {
            *result = acf::from(t.integer_value);
            eat_token(lexer);
        }
        break;

        case acf_token_type::floating:
        {
            *result = acf::from(t.floating_value);
            eat_token(lexer);
        }
        break;

        case acf_token_type::identifier:
        {
            acf custom_value;
            bool success = parse_constructor_call(lexer, &custom_value);
            if (success)
            {
                *result = custom_value;
            }
            else
            {
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

            *result = acf::from(s);
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
                return false;
            }
        }
        break;

        case acf_token_type::keyword_bool:    *result = acf::from(acf_type_t::boolean);  eat_token(lexer); break;
        case acf_token_type::keyword_int:     *result = acf::from(acf_type_t::integer);  eat_token(lexer); break;
        case acf_token_type::keyword_float:   *result = acf::from(acf_type_t::floating); eat_token(lexer); break;
        case acf_token_type::keyword_string:  *result = acf::from(acf_type_t::string);   eat_token(lexer); break;
        case acf_token_type::keyword_array:   *result = acf::from(acf_type_t::array);    eat_token(lexer); break;
        case acf_token_type::keyword_object:  *result = acf::from(acf_type_t::object);   eat_token(lexer); break;

        default:
        {
            acf_parser_report_error(lexer, "Expected value but got something else <INSERT SOMETHING ELSE HERE>\n");
            acf_parser_highlight_token(lexer, t);
            return false;
        }
    }

    return true;
}


bool parse_key_value_pair(acf_lexer *lexer, string *key, acf *value)
{
    auto identifier_token = eat_token(lexer);
    if (identifier_token.type != acf_token_type::identifier)
    {
        acf_parser_report_error(lexer, "Expected identifier for key-value pair, but got %s.\n", get_acf_token_type_string(identifier_token.type));
        acf_parser_highlight_token(lexer, identifier_token); // @todo: When this is the EOF, it produces just an '\n' in the error_buffer.
        return false;
    }

    auto equals_token = eat_token(lexer);
    if (equals_token.type != acf_token_type::equals)
    {
        acf_parser_report_error(lexer, "Expected '='\n");
        acf_parser_highlight_token(lexer, equals_token);
        return false;
    }

    *key = identifier_token.span;
    bool success = parse_value(lexer, value);
    return success;
}


bool parse_object(acf_lexer *lexer, acf *result, bool braces_optional)
{
    acf_token open_brace_token = get_token(lexer);
    if (open_brace_token.type == acf_token_type::brace_open)
    {
        eat_token(lexer);

        // Ensure there is closing brace to this open brace.
        braces_optional = false;
    }
    else
    {
        if (open_brace_token.type != acf_token_type::identifier)
        {
            acf_parser_report_error(lexer, "Presumably, top-level braces are omitted, but then identifier is expected here:\n");
            acf_parser_highlight_token(lexer, open_brace_token);
            return false;
        }

        if (!braces_optional)
        {
            acf_parser_report_error(lexer, "Braces are required by the caller site, but the '%s' is found.\n", get_acf_token_type_string(open_brace_token.type));
            acf_parser_highlight_token(lexer, open_brace_token);
            return false;
        }
    }

    acf object_result = acf::null();
    while (true)
    {
        auto t = get_token(lexer);
        if ((t.type == acf_token_type::brace_close) ||
            (braces_optional && t.type == acf_token_type::end_of_file))
        {
            break; // It's ok!
        }

        string key;
        acf value;

        // If this returns an error, we know it is a legitimate error!
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
                acf_parser_report_error(lexer, "Key '%.*s' already defined in this object.\n", STRING_PRINT_(key));
                return false;
            }
            else
            {
                object_result.push(key, value);
            }
        }
        else
        {
            // The error should be stored from the parse_key_value_pair call.
            return false;
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
            acf_parser_report_error(lexer, "Closing brackets required, but encountered %s here:\n", get_acf_token_type_string(close_brace_token.type));
            acf_parser_highlight_token(lexer, close_brace_token);
            return false;
        }
    }

    *result = object_result;
    return true;
}


INTERNAL
bool parse_type(acf_lexer *lexer, acf_type_t *type)
{
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
        case acf_token_type::keyword_type:   { *type = acf_type_t::type; } break;
        default:
        {
            // @note: Now I forbid nested custom types. They will overcomplicate matters
            // and what I want to achieve is possible without them anyway.
            acf_parser_report_error(lexer, "Expected one of the basic types here:\n");
            acf_parser_highlight_token(lexer, type_name);
            return false;
        }
    }

    return true;
}

bool parse_directive(acf_lexer *lexer)
{
    auto pound_token = eat_token(lexer);
    if (pound_token.type != acf_token_type::pound)
    {
        acf_parser_report_error(lexer, "Expected '#', but got %s\n", get_acf_token_type_string(pound_token.type));
        acf_parser_highlight_token(lexer, pound_token);
        return false;
    }

    auto directive_token = eat_token(lexer);
    if (directive_token.type != acf_token_type::identifier)
    {
        acf_parser_report_error(lexer, "There should be an identifier after '#'\n");
        acf_parser_highlight_token(lexer, directive_token);
        return false;
    }

    if (directive_token.span != "newtype")
    {
        acf_parser_report_error(lexer, "Currently the only supported directive is 'newtype', but '%.*s' is given\n", STRING_PRINT_(directive_token.span));
        acf_parser_highlight_token(lexer, directive_token);
        return false;
    }

    auto name_token = eat_token(lexer);
    if (name_token.type != acf_token_type::identifier)
    {
        acf_parser_report_error(lexer, "You should specify name of your type here:\n");
        acf_parser_highlight_token(lexer, name_token);
        return false;
    }

    auto open_paren_token = eat_token(lexer);
    if (open_paren_token.type != acf_token_type::parentheses_open)
    {
        acf_parser_report_error(lexer, "Expected '('\n");
        acf_parser_highlight_token(lexer, open_paren_token);
        return false;
    }

    acf_constructor_arguments args = {};
    while (true)
    {
        auto t = get_token(lexer);
        if (t.type == acf_token_type::parentheses_close)
        {
            break; // Ok return.
        }

        acf_type_t arg_type;
        bool success = parse_type(lexer, &arg_type);
        if (success)
        {
            if (args.argument_count < ARRAY_COUNT(args.arguments))
            {
                args.arguments[args.argument_count++] = arg_type;
            }
        }
        else
        {
            // If we encountered an error here, we know for sure that it is legitimate error.
            return false;
        }

        acf_token comma = get_token(lexer);
        if (comma.type == acf_token_type::comma)
        {
            // Consume optional comma, if present.
            eat_token(lexer);
        }
    }

    auto close_paren_token = eat_token(lexer);
    if (close_paren_token.type != acf_token_type::parentheses_close)
    {
        acf_parser_report_error(lexer, "Expected '('\n");
        acf_parser_highlight_token(lexer, open_paren_token);
        return false;
    }

    register_acf_new_type(lexer, name_token.span, args);

    return true;
}


bool parse_acf(string buffer, acf *result)
{
    if (!buffer.is_valid())
    {
        *result = acf::null();
        return false;
    }

    acf_lexer lexer;
    initialize_acf_lexer(&lexer, buffer);

    bool success = true;
    do {
        auto t = get_token(&lexer);
        if (t.type == acf_token_type::pound) {
            auto checkpoint = lexer;
            success = parse_directive(&lexer);
            if (!success)
            {
                success = false;
                break;
            }
        }
        else
        {
            break;
        }
    } while (success);

    if (success)
    {
        success = parse_object(&lexer, result, true);
    }

    if (!success)
    {
        osOutputDebugString("%s\n", lexer.error_buffer);
    }

    return success;
}

#endif // ACF_LIB_IMPLEMENTATION
#endif // ACF_LIB_HPP
