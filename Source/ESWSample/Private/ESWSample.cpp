// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ESWSample.h"
#include "ESWSampleStyle.h"
#include "ESWSampleCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "SEditorViewportViewMenu.h"
#include "UnrealEdGlobals.h"

static const FName ESWSampleTabName("ESWSample");

#define LOCTEXT_NAMESPACE "FESWSampleModule"

// If your editor implements the extension "ILevelEditor::RegenerateMen()", enable this macro.
//#define IMPLEMENT_ILevelEditor_RegenerateMen

// If your editor implements the extension "SEditorViewportViewMenu::GetMenuExtenders()", enable this macro.
//#define IMPLEMENT_SEditorViewportViewMenu_GetMenuExtenders

void RefreshLevelEditorMenuAndToolBar()
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
	auto LevelEditorWeak = LevelEditorModule.GetLevelEditorInstance();
	if (LevelEditorWeak.IsValid())
	{
		if (auto LevelEditor = LevelEditorWeak.Pin())
		{
#ifdef IMPLEMENT_ILevelEditor_RegenerateMen
			LevelEditor->RegenerateMenu();
#endif
		}
	}
}

void SearchSLevelEditorViewportViewMenuAndRemoveExtension(const TSharedRef<SWidget>& Widget, const TSharedRef< const FExtensionBase >& MenuExtension)
{
	if (Widget->GetType() == FName("SLevelEditorViewportViewMenu"))
	{
		auto EditorViewportViewMenu = StaticCastSharedRef<SEditorViewportViewMenu>(Widget);
#ifdef IMPLEMENT_SEditorViewportViewMenu_GetMenuExtenders
		const TSharedPtr<class FExtender>& MenuExtenders = EditorViewportViewMenu->GetMenuExtenders();
		MenuExtenders->RemoveExtension(MenuExtension);
#endif
	}
	else
	{
		if (auto Children = Widget->GetAllChildren())
		{
			for (int32 ChildIndex = 0; ChildIndex < Children->Num(); ChildIndex++)
			{
				auto Child = Children->GetChildAt(ChildIndex);
				SearchSLevelEditorViewportViewMenuAndRemoveExtension(Child, MenuExtension);
			}
		}
	}
}

void SearchSLevelEditorViewportViewMenuAndRemoveExtension(const TSharedRef< const FExtensionBase >& MenuExtension)
{
	const FName LevelEditorModuleName("LevelEditor");
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(LevelEditorModuleName);
	auto LevelEditorWeak = LevelEditorModule.GetLevelEditorInstance();
	if (LevelEditorWeak.IsValid())
	{
		if (auto LevelEditor = LevelEditorWeak.Pin())
		{
			SearchSLevelEditorViewportViewMenuAndRemoveExtension(LevelEditor.ToSharedRef(), MenuExtension);
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
		MenuExtender = MakeShareable(new FExtender());
		MenuExtension = MenuExtender->AddMenuExtension("WindowLayout", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::AddMenuExtension));

		LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
	}
	
	{
		ToolbarExtender = MakeShareable(new FExtender);
		ToolbarExtension = ToolbarExtender->AddToolBarExtension("Settings", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::AddToolbarExtension));
		
		LevelEditorModule.GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
	}
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ESWSampleTabName, FOnSpawnTab::CreateSP(Impl.ToSharedRef(), &FESWSampleImpl::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FESWSampleTabTitle", "ESWSample"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	RefreshLevelEditorMenuAndToolBar();
}

void FESWSampleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	SearchSLevelEditorViewportViewMenuAndRemoveExtension(MenuExtension.ToSharedRef());

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	LevelEditorModule.GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
	LevelEditorModule.GetToolBarExtensibilityManager()->RemoveExtender(ToolbarExtender);
	MenuExtension.Reset();
	MenuExtender.Reset();
	ToolbarExtension.Reset();
	ToolbarExtender.Reset();
	PluginCommands->UnmapAction(FESWSampleCommands::Get().OpenPluginWindow);

	Impl.Reset();
	PluginCommands.Reset();

	RefreshLevelEditorMenuAndToolBar();

	FESWSampleStyle::Shutdown();

	FESWSampleCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ESWSampleTabName);

	if (GEngine)
	{
		GEngine->ForceGarbageCollection(true);
	}
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