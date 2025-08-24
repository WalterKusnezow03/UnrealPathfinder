#pragma once

#include "CoreMinimal.h"
#include "trackedActors.h"
#include <map>
#include "EntityManagerGeneric.h"

/**
 * base class to store pointers if an aactor derived class is not needed anymore but 
 * destruction isnt wanted, rather reuse. Dont remove from ram.
 */
class GAMECORE_API EntityManagerBase {

public:
    EntityManagerBase();
	virtual ~EntityManagerBase(); //overriden from sub

    static void BeginPlayBase();
    static void EndPlay();
    static EntityManagerBase *instanceBase();
    static EntityManagerBase *instancePtr;

    //add entity section
    virtual void add(ETrackedActors type, AActor *ptr);



    template<typename T>
    T *SpawnAActor(UWorld *world){
        static_assert(TIsDerivedFrom<T, AActor>::IsDerived, "T must be derived from AActor");

        if (world)
        {
            FActorSpawnParameters SpawnParams;
            UClass* classType = T::StaticClass();
            FVector Location = FVector::ZeroVector;
            FRotator Rotation = FRotator::ZeroRotator;

            AActor* spawned = world->SpawnActor<AActor>(classType, Location, Rotation, SpawnParams);
            if (spawned)
            {
                return Cast<T>(spawned);
            }
        }
        return nullptr;
    }


    template <typename T>
    T *requestByEnum(ETrackedActors type, UWorld *world){
        static_assert(TIsDerivedFrom<T, AActor>::IsDerived, "T must be derived from AActor");

        EntityManagerGeneric<AActor> &ref = find(type);
        AActor *ptr = ref.getFirstActor(); //may be nullptr
        if(ptr == nullptr){
            T *spawned = SpawnAActor<T>(world);
            return spawned;
        }else{
            T *casted = Cast<T>(ptr);
            if(casted){
                return casted;
            }
        }
        return nullptr;
    }




protected:
    std::map<ETrackedActors, EntityManagerGeneric<AActor>> TrackedActorsMap; //other can be tracked by a subclass.

    EntityManagerGeneric<AActor> &find(ETrackedActors type);

    


};