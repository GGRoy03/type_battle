const char* VShaderCodeCube =
"#version 450 core                               \n"
"layout(location=0) in vec3 a_pos;               \n"
"layout(location=1) in vec2 a_uv;                \n"
"layout(location=2) in vec3 a_norm;              \n"
"                                                \n"
"layout(std140, binding = 0) uniform CameraUBO { \n"
"    mat4 u_MVP;                                 \n"
"};                                              \n"
"                                                \n"
"                                                \n"
"out gl_PerVertex { vec4 gl_Position; };         \n"
"out vec3 v_norm;                                \n"
"out vec2 v_uv;                                  \n"
"                                                \n"
"void main() {                                   \n"
"    gl_Position = u_MVP * vec4(a_pos, 1.0);     \n"
"    v_norm = a_norm;                            \n"
"    v_uv   = a_uv;                              \n"
"}                                               \n"
;

const char* FShaderCodeCube =
"#version 450 core                                 \n"
"                                                  \n"
"layout(std140, binding = 1) uniform MaterialUBO { \n"
"    vec3 Ambient;                                 \n"
"    vec3 Diffuse;                                 \n"
"    vec3 Specular; float _Pad0;                   \n"
"    float Shininess;                              \n"
"    float Dissolve;                               \n"
"};                                                \n"
"                                                  \n"
"in vec3 v_norm;                                   \n"
"in vec2 v_uv;                                     \n"
"out vec4 o_color;                                 \n"
"                                                  \n"
"void main() {                                     \n"
"    o_color = vec4(Diffuse, 1);            \n"
"}                                                 \n"
;

const char* VShaderCodeResource =
"#version 450 core                                                   \n"
"                                                                    \n"
"layout(std140, binding = 0) uniform CameraUBO {                     \n"
"    mat4 uProjView;                                                 \n"
"};                                                                  \n"
"                                                                    \n"
"layout(location = 0) in vec3 inPosition;                            \n"
"layout(location = 1) in vec2 inUV;                                  \n"
"layout(location = 2) in vec3 inNormal;                              \n"
"layout(location = 3) in mat4 instanceWorld;                         \n"
"                                                                    \n"
"out gl_PerVertex { vec4 gl_Position; };                             \n"
"out vec3 v_norm;                                                    \n"
"out vec2 v_uv;                                                      \n"
"                                                                    \n"
"void main() {                                                       \n"
"    gl_Position = uProjView * instanceWorld * vec4(inPosition, 1.0);\n"
"    v_norm = mat3(instanceWorld) * inNormal;                        \n"
"    v_uv   = inUV;                                                  \n"
"}                                                                   \n"
;

const char* FShaderCodeResource =
"#version 450 core                                 \n"
"                                                  \n"
"layout(std140, binding = 1) uniform MaterialUBO { \n"
"    vec3 Ambient;                                 \n"
"    vec3 Diffuse;                                 \n"
"    vec3 Specular; float _Pad0;                   \n"
"    float Shininess;                              \n"
"    float Dissolve;                               \n"
"};                                                \n"
"                                                  \n"
"in vec2 v_uv;                                     \n"
"in vec3 v_norm;                                   \n"
"out vec4 o_color;                                 \n"
"                                                  \n"
"void main() {                                     \n"
"    o_color = vec4(Diffuse, 1);                   \n"
"}                                                 \n"
;


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



global GLuint g_VShaderCube;
global GLuint g_FShaderCube;

internal GLuint
Create3DPipeline(void)
{
    g_VShaderCube = glCreateShaderProgramv(GL_VERTEX_SHADER  , 1, &VShaderCodeCube);
    g_FShaderCube = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FShaderCodeCube);

    GLint ok;
    glGetProgramiv(g_VShaderCube, GL_LINK_STATUS, &ok);
    if (!ok) {
        char msg[1024]; glGetProgramInfoLog(g_VShaderCube, sizeof(msg), NULL, msg);
        PrintDebug(msg); Assert(!"Vertex shader link failed");
    }
    glGetProgramiv(g_FShaderCube, GL_LINK_STATUS, &ok);
    if (!ok) {
        char msg[1024]; glGetProgramInfoLog(g_FShaderCube, sizeof(msg), NULL, msg);
        PrintDebug(msg); Assert(!"Fragment shader link failed");
    }

    GLuint pipeline;
    glGenProgramPipelines(1, &pipeline);
    glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT,   g_VShaderCube);
    glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, g_FShaderCube);

    return pipeline;
}

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

internal GLuint
CreateResourcePipeline(void)
{
    GLuint VShader = glCreateShaderProgramv(GL_VERTEX_SHADER  , 1, &VShaderCodeResource);
    GLuint FShader = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &FShaderCodeResource);

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

internal void
CreateLayoutFromModifiers(u8 Modifiers, u32 BufferIndex, GLuint VAO)
{
    u32 Input = 0;
    u32 Offset = 0;

    glVertexArrayAttribFormat(VAO, Input, 3, GL_FLOAT, GL_FALSE, Offset);
    glVertexArrayAttribBinding(VAO, Input, BufferIndex);
    glEnableVertexArrayAttrib(VAO, Input);
    Offset += sizeof(vec3);

    if (Modifiers & MESH_MODIFIER_UV_BIT)
    {
        Input++;
        glVertexArrayAttribFormat(VAO, Input, 2, GL_FLOAT, GL_FALSE, Offset);
        glVertexArrayAttribBinding(VAO, Input, BufferIndex);
        glEnableVertexArrayAttrib(VAO, Input);
        Offset += sizeof(vec2);
    }

    if (Modifiers & MESH_MODIFIER_NORMAL_BIT)
    {
        Input++;
        glVertexArrayAttribFormat(VAO, Input, 3, GL_FLOAT, GL_FALSE, Offset);
        glVertexArrayAttribBinding(VAO, Input, BufferIndex);
        glEnableVertexArrayAttrib(VAO, Input);
        Offset += sizeof(vec3);
    }

    if (Modifiers & MESH_MODIFIER_COLOR_BIT)
    {
        Input++;
        glVertexArrayAttribFormat(VAO, Input, 3, GL_FLOAT, GL_FALSE, Offset);
        glVertexArrayAttribBinding(VAO, Input, BufferIndex);
        glEnableVertexArrayAttrib(VAO, Input);
        Offset += sizeof(vec3);
    }
}

internal void
CreateLayoutForMatrix(u32 StartSlot, u32 BindIndex, GLint VAO)
{
    for(u32 Row = 0; Row < 4; Row++)
    {
        GLuint Attribute = StartSlot + Row;
        glEnableVertexArrayAttrib(VAO, Attribute);
        glVertexArrayAttribFormat(VAO, Attribute, 4,  GL_FLOAT, GL_FALSE,
                                  sizeof(vec4) * Row); 
        glVertexArrayAttribBinding(VAO, Attribute, BindIndex);
        glVertexArrayBindingDivisor(VAO, BindIndex, 1);
    }
}
