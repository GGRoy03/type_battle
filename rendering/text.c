#define ID_OFFSET 32

typedef struct
{
    u32 ID;
    u32 PositionInAtlasX;
    u32 PositionInAtlasY;
    u32 SizeX;
    u32 SizeY;
    i32 OffsetX;
    i32 OffsetY;
    u32 XAdvance;
} glyph;

typedef struct
{
    glyph *Glyphs;
    u32    GlyphCount;
    u32    TextureSizeX;
    u32    TextureSizeY;
    u32    LineHeight;

    GLuint TextureID;

    bool Loaded;
    bool Initialized;
} text_map;

bool IsLetter(u8 c)
{
    return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
}

internal bool
LineStartsWith(u8* Line, char* Pattern)
{
    while (*Pattern)
    {
        if (*Pattern++ != *Line++) return false;
    }

    return true;
}

internal text_map 
LoadTextMap(char *Path, platform_context *Platform, transient_allocator *Allocator)
{
    text_map             Result     = {0};
    platform_read_result FileBuffer = Platform->ReadFile(Path, Allocator);
    u32                  At         = 0;

    while(At < FileBuffer.ContentSize)
    {
        u8 *LineStart = FileBuffer.Content + At;

        if(LineStartsWith(LineStart, "common"))
        {
            u32 DummyBase;
            sscanf_s((const char*)LineStart, "common lineHeight=%u base=%u scaleW=%u scaleH=%u"
                    ,&Result.LineHeight, &DummyBase, &Result.TextureSizeX, &Result.TextureSizeY);
        }
        else if (LineStartsWith(LineStart, "chars"))
        {
            sscanf_s((const char*)LineStart, "chars count=%u", &Result.GlyphCount);
            Result.Glyphs = AllocateTransient(Allocator, Result.GlyphCount * sizeof(glyph));
        }
        else if(LineStartsWith(LineStart, "char"))
        {
            glyph Glyph = {0};
            sscanf_s((const char*)LineStart, "char id=%u x=%u y=%u width=%u height=%u xoffset=%u yoffset=%u xadvance=%u",
                     &Glyph.ID, &Glyph.PositionInAtlasX, &Glyph.PositionInAtlasY, &Glyph.SizeX, &Glyph.SizeY, &Glyph.OffsetX,
                     &Glyph.OffsetY, &Glyph.XAdvance);

            // BUG: This will fail if IDs are not sequential
            u32 GlyphIndex = Glyph.ID - ID_OFFSET;
            if(GlyphIndex >= 0)
            {
                Result.Glyphs[GlyphIndex] = Glyph;
            }
        }
        // NOTE: We skip kerning for now.

        while(FileBuffer.Content[At] != '\n')    At++;
        while(!IsLetter(FileBuffer.Content[At])) At++;
    }

    FreeTransient(Allocator, FileBuffer.Content, FileBuffer.ContentSize);

    Result.Loaded = true;
    return Result;
}
