// WARN: Does not check for overlows. And can only write to the first
// index.
internal void
Draw2DText(vec2 Position, u8 *Text, pipeline_state PipelineState,
         draw_command *Commands)
{
    draw_command *Command  = Commands + 0;
    Command->PipelineState = PipelineState;
    Command->Type          = DRAW_COMMAND_TEXT;

    draw_text_payload *Payload = &Command->Payload.Text;
    Payload->Text              = Text;
    Payload->Position          = Position;
}

internal void
ExecuteDrawCommands(frame_context *Frame)
{
    // WARN: We use local_persist just to test the text rendering logic
    local_persist GLuint VertexBuffer;

    if(VertexBuffer == 0)
    {
        glCreateBuffers(1, &VertexBuffer);
        glNamedBufferStorage(VertexBuffer, Kilobytes(1), NULL, GL_DYNAMIC_STORAGE_BIT);
    }

    for(u32 Index = 0; Index < Frame->CommandCount; Index++)
    {
        draw_command *Command = Frame->Commands + Index;

        // WARN: We don't want to do these expensive state changes for every
        // draw command. We must find a way to sort them in some order.
        glVertexArrayVertexBuffer(Command->PipelineState.VertexArrayObject, 0,
                                  VertexBuffer, 0, Command->PipelineState.InputStride);

        glBindProgramPipeline(Command->PipelineState.Pipeline);
        glBindVertexArray(Command->PipelineState.VertexArrayObject);

        switch(Command->Type)
        {

        case DRAW_COMMAND_MESH:
        {
        } break;

        case DRAW_COMMAND_INSTANCED_MESH:
        {
        } break;

        case DRAW_COMMAND_TEXT:
        {
           ///* draw_text_payload *P       = &Command->Payload.Text;
           // text_map          *TextMap = &Frame->TextMap;*/
           // // u8                *Char    = P->Text;
           // // 
           // // center of screen in NDC
           // float halfW = 0.25f;  // quarter‐screen width
           // float halfH = 0.1f;   // tenth‐screen height

           // // a simple quad in NDC, no UVs matter for now
           // float quad[6][4] = {
           //   {-halfW, -halfH, 0,0},
           //   { halfW, -halfH, 1,0},
           //   { halfW,  halfH, 1,1},

           //   {-halfW, -halfH, 0,0},
           //   { halfW,  halfH, 1,1},
           //   {-halfW,  halfH, 0,1},
           // };

           // // upload it
           // glNamedBufferSubData(VertexBuffer, 0, sizeof(quad), quad);

           // // disable everything else
           // glDisable(GL_DEPTH_TEST);
           // glDisable(GL_CULL_FACE);
           // glEnable(GL_BLEND);
           // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

           // // use a solid‐color fragment shader
           // glDrawArrays(GL_TRIANGLES, 0, 6);

            draw_text_payload *P       = &Command->Payload.Text;
            text_map          *TextMap = &Frame->TextMap;
            u8                *Char    = P->Text;

            f32 ScreenX = P->Position.x;
            f32 ScreenY = P->Position.y;

            size_t  SubDataOffset = 0;
            GLsizei VertexCount   = 0;

            while(*Char)
            {
                u32    GlyphIndex = *Char - ID_OFFSET;
                glyph *Glyph      =  TextMap->Glyphs + GlyphIndex;

                f32 XPos   = ScreenX + Glyph->OffsetX;
                f32 YPos   = ScreenY + Glyph->OffsetY;
                u32 Width  = Glyph->SizeX;
                u32 Height = Glyph->SizeY;

                f32 ScreenW = 1909;
                f32 ScreenH = 980;

                f32 XPosNDC = (XPos / ScreenW) * 2.0f - 1.0f;
                f32 YPosNDC = 1.0f - (YPos / ScreenH) * 2.0f;

                f32 WidthNDC  = ((f32)Width / ScreenW) * 2.0f;
                f32 HeightNDC = ((f32)Height / ScreenH) * 2.0f;


                f32 u0 = (f32)Glyph->PositionInAtlasX       / TextMap->TextureSizeX;
                f32 v0 = (f32)Glyph->PositionInAtlasY       / TextMap->TextureSizeY;
                f32 u1 = (f32)(Glyph->PositionInAtlasX + Width ) / TextMap->TextureSizeX;
                f32 v1 = (f32)(Glyph->PositionInAtlasY + Height) / TextMap->TextureSizeY;

                f32 TextQuad[24] =
                {
                    XPosNDC        , YPosNDC - HeightNDC, u0, v1,
                    XPosNDC        , YPosNDC            , u0, v0,
                    XPosNDC + WidthNDC, YPosNDC            , u1, v0,

                    XPosNDC        , YPosNDC - HeightNDC, u0, v1,
                    XPosNDC + WidthNDC, YPosNDC            , u1, v0,
                    XPosNDC + WidthNDC, YPosNDC - HeightNDC, u1, v1
                };

                glNamedBufferSubData(VertexBuffer, SubDataOffset, sizeof(TextQuad),
                                     TextQuad);

                SubDataOffset += sizeof(TextQuad);
                ScreenX       += Glyph->XAdvance;
                Char          += 1;
                VertexCount   += 6;
            }

            glBindTextureUnit(0, TextMap->TextureID);
            glDrawArrays(GL_TRIANGLES, 0, VertexCount); 
        }

        }
    }

    Frame->CommandCount = 0;
}
