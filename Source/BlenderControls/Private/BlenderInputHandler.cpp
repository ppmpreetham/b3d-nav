/*
* Blender Controls Plugin for Unreal Engine
* Created by: ppmpreetham
* Date: 2025-03-04
*/

#include "BlenderInputHandler.h"
#include "Framework/Application/SlateApplication.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"

FBlenderInputHandler::FBlenderInputHandler()
	: bIsEnabled(false)
	, bIsTransforming(false)
	, bIsNumericInput(false)
	, CurrentMode(EBlenderTransformMode::None)
	, CurrentAxis(EBlenderTransformAxis::None)
	, NumericBuffer(TEXT(""))
	, LastMousePosition(FVector2D::ZeroVector)
{
	RegisterInputProcessor();
}

FBlenderInputHandler::~FBlenderInputHandler()
{
	UnregisterInputProcessor();
}

void FBlenderInputHandler::RegisterInputProcessor()
{
	if (bIsEnabled && FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().RegisterInputPreProcessor(SharedThis(this));
	}
}

void FBlenderInputHandler::UnregisterInputProcessor()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(SharedThis(this));
	}
}

void FBlenderInputHandler::ToggleEnabled()
{
	bIsEnabled = !bIsEnabled;
	
	if (bIsEnabled)
	{
		RegisterInputProcessor();
		UE_LOG(LogTemp, Display, TEXT("Blender Controls: Enabled"));
	}
	else
	{
		UnregisterInputProcessor();
		UE_LOG(LogTemp, Display, TEXT("Blender Controls: Disabled"));
	}
}

void FBlenderInputHandler::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
	// Nothing needed for tick processing
}

bool FBlenderInputHandler::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	if (!bIsEnabled)
	{
		return false;
	}

	const FKey Key = InKeyEvent.GetKey();

	// Check if we're currently in transform mode
	if (bIsTransforming)
	{
		// Check for axis keys
		EBlenderTransformAxis Axis;
		if (IsAxisKey(InKeyEvent, Axis))
		{
			SetTransformAxis(Axis);
			return true;
		}
		
		// Check for numeric input
		FString Digit;
		if (IsNumericKey(InKeyEvent, Digit))
		{
			bIsNumericInput = true;
			NumericBuffer.Append(Digit);
			UE_LOG(LogTemp, Display, TEXT("Numeric Input: %s"), *NumericBuffer);
			return true;
		}

		// Enter key applies numeric transform
		if (Key == EKeys::Enter || Key == EKeys::SpaceBar)
		{
			if (bIsNumericInput && !NumericBuffer.IsEmpty())
			{
				ApplyNumericTransform();
			}
			EndTransform(true);
			return true;
		}

		// Escape key cancels transform
		if (Key == EKeys::Escape)
		{
			EndTransform(false);
			return true;
		}

		return false;
	}
	else
	{
		// Check for transform mode initializers
		if (Key == EKeys::G)
		{
			BeginTransform(EBlenderTransformMode::Grab);
			return true;
		}
		else if (Key == EKeys::R)
		{
			BeginTransform(EBlenderTransformMode::Rotate);
			return true;
		}
		else if (Key == EKeys::S)
		{
			BeginTransform(EBlenderTransformMode::Scale);
			return true;
		}
	}

	return false;
}

bool FBlenderInputHandler::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	// We don't need to handle key up events in most cases
	return false;
}

bool FBlenderInputHandler::HandleMouseMoveEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (!bIsEnabled || !bIsTransforming || bIsNumericInput)
	{
		return false;
	}
	
	// Calculate mouse delta
	const FVector2D CurrentPosition = MouseEvent.GetScreenSpacePosition();
	const FVector2D Delta = CurrentPosition - LastMousePosition;
	LastMousePosition = CurrentPosition;

	if (!Delta.IsNearlyZero())
	{
		// Use mouse delta to determine transform amount
		// Scale factor can be adjusted for better feel
		const float ScaleFactor = 0.01f;
		const float TransformValue = Delta.X * ScaleFactor;
		
		// Get the appropriate axis vector
		FVector AxisVector = GetAxisVector(CurrentAxis);
		
		// Apply the transformation
		ApplyTransform(AxisVector, TransformValue);
		
		return true;
	}
	
	return false;
}

bool FBlenderInputHandler::HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	if (!bIsEnabled || !bIsTransforming)
	{
		return false;
	}
	
	// Left mouse button applies the transformation
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		EndTransform(true);
		return true;
	}
	
	// Right mouse button cancels the transformation
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		EndTransform(false);
		return true;
	}
	
	return false;
}

bool FBlenderInputHandler::HandleMouseButtonUpEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent)
{
	// We handle the actions on button down instead
	return false;
}

void FBlenderInputHandler::BeginTransform(EBlenderTransformMode Mode)
{
	if (!GEditor || bIsTransforming)
	{
		return;
	}
	
	// Store the current selection state for potential cancellation
	// In a real implementation, you'd want to store the original transform of all selected actors
	
	bIsTransforming = true;
	CurrentMode = Mode;
	CurrentAxis = EBlenderTransformAxis::All; // Default to all axes
	bIsNumericInput = false;
	NumericBuffer.Empty();
	
	// Capture current mouse position for relative movement
	if (FSlateApplication::IsInitialized())
	{
		LastMousePosition = FSlateApplication::Get().GetCursorPos();
	}
	
	// Log the mode
	FString ModeText;
	switch (Mode)
	{
	case EBlenderTransformMode::Grab:
		ModeText = TEXT("Grab (Translation)");
		break;
	case EBlenderTransformMode::Rotate:
		ModeText = TEXT("Rotate");
		break;
	case EBlenderTransformMode::Scale:
		ModeText = TEXT("Scale");
		break;
	default:
		ModeText = TEXT("Unknown");
		break;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Begin Transform: %s"), *ModeText);
}

void FBlenderInputHandler::EndTransform(bool bApply)
{
	if (!bIsTransforming)
	{
		return;
	}
	
	if (!bApply)
	{
		// Restore original transforms here (in a real implementation)
		UE_LOG(LogTemp, Display, TEXT("Transform Cancelled"));
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Transform Applied"));
	}
	
	bIsTransforming = false;
	CurrentMode = EBlenderTransformMode::None;
	CurrentAxis = EBlenderTransformAxis::None;
	bIsNumericInput = false;
	NumericBuffer.Empty();
}

void FBlenderInputHandler::ApplyTransform(const FVector& Direction, float Value)
{
	if (!bIsTransforming || !GEditor)
	{
		return;
	}

	TransformSelectedActors(Direction, Value);
}

void FBlenderInputHandler::ApplyNumericTransform()
{
	if (!bIsTransforming || NumericBuffer.IsEmpty())
	{
		return;
	}
	
	float NumericValue = FCString::Atof(*NumericBuffer);
	FVector AxisVector = GetAxisVector(CurrentAxis);
	
	TransformSelectedActors(AxisVector, NumericValue);
	
	UE_LOG(LogTemp, Display, TEXT("Applied Numeric Transform: %f on axis %d"), NumericValue, static_cast<int32>(CurrentAxis));
}

void FBlenderInputHandler::SetTransformAxis(EBlenderTransformAxis Axis)
{
	if (CurrentAxis == Axis)
	{
		// Toggle back to all axes if the same axis is selected twice
		CurrentAxis = EBlenderTransformAxis::All;
	}
	else
	{
		CurrentAxis = Axis;
	}
	
	FString AxisText;
	switch (CurrentAxis)
	{
	case EBlenderTransformAxis::X:
		AxisText = TEXT("X");
		break;
	case EBlenderTransformAxis::Y:
		AxisText = TEXT("Y");
		break;
	case EBlenderTransformAxis::Z:
		AxisText = TEXT("Z");
		break;
	case EBlenderTransformAxis::All:
		AxisText = TEXT("All");
		break;
	default:
		AxisText = TEXT("None");
		break;
	}
	
	UE_LOG(LogTemp, Display, TEXT("Transform Axis Set: %s"), *AxisText);
}

FVector FBlenderInputHandler::GetAxisVector(EBlenderTransformAxis Axis)
{
	switch (Axis)
	{
	case EBlenderTransformAxis::X:
		return FVector(1.0f, 0.0f, 0.0f);
	case EBlenderTransformAxis::Y:
		return FVector(0.0f, 1.0f, 0.0f);
	case EBlenderTransformAxis::Z:
		return FVector(0.0f, 0.0f, 1.0f);
	case EBlenderTransformAxis::All:
	default:
		// For "all axes" in translation, use X (could be improved to use camera forward)
		// For rotation/scaling, this would be handled differently
		if (CurrentMode == EBlenderTransformMode::Grab)
		{
			return FVector(1.0f, 1.0f, 1.0f);
		}
		else if (CurrentMode == EBlenderTransformMode::Rotate)
		{
			return FVector(0.0f, 0.0f, 1.0f); // Default to Z-axis rotation
		}
		else // Scale
		{
			return FVector(1.0f, 1.0f, 1.0f); // Uniform scale
		}
	}
}

bool FBlenderInputHandler::IsTransformKey(const FKeyEvent& KeyEvent)
{
	const FKey Key = KeyEvent.GetKey();
	return Key == EKeys::G || Key == EKeys::R || Key == EKeys::S;
}

bool FBlenderInputHandler::IsAxisKey(const FKeyEvent& KeyEvent, EBlenderTransformAxis& OutAxis)
{
	const FKey Key = KeyEvent.GetKey();
	
	if (Key == EKeys::X)
	{
		OutAxis = EBlenderTransformAxis::X;
		return true;
	}
	else if (Key == EKeys::Y)
	{
		OutAxis = EBlenderTransformAxis::Y;
		return true;
	}
	else if (Key == EKeys::Z)
	{
		OutAxis = EBlenderTransformAxis::Z;
		return true;
	}
	
	return false;
}

bool FBlenderInputHandler::IsNumericKey(const FKeyEvent& KeyEvent, FString& OutDigit)
{
	const FKey Key = KeyEvent.GetKey();
	
	// Check for number keys (0-9)
	if (Key == EKeys::Zero || Key == EKeys::NumPadZero)
	{
		OutDigit = TEXT("0");
		return true;
	}
	else if (Key == EKeys::One || Key == EKeys::NumPadOne)
	{
		OutDigit = TEXT("1");
		return true;
	}
	else if (Key == EKeys::Two || Key == EKeys::NumPadTwo)
	{
		OutDigit = TEXT("2");
		return true;
	}
	else if (Key == EKeys::Three || Key == EKeys::NumPadThree)
	{
		OutDigit = TEXT("3");
		return true;
	}
	else if (Key == EKeys::Four || Key == EKeys::NumPadFour)
	{
		OutDigit = TEXT("4");
		return true;
	}
	else if (Key == EKeys::Five || Key == EKeys::NumPadFive)
	{
		OutDigit = TEXT("5");
		return true;
	}
	else if (Key == EKeys::Six || Key == EKeys::NumPadSix)
	{
		OutDigit = TEXT("6");
		return true;
	}
	else if (Key == EKeys::Seven || Key == EKeys::NumPadSeven)
	{
		OutDigit = TEXT("7");
		return true;
	}
	else if (Key == EKeys::Eight || Key == EKeys::NumPadEight)
	{
		OutDigit = TEXT("8");
		return true;
	}
	else if (Key == EKeys::Nine || Key == EKeys::NumPadNine)
	{
		OutDigit = TEXT("9");
		return true;
	}
	else if (Key == EKeys::Period)
	{
		// Allow decimal point
		OutDigit = TEXT(".");
		return true;
	}
	else if (Key == EKeys::Hyphen || Key == EKeys::Subtract)
	{
		// Allow negative numbers
		OutDigit = TEXT("-");
		return true;
	}
	
	return false;
}

void FBlenderInputHandler::TransformSelectedActors(const FVector& Direction, float Value)
{
	if (!GEditor)
	{
		return;
	}
	
	USelection* SelectedActors = GEditor->GetSelectedActors();
	for (FSelectionIterator It(*SelectedActors); It; ++It)
	{
		if (AActor* Actor = Cast<AActor>(*It))
		{
			// Get the current transform
			FTransform ActorTransform = Actor->GetActorTransform();
			
			// Apply the transformation based on mode
			switch (CurrentMode)
			{
			case EBlenderTransformMode::Grab:
				{
					// Translation
					FVector Translation = Direction * Value;
					ActorTransform.AddToTranslation(Translation);
					break;
				}
			case EBlenderTransformMode::Rotate:
				{
					// Rotation (convert to degrees)
					const float AngleInDegrees = Value * 50.0f;  // Scaling factor for better control
					FQuat DeltaRotation = FQuat(Direction, FMath::DegreesToRadians(AngleInDegrees));
					ActorTransform.SetRotation(DeltaRotation * ActorTransform.GetRotation());
                    break;
				}
			case EBlenderTransformMode::Scale:
				{
					// Scaling (make sure to never scale by 0)
					FVector Scale = FVector::OneVector;
					
					if (CurrentAxis == EBlenderTransformAxis::All)
					{
						// Uniform scaling for all axes
						float ScaleFactor = 1.0f + Value;
						Scale *= ScaleFactor;
					}
					else
					{
						// Scale specific axis
						Scale = FVector::OneVector;
						if (Direction.X > 0.0f) Scale.X = 1.0f + Value;
						if (Direction.Y > 0.0f) Scale.Y = 1.0f + Value;
						if (Direction.Z > 0.0f) Scale.Z = 1.0f + Value;
					}
					
					// Apply the scale
					FVector NewScale = ActorTransform.GetScale3D() * Scale;
					
					// Prevent zero or negative scale
					NewScale.X = FMath::Max(0.001f, NewScale.X);
					NewScale.Y = FMath::Max(0.001f, NewScale.Y);
					NewScale.Z = FMath::Max(0.001f, NewScale.Z);
					
					ActorTransform.SetScale3D(NewScale);
					break;
				}
			default:
				break;
			}
			
			// Apply the new transform to the actor
			Actor->SetActorTransform(ActorTransform, false, nullptr, ETeleportType::None);
		}
	}
	
	// Request redraw of viewports
	if (GEditor)
	{
		GEditor->RedrawLevelEditingViewports();
	}
}