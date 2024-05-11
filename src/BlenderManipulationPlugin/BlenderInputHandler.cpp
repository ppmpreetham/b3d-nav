// BlenderInputHandler.h

#include BlenderInputHandler.h
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"

void ABlenderInputHandler::ManipulateObject()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (!PlayerController)
        return;

    // Get mouse position
    FVector2D MousePosition;
    PlayerController->GetMousePosition(MousePosition.X, MousePosition.Y);

    // Convert mouse position to world space
    FVector WorldLocation, WorldDirection;
    PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

    if (CurrentAction == "Grab")
    {
        // Moveing along the camera's view plane
        FVector NewLocation = WorldLocation + WorldDirection * NumericInputValue;
        SetActorLocation(NewLocation);
    }
    else if (CurrentAction == "Rotate")
    {
        // Calculate rotation axis
        FVector RotationAxis = WorldDirection;
        RotationAxis.Z = 0; // Ensure rotation is tangent to the screen
        RotationAxis.Normalize();

        // Rotating the object around the calculated axis
        FRotator NewRotation = FRotator(0, NumericInputValue, 0);
        AddActorWorldRotation(NewRotation);
    }
    else if (CurrentAction == "Scale")
    {
        // Scaling uniformly
        FVector NewScale = GetActorScale3D() * (1 + NumericInputValue);
        SetActorScale3D(NewScale);
    }

    // Apply axis locking if enabled
    if (bIsAxisLocked)
    {
        // Constrain manipulation to the locked axis
        if (LockedAxis.X != 0)
        {
            // If X axis is locked, reset Y and Z components
            SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z));
        }
        else if (LockedAxis.Y != 0)
        {
            // If Y axis is locked, reset X and Z components
            SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z));
        }
        else if (LockedAxis.Z != 0)
        {
            // If Z axis is locked, reset X and Y components
            SetActorLocation(FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z));
        }
    }
}