// example how to set up OpenGL core context on Windows
// and use basic functionality of OpenGL 4.5 version

// important extension functionality used here:
// (4.3) KHR_debug:                     https://www.khronos.org/registry/OpenGL/extensions/KHR/KHR_debug.txt
// (4.5) ARB_direct_state_access:       https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_direct_state_access.txt
// (4.1) ARB_separate_shader_objects:   https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_separate_shader_objects.txt
// (4.2) ARB_shading_language_420pack:  https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_shading_language_420pack.txt
// (4.3) ARB_explicit_uniform_location: https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_explicit_uniform_location.txt

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <sys/stat.h>

#include <GL/gl.h>
#include "rendering/opengl/third_party/glcorearb.h"
#include "rendering/opengl/third_party/wgltext.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <stddef.h>

#include <intrin.h>
#define Assert(cond) do { if (!(cond)) __debugbreak(); } while (0)

#include <stdint.h>
#include <stdio.h>

#pragma comment (lib, "gdi32.lib")
#pragma comment (lib, "user32.lib")

#define STR2(x) #x
#define STR(x) STR2(x)
#define UNUSED(x) (void)(x)

#include "type_battle_platform.h"

internal void* AllocateTransient(transient_allocator* Allocator, size_t Size);
internal void  FreeTransient(transient_allocator* Allocator, void* Pointer, size_t Size);

internal void*
Win32Allocate(size_t Size)
{
    return VirtualAlloc(NULL, Size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32Free(void* Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

internal platform_read_result
Win32ReadFile(const char *Path, transient_allocator *Memory)
{
    Assert(Path);

    platform_read_result Result = {};

    HANDLE FileHandle = CreateFileA(Path, GENERIC_READ, FILE_SHARE_READ, NULL,
                                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    Assert(FileHandle != INVALID_HANDLE_VALUE);

    struct _stat64 FileInfo;
    _stat64(Path, &FileInfo);

    Result.Content = (u8*)AllocateTransient(Memory, FileInfo.st_size);
    if (Result.Content)
    {
        DWORD BytesRead = 0;
        BOOL  ValidRead = ReadFile(FileHandle, Result.Content,
                                   (DWORD)FileInfo.st_size, &BytesRead, NULL);

        if (!ValidRead || BytesRead != FileInfo.st_size)
        {
            FreeTransient(Memory, Result.Content, FileInfo.st_size);
            Result.Content     = NULL;
            Result.ContentSize = 0;
        }
        else
        {
            Result.ContentSize = BytesRead;
        }
    }

    CloseHandle(FileHandle);
    return Result;
}

internal void
Win32ListDirectoryFiles(char* Directory, const char* Extension,
                        transient_allocator* Allocator, char** OutBuffer, u32* OutCount)
{
    u32 Capacity     = 16;
    u32 Count        = 0;
    char* ListBuffer = (char*)AllocateTransient(Allocator, Capacity * MAX_PATH);

    char SearchPattern[MAX_PATH];
    snprintf(SearchPattern, MAX_PATH, "%s\\*%s", Directory, Extension);

    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(SearchPattern, &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        *OutBuffer = NULL;
        *OutCount = 0;
        return;
    }

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            continue;
        }

        if (Count == Capacity)
        {
            u32   NewCap        = Capacity * 2;
            char* NewListBuffer = (char*)AllocateTransient(Allocator,
                                                           NewCap * MAX_PATH);
            memcpy(NewListBuffer, ListBuffer, Capacity * MAX_PATH);

            ListBuffer = NewListBuffer;
            Capacity   = NewCap;
        }

        char* Slot = ListBuffer + (Count * MAX_PATH);
        snprintf(Slot, MAX_PATH, "%s\\%s", Directory, fd.cFileName);
        ++Count;

    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);

    *OutBuffer = ListBuffer;
    *OutCount  = Count;
}

internal void
Win32FreeFileList(char* List, u32 Count, transient_allocator* Allocator)
{
    if (!List) return;

    FreeTransient(Allocator, List, Count * MAX_PATH);
}

internal void
Win32GetClientSize(void* Window, i32 *Width, i32* Height)
{
    RECT Rect;
    GetClientRect((HWND)Window, &Rect);
    *Width  = Rect.right  - Rect.left;
    *Height = Rect.bottom - Rect.top;
}

typedef struct
{
    PlatformReadFile  *ReadFile;
    PlatformAllocate  *Allocate;
    PlatformFree      *Free;

    PlatformListDirectoryFiles *ListDirectoryFiles;
    PlatformFreeFileList       *FreeFileList;

    PlatformGetClientSize *GetClientSize;
    void*                  DeviceContext;
    void*                  WindowHandle;

    char Root[MAX_PLATFORM_PATH];
} platform_context;

// TODO: Make these debug platform functions?
static void FatalError(const char* message)
{
    MessageBoxA(NULL, message, "Error", MB_ICONEXCLAMATION);
    ExitProcess(0);
}

static inline
void PrintDebug(const char* Message)
{
    OutputDebugStringA(Message);
}

// CPP Files
#include "math/sim_math.cpp"
#include "memory/transient.cpp"
#include "rendering/opengl/renderer.cpp"
#include "game/update.cpp"

// Renderer before jani.cpp
#include "backend/renderers/opengl/jani_opengl.cpp"
#include "jani.cpp"

#ifndef NDEBUG
static void APIENTRY DebugCallback(
    GLenum Source, GLenum Type, GLuint ID, GLenum Severity,
    GLsizei Length, const GLchar* Message, const void* User)
{
    UNUSED(User);
    UNUSED(Length);
    UNUSED(ID);
    UNUSED(Type);
    UNUSED(Source);

    OutputDebugStringA(Message);
    OutputDebugStringA("\n");
    if (Severity == GL_DEBUG_SEVERITY_HIGH || Severity == GL_DEBUG_SEVERITY_MEDIUM)
    {
        if (IsDebuggerPresent())
        {
            Assert(!"OpenGL error - check the callstack in debugger");
        }
        FatalError("OpenGL API usage error! Use debugger to examine call stack!");
    }
}
#endif

static LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(wnd, msg, wparam, lparam);
}

// ===================

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool IsDown)
{
    if(NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

internal bool
Win32ProcessMessages(game_controller_input *Keyboard)
{
    MSG Message;
    while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        switch(Message.message)
        {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        {
            u32  VKCode  = (u32)Message.wParam;
            bool WasDown = ((Message.lParam & (1 << 30)) != 0);
            bool IsDown  = ((Message.lParam & (1 << 31)) == 0);

            if(WasDown != IsDown)
            {

                if(VKCode == 'W')
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Actions.CameraForward,
                                                IsDown);
                }
                else if(VKCode == 'A')
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Actions.CameraLeft, IsDown);
                }
                else if(VKCode == 'S')
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Actions.CameraBackward,
                                                IsDown);
                }
                else if(VKCode == 'D')
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Actions.CameraRight, 
                                                IsDown);
                }
                else if(VKCode == 'R')
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Actions.CameraUp, IsDown);
                }
                else if(VKCode == 'F')
                {
                    Win32ProcessKeyboardMessage(&Keyboard->Actions.CameraDown, IsDown);
                }
            }
        } break;

        case WM_QUIT:
        {
            return false;
        } break;

        default:
        {
            TranslateMessage(&Message);
            DispatchMessageA(&Message);
        } break;

        }
    }

    return true;
}

// ==================

int WINAPI WinMain(HINSTANCE Instance, HINSTANCE PreviousInstance, LPSTR CommandLine,
                   int ShowMode)
{
    UNUSED(PreviousInstance);
    UNUSED(CommandLine);
    UNUSED(ShowMode);

    GetWglFunctions();

    WNDCLASSEXW WindowClass =
    {
        .cbSize        = sizeof(WindowClass),
        .lpfnWndProc   = WindowProc,
        .hInstance     = Instance,
        .hIcon         = LoadIcon(NULL, IDI_APPLICATION),
        .hCursor       = LoadCursor(NULL, IDC_ARROW),
        .lpszClassName = L"opengl_window_class",
    };
    ATOM Atom = RegisterClassExW(&WindowClass);
    Assert(Atom && "Failed to register window class");

    int   Width   = CW_USEDEFAULT;
    int   Height  = CW_USEDEFAULT;
    DWORD EXStyle = WS_EX_APPWINDOW;
    DWORD Style   = WS_OVERLAPPEDWINDOW;

    HWND Window = CreateWindowExW(
        EXStyle, WindowClass.lpszClassName, L"OpenGL Window", Style,
        CW_USEDEFAULT, CW_USEDEFAULT, Width, Height,
        NULL, NULL, WindowClass.hInstance, NULL);
    Assert(Window && "Failed to create window");

    HDC DeviceContext = GetDC(Window);
    Assert(DeviceContext && "Failed to window device context");

    SetOpenGLPixelFormat(DeviceContext);
    CreateOpenGLContext(DeviceContext);

    BOOL vsync = TRUE;
    wglSwapIntervalEXT(vsync ? 1 : 0);

    ShowWindow(Window, SW_SHOWDEFAULT);

    LARGE_INTEGER freq, c1;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&c1);

    // A bunch of random init code.
    platform_context Platform = {0};
    Platform.ReadFile         = Win32ReadFile;
    Platform.Allocate         = Win32Allocate;
    Platform.Free             = Win32Free;

    Platform.ListDirectoryFiles = Win32ListDirectoryFiles;
    Platform.FreeFileList       = Win32FreeFileList;

    Platform.GetClientSize = Win32GetClientSize;
    Platform.DeviceContext = DeviceContext;
    Platform.WindowHandle  = Window;

    DWORD Length = GetCurrentDirectoryA(MAX_PLATFORM_PATH, Platform.Root);
    if (Length == 0 || Length >= MAX_PLATFORM_PATH) 
    {
        FatalError("GetCurrentDirectoryA failed or path too long");
    }

    size_t BaseLength   = Length;
    const char* Suffix  = "/../sim/game";
    size_t SuffixLength = strlen(Suffix);
    if (BaseLength + SuffixLength + 1 > MAX_PLATFORM_PATH) 
    {
        FatalError("Resulting path would overflow Platform.Root");
    }

    i32 Written = snprintf(Platform.Root + BaseLength,
                           MAX_PLATFORM_PATH - BaseLength, "%s", Suffix);
    if (Written < 0 || (size_t)Written >= MAX_PLATFORM_PATH - BaseLength) 
    {
        FatalError("snprintf failed or truncated");
    }


    transient_allocator Allocator = InitializeTransientAllocator(Megabytes(500));
    game_state          GameState = {0};

    game_input Input = {0};
    Input.DtForFrame = 0.006f;

    u32 Noob = LoadStaticMeshFromDisk(nullptr, &Platform, &Allocator);
    UNUSED(Noob);

    for (;;)
    {
        if(!Win32ProcessMessages(&Input.Controller)) break;

        // WARN: Move this in the update code?
        RECT Rect;
        GetClientRect(Window, &Rect);
        Width  = Rect.right  - Rect.left;
        Height = Rect.bottom - Rect.top;

        if (Width != 0 && Height != 0)
        {
            UpdateGameAndRender(&GameState, &Input.Controller, &Allocator, &Platform,
                                Input.DtForFrame);
        }
        else
        {
            if (vsync)
            {
                Sleep(10);
            }
        }
    }
}
