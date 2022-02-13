#pragma once


http::Response serve_favicon(http::Request request);


#if UNITY_BUILD
#include "user_controllers.cpp"
#endif
