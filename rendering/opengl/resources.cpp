const char* VShaderText =
"#version 450 core                         \n"
"                                          \n"
"layout(location = 0) in vec2 aPos;        \n"
"layout(location = 1) in vec2 aUV;         \n"
"                                          \n"
"out gl_PerVertex { vec4 gl_Position; };   \n"
"out vec2 vUV;                             \n"
"                                          \n"
"void main() {                             \n"
"    gl_Position = vec4(aPos, 0.0, 1.0);   \n"  // <-- no projection multiply
"    vUV          = aUV;                   \n"
"}                                         \n";
;

const char* FShaderText =
"#version 450 core                              \n"
"\n"
"in vec2 vUV;                                   \n"
"\n"
"uniform sampler2D uFont;                       \n"
"\n"
"out vec4 fragColor;                            \n"
"\n"
"void main() {                                  \n"
"    float alpha = texture(uFont, vUV).r;       \n"
"    fragColor   = vec4(1.0, 1.0, 1.0, alpha);   \n"
"    if (alpha < 0.1) discard;                  \n"
"}                                              \n"
;



internal GLuint
CreateTextPipeline(void)
{
    GLuint VShader = glCreateShaderProgramv(GL_VERTEX_SHADER  , 1, &VShaderText);
    GLuint FShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FShaderText);

    GLint ok;
    glGetProgramiv(VShader, GL_LINK_STATUS, &ok);
    if (!ok) {
        char msg[1024]; glGetProgramInfoLog(VShader, sizeof(msg), NULL, msg);
        PrintDebug(msg); Assert(!"Vertex shader link failed");
    }
    glGetProgramiv(FShader, GL_LINK_STATUS, &ok);
    if (!ok) {
        char msg[1024]; glGetProgramInfoLog(FShader, sizeof(msg), NULL, msg);
        PrintDebug(msg); Assert(!"Fragment shader link failed");
    }

    GLuint Pipeline;
    glGenProgramPipelines(1, &Pipeline);
    glUseProgramStages(Pipeline, GL_VERTEX_SHADER_BIT  , VShader);
    glUseProgramStages(Pipeline, GL_FRAGMENT_SHADER_BIT, FShader);

    return Pipeline;
}

#define STATIC_MESH_MAX 256
global static_tri_mesh StaticMeshPool[STATIC_MESH_MAX];
global u32             StaticMeshCount;

internal inline u32
PushNewStaticMesh(static_tri_mesh Mesh)
{
    u32 Index = StaticMeshCount;
    StaticMeshPool[Index] = Mesh;
    ++StaticMeshCount;

    return Index;
}

typedef enum
{
    LIGHTNING_NONE,

    LIGHTNING_COLOR,
    LIGHTNING_COLOR_AMBIENT,
    LIGHTNING_COLOR_AMBIENT_SPECULAR,
} LIGHTNING_TYPE;

typedef struct
{
    f32 Ambient[4];
    f32 Diffuse[4];
    f32 Shininess;
    f32 Dissolve;
} material_color_ambient;

// Must be 16-byte aligned and exactly 4 vec4-slots (64 bytes) in total
typedef struct 
{
    f32 Ambient[4];     // { r, g, b, _pad0 }
    f32 Diffuse[4];     // { r, g, b, _pad1 }
    f32 Specular[4];    // { r, g, b, _pad2 }

    f32 Shininess;      // offset  0
    f32 Dissolve;       // offset  4
    f32 _Padding0;      // offset  8
    f32 _Padding1;      // offset 12
} material_color_ambient_specular;


#define MAX_MATERIAL       1024
#define MATERIAL_BIND_SLOT 1 // Move this.

global GLuint Materials[MAX_MATERIAL];
global u32    MaterialCount;

internal inline u32
CreateNewMaterial(void* Data, size_t Size)
{
    GLuint* Material = Materials + MaterialCount;

    glCreateBuffers(1, Material);
    glNamedBufferStorage(*Material, Size, Data,  0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, *Material);

    u32 Index = MaterialCount;
    ++MaterialCount;
    return Index;
}
