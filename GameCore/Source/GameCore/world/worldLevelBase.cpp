#include "worldLevelBase.h"

TArray<FVector> worldLevelBase::outpostsToCreate;

void worldLevelBase::addOutpostAt(FVector &pos){
    outpostsToCreate.Add(pos);
}