typedef enum
{
    DRAW_COMMAND_NONE,

    DRAW_COMMAND_MESH,
    DRAW_COMMAND_INSTANCED_MESH,
    DRAW_COMMAND_TEXT,
} DRAW_COMMAND_TYPE;

typedef struct
{
    GLuint  Pipeline;
    GLuint  VertexArrayObject;
    GLsizei InputStride;
} pipeline_state;

typedef struct
{
    u32 MeshIndex;
    u32 SubMeshIndex;
} draw_mesh_payload;

typedef struct
{
    u32   MeshIndex;
    void* IntanceData;
    u32   InstanceCount;
} draw_instanced_mesh_payload;

typedef struct
{
    u8   *Text;
    vec2 Position;
} draw_text_payload;

typedef struct
{
    DRAW_COMMAND_TYPE Type;
    pipeline_state    PipelineState;

    union
    {
        draw_mesh_payload           Mesh;
        draw_instanced_mesh_payload InstancedMesh;
        draw_text_payload           Text;
    } Payload;

    // ?
    size_t IndexOffset;
    u32    IndexCount;
    u32    BaseVertex;
} draw_command;


// WARN: This is a bit messy. It will get bad quick. When we are done with the
// text clean up the code.

#define STB_IMAGE_IMPLEMENTATION
#include "rendering/third_party/stb_image.h"

#include "rendering/text.cpp"

#define MAXIMUM_COMMAND 1024
typedef struct
{
    GLuint   CameraBuffer;
    text_map TextMap;

    u32           CommandCount;
    draw_command *Commands;
} frame_context;


#include "rendering/geometry/tri_mesh.cpp"
#include "setup.cpp"
#include "resources.cpp"
#include "commands.cpp"
#include "rendering/assets.cpp"
#include "rendering/camera.cpp"

internal inline void 
BeginFrame(camera *Camera, frame_context *FrameContext, platform_context *Platform,
           transient_allocator *Allocator)
{
    glViewport(0, 0, Camera->ClientWidth, Camera->ClientHeight);

    glClearColor(0.392f, 0.584f, 0.929f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(FrameContext->CameraBuffer == 0)
    {
        glCreateBuffers(1, &FrameContext->CameraBuffer);

        glNamedBufferStorage(FrameContext->CameraBuffer, sizeof(mat4),
                             NULL, GL_DYNAMIC_STORAGE_BIT);
        glObjectLabel(GL_BUFFER, FrameContext->CameraBuffer, -1, "CameraBuffer");
    }

    mat4 ViewProj = Mat4_Multiply(Camera->CameraToClip, Camera->WorldToCamera);
    glNamedBufferSubData(FrameContext->CameraBuffer, 0, sizeof(mat4),
                         ViewProj.AsArray);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, FrameContext->CameraBuffer); // WARN: Do we have to bind this every frame?

    if (!FrameContext->TextMap.Initialized)
    {
        FrameContext->TextMap = LoadTextMap("D:/Work/type_battle/misc/test_font.fnt", Platform,
                                            Allocator);

        if (FrameContext->TextMap.Loaded)
        {
            i32 TexWidth, TexHeight, Channels;
            u8 *Pixels = stbi_load("D:/Work/type_battle/misc/test_font_0.png", &TexWidth, &TexHeight, &Channels, 4);
            if (!Pixels)
            {
                FatalError("FAILED TO LOAD FONT TEXTURE.");
            }

            /*Assert(TexWidth  == FrameContext->TextMap.TextureSizeX &&
                   TexHeight == FrameContext->TextMap.TextureSizeY);*/

            glCreateTextures(GL_TEXTURE_2D, 1, &FrameContext->TextMap.TextureID);

            GLuint TextureID = FrameContext->TextMap.TextureID;
            glTextureStorage2D(TextureID, 1, GL_RGBA8, TexWidth, TexHeight);

            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTextureSubImage2D(TextureID, 0, 0, 0, TexWidth, TexHeight,
                                GL_RGBA, GL_UNSIGNED_BYTE, Pixels);

            glTextureParameteri(TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTextureParameteri(TextureID, GL_TEXTURE_WRAP_S    , GL_CLAMP_TO_EDGE);
            glTextureParameteri(TextureID, GL_TEXTURE_WRAP_T    , GL_CLAMP_TO_EDGE);

            glBindTextureUnit(0, TextureID);

            FrameContext->TextMap.Initialized = true;
        }
    }

    if (!FrameContext->Commands)
    {
        FrameContext->Commands = (draw_command*)AllocateTransient(Allocator, MAXIMUM_COMMAND * sizeof(draw_command));
    }
}

internal inline void
EndFrame(void* DeviceContext, frame_context *Frame)
{
    // NOTE: This should be different, but the logic is there.
    ExecuteDrawCommands(Frame);

    if(!SwapBuffers((HDC)DeviceContext))
    {
        FatalError("Failed to swap OpenGL buffers!");
    }
}
