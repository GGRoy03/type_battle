#pragma comment (lib, "opengl32.lib")

static void APIENTRY DebugCallback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message, const void* user);

static void FatalError(const char* message);

#define GL_FUNCTIONS(X) \
    X(PFNGLCREATEBUFFERSPROC,                   glCreateBuffers                  ) \
    X(PFNGLNAMEDBUFFERSTORAGEPROC,              glNamedBufferStorage             ) \
    X(PFNGLBINDVERTEXARRAYPROC,                 glBindVertexArray                ) \
    X(PFNGLCREATEVERTEXARRAYSPROC,              glCreateVertexArrays             ) \
    X(PFNGLVERTEXARRAYATTRIBBINDINGPROC,        glVertexArrayAttribBinding       ) \
    X(PFNGLVERTEXARRAYVERTEXBUFFERPROC,         glVertexArrayVertexBuffer        ) \
    X(PFNGLVERTEXARRAYATTRIBFORMATPROC,         glVertexArrayAttribFormat        ) \
    X(PFNGLENABLEVERTEXARRAYATTRIBPROC,         glEnableVertexArrayAttrib        ) \
    X(PFNGLCREATESHADERPROGRAMVPROC,            glCreateShaderProgramv           ) \
    X(PFNGLGETPROGRAMIVPROC,                    glGetProgramiv                   ) \
    X(PFNGLGETPROGRAMINFOLOGPROC,               glGetProgramInfoLog              ) \
    X(PFNGLGENPROGRAMPIPELINESPROC,             glGenProgramPipelines            ) \
    X(PFNGLUSEPROGRAMSTAGESPROC,                glUseProgramStages               ) \
    X(PFNGLBINDPROGRAMPIPELINEPROC,             glBindProgramPipeline            ) \
    X(PFNGLPROGRAMUNIFORMMATRIX2FVPROC,         glProgramUniformMatrix2fv        ) \
    X(PFNGLBINDTEXTUREUNITPROC,                 glBindTextureUnit                ) \
    X(PFNGLCREATETEXTURESPROC,                  glCreateTextures                 ) \
    X(PFNGLTEXTUREPARAMETERIPROC,               glTextureParameteri              ) \
    X(PFNGLTEXTURESTORAGE2DPROC,                glTextureStorage2D               ) \
    X(PFNGLTEXTURESUBIMAGE2DPROC,               glTextureSubImage2D              ) \
    X(PFNGLDEBUGMESSAGECALLBACKPROC,            glDebugMessageCallback           ) \
    X(PFNGLVERTEXARRAYELEMENTBUFFERPROC,        glVertexArrayElementBuffer       ) \
    X(PFNGLBINDBUFFERBASEPROC,                  glBindBufferBase                 ) \
    X(PFNGLNAMEDBUFFERSUBDATAPROC,              glNamedBufferSubData             ) \
    X(PFNGLNAMEDBUFFERDATAPROC,                 glNamedBufferData                ) \
    X(PFNGLDRAWELEMENTSBASEVERTEXPROC,          glDrawElementsBaseVertex         ) \
    X(PFNGLVERTEXARRAYBINDINGDIVISORPROC,       glVertexArrayBindingDivisor      ) \
    X(PFNGLDRAWELEMENTSINSTANCEDBASEVERTEXPROC, glDrawElementsInstancedBaseVertex) \
    X(PFNGLMAPNAMEDBUFFERRANGEPROC,             glMapNamedBufferRange            ) \
    X(PFNGLOBJECTLABELPROC,                     glObjectLabel)


#define X(type, name) static type name;
GL_FUNCTIONS(X)
#undef X

static int StringsAreEqual(const char* src, const char* dst, size_t dstlen)
{
    while (*src && dstlen-- && *dst)
    {
        if (*src++ != *dst++)
        {
            return 0;
        }
    }

    return (dstlen && *src == *dst) || (!dstlen && *src == 0);
}

static PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

static void GetWglFunctions(void)
{
    // to get WGL functions we need valid GL context, so create dummy window for dummy GL context
    HWND dummy = CreateWindowExW(
        0, L"STATIC", L"DummyWindow", WS_OVERLAPPED,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, NULL, NULL);
    Assert(dummy && "Failed to create dummy window");

    HDC dc = GetDC(dummy);
    Assert(dc && "Failed to get device context for dummy window");

    PIXELFORMATDESCRIPTOR desc =
    {
        .nSize = sizeof(desc),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 24,
    };

    int format = ChoosePixelFormat(dc, &desc);
    if (!format)
    {
        FatalError("Cannot choose OpenGL pixel format for dummy window!");
    }

    int ok = DescribePixelFormat(dc, format, sizeof(desc), &desc);
    Assert(ok && "Failed to describe OpenGL pixel format");

    // reason to create dummy window is that SetPixelFormat can be called only once for the window
    if (!SetPixelFormat(dc, format, &desc))
    {
        FatalError("Cannot set OpenGL pixel format for dummy window!");
    }

    HGLRC rc = wglCreateContext(dc);
    Assert(rc && "Failed to create OpenGL context for dummy window");

    ok = wglMakeCurrent(dc, rc);
    Assert(ok && "Failed to make current OpenGL context for dummy window");

    // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_extensions_string.txt
    PFNWGLGETEXTENSIONSSTRINGARBPROC wglGetExtensionsStringARB 
        = (PFNWGLGETEXTENSIONSSTRINGARBPROC)wglGetProcAddress("wglGetExtensionsStringARB");
    if (!wglGetExtensionsStringARB)
    {
        FatalError("OpenGL does not support WGL_ARB_extensions_string extension!");
    }

    const char* ext = wglGetExtensionsStringARB(dc);
    Assert(ext && "Failed to get OpenGL WGL extension string");

    const char* start = ext;
    for (;;)
    {
        while (*ext != 0 && *ext != ' ')
        {
            ext++;
        }

        size_t length = ext - start;
        if (StringsAreEqual("WGL_ARB_pixel_format", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_pixel_format.txt
            wglChoosePixelFormatARB =
                (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
        }
        else if (StringsAreEqual("WGL_ARB_create_context", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/ARB/WGL_ARB_create_context.txt
            wglCreateContextAttribsARB 
                = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
        }
        else if (StringsAreEqual("WGL_EXT_swap_control", start, length))
        {
            // https://www.khronos.org/registry/OpenGL/extensions/EXT/WGL_EXT_swap_control.txt
            wglSwapIntervalEXT 
                = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
        }

        if (*ext == 0)
        {
            break;
        }

        ext++;
        start = ext;
    }

    if (!wglChoosePixelFormatARB || !wglCreateContextAttribsARB || !wglSwapIntervalEXT)
    {
        FatalError("OpenGL does not support required WGL extensions for modern context!");
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(rc);
    ReleaseDC(dummy, dc);
    DestroyWindow(dummy);
}

static void SetOpenGLPixelFormat(HDC DeviceContext)
{
    int Attributes[] =
    {
        WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
        WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
        WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
        WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,
        WGL_COLOR_BITS_ARB,     24,
        WGL_DEPTH_BITS_ARB,     24,
        WGL_STENCIL_BITS_ARB,   8,
        0,
    };

    int  Format;
    UINT Formats;
    if (!wglChoosePixelFormatARB(DeviceContext, Attributes, NULL, 1, &Format, &Formats) || Formats == 0)
    {
        FatalError("OpenGL does not support required pixel format!");
    }

    PIXELFORMATDESCRIPTOR Desc = { .nSize = sizeof(Desc) };
    int ok = DescribePixelFormat(DeviceContext, Format, sizeof(Desc), &Desc);
    Assert(ok && "Failed to describe OpenGL pixel format");

    if (!SetPixelFormat(DeviceContext, Format, &Desc))
    {
        FatalError("Cannot set OpenGL selected pixel format!");
    }
}

static void CreateOpenGLContext(HDC DeviceContext)
{
    int Attributes[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
        WGL_CONTEXT_MINOR_VERSION_ARB, 5,
        WGL_CONTEXT_PROFILE_MASK_ARB,  WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#ifndef NDEBUG
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
#endif
        0,
    };

    HGLRC rc = wglCreateContextAttribsARB(DeviceContext, NULL, Attributes);
    if (!rc)
    {
        FatalError("Cannot create modern OpenGL context! OpenGL version 4.5 not supported?");
    }

    BOOL ok = wglMakeCurrent(DeviceContext, rc);
    Assert(ok && "Failed to make current OpenGL context");

#define X(type, name) name = (type)wglGetProcAddress(#name); Assert(name);
        GL_FUNCTIONS(X)
#undef X

#ifndef NDEBUG
        glDebugMessageCallback(&DebugCallback, NULL);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
#endif

}
