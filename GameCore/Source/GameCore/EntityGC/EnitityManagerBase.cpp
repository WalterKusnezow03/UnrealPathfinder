#include "EntityManagerBase.h"
#include "EntityManagerGeneric.h"

template class EntityManagerGeneric<AActor>;
EntityManagerBase *EntityManagerBase::instancePtr = nullptr;



void EntityManagerBase::BeginPlayBase(){
    if(instancePtr != nullptr){
        EndPlay();
    }
    EntityManagerBase::instancePtr = new EntityManagerBase();
}

void EntityManagerBase::EndPlay(){
    delete instancePtr;
    instancePtr = nullptr;
}

EntityManagerBase *EntityManagerBase::instanceBase(){
    if(instancePtr == nullptr){
        BeginPlayBase();
    }
    return instancePtr;
}



EntityManagerBase::EntityManagerBase(){

}
EntityManagerBase::~EntityManagerBase(){

}





//add entity section
void EntityManagerBase::add(ETrackedActors type, AActor *actorPtr){
    if(actorPtr != nullptr){
        EntityManagerGeneric<AActor> &ref = find(type);
        ref.add(actorPtr);
    }
}



EntityManagerGeneric<AActor> &EntityManagerBase::find(ETrackedActors type){
    if(TrackedActorsMap.find(type) == TrackedActorsMap.end()){
        TrackedActorsMap[type] = EntityManagerGeneric<AActor>();
    }
    return TrackedActorsMap[type];
}





/**
 * spawning
 */



