#include "PlayerInfo.h"

FVector PlayerInfo::playerLocationData = FVector::ZeroVector;
FVector PlayerInfo::playerLookDirData = FVector::ZeroVector;
FRotator PlayerInfo::playerRotationData = FRotator::ZeroRotator;

FVector PlayerInfo::playerLocation(){
    return playerLocationData;
}

FVector PlayerInfo::playerLookDir(){
    return playerLookDirData;
}

FRotator PlayerInfo::playerRotation(){
    return playerRotationData;
}


void PlayerInfo::Update(
    FVector location,
    FRotator rotation,
    FVector lookDir
){
    playerLocationData = location;
    playerLookDirData = lookDir;
    playerRotationData = rotation;
}