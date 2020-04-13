// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ESWSample.h"
#include "ESWSampleStyle.h"
#include "ESWSampleCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

static const FName ESWSampleTabName("ESWSample");

#define LOCTEXT_NAMESPACE "FESWSampleModule"

void RefreshLevelEditorToolBar()
{
	const FName LevelEditorModuleName("LevelEditor");
	const FTabId ToolbarTabId(FName("LevelEditorToolBar"));
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LevelEditorModuleName);
	if (auto TabManager = LevelEditorModule.GetLevelEditorTabManager())
	{
		if (auto Toolbar = TabManager->FindExistingLiveTab(ToolbarTabId))
		{
			Toolbar->RequestCloseTab();
			Toolbar.Reset();
			TabManager->InvokeTab(ToolbarTabId);
		}
	}
}
class FESWSampleImpl
{
public:
	FESWSampleImpl()
	{
	}
	~FESWSampleImpl()
	{
		if (PluginTab.IsValid())
		{
			if (auto PluginTabPtr = PluginTab.Pin())
			{
				PluginTabPtr->RequestCloseTab();
			}
			PluginTab.Reset();
		}
	}

public:
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);

private:
	TWeakPtr<SDockTab> PluginTab;
};

void FESWSampleModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FESWSampleStyle::Initialize();
	FESWSampleStyle::ReloadTextures();

	FESWSampleCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);
	Impl = MakeShareable(new FESWSampleImpl);

	PluginCommands->MapAction(
		FESWSampleCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::PluginButtonClicked),
		FCanExecuteAction());
		
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	
	{
		TSharedPtr<FExtender> MenuExtender = MakeShareable(new FExtender());
		MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ESWSampleTabName, FOnSpawnTab::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FESWSampleTabTitle", "ESWSample"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FESWSampleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	Impl.Reset();
	PluginCommands.Reset();

	RefreshLevelEditorToolBar();

	FESWSampleStyle::Shutdown();

	FESWSampleCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ESWSampleTabName);
}

TSharedRef<SDockTab> FESWSampleImpl::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FESWSampleModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("ESWSample.cpp"))
		);

	return SAssignNew(PluginTab, SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			// Put your tab content here!
			SNew(SBox)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(WidgetText)
			]
		];
}

void FESWSampleImpl::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->InvokeTab(ESWSampleTabName);
}

void FESWSampleImpl::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FESWSampleCommands::Get().OpenPluginWindow);
}

void FESWSampleImpl::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FESWSampleCommands::Get().OpenPluginWindow);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FESWSampleModule, ESWSample)