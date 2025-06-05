typedef struct
{
    i32 ClientWidth;
    i32 ClientHeight;

    f32  FieldOfView;
    vec3 Position;
    vec3 Forward;
    vec3 Up;
    vec3 Right;
    mat4 WorldToCamera;
    mat4 CameraToClip;

    bool Initialized;
} camera;

internal void
UpdateCamera(camera *Camera, game_controller_input *Input,
             platform_context *Platform, f32 DeltaTime)
{
    if (!Camera->Initialized)
    {
        Platform->GetClientSize(Platform->WindowHandle, &Camera->ClientWidth,
                                &Camera->ClientHeight);

        Camera->FieldOfView = 45.0f;
        Camera->Position    = Vec3(0.0f, 0.0f, 5.0f);
        Camera->Forward     = Vec3(0.0f, 0.0f, -1.0f);
        Camera->Up          = Vec3(0.0f, 1.0f, 0.0f);
        Camera->Right       = Vec3_VectorProduct(Camera->Forward, Camera->Up);

        vec3    Target        = Vec3_Add(Camera->Position, Camera->Forward);
        Camera->WorldToCamera = Mat4_FocusMatrix(Camera->Up, Camera->Position, Target);

        if (Camera->ClientHeight > 0)
        {
            f32     AspectRatio = (f32)Camera->ClientWidth / Camera->ClientHeight;
            Camera->CameraToClip = Mat4_Perspective(Camera->FieldOfView, AspectRatio,
                                                    0.1f, 100.0f);

            Camera->Initialized = true;
        }

        /*glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);*/
    }

    f32  CameraSpeed   = 10.0f;
    vec3 MovementDelta = {0};

    if(Input->Actions.CameraForward.EndedDown)
    {
        MovementDelta = Vec3_Scale(Camera->Forward, CameraSpeed * DeltaTime);
    }
    if(Input->Actions.CameraBackward.EndedDown)
    {
        MovementDelta = Vec3_Add(MovementDelta, Vec3_Scale(Camera->Forward,
                                                           -CameraSpeed * DeltaTime));
    }
    if(Input->Actions.CameraRight.EndedDown)
    {
        MovementDelta = Vec3_Add(MovementDelta, Vec3_Scale(Camera->Right,
                                                           CameraSpeed * DeltaTime));
    }
    if(Input->Actions.CameraLeft.EndedDown)
    {
        MovementDelta = Vec3_Add(MovementDelta, Vec3_Scale(Camera->Right,
                                                           -CameraSpeed * DeltaTime));
    }
    if(Input->Actions.CameraUp.EndedDown)
    {
        MovementDelta = Vec3_Add(MovementDelta, Vec3_Scale(Camera->Up,
                                                           CameraSpeed * DeltaTime));
    }
    if(Input->Actions.CameraDown.EndedDown)
    {
        MovementDelta = Vec3_Add(MovementDelta, Vec3_Scale(Camera->Up,
                                                           -CameraSpeed * DeltaTime));
    }

    Camera->Position = Vec3_Add(Camera->Position, MovementDelta);

    vec3 Target           = Vec3_Add(Camera->Position, Camera->Forward);
    Camera->WorldToCamera = Mat4_FocusMatrix(Camera->Up, Camera->Position, Target);
}
