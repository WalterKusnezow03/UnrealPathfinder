// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerControllerBase.h"

#include "GameCore/DebugHelper.h"
#include "Animation/AnimSequence.h"
#include "GameCore/team/teamEnum.h"

#include <cmath>
#include "GameCore/interfaces/Interactinterface.h"
#include "GameFramework/Character.h" // Falls noch nicht inkludiert
#include "GameFramework/CharacterMovementComponent.h"
#include "AssetPlugin/gamestart/assetEnums/weaponAttachmentEnum.h"
#include "Components/CapsuleComponent.h" // Include for UCapsuleComponent
#include "Camera/CameraComponent.h" // Include for UCameraComponent

#include "GameCore/PlayerInfo/PlayerInfo.h"



// Sets default values
APlayerControllerBase::APlayerControllerBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    

    aiming = false;

    // Set capsule size
    float radius = 45.0f;
    float halfHeight = 85.0f; //100cm leg, 70 arms / torso = 170 -> 170 /2 = 85.0f
    GetCapsuleComponent()->InitCapsuleSize(radius, halfHeight);

    CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
    CameraComponent->SetupAttachment(GetCapsuleComponent());
    //CameraComponent->SetRelativeLocation(FVector(-39.56f, 1.75f, 84.f)); // Position the camera
    //CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, halfHeight * 2.0f)); // Position the camera
    CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, halfHeight)); // Position the camera
    CameraComponent->bUsePawnControlRotation = true;

    DebugHelper::logMessage("debugsetup player capsule");

    // Set default values
    TurnRateGamepad = 45.f;
    LookUpRateGamepad = 45.f;

	TurnRateGamepad = 90.f;
    LookUpRateGamepad = 90.f;

	health = 100;

    sprinting = false;


    

}

// Called when the game starts or when spawned
void APlayerControllerBase::BeginPlay()
{
	Super::BeginPlay();
    setTeam(teamEnum::playerTeam);

}

// Called to bind functionality to input
void APlayerControllerBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    // Bind input actions
    PlayerInputComponent->BindAxis("MoveForward", this, &APlayerControllerBase::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &APlayerControllerBase::MoveRight);
    PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
    PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
    PlayerInputComponent->BindAxis("TurnRate", this, &APlayerControllerBase::TurnAtRate);
    PlayerInputComponent->BindAxis("LookUpRate", this, &APlayerControllerBase::LookUpAtRate);

    PlayerInputComponent->BindAction("openPauseKey", IE_Pressed, this, &APlayerControllerBase::openPauseMenu);

    PlayerInputComponent->BindAction("InteractKey", IE_Pressed, this, &APlayerControllerBase::interact);
	PlayerInputComponent->BindAction("ReloadKey", IE_Pressed, this, &APlayerControllerBase::reload);
	PlayerInputComponent->BindAction("DropKey", IE_Pressed, this, &APlayerControllerBase::drop);
    PlayerInputComponent->BindAction("JumpKey", IE_Pressed, this, &APlayerControllerBase::Jump);

    PlayerInputComponent->BindAction("sprintKey", IE_Pressed, this, &APlayerControllerBase::sprint);
    PlayerInputComponent->BindAction("sprintKey", IE_Released, this, &APlayerControllerBase::sprint);

    // Bind the key down action, here, not needed
	// PlayerInputComponent->BindAction("InteractKey", IE_Pressed, this, &AplayerScript::OnInteractKeyDown);
	// Bind the key up action
    PlayerInputComponent->BindAction("leftMouse", IE_Pressed, this, &APlayerControllerBase::leftMouseDown);
	PlayerInputComponent->BindAction("leftMouse", IE_Released, this, &APlayerControllerBase::leftMouseUp);

    PlayerInputComponent->BindAction("rightMouse", IE_Pressed, this, &APlayerControllerBase::aim);
    PlayerInputComponent->BindAction("rightMouse", IE_Released, this, &APlayerControllerBase::aim);

}

// Called every frame
void APlayerControllerBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    processPendingRecoil();

    PlayerInfo::Update(
        GetActorLocation(),
        cameraRotation(),
        playerLookDir()
    );
    
}



/**
 * Implemented take damage method from interface, need to override it, this method is not valid yet
 */
void APlayerControllerBase::takedamage(int d)
{
    takedamage(d, false);
}

void APlayerControllerBase::takedamage(int d, FVector &hitpoint){
    takedamage(d, hitpoint, false);
}

void APlayerControllerBase::takedamage(int d, bool surpressed){
    health -= d;
	if(health <= 0){
		health = 0;
	}
}

void APlayerControllerBase::takedamage(int d, FVector &hitpoint, bool surpressed){
    takedamage(d, surpressed);
}

//Movement
/**
 * moves the player forward
 */
void APlayerControllerBase::MoveForward(float Value)
{
    playerInputContainer.setForwardAxis(Value);

    if ((Controller != nullptr) && (Value != 0.0f))
    {
        GetCharacterMovement()->MaxWalkSpeed = BASE_SPEED;
        GetCharacterMovement()->MinAnalogWalkSpeed = BASE_SPEED - 1;
        if (sprinting) {
            GetCharacterMovement()->MaxWalkSpeed = SPRINT_SPEED;
            GetCharacterMovement()->MinAnalogWalkSpeed = SPRINT_SPEED - 1;
        }
        

        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        if(true){
            AddMovementInput(Direction, Value);
        }
    
        isWalking = true;


    }
}

void APlayerControllerBase::MoveRight(float Value)
{
    playerInputContainer.setRightAxis(Value);

    if ( (Controller != nullptr) && (Value != 0.0f) )
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);

    }
}

void APlayerControllerBase::TurnAtRate(float Rate)
{
    if(isPaused){
        return;
    }
    float yawRate = Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds();
    AddControllerYawInput(yawRate);
    //AddControllerYawInput(Rate * TurnRateGamepad * GetWorld()->GetDeltaSeconds());
}

void APlayerControllerBase::LookUpAtRate(float Rate)
{
    if(isPaused){
        return;
    }

    //add if look up at rate has higher priority than "this" tick
    Rate += pendingRecoil;
    pendingRecoil = 0.0f;
    

    if(Controller){
        float DeltaPitch = Rate * LookUpRateGamepad * GetWorld()->GetDeltaSeconds();
        FRotator currentRotaion = Controller->GetControlRotation();
        float newPichAbs = std::abs(currentRotaion.Pitch + DeltaPitch);
        if(newPichAbs < 89.0f){
            AddControllerPitchInput(Rate * LookUpRateGamepad * GetWorld()->GetDeltaSeconds());
        }    
    }
    
	//AddControllerPitchInput(Rate * LookUpRateGamepad * GetWorld()->GetDeltaSeconds());
	//UE_LOG(LogTemp, Warning, TEXT("Turning at rate: %f"), Rate);
}


/// @brief add pending recoil from a weapon for example
/// @param value 
void APlayerControllerBase::addPendingRecoil(float value){
    pendingRecoil += value;
}


/// @brief adds the pending recoil on tick
void APlayerControllerBase::processPendingRecoil(){
    LookUpAtRate(pendingRecoil);
    pendingRecoil = 0.0f; //reset, no double add
}   

void APlayerControllerBase::Jump(){
    if(isPaused){
        return;
    }

    if (CanJump()){
        ACharacter::Jump(); // Calls the base class jump function
    }
}



void APlayerControllerBase::sprint(){
    sprinting = !sprinting;
}

/**
 * allows the player to interact (for example picking up a weapon by pressing "E")
 */
void APlayerControllerBase::interact(){
	playerInputContainer.setInteractKeyPressedTrue();

    interactedActorPointer = nullptr; //reset!
    interactedActorPointer = performRaycast();

    if(interactedActorPointer){
        IInteractinterface *interactable = Cast<IInteractinterface>(interactedActorPointer);
        if(interactable){
            interactable->interact();

            //reset ptr (?)
            interactedActorPointer = nullptr;
        }
    }
    
}

AActor* APlayerControllerBase::performRaycast()
{
	// Get the camera location and rotation
    FVector CameraLocation = CameraComponent->GetComponentLocation();
    FRotator CameraRotation = CameraComponent->GetComponentRotation();

    // Define the start and end vectors for the raycast
    FVector Start = CameraLocation;

    //100 = 1m
    //500m = 
    FVector End = Start + (CameraRotation.Vector() * 200.0f); //2 meter in front of the camera

    // Perform the raycast
    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this); // Ignore the character itself

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);



    // If the raycast hit something, log the hit actor's name
    if (bHit)
    {
		AActor* actor = HitResult.GetActor();
        return actor;
    }
    return nullptr;
}


// VIRTUAL !
void APlayerControllerBase::drop(){
    
}

void APlayerControllerBase::reload(){
    
}

void APlayerControllerBase::aim(){
    aiming = !aiming;
}
//VITRUAL END!



/// @brief sets the left mouse holding status for the weapon
/// @param h 
void APlayerControllerBase::setHolding(bool h){
    holding = h;
}


void APlayerControllerBase::leftMouseDown(){
    setHolding(true);
}

void APlayerControllerBase::leftMouseUp(){
    setHolding(false);
}


void APlayerControllerBase::setTeam(teamEnum teamIn){
    this->team = teamIn;
    // referenceManager::verifyTeam(teamIn);
}

teamEnum APlayerControllerBase::getTeam(){
    return team;
}


FRotator APlayerControllerBase::cameraRotation(){
    FRotator rotator;
    if(CameraComponent){
        return CameraComponent->GetComponentRotation();
    }
    return rotator;
}

FVector APlayerControllerBase::playerLookDir(){
    if(CameraComponent){
        return CameraComponent->GetForwardVector();
    }
    return FVector(0, 0, 0);
}

/**
 * --- helper ---
 */
void APlayerControllerBase::showCursor(bool show){
    // Assuming you're inside your ACharacter or subclass (e.g., AMyCharacter)
    APlayerController* PlayerController = Cast<APlayerController>(GetController());
    if (PlayerController)
    {
        // Toggle mouse cursor visibility
        PlayerController->bShowMouseCursor = show;

        //game only custom dispatch no ui - CUSTOM DIPATCH CLICKS
        FInputModeGameOnly InputMode;
        PlayerController->SetInputMode(InputMode);
        
        
    }

}

void APlayerControllerBase::setPaused(bool in){
    isPaused = in;
}

void APlayerControllerBase::openPauseMenu(){

}


/**
 * player steering
 */
InputContainer &APlayerControllerBase::input(){
    return playerInputContainer;
}
void APlayerControllerBase::setDriverLocation(FVector &location){
    SetActorLocation(location);
}