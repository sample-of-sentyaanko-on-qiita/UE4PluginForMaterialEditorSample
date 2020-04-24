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
	
	/** This function is for registering to the MaterialEditorModule's OnMaterialEditorOpened event. */
public:
	void OnMaterialEditorOpened(TWeakPtr<class IMaterialEditor> InWeakMaterialEditor);

	/** This function is called from the MaterialEditorModule's OnMaterialEditorClosed event via the FESWSampleInstanceObject's OnMaterialEditorClosed function. */
public:
	void OnMaterialEditorClosed(class FESWSampleInstanceObject* InstanceObject);

	/** This delegate handle holds the MaterialEditorModule's OnMaterialEditorOpened event. */
private:
	FDelegateHandle OnMaterialEditorOpenedHandle;
	FDelegateHandle OnMaterialFunctionEditorOpenedHandle;

	/** This array holds the control class of the MaterialNodeList tab for each MaterialEditorWindow. */
private:
	TArray<TSharedPtr<class FESWSampleInstanceObject>>	InstanceObjects;
};
