#include "jani.h"
#include "backend/renderers/opengl/jani_opengl.h"

const char* VShader =
"#version 450 core                         \n"
"                                          \n"
"layout(location = 0) in vec2 aPos;        \n"
"                                          \n"
"out gl_PerVertex { vec4 gl_Position; };   \n"
"                                          \n"
"void main() {                             \n"
"    gl_Position = vec4(aPos, 0.0, 1.0);   \n"
"}                                         \n";
;

const char* FShader =
"#version 450 core                          \n"
"                                           \n"
"out vec4 fragColor;                        \n"
"                                           \n"
"void main() {                              \n"
"    fragColor = vec4(1.0, 0.0, 0.0, 1.0);  \n"
"}                                          \n"
;

using namespace JANI;

internal void
RenderUI(jani_context *UIContext)
{
    JANI::BeginUIFrame(UIContext);

    static jani_pipeline_handle BasePipeline;
    static bool                 PipelineCreated;
    if (!PipelineCreated)
    {
        jani_pipeline_info PipelineInfo = {};
        PipelineInfo.VertexShaderByteCode = VShader;
        PipelineInfo.PixelShaderByteCode = FShader;

        jani_shader_input ShaderInputs[1] =
        {
            {JANI_F32, 0, 2}, // Positions (Type, BindSlot, Count)
        };
        PipelineInfo.Inputs = ShaderInputs;
        PipelineInfo.InputCount = 1;

        PipelineInfo.DefaultVertexBufferSize = Kilobytes(1);
        PipelineInfo.DefaultIndexBufferSize = Kilobytes(1);

        BasePipeline = JANI::CreatePipeline(UIContext, PipelineInfo);
        PipelineCreated = true;
    }

    JANI::SetPipelineState(UIContext->Backend, BasePipeline);
    JANI::DrawBox(UIContext);

    JANI::EndUIFrame(UIContext);
}
