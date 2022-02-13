#include "http.hpp"
#include <os/file.hpp>


using http::Request;
using http::Response;

Response serve_favicon(Request request) {
    static string body {};

    if (is_empty(body)) {
        body = os::load_entire_file("favicon.ico");
    }


    Response result;
    result.code = http::response_code(200);
    result.body = body;

    return result;
}
