// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameCore/Input/InputContainer.h"
#include "Steeringinterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class USteeringinterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * interface designed for player or bot interaction: for example click a button,
 * jump a wall or seat a car
 */
class ISteeringinterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// pure virtual like this
	// virtual void a() = 0;
    virtual InputContainer &input() = 0;
    virtual void setDriverLocation(FVector &location) = 0;
};
