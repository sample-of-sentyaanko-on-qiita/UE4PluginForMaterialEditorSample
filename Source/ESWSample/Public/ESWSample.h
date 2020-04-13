// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FESWSampleModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	TSharedPtr<class FUICommandList> PluginCommands;
	TSharedPtr<class FESWSampleImpl> Impl;
	TSharedPtr<FExtender> MenuExtender;
	TSharedPtr< const FExtensionBase > MenuExtension;
	TSharedPtr<FExtender> ToolbarExtender;
	TSharedPtr< const FExtensionBase > ToolbarExtension;
};
