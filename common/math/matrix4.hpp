#pragma once

#include <defines.hpp>
#include "vector4.hpp"


struct matrix4
{
    union
    {
        struct
        {
            float32 _11, _12, _13, _14;
            float32 _21, _22, _23, _24;
            float32 _31, _32, _33, _34;
            float32 _41, _42, _43, _44;
        };
        struct
        {
            vector4 sx, sy, sz, sw;
        };
        vector4 row[4];
        float32 e[4][4];
    };

    STATIC CONSTANT
    matrix4 identity;

    float32 *get_data() { return &_11; }
    float32 const *get_data() const { return &_11; }
};


using m4 = matrix4;


matrix4 make_matrix4(f32 _11, f32 _12, f32 _13, f32 _14,
                     f32 _21, f32 _22, f32 _23, f32 _24,
                     f32 _31, f32 _32, f32 _33, f32 _34,
                     f32 _41, f32 _42, f32 _43, f32 _44)
{
    matrix4 result;
    result.sx = { _11, _12, _13, _14 };
    result.sy = { _21, _22, _23, _24 };
    result.sz = { _31, _32, _33, _34 };
    result.sw = { _41, _42, _43, _44 };
    return result;
}


void transpose(matrix4& m)
{
    SWAP(m._12, m._21);
    SWAP(m._13, m._31);
    SWAP(m._14, m._41);
    SWAP(m._23, m._32);
    SWAP(m._24, m._42);
    SWAP(m._34, m._43);
}

matrix4 transposed(matrix4 m)
{
    transpose(m);
    return m;
}

matrix4 make_projection_matrix(float32 w, float32 h, float32 n, float32 f)
{
    matrix4 projection = {};

    projection._11 = 2.0f * n / w;
    projection._22 = 2.0f * n / h;
    projection._33 = -(f + n) / (f - n);
    projection._34 = -(f * n) / (f - n);
    projection._43 = -1.0f;

    return projection;
}

matrix4 make_projection_matrix_fov(float32 fov, float32 aspect_ratio, float32 n, float32 f)
{
    //     w/2
    //   +-----+
    //   |    /
    //   |   /
    // n |  /
    //   | / angle = fov/2
    //   |/  tg(fov / 2) = (w/2) / n
    //   +   => 2n / w = 1 / tg(fov / 2)

    float32 tf2 = (1.0f / tanf(0.5f * fov));

    matrix4 projection = {};

    projection._11 = tf2;
    projection._22 = tf2 * aspect_ratio;
    projection._33 = -(f + n) / (f - n);
    projection._34 = -(f * n) / (f - n);
    projection._43 = -1.0f;

    return projection;
}

matrix4 make_look_at_matrix(vector3 eye, vector3 center, vector3 up)
{
    vector3 f = normalized(center - eye);
    vector3 s = normalized(cross(f, up));
    vector3 u = cross(s, f);

    matrix4 result =
    {
        s.x, u.x, -f.x, -dot(s, eye),
        s.y, u.y, -f.y, -dot(u, eye),
        s.z, u.z, -f.z,  dot(f, eye),
          0,   0,    0,            1,
    };

    return result;
}

matrix4 operator + (matrix4 a, matrix4 b)
{
    matrix4 result;

    result.sx = { a._11 + b._11, a._12 + b._12, a._13 + b._13, a._14 + b._14 };
    result.sy = { a._21 + b._21, a._22 + b._22, a._23 + b._23, a._24 + b._24 };
    result.sz = { a._31 + b._31, a._32 + b._32, a._33 + b._33, a._34 + b._34 };
    result.sw = { a._41 + b._41, a._42 + b._42, a._43 + b._43, a._44 + b._44 };

    return result;
}

matrix4 operator - (matrix4 a, matrix4 b)
{
    matrix4 result;

    result.sx = { a._11 - b._11, a._12 - b._12, a._13 - b._13, a._14 - b._14 };
    result.sy = { a._21 - b._21, a._22 - b._22, a._23 - b._23, a._24 - b._24 };
    result.sz = { a._31 - b._31, a._32 - b._32, a._33 - b._33, a._34 - b._34 };
    result.sw = { a._41 - b._41, a._42 - b._42, a._43 - b._43, a._44 - b._44 };

    return result;
}

matrix4 operator * (matrix4 a, float32 c)
{
    matrix4 result;

    result.sx = { a._11*c, a._12*c, a._13*c, a._14*c };
    result.sy = { a._21*c, a._22*c, a._23*c, a._24*c };
    result.sz = { a._31*c, a._32*c, a._33*c, a._34*c };
    result.sw = { a._41*c, a._42*c, a._43*c, a._44*c };

    return result;
}

matrix4 operator * (float32 c, matrix4 a)
{
    return (a * c);
}

vector4 operator * (matrix4 a, vector4 v)
{
    vector4 result;

    result.x = a._11*v._1 + a._12*v._2 + a._13*v._3 + a._14*v._4;
    result.y = a._21*v._1 + a._22*v._2 + a._23*v._3 + a._24*v._4;
    result.z = a._31*v._1 + a._32*v._2 + a._33*v._3 + a._34*v._4;
    result.w = a._41*v._1 + a._42*v._2 + a._43*v._3 + a._44*v._4;

    return result;
}

vector4 operator * (vector4 v, matrix4 a)
{
    vector4 result;

    result.x = a._11*v._1 + a._21*v._2 + a._31*v._3 + a._41*v._4;
    result.y = a._12*v._1 + a._22*v._2 + a._32*v._3 + a._42*v._4;
    result.z = a._13*v._1 + a._23*v._2 + a._33*v._3 + a._43*v._4;
    result.w = a._14*v._1 + a._24*v._2 + a._34*v._3 + a._44*v._4;

    return result;
}

matrix4 operator * (matrix4 a, matrix4 b)
{
    matrix4 result;

    result._11 = a._11*b._11 + a._12*b._21 + a._13*b._31 + a._14*b._41;
    result._12 = a._11*b._12 + a._12*b._22 + a._13*b._32 + a._14*b._42;
    result._13 = a._11*b._13 + a._12*b._23 + a._13*b._33 + a._14*b._43;
    result._14 = a._11*b._14 + a._12*b._24 + a._13*b._34 + a._14*b._44;

    result._21 = a._21*b._11 + a._22*b._21 + a._23*b._31 + a._24*b._41;
    result._22 = a._21*b._12 + a._22*b._22 + a._23*b._32 + a._24*b._42;
    result._23 = a._21*b._13 + a._22*b._23 + a._23*b._33 + a._24*b._43;
    result._24 = a._21*b._14 + a._22*b._24 + a._23*b._34 + a._24*b._44;

    result._31 = a._31*b._11 + a._32*b._21 + a._33*b._31 + a._34*b._41;
    result._32 = a._31*b._12 + a._32*b._22 + a._33*b._32 + a._34*b._42;
    result._33 = a._31*b._13 + a._32*b._23 + a._33*b._33 + a._34*b._43;
    result._34 = a._31*b._14 + a._32*b._24 + a._33*b._34 + a._34*b._44;

    result._41 = a._41*b._11 + a._42*b._21 + a._43*b._31 + a._44*b._41;
    result._42 = a._41*b._12 + a._42*b._22 + a._43*b._32 + a._44*b._42;
    result._43 = a._41*b._13 + a._42*b._23 + a._43*b._33 + a._44*b._43;
    result._44 = a._41*b._14 + a._42*b._24 + a._43*b._34 + a._44*b._44;

    return result;
}


#if UNITY_BUILD
#include "matrix4.cpp"
#endif // UNITY_BUILD
