typedef struct
{
    camera        Camera;
    frame_context Frame;
} game_state;

internal void
UpdateGameAndRender(game_state *GameState, game_controller_input *Input,
                    transient_allocator *Allocator, platform_context *Platform,
                    f32 DeltaTime)
{
    local_persist pipeline_state TextPipeline;
    if(TextPipeline.Pipeline == 0)
    {
        TextPipeline.Pipeline = CreateTextPipeline();

        // TODO: Create the VAO.
        glCreateVertexArrays(1, &TextPipeline.VertexArrayObject);
        GLuint VAO = TextPipeline.VertexArrayObject;

        glVertexArrayAttribFormat (VAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(VAO, 0, 0);
        glEnableVertexArrayAttrib (VAO, 0);

        glVertexArrayAttribFormat (VAO, 1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32));
        glVertexArrayAttribBinding(VAO, 1, 0);
        glEnableVertexArrayAttrib (VAO, 1);

        // 2 Pos and 2 UVs
        TextPipeline.InputStride = 4 * sizeof(f32);
    }

    camera        *Camera = &GameState->Camera;
    frame_context *Frame  = &GameState->Frame;

    UpdateCamera(Camera, Input, Platform, DeltaTime); 

    BeginFrame(Camera, Frame, Platform, Allocator);

    u8 TextToRender[16] = { "Hello" };
    Draw2DText(Vec2(500.0f, 500.0f), TextToRender, TextPipeline, Frame->Commands);
    Frame->CommandCount = 1;

    EndFrame(Platform->DeviceContext, Frame);
}
