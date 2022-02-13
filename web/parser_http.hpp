#pragma once

#include <defines.hpp>
#include <string.hpp>
#include "http.hpp"


namespace http {

Request parse_request(string data);
Response parse_response(string data);




} // namespace http


#if UNITY_BUILD
#include "parser_http.cpp"
#endif // UNITY_BUILD
