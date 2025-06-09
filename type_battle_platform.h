#include <stdbool.h>

#define internal      static
#define local_persist static 
#define global        static

#define Kilobytes(Amount) ((Amount) * 1024)
#define Megabytes(Amount) (Kilobytes(Amount) * 1024)
#define Gigabytes(Amount) (Megabytes(Amount) * 1024)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

#define MAX_PLATFORM_PATH 256

typedef struct
{
    u8*    Content;
    size_t ContentSize;
} platform_read_result;

typedef struct transient_allocator transient_allocator;

typedef platform_read_result PlatformReadFile   (const char *Path, transient_allocator *Memory);
typedef bool                 PlatformWriteFile  (char *Path, void* Data, size_t Size);
typedef bool                 PlatformDeleteFile (char *Path);

typedef void                 PlatformListDirectoryFiles    (char* Directory, const char* Extension, transient_allocator* Allocator,
                                                            char** OutList, u32* OutCount);
typedef void                 PlatformFreeFileList          (char* List, u32 Count, transient_allocator* Allocator);

// Memory
typedef void*     PlatformAllocate          (u64 Size);
typedef void      PlatformFree              (void* Memory);

// Window
typedef void PlatformGetClientSize (void* WindowHandle, i32 *Width, i32* Height);

typedef struct
{
    bool EndedDown;
    u32  HalfTransitionCount;
} game_button_state;

enum { KEYBOARD_KEY_COUNT          = 256 };
enum { KEYBOARD_EVENT_BUFFER_COUNT = 512 };

struct keyboard_event
{
    u8   VKCode;
    bool IsDown;
};

typedef struct
{
    u8  CharBuffer[KEYBOARD_EVENT_BUFFER_COUNT];
    u32 RecordedCharacters;
    u32 EventHead;
    u32 EventTail;

    i32  MouseX, MouseY, MouseZ;
    game_button_state MouseButtons[5];
} game_controller_input;

#define MOVE_INPUT_HEAD_TO_NEXT(I) (I->EventHead + 1) % KEYBOARD_EVENT_BUFFER_COUNT
#define GET_LAST_WRITTEN_INDEX(I)  (I->EventHead + (KEYBOARD_EVENT_BUFFER_COUNT - 1)) % KEYBOARD_EVENT_BUFFER_COUNT


typedef struct
{
    union
    {
        game_button_state Buttons[1];

        struct
        {
            game_button_state Dummy;
        } Actions;
    };

    i32 MouseX, MouseY, MouseZ;
    game_button_state MouseButtons[5];
} game_controller_input2;

typedef struct
{
    game_controller_input Controller;
    f32                   DtForFrame;
} game_input;
