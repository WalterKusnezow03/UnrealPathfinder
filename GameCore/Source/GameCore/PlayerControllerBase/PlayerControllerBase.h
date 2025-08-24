// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h" // Include for UCapsuleComponent
#include "Camera/CameraComponent.h" // Include for UCameraComponent
#include "GameCore/team/teamEnum.h"
#include "GameCore/interfaces/Damageinterface.h"
#include "GameCore/util/timer.h"
#include "GameCore/Input/InputContainer.h"
#include "GameCore/interfaces/Steeringinterface.h"

#include "PlayerControllerBase.generated.h"

UCLASS()
class GAMECORE_API APlayerControllerBase : public ACharacter, public IDamageinterface, public ISteeringinterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerControllerBase();
	virtual void takedamage(int d) override;
	virtual void takedamage(int d, FVector &hitpoint) override;
	virtual void takedamage(int d, bool surpressed) override;
	virtual void takedamage(int d, FVector &hitpoint, bool surpressed) override;
	virtual void setTeam(teamEnum teamIn) override;
	virtual teamEnum getTeam() override;

	void showCursor(bool show);
	void setPaused(bool in);


	//steering
	virtual InputContainer &input() override;
	virtual void setDriverLocation(FVector &location) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//derived from ACharacter
	virtual void Jump() override;

	//tracks input for steering vehicles, and unseat
	InputContainer playerInputContainer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	UPROPERTY(VisibleAnywhere)
    class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* CameraComponentSecondary;

	FRotator cameraRotation();
	FVector playerLookDir();

protected:
    //LATEST INTERACTED ACTOR POINTER
    AActor *interactedActorPointer = nullptr;

    teamEnum team;

	bool aiming;
	bool holding;
	bool sprinting;
	void sprint();
	static const int BASE_SPEED = 600;
	static const int SPRINT_SPEED = 700;
    float playerMaxMovementVelocity = 500.0f;

	bool isWalking;

    AActor *performRaycast();

    void TurnAtRate(float Rate);
    void LookUpAtRate(float Rate);
	
	virtual void MoveForward(float Value);
    virtual void MoveRight(float Value);
    
	virtual void interact();
	virtual void reload();
	virtual void drop();
	virtual void aim();

	void setHolding(bool h);
    

    void leftMouseUp();
    void leftMouseDown();

    virtual void openPauseMenu();

    UPROPERTY(EditAnywhere)
    float TurnRateGamepad;

    UPROPERTY(EditAnywhere)
    float LookUpRateGamepad;

	int health;


	bool isPaused = false;

	void processPendingRecoil();
	void addPendingRecoil(float value);
	float pendingRecoil = 0.0f;
};
