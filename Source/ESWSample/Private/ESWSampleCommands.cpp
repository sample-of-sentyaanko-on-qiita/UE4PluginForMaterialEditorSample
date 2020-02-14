// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ESWSampleCommands.h"

#define LOCTEXT_NAMESPACE "FESWSampleModule"

void FESWSampleCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "ESWSample", "Bring up ESWSample window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
