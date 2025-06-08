#include "jani.h"
#include "backend/renderers/opengl/jani_opengl.h"

#include "player.cpp"
#include "ui.cpp"

#define TYPE_BATTLE_SHOW_UI

typedef struct
{
    camera         Camera;
    frame_context  Frame;
    player_actions Actions;
    jani_context   UIContext;
} game_state;

internal void
UpdateGameAndRender(game_state *GameState, game_controller_input *Input,
                    transient_allocator *Allocator, platform_context *Platform,
                    f32 DeltaTime)
{
    camera         *Camera    = &GameState->Camera;
    frame_context  *Frame     = &GameState->Frame;
    player_actions *Actions   = &GameState->Actions;
    jani_context   *UIContext = &GameState->UIContext;

    UpdateCamera(Camera, Input, Platform, DeltaTime); 

    BeginFrame(Camera, Frame, Platform, Allocator);

#ifdef TYPE_BATTLE_SHOW_UI
    RenderUI(UIContext);
#endif

    UpdatePlayerActions(Actions);
    RenderGame(Frame);

    EndFrame(Platform->DeviceContext);
}
