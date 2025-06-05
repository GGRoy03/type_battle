// TODO: Profile this code and optimize it.

// BUG: Some part of this code is wrong, nice.

typedef struct mat4 
{
    union 
    {
        struct { vec4 c0,c1,c2,c3; };
        f32 AsArray[16];
    };
} mat4;

internal inline mat4
Mat4_FromColumns(vec4 C0, vec4 C1, vec4 C2, vec4 C3)
{
    return (mat4){ .c0 = C0, .c1 = C1, .c2 = C2, .c3 = C3 };
}

internal inline mat4
Mat4_InversePerspective(f32 FieldOfView, f32 AspectRatio, f32 Near, f32 Far)
{
    f32 FovRad     = FieldOfView * (3.14159265358979323846f / 180.0f);
    f32 TwoNearFar = 2.0f * Near * Far;
    f32 InvScale   = tanf(FovRad / 2.0f);

    return (mat4){ .AsArray = {
        AspectRatio * InvScale, 0.0f    , 0.0f, 0.0f,
        0.0f                  , InvScale, 0.0f, 0.0f,
        0.0f                  , 0.0f    , 0.0f, (Far - Near) / TwoNearFar,
        0.0f                  , 0.0f    , -1.0f, (Far + Near) / TwoNearFar
    }};
}

internal inline mat4
Mat4_Perspective(f32 FieldOfView, f32 AspectRatio, f32 Near, f32 Far)
{
    f32 FovRad = FieldOfView * (3.14159265358979323846f / 180.0f);
    f32 Focal  = tanf(FovRad / 2.0f);

    return (mat4){ .AsArray = {
        1.0f/(AspectRatio*Focal), 0.0       , 0.0f                     , 0.0f,
        0.0f                    , 1.0f/Focal, 0.0f                     , 0.0f,
        0.0f                    , 0.0       , -(Far+Near)/(Far-Near)   , -1.0f,
        0.0f                    , 0.0f      , -2.0f*Far*Near/(Far-Near), 0.0f
    }};
}

internal inline mat4
Mat4_InitIdentity()
{
    return (mat4){ .AsArray = {
        1.0f,    0.0f, 0.0f, 0.0f,
        0.0f, 1.0f,    0.0f, 0.0f,
        0.0f, 0.0f, 1.0f,    0.0f,
        0.0f, 0.0f, 0.0f, 1.0f 
    }};
}

internal inline mat4
Mat4_Scale(f32 ScaleX, f32 ScaleY, f32 ScaleZ)
{
    return (mat4){ .AsArray = {
        ScaleX, 0.0f,   0.0f,   0.0f,
        0.0f,   ScaleY, 0.0f,   0.0f,
        0.0f,   0.0f,   ScaleZ, 0.0f,
        0.0f,   0.0f,   0.0f,   1.0f
    }};
}

internal inline mat4
Mat4_Translate(vec3 Translation)
{
    mat4 M = Mat4_InitIdentity(1.0f);
    M.AsArray[12] = Translation.x;
    M.AsArray[13] = Translation.y;
    M.AsArray[14] = Translation.z;
    return M;
}

internal inline mat4
Mat4_Multiply(mat4 A, mat4 B)
{
    mat4 R;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            R.AsArray[j * 4 + i] =
                A.AsArray[0 * 4 + i] * B.AsArray[j * 4 + 0] +
                A.AsArray[1 * 4 + i] * B.AsArray[j * 4 + 1] +
                A.AsArray[2 * 4 + i] * B.AsArray[j * 4 + 2] +
                A.AsArray[3 * 4 + i] * B.AsArray[j * 4 + 3];
        }
    }
    return R;
}

internal inline vec4
Mat4_MultiplyVector(mat4 Matrix, vec4 Vector)
{
    vec4 Result;
    Result.x = Vector.x * Matrix.AsArray[0]  + Vector.y * Matrix.AsArray[4]  +
               Vector.z * Matrix.AsArray[8]  + Vector.w * Matrix.AsArray[12];
    Result.y = Vector.x * Matrix.AsArray[1]  + Vector.y * Matrix.AsArray[5]  +
               Vector.z * Matrix.AsArray[9]  + Vector.w * Matrix.AsArray[13];
    Result.z = Vector.x * Matrix.AsArray[2]  + Vector.y * Matrix.AsArray[6]  +
               Vector.z * Matrix.AsArray[10] + Vector.w * Matrix.AsArray[14];
    Result.w = Vector.x * Matrix.AsArray[3]  + Vector.y * Matrix.AsArray[7]  +
               Vector.z * Matrix.AsArray[11] + Vector.w * Matrix.AsArray[15];
    return Result;
}

internal inline mat4
Mat4_FocusMatrix(vec3 Up, vec3 CamPos, vec3 Focus)
{
    vec3 Forward = Vec3_Normalize(Vec3_Subtract(Focus, CamPos));  // FIXED: reversed subtraction
    vec3 Right   = Vec3_Normalize(Vec3_VectorProduct(Forward, Up));  // FIXED: swapped order
    vec3 UpVec   = Vec3_VectorProduct(Right, Forward);  // FIXED: swapped order

    mat4 R = (mat4){ .AsArray = {
        Right.x,  UpVec.x,  -Forward.x, 0.0f,  // FIXED: negated Forward
        Right.y,  UpVec.y,  -Forward.y, 0.0f,  // FIXED: negated Forward
        Right.z,  UpVec.z,  -Forward.z, 0.0f,  // FIXED: negated Forward
        0.0f,     0.0f,     0.0f,       1.0f
    }};

    mat4 T = Mat4_Translate((vec3){ -CamPos.x, -CamPos.y, -CamPos.z });
    return Mat4_Multiply(R, T);
}

internal inline mat4
Mat4_InverseFocusMatrix(mat4 FocusMatrix)
{
    vec3 r0 = Vec3(FocusMatrix.AsArray[0],  FocusMatrix.AsArray[1],  FocusMatrix.AsArray[2]);
    vec3 r1 = Vec3(FocusMatrix.AsArray[4],  FocusMatrix.AsArray[5],  FocusMatrix.AsArray[6]);
    vec3 r2 = Vec3(-FocusMatrix.AsArray[8], -FocusMatrix.AsArray[9], -FocusMatrix.AsArray[10]);  // FIXED: negated for right-handed
    vec3 T  = Vec3(FocusMatrix.AsArray[12], FocusMatrix.AsArray[13], FocusMatrix.AsArray[14]);

    f32 iTx = -Vec3_Dot(r0, T);
    f32 iTy = -Vec3_Dot(r1, T);
    f32 iTz = -Vec3_Dot(r2, T);

    return (mat4){ .AsArray = {
        r0.x, r1.x, r2.x, 0.0f,
        r0.y, r1.y, r2.y, 0.0f,
        r0.z, r1.z, r2.z, 0.0f,
        iTx,  iTy,  iTz,  1.0f
    }};
}