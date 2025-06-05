// TODO: Look into inverse square roots instead of sqrt
// TODO: Look into forcing SIMD depending on the CPU 

struct vec4
{
    f32 x, y, z, w;
};

struct vec3
{
    f32 x, y, z;
};

struct vec2
{
    f32 x, y;
};

// ========================================
// [INITIALIZERS]
// ========================================

internal inline vec4
Vec4(f32 x, f32 y, f32 z, f32 w)
{
    vec4 Result = {x, y, z, w};
    return Result;
}

internal inline vec3
Vec3(f32 x, f32 y, f32 z)
{
    vec3 Result = {x, y, z};
    return Result;
}

internal inline vec2
Vec2(f32 x, f32 y)
{
    vec2 Result = {x, y};
    return Result;
}

// ========================================
// [VEC2 OPERATIONS]
// ========================================

internal inline f32
Vec2_Length(vec2 v)
{
    f32 SquaredSum = 0;
    SquaredSum    += v.x * v.x;
    SquaredSum    += v.y * v.y;

    if (SquaredSum >= 0)
    {
        return sqrtf(SquaredSum);
    }

    return 0;
}

internal inline f32
Vec2_Dot(vec2 A, vec2 B)
{
    return A.x * B.x +
           A.y * B.y;
}

internal inline vec2
Vec2_Add(vec2 A, vec2 B)
{
    vec2 Result = { A.x + B.x, A.y + B.y };
    return Result;
}

internal inline vec2
Vec2_Subtract(vec2 A, vec2 B)
{
    vec2 Result = { A.x - B.x, A.y - B.y };
    return Result;
}

internal inline vec2
Vec2_Scale(f32 S, vec2 V)
{
    vec2 Result = { V.x * S, V.y * S };
    return Result;
}

internal inline vec2
Vec2_Normalize(vec2 V)
{
    f32 Length = Vec2_Length(V);
    if (Length == 0.0f)
    {
        return Vec2(0.0f, 0.0f);
    }
    vec2 Result = { V.x / Length, V.y / Length };
    return Result;
}

internal inline bool
Vec2_IsZero(vec2 V)
{
    return V.x == 0.0f &&
           V.y == 0.0f;
}

internal inline bool
Vec2_SameVector(vec2 A, vec2 B)
{
    return A.x == B.x &&
           A.y == B.y;
}

// ========================================
// [VEC3 OPERATIONS]
// ========================================

internal inline f32
Vec3_Length(vec3 v)
{
    f32 SquaredSum = 0;
    SquaredSum    += v.x * v.x;
    SquaredSum    += v.y * v.y;
    SquaredSum    += v.z * v.z;

    if (SquaredSum >= 0)
    {
        return sqrtf(SquaredSum);
    }

    return 0;
}

internal inline f32
Vec3_Dot(vec3 A, vec3 B)
{
    return A.x * B.x +
           A.y * B.y +
           A.z * B.z;
}

internal inline vec3
Vec3_Add(vec3 A, vec3 B)
{
    vec3 Result = { A.x + B.x,
                    A.y + B.y,
                    A.z + B.z };
    return Result;
}

internal inline vec3
Vec3_Subtract(vec3 A, vec3 B)
{
    vec3 Result = { A.x - B.x,
                    A.y - B.y,
                    A.z - B.z };
    return Result;
}

internal inline vec3
Vec3_Scale(vec3 V, f32 S)
{
    vec3 Result = { V.x * S,
                    V.y * S,
                    V.z * S };
    return Result;
}

internal inline vec3
Vec3_Normalize(vec3 V)
{
    f32 Length = Vec3_Length(V);
    if (Length == 0.0f)
    {
        return Vec3(0.0f, 0.0f, 0.0f);
    }
    vec3 Result = { V.x / Length,
                    V.y / Length,
                    V.z / Length };
    return Result;
}

internal inline bool
Vec3_IsZero(vec3 V)
{
    return V.x == 0.0f &&
           V.y == 0.0f &&
           V.z == 0.0f;
}

internal inline bool
Vec3_SameVector(vec3 A, vec3 B)
{
    return A.x == B.x &&
           A.y == B.y &&
           A.z == B.z;
}

internal inline vec3
Vec3_Multiply(vec3 A, vec3 B)
{
    vec3 Result = { A.x * B.x,
                    A.y * B.y,
                    A.z * B.z };
    return Result;
}

internal inline vec3
Vec3_VectorProduct(vec3 vl, vec3 vr)
{
    vec3 Result = Vec3 
    (
        (vl.y * vr.z) - (vl.z * vr.y),
        (vl.z * vr.x) - (vl.x * vr.z),
        (vl.x * vr.y) - (vl.y * vr.x)
    );

    return Result;
}

// ========================================
// [VEC4 OPERATIONS]
// ========================================

internal inline f32
Vec4_Dot(vec4 A, vec4 B)
{
    return A.x * B.x +
           A.y * B.y +
           A.z * B.z +
           A.w * B.w;
}

internal inline vec4
Vec4_Add(vec4 A, vec4 B)
{
    vec4 Result = { A.x + B.x,
                    A.y + B.y,
                    A.z + B.z,
                    A.w + B.w };
    return Result;
}

internal inline vec4
Vec4_Subtract(vec4 A, vec4 B)
{
    vec4 Result = { A.x - B.x,
                    A.y - B.y,
                    A.z - B.z,
                    A.w - B.w };
    return Result;
}

internal inline vec4
Vec4_Multiply(vec4 A, vec4 B)
{
    vec4 Result = { A.x * B.x,
                    A.y * B.y,
                    A.z * B.z,
                    A.w * B.w };
    return Result;
}

internal inline vec4
Vec4_Scale(f32 S, vec4 V)
{
    vec4 Result = { V.x * S,
                    V.y * S,
                    V.z * S,
                    V.w * S };
    return Result;
}

internal inline bool
Vec4_IsZero(vec4 V)
{
    return V.x == 0.0f &&
           V.y == 0.0f &&
           V.z == 0.0f &&
           V.w == 0.0f;
}

internal inline bool
Vec4_SameVector(vec4 A, vec4 B)
{
    return A.x == B.x &&
           A.y == B.y &&
           A.z == B.z &&
           A.w == B.w;
}
