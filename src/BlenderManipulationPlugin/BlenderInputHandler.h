#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

UCLASS()
class YOURPLUGIN_API ABlenderInputHandler : public AActor
{
    GENERATED_BODY()

public:
    ABlenderInputHandler();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Input handling functions
    void HandleGrabInput();
    void HandleRotateInput();
    void HandleScaleInput();

    void EnterModalMode();     // Modal mode logic
    void ExitModalMode();
    
    void ManipulateObject(); // Screen space manipulation

private:
    bool bIsModalModeActive;
    FString CurrentAction;

    bool bIsAxisLocked; // Axis locking
    FVector LockedAxis;

    float NumericInputValue; // Numeric input
};