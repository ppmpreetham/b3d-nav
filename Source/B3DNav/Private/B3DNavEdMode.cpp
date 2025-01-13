#include "B3DNavEdMode.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"
#include "Engine/Selection.h"
#include "Editor.h"

const FEditorModeID FB3DNavEdMode::EM_B3DNavEdModeId = TEXT("EM_B3DNav");

FB3DNavEdMode::FB3DNavEdMode()
    : CurrentModalState(EModalState::None)
    , bAxisLocked(false)
    , LockedAxis(-1)
    , CurrentValue(0.0f)
{
}

FB3DNavEdMode::~FB3DNavEdMode()
{
}

void FB3DNavEdMode::Enter()
{
    FEdMode::Enter();
    
    // Hide the regular transform widgets when entering the mode
    GetModeManager()->GetSelectedMode()->GetModeManager()->SetShowWidget(false);
}

void FB3DNavEdMode::Exit()
{
    ExitCurrentMode(false);
    FEdMode::Exit();
}

bool FB3DNavEdMode::UsesToolkits() const
{
    return false;
}

void FB3DNavEdMode::EnterTranslateMode(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
    CurrentModalState = EModalState::Translate;
    CacheSelectedActors();
    
    // Capture initial mouse position
    FIntPoint MousePos;
    Viewport->GetMousePos(MousePos);
    TransformData.MouseStart = FVector2D(MousePos);
    
    // Cache initial transforms
    for (AActor* Actor : SelectedActors)
    {
        FTransformData Data;
        Data.OriginalLocation = Actor->GetActorLocation();
        ActorTransformData.Add(Data);
    }
    
    // Capture mouse
    Viewport->CaptureMouse(true);
    FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void FB3DNavEdMode::EnterRotateMode(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
    CurrentModalState = EModalState::Rotate;
    CacheSelectedActors();
    
    FIntPoint MousePos;
    Viewport->GetMousePos(MousePos);
    TransformData.MouseStart = FVector2D(MousePos);
    
    for (AActor* Actor : SelectedActors)
    {
        FTransformData Data;
        Data.OriginalRotation = Actor->GetActorRotation();
        ActorTransformData.Add(Data);
    }
    
    Viewport->CaptureMouse(true);
    FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

void FB3DNavEdMode::EnterScaleMode(FEditorViewportClient* ViewportClient, FViewport* Viewport)
{
    CurrentModalState = EModalState::Scale;
    CacheSelectedActors();
    
    FIntPoint MousePos;
    Viewport->GetMousePos(MousePos);
    TransformData.MouseStart = FVector2D(MousePos);
    
    for (AActor* Actor : SelectedActors)
    {
        FTransformData Data;
        Data.OriginalScale = Actor->GetActorScale3D();
        ActorTransformData.Add(Data);
    }
    
    Viewport->CaptureMouse(true);
    FSlateApplication::Get().SetAllUserFocusToGameViewport();
}

bool FB3DNavEdMode::MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY)
{
    if (CurrentModalState == EModalState::None)
    {
        return false;
    }

    FVector2D CurrentMouse(MouseX, MouseY);
    FVector2D MouseDelta = CurrentMouse - TransformData.MouseStart;
    
    UpdateTransform(ViewportClient, Viewport, MouseDelta);
    return true;
}

void FB3DNavEdMode::UpdateTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FVector2D& MouseDelta)
{
    if (TransformData.bIsNumericInput)
    {
        return;
    }

    switch (CurrentModalState)
    {
        case EModalState::Translate:
        {
            FVector DragDelta = GetDragDelta(ViewportClient, MouseDelta);
            if (bAxisLocked)
            {
                // Zero out non-locked axes
                FVector AxisMask = FVector::ZeroVector;
                if (LockedAxis >= 0 && LockedAxis < 3)
                {
                    AxisMask[LockedAxis] = 1.0f;
                }
                DragDelta *= AxisMask;
            }

            for (int32 i = 0; i < SelectedActors.Num(); ++i)
            {
                AActor* Actor = SelectedActors[i];
                const FTransformData& Data = ActorTransformData[i];
                Actor->SetActorLocation(Data.OriginalLocation + DragDelta);
            }
            break;
        }
        case EModalState::Rotate:
        {
            float RotationDelta = MouseDelta.X * 0.5f; // Adjust sensitivity as needed
            if (bAxisLocked)
            {
                FRotator Rotation = FRotator::ZeroRotator;
                if (LockedAxis == 0) Rotation.Roll = RotationDelta;
                else if (LockedAxis == 1) Rotation.Pitch = RotationDelta;
                else if (LockedAxis == 2) Rotation.Yaw = RotationDelta;

                for (int32 i = 0; i < SelectedActors.Num(); ++i)
                {
                    AActor* Actor = SelectedActors[i];
                    const FTransformData& Data = ActorTransformData[i];
                    Actor->SetActorRotation(Data.OriginalRotation + Rotation);
                }
            }
            else
            {
                // Default to yaw rotation if no axis is locked
                for (int32 i = 0; i < SelectedActors.Num(); ++i)
                {
                    AActor* Actor = SelectedActors[i];
                    const FTransformData& Data = ActorTransformData[i];
                    FRotator NewRotation = Data.OriginalRotation;
                    NewRotation.Yaw += RotationDelta;
                    Actor->SetActorRotation(NewRotation);
                }
            }
            break;
        }
        case EModalState::Scale:
        {
            float ScaleFactor = 1.0f + (MouseDelta.X * 0.01f);
            if (ScaleFactor < 0.01f) ScaleFactor = 0.01f;

            for (int32 i = 0; i < SelectedActors.Num(); ++i)
            {
                AActor* Actor = SelectedActors[i];
                const FTransformData& Data = ActorTransformData[i];
                
                if (bAxisLocked)
                {
                    FVector NewScale = Data.OriginalScale;
                    if (LockedAxis >= 0 && LockedAxis < 3)
                    {
                        NewScale[LockedAxis] *= ScaleFactor;
                    }
                    Actor->SetActorScale3D(NewScale);
                }
                else
                {
                    Actor->SetActorScale3D(Data.OriginalScale * ScaleFactor);
                }
            }
            break;
        }
    }

    ViewportClient->Invalidate();
}

FVector FB3DNavEdMode::GetDragDelta(FEditorViewportClient* ViewportClient, const FVector2D& MouseDelta) const
{
    FVector DragDelta;
    
    if (ViewportClient->IsOrtho())
    {
        float Scale = ViewportClient->GetOrthoZoom() * 0.01f;
        DragDelta = FVector(MouseDelta.X * Scale, MouseDelta.Y * Scale, 0.f);
        
        switch (ViewportClient->GetViewportType())
        {
            case LVT_OrthoXY:
                DragDelta = FVector(DragDelta.X, DragDelta.Y, 0.f);
                break;
            case LVT_OrthoXZ:
                DragDelta = FVector(DragDelta.X, 0.f, DragDelta.Y);
                break;
            case LVT_OrthoYZ:
                DragDelta = FVector(0.f, DragDelta.X, DragDelta.Y);
                break;
        }
    }
    else
    {
        FVector MoveDelta = FVector(MouseDelta.X, MouseDelta.Y, 0.f);
        FVector ViewDir = ViewportClient->GetViewRotation().Vector();
        DragDelta = FVector::VectorPlaneProject(MoveDelta, ViewDir) * ViewportClient->GetOrthoZoom() * 0.01f;
    }
    
    return DragDelta;
}

bool FB3DNavEdMode::InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event)
{
    if (Event == IE_Pressed)
    {
        // Handle numeric input
        if (Key.IsNumeric() || Key == EKeys::Period || Key == EKeys::Minus)
        {
            if (CurrentModalState != EModalState::None)
            {
                TransformData.bIsNumericInput = true;
                TransformData.NumericInput += Key.ToString();
                return true;
            }
        }
        
        // Modal mode activation
        if (Key == EKeys::G)
        {
            EnterTranslateMode(ViewportClient, Viewport);
            return true;
        }
        else if (Key == EKeys::R)
        {
            EnterRotateMode(ViewportClient, Viewport);
            return true;
        }
        else if (Key == EKeys::S)
        {
            EnterScaleMode(ViewportClient, Viewport);
            return true;
        }
        
        // Axis constraints
        if (CurrentModalState != EModalState::None)
        {
            if (Key == EKeys::X)
            {
                bAxisLocked = true;
                LockedAxis = 0;
                return true;
            }
            else if (Key == EKeys::Y)
            {
                bAxisLocked = true;
                LockedAxis = 1;
                return true;
            }
            else if (Key == EKeys::Z)
            {
                bAxisLocked = true;
                LockedAxis = 2;
                return true;
            }
        }
        
        // Confirm or cancel transformation
        if (Key == EKeys::Enter || Key == EKeys::LeftMouseButton)
        {
            if (CurrentModalState != EModalState::None)
            {
                ExitCurrentMode(true);
                return true;
            }
        }
        else if (Key == EKeys::Escape || Key == EKeys::RightMouseButton)
        {
            if (CurrentModalState != EModalState::None)
            {
                ExitCurrentMode(false);
                return true;
            }
        }
    }
    
    return FEdMode::InputKey(ViewportClient, Viewport, Key, Event);
}

void FB3DNavEdMode::ExitCurrentMode(bool bAccept)
{
    if (!bAccept)
    {
        // Restore original transforms
        for (int32 i = 0; i < SelectedActors.Num(); ++i)
        {
            AActor* Actor = SelectedActors[i];
            const FTransformData& Data = ActorTransformData[i];
            
            switch (CurrentModalState)
            {
                case EModalState::Translate:
                    Actor->SetActorLocation(Data.OriginalLocation);
                    break;
                case EModalState::Rotate:
                    Actor->SetActorRotation(Data.OriginalRotation);
                    break;
                case EModalState::Scale:
                    Actor->SetActorScale3D(Data.OriginalScale);
                    break;
            }
        }
    }

    // Reset state
    CurrentModalState = EModalState::None;
    bAxisLocked = false;
    LockedAxis = -1;
    TransformData = FTransformData();
    SelectedActors.Empty();
    ActorTransformData.Empty();
    
    // Release mouse capture
    if (GEditor && GEditor->GetActiveViewport())
    {
        GEditor->GetActiveViewport()->CaptureMouse(false);
    }
}

void FB3DNavEdMode::CacheSelectedActors()
{
    SelectedActors.Empty();
    ActorTransformData.Empty();
    GEditor->GetSelectedActors()->GetSelectedObjects<AActor>(SelectedActors);
}

void FB3DNavEdMode::Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI)
{
    FEdMode::Render(View, Viewport, PDI);
}