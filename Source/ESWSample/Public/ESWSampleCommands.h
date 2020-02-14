// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ESWSampleStyle.h"

class FESWSampleCommands : public TCommands<FESWSampleCommands>
{
public:

	FESWSampleCommands()
		: TCommands<FESWSampleCommands>(TEXT("ESWSample"), NSLOCTEXT("Contexts", "ESWSample", "ESWSample Plugin"), NAME_None, FESWSampleStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};