// B3DNavEdMode.h
#pragma once

#include "CoreMinimal.h"
#include "EdMode.h"
#include "Framework/Application/SlateApplication.h"

class FB3DNavEdMode : public FEdMode
{
public:
    const static FEditorModeID EM_B3DNavEdModeId;

    FB3DNavEdMode();
    virtual ~FB3DNavEdMode();

    // FEdMode interface
    virtual void Enter() override;
    virtual void Exit() override;
    virtual bool UsesToolkits() const override;
    virtual bool InputKey(FEditorViewportClient* ViewportClient, FViewport* Viewport, FKey Key, EInputEvent Event) override;
    virtual bool InputDelta(FEditorViewportClient* InViewportClient, FViewport* InViewport, FVector& InDrag, FRotator& InRot, FVector& InScale) override;
    virtual void Render(const FSceneView* View, FViewport* Viewport, FPrimitiveDrawInterface* PDI) override;
    virtual bool MouseMove(FEditorViewportClient* ViewportClient, FViewport* Viewport, int32 MouseX, int32 MouseY) override;

private:
    enum class EModalState
    {
        None,
        Translate,
        Rotate,
        Scale
    };

    struct FTransformData
    {
        FVector OriginalLocation;
        FRotator OriginalRotation;
        FVector OriginalScale;
        FVector2D MouseStart;
        FString NumericInput;
        bool bIsNumericInput;
    };
    
    // State tracking
    EModalState CurrentModalState;
    FTransformData TransformData;
    bool bAxisLocked;
    int32 LockedAxis;
    float CurrentValue;
    
    // Selected actors cache
    TArray<AActor*> SelectedActors;
    TArray<FTransformData> ActorTransformData;
    
    // Helper functions
    void EnterTranslateMode(FEditorViewportClient* ViewportClient, FViewport* Viewport);
    void EnterRotateMode(FEditorViewportClient* ViewportClient, FViewport* Viewport);
    void EnterScaleMode(FEditorViewportClient* ViewportClient, FViewport* Viewport);
    void ExitCurrentMode(bool bAccept);
    bool ProcessNumericInput(const FString& Input);
    void UpdateTransform(FEditorViewportClient* ViewportClient, FViewport* Viewport, const FVector2D& MouseDelta);
    FVector GetDragDelta(FEditorViewportClient* ViewportClient, const FVector2D& MouseDelta) const;
    void CacheSelectedActors();
};

// B3DNavEdMode.cpp
#include "B3DNavEdMode.h"
#include "EditorModeManager.h"
#include "EditorViewportClient.h"

FB3DNavEdMode::FB3DNavEdMode()
    : CurrentModalState(EModalState::None)
    , bAxisLocked(false)
    , LockedAxis(-1)
    , CurrentValue(0.0f)
{
}

void FB3DNavEdMode::Enter()
{
    FEdMode::Enter();
    // Hide the regular transform widgets
    GetModeManager()->GetSelectedMode()->GetModeManager()->SetShowWidget(false);
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
                DragDelta = FVector::ZeroVector;
                if (LockedAxis >= 0 && LockedAxis < 3)
                {
                    DragDelta[LockedAxis] = MouseDelta.X * ViewportClient->GetOrthoZoom() * 0.01f;
                }
            }

            // Apply transformation to all selected actors
            for (int32 i = 0; i < SelectedActors.Num(); ++i)
            {
                AActor* Actor = SelectedActors[i];
                const FTransformData& Data = ActorTransformData[i];
                Actor->SetActorLocation(Data.OriginalLocation + DragDelta);
            }
            break;
        }
        // Similar cases for Rotate and Scale...
    }

    // Request viewport redraw
    ViewportClient->Invalidate();
}

FVector FB3DNavEdMode::GetDragDelta(FEditorViewportClient* ViewportClient, const FVector2D& MouseDelta) const
{
    // Convert screen space movement to world space
    FVector DragDelta;
    
    if (ViewportClient->IsOrtho())
    {
        // Handle orthographic viewport drag
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
        // Handle perspective viewport drag
        FVector MoveDelta = FVector(MouseDelta.X, MouseDelta.Y, 0.f);
        FVector RayOrigin;
        FVector RayDirection;
        FVector ViewDir = ViewportClient->GetViewRotation().Vector();
        
        // Project movement onto view plane
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
            Actor->SetActorLocation(Data.OriginalLocation);
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