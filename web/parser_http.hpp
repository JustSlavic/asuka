#pragma once

#include <defines.hpp>
#include <string.hpp>


namespace http {


enum Method {
    HTTP_REQUEST_NONE = 0,
    HTTP_REQUEST_GET,
    HTTP_REQUEST_POST,
    HTTP_REQUEST_DELETE,
    // @todo: Check if there are more HTTP methods.
};


enum ResponseCode {
    HTTP_RESPONSE_NONE = 0,

    HTTP_RESPONSE_OK = 200,
    HTTP_RESPONSE_NOT_FOUND = 404,
    HTTP_RESPONSE_INTERNAL_SERVER_ERROR = 500,

    // @todo: More response codes!
};


struct Request {
    Method method;
    int32 version_major;
    int32 version_minor;
    // @todo: Parse path right away? Probably into a hash map?
    string path;
    // @todo: Header fields. Probably hash map?
    string body;
};


struct Response {

};


Request parse_request(string data);
Response parse_response(string data);


} // namespace http


#if UNITY_BUILD
#include "parser_http.cpp"
#endif // UNITY_BUILD
