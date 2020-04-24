// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ESWSample.h"
#include "ESWSampleStyle.h"
#include "ESWSampleCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "MaterialEditorModule.h"
#include "IMaterialEditor.h"

#define LOCTEXT_NAMESPACE "FESWSampleModule"

namespace
{
	const FName ESWSampleTabName("ESWSample");
	const FText ESWSampleTabTitle(LOCTEXT("ESWSampleTabTitle", "ESWSample"));
}
class FESWSampleInstanceObject : public TSharedFromThis<FESWSampleInstanceObject>
{
public:
	FESWSampleInstanceObject(TWeakPtr<IMaterialEditor> InWeakMaterialEditor)
		: WeakMaterialEditor(InWeakMaterialEditor)
	{
	}
	~FESWSampleInstanceObject()
	{
		if (WeakSpawnedPluginTab.IsValid())
		{
			if (auto SpawnedPluginTab = WeakSpawnedPluginTab.Pin())
			{
				SpawnedPluginTab->RequestCloseTab();
			}
		}

		if (WeakMaterialEditor.IsValid())
		{
			if (auto MaterialEditor = WeakMaterialEditor.Pin())
			{
				if (OnMaterialEditorClosedHandle.IsValid())
				{
					MaterialEditor->OnMaterialEditorClosed().Remove(OnMaterialEditorClosedHandle);
				}
				if (OnUnregisterTabSpawnersHandle.IsValid())
				{
					MaterialEditor->OnUnregisterTabSpawners().Remove(OnUnregisterTabSpawnersHandle);
				}
				if (OnRegisterTabSpawnersHandle.IsValid())
				{
					MaterialEditor->OnRegisterTabSpawners().Remove(OnRegisterTabSpawnersHandle);
				}

				if (ToolbarExtender.IsValid())
				{
					MaterialEditor->GetToolBarExtensibilityManager()->RemoveExtender(ToolbarExtender);
				}
				if (MenuExtender.IsValid())
				{
					MaterialEditor->GetMenuExtensibilityManager()->RemoveExtender(MenuExtender);
				}
			}
		}
		if (PluginCommands.IsValid())
		{
			PluginCommands->UnmapAction(FESWSampleCommands::Get().OpenPluginWindow);
		}
		WeakSpawnedPluginTab.Reset();

		OnMaterialEditorClosedHandle.Reset();
		OnUnregisterTabSpawnersHandle.Reset();
		OnRegisterTabSpawnersHandle.Reset();

		ToolbarExtender.Reset();

		MenuExtender.Reset();

		PluginCommands.Reset();

		if (WeakMaterialEditor.IsValid())
		{
			if (auto MaterialEditor = WeakMaterialEditor.Pin())
			{
				MaterialEditor->RegenerateMenusAndToolbars();
			}
		}
		WeakMaterialEditor.Reset();
	}
	
public:
	void OnMaterialEditorOpened(FESWSampleModule* ESWSampleModule)
	{
		if (WeakMaterialEditor.IsValid())
		{
			if (auto MaterialEditor = WeakMaterialEditor.Pin())
			{
				auto SharedThis = AsShared();
				PluginCommands = MakeShareable(new FUICommandList);
				PluginCommands->MapAction(
					FESWSampleCommands::Get().OpenPluginWindow,
					FExecuteAction::CreateSP(SharedThis, &FESWSampleInstanceObject::PluginButtonClicked),
					FCanExecuteAction());
			
				MenuExtender = MakeShareable(new FExtender);
				MenuExtender->AddMenuExtension("MaterialEditor", EExtensionHook::After, PluginCommands, FMenuExtensionDelegate::CreateSP(SharedThis, &FESWSampleInstanceObject::AddMenuExtension));
				MaterialEditor->GetMenuExtensibilityManager()->AddExtender(MenuExtender);
			
				ToolbarExtender = MakeShareable(new FExtender);
				ToolbarExtender->AddToolBarExtension("Stats", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateSP(SharedThis, &FESWSampleInstanceObject::AddToolbarExtension));
				MaterialEditor->GetToolBarExtensibilityManager()->AddExtender(ToolbarExtender);
			
				OnRegisterTabSpawnersHandle = MaterialEditor->OnRegisterTabSpawners().AddSP(SharedThis, &FESWSampleInstanceObject::OnRegisterTabSpawners);
				OnUnregisterTabSpawnersHandle = MaterialEditor->OnUnregisterTabSpawners().AddSP(SharedThis, &FESWSampleInstanceObject::OnUnregisterTabSpawners);
				OnMaterialEditorClosedHandle = MaterialEditor->OnMaterialEditorClosed().AddLambda([this, ESWSampleModule]() {
					OnMaterialEditorClosed();
					ESWSampleModule->OnMaterialEditorClosed(this);
				});
			}
		}
	}
	void OnRegisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
	{
		TabManager->RegisterTabSpawner(ESWSampleTabName, FOnSpawnTab::CreateSP(AsShared(), &FESWSampleInstanceObject::OnSpawnPluginTab))
			.SetDisplayName(ESWSampleTabTitle)
			.SetMenuType(ETabSpawnerMenuType::Hidden);
	}
	void OnUnregisterTabSpawners(const TSharedRef<FTabManager>& TabManager)
	{
		TabManager->UnregisterTabSpawner(ESWSampleTabName);
	}
	void OnMaterialEditorClosed()
	{
		WeakMaterialEditor.Reset();
	}
	TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
	{
		FText WidgetText = FText::Format(
			LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
			FText::FromString(TEXT("FESWSampleModule::OnSpawnPluginTab")),
			FText::FromString(TEXT("ESWSample.cpp"))
		);
		return
			SAssignNew(WeakSpawnedPluginTab, SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				// SAssignNew(ESWSampleView, SESWSampleView, WeakMaterialEditor)
				SNew(STextBlock)
				.Text(WidgetText)
			];
	}
	void AddMenuExtension(FMenuBuilder& Builder)
	{
		Builder.AddMenuEntry(FESWSampleCommands::Get().OpenPluginWindow);
	}
	void AddToolbarExtension(FToolBarBuilder& Builder)
	{
		Builder.AddToolBarButton(FESWSampleCommands::Get().OpenPluginWindow);
	}
	void PluginButtonClicked()
	{
		if (WeakMaterialEditor.IsValid())
		{
			if (auto MaterialEditor = WeakMaterialEditor.Pin())
			{
				MaterialEditor->GetTabManager()->InvokeTab(ESWSampleTabName);
			}
		}
	}

private:
	/** Pointer to Material Editor or to Material Instance Editor set by constructor */
	TWeakPtr<IMaterialEditor> WeakMaterialEditor;

	TSharedPtr<FUICommandList> PluginCommands;

	TSharedPtr<FExtender> MenuExtender;

	TSharedPtr<FExtender> ToolbarExtender;

	FDelegateHandle OnRegisterTabSpawnersHandle;
	FDelegateHandle OnUnregisterTabSpawnersHandle;
	FDelegateHandle OnMaterialEditorClosedHandle;

	TWeakPtr<SDockTab> WeakSpawnedPluginTab;

//	TSharedPtr<SESWSampleView> ESWSampleView;

};

void FESWSampleModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FESWSampleStyle::Initialize();
	FESWSampleStyle::ReloadTextures();

	FESWSampleCommands::Register();
	
	auto OnOpened = [this](TWeakPtr<IMaterialEditor> InWeakMaterialEditor) {OnMaterialEditorOpened(InWeakMaterialEditor); };
	IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
	OnMaterialEditorOpenedHandle = MaterialEditorModule.Get().OnMaterialEditorOpened().AddLambda(OnOpened);
	OnMaterialFunctionEditorOpenedHandle = MaterialEditorModule.Get().OnMaterialFunctionEditorOpened().AddLambda(OnOpened);
}

void FESWSampleModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	if (FModuleManager::Get().IsModuleLoaded("MaterialEditor"))
	{
		IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
		if (OnMaterialEditorOpenedHandle.IsValid())
		{
			MaterialEditorModule.Get().OnMaterialEditorOpened().Remove(OnMaterialEditorOpenedHandle);
			OnMaterialEditorOpenedHandle.Reset();
		}
		if (OnMaterialFunctionEditorOpenedHandle.IsValid())
		{
			MaterialEditorModule.Get().OnMaterialFunctionEditorOpened().Remove(OnMaterialFunctionEditorOpenedHandle);
			OnMaterialFunctionEditorOpenedHandle.Reset();
		}
	}
	InstanceObjects.Empty();

	FESWSampleStyle::Shutdown();

	FESWSampleCommands::Unregister();

	if (GEngine)
	{
		GEngine->ForceGarbageCollection(true);
	}
}

void FESWSampleModule::OnMaterialEditorOpened(TWeakPtr<IMaterialEditor> InWeakMaterialEditor)
{
	if (InWeakMaterialEditor.IsValid())
	{
		if (auto MaterialEditor = InWeakMaterialEditor.Pin())
		{
			TSharedPtr<FESWSampleInstanceObject> InstanceObject(new FESWSampleInstanceObject(InWeakMaterialEditor));
			InstanceObjects.Add(InstanceObject);
			InstanceObject->OnMaterialEditorOpened(this);
		}
	}
}

void FESWSampleModule::OnMaterialEditorClosed(FESWSampleInstanceObject* InstanceObject)
{
	InstanceObjects.RemoveAll([InstanceObject](const TSharedPtr<FESWSampleInstanceObject>& instance) {
		return instance.Get() == InstanceObject;
	});
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FESWSampleModule, ESWSample)