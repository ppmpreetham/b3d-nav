#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Framework/Application/IInputProcessor.h"
#include "EditorViewportClient.h"

enum class EBlenderTransformMode
{
	None,
	Grab,   // 'G' key - Translation
	Rotate, // 'R' key - Rotation
	Scale   // 'S' key - Scale
};

enum class EBlenderTransformAxis
{
	None,
	X,
	Y,
	Z,
	All
};

class FBlenderInputHandler : public TSharedFromThis<FBlenderInputHandler>, public IInputProcessor
{
public:
	FBlenderInputHandler();
	virtual ~FBlenderInputHandler();

	// IInputProcessor interface
	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override;
	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override;
	virtual bool HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;
	virtual bool HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override;

	void ToggleEnabled();
	
private:
	bool bIsEnabled;
	bool bIsTransforming;
	bool bIsNumericInput;
	EBlenderTransformMode CurrentMode;
	EBlenderTransformAxis CurrentAxis;
	FString NumericBuffer;
	FVector2D LastMousePosition;
	
	// Transform functions
	void BeginTransform(EBlenderTransformMode Mode);
	void EndTransform(bool bApply = true);
	void ApplyTransform(const FVector& Direction, float Value);
	void ApplyNumericTransform();
	
	// Helper functions
	void SetTransformAxis(EBlenderTransformAxis Axis);
	FVector GetAxisVector(EBlenderTransformAxis Axis);
	bool IsTransformKey(const FKeyEvent& KeyEvent);
	bool IsAxisKey(const FKeyEvent& KeyEvent, EBlenderTransformAxis& OutAxis);
	bool IsNumericKey(const FKeyEvent& KeyEvent, FString& OutDigit);
	void RegisterInputProcessor();
	void UnregisterInputProcessor();
	void TransformSelectedActors(const FVector& Direction, float Value);
};