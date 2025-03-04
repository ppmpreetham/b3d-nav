#include "BlenderControlsStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

TSharedPtr<FSlateStyleSet> FBlenderControlsStyle::StyleInstance = nullptr;

void FBlenderControlsStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FBlenderControlsStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FBlenderControlsStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("BlenderControlsStyle"));
	return StyleSetName;
}

void FBlenderControlsStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FBlenderControlsStyle::Get()
{
	return *StyleInstance;
}

TSharedRef<FSlateStyleSet> FBlenderControlsStyle::Create()
{
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("BlenderControlsStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("BlenderControls")->GetBaseDir() / TEXT("Resources"));

	// Default icon size is 40x40
	Style->Set("BlenderControls.PluginAction", new FSlateImageBrush(Style->RootToContentDir(TEXT("ButtonIcon_40x"), TEXT(".png")), FVector2D(40.0f, 40.0f)));

	return Style;
}