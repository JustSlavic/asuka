#include "http.hpp"


namespace http {


string serialize_request(Request request) {
    // return request.body;
    string result {};
    return result;
}


string serialize_response(Response response) {
    string result = response.body;
    return result;
}


} // http
