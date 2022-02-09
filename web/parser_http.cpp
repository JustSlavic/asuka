#include "parser_http.hpp"
#include <lexer.hpp>


namespace http {


void advance(string *buffer, int n = 1) {
    if (n > buffer->size) {
        buffer->data = 0;
        buffer->size = 0;
    } else {
        buffer->data += n;
        buffer->size -= n;
    }
}


// str have to be null-terminated c-string.
bool32 check_next_string(string buffer, char *str) {
    bool32 equal = true;
    while ((*buffer.data != 0) && (buffer.size > 0) && (*str != 0)) {
        if (*buffer.data != *str) {
            equal = false;
            break;
        }

        advance(&buffer);
        str += 1;
    }

    if (equal && (((*buffer.data == 0) || (buffer.size > 0)) && (*str != 0))) {
        // Buffer string ended, while passed str string did not.
        equal = false;
    }

    return equal;
}


typedef bool32 predicate(char c);


bool32 is_space(char c) {
    bool32 result = (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n');
    return result;
}

bool32 is_newline(char c) {
    bool32 result = (c == '\r') || (c == '\n');
    return result;
}


string consume_while(string *buffer, predicate *f) {
    string result {};
    result.data = buffer->data;

    while (buffer->size > 0) {
        char c = *buffer->data;
        if (f(c)) {
            advance(buffer);
            result.size += 1;
        } else {
            break;
        }
    }

    return result;
}


string consume_until(string *buffer, predicate *f) {
    string result {};
    result.data = buffer->data;

    while (buffer->size > 0) {
        char c = *buffer->data;
        if (f(c)) {
            break;
        } else {
            advance(buffer);
            result.size += 1;
        }
    }

    return result;
}


void consume_spaces(string *buffer) {
    consume_while(buffer, is_space);
}


Request parse_request(string data) {
    Request request {};

    bool32 is_get = check_next_string(data, "GET");
    if (is_get) {
        advance(&data, 3);
        request.method = HTTP_REQUEST_GET;
    } else {
        bool32 is_post = check_next_string(data, "POST");
        if (is_post) {
            request.method = HTTP_REQUEST_POST;
            advance(&data, 4);
        } else {
            printf("Could not parse HTTP request!\n");
            return Request{};
        }
    }

    consume_spaces(&data);

    string path = consume_until(&data, [](char c) -> bool32 { return c == ' '; });
    request.path = path;
    
    printf("Parsed path: '%.*s'\n", (int)path.size, path.data);

    consume_spaces(&data);

    bool32 is_http = check_next_string(data, "HTTP");
    if (!is_http) {
        printf("HTTP protocol header not recognized.\n");
        return Request{};
    }

    advance(&data, 4);

    bool32 is_slash = check_next_string(data, "/");
    if (!is_slash) {
        printf("Expected '/' followed by the version of protocol.\n");
        return Request{};
    }

    advance(&data);

    // @todo: Parse version.
    string version = consume_until(&data, is_newline);
    printf("This is HTTP version %.*s.\n", (int)version.size, version.data);

    while (data.size > 0) {
        {
            // @note: Here I try to consume exactly one new line (\r\n or \n)
            char c = *data.data;
            if (c == '\r') {
                advance(&data);
            }
            c = *data.data;
            if (c == '\n') {
                advance(&data);
            }
        }

        char c = *data.data;
        if (c == 0 || is_newline(c)) {
            break;
        }

        string header_key = consume_until(&data, [](char c) -> bool32 { return c == ':'; });
        advance(&data); // eat ':'

        consume_spaces(&data);

        string header_value = consume_until(&data, is_newline);

        printf("HEADER: '%.*s' => '%.*s'\n",
            (int)header_key.size, header_key.data,
            (int)header_value.size, header_value.data);
    }

    consume_spaces(&data);

    string body = data;
    printf("BODY:\n%.*s\nBODY END\n\n", (int)body.size, body.data);

    request.body = body;
    return request;
}


} // namespace http
