#ifndef SON_LIB_HPP
#define SON_LIB_HPP

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


namespace Asuka
{

struct son
{
    enum class type
    {
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
    // using string_t = asuka::string;
    // using object_t = std::vector<std::pair<std::string, son>>;
    // using array_t = std::vector<son>;

    type m_type;
    union
    {
        boolean_t  m_boolean;
        integer_t  m_integer;
        floating_t m_floating;
        // string_t   m_string;
        // object_t   m_object;
        // array_t    m_array;
    };

public:
    ~son();

    son(); // Contructor for null object.
    son(type t); // Default value of that type.
    // son(boolean_t v);
    // son(integer_t v);
    // son(int v) : son(static_cast<integer_t>(v)) {}
    // son(floating_t v);
    // son(const char* s);
    // son(string_t s);
    // son(std::initializer_list<son>);

    // void swap(son& other);

    // son(const son& other);
    // son(son&& other);

    // son& operator=(const son& other);
    // son& operator=(son&& other);

    // son &operator=(bool value);

    type get_type() const { return m_type; }

    bool is_null()     const { return m_type == type::null; }
    bool is_boolean()  const { return m_type == type::boolean; }
    bool is_integer()  const { return m_type == type::integer; }
    bool is_floating() const { return m_type == type::floating; }
    bool is_string()   const { return m_type == type::string; }
    bool is_object()   const { return m_type == type::object; }
    bool is_array()    const { return m_type == type::array; }

    // bool get_boolean() const { ASSERT(is_boolean()); return m_boolean; }
    // integer_t get_integer() const { ASSERT(is_integer()); return m_integer; }
    // floating_t get_floating() const { ASSERT(is_floating()); return m_floating; }
    // string_t get_string() const { ASSERT(is_string()); return *(string_t*)m_value.storage_; }
};

#ifdef SON_LIB_IMPLEMENTATION

    namespace Internal
    {
        // @todo: Parsing

        enum TokenType
        {

        };

        struct Token
        {
            TokenType type;

            u32 line;
            u32 column;
        };

        struct SonLexer
        {
            Token
        };
    } // namespace Internal

#endif // SON_LIB_IMPLEMENTATION

} // namespace Asuka

#endif // SON_LIB_HPP
