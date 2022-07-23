#pragma once

#include <defines.hpp>
#include "vector4.hpp"


struct matrix4
{
    union
    {
        struct
        {
            f32 _11, _12, _13, _14;
            f32 _21, _22, _23, _24;
            f32 _31, _32, _33, _34;
            f32 _41, _42, _43, _44;
        };
        v4 row[4];
        f32 e[4][4];
    };

    STATIC matrix4 identity;
};

using m4 = matrix4;


#if UNITY_BUILD
#include "matrix4.cpp"
#endif // UNITY_BUILD
