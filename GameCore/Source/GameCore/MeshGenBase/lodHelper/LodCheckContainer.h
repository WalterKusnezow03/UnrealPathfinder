#pragma once

#include "CoreMinimal.h"
#include "GameCore/MeshGenBase/ELod.h"
#include <map>

class GAMECORE_API LodCheckContainer{

public:
    LodCheckContainer();
    LodCheckContainer(FVector &a, FVector &b);
    ~LodCheckContainer();

    void checkLod(FVector &a, FVector &b);

    void modifyUpperDistanceLimitFor(ELod lod, int newMaxDistance);
    bool lodWasEdgeCaseToNextLod();
    bool hideActorByLod();
    ELod lod();

private:
    std::map<ELod, int> distances;

    bool hideActorFlag = false;

    int edgeDistanceFrom = 1000;

    FVector thisLocation;
    FVector playerLocation;

    ELod lodFound = ELod::lodFar;
    bool lodWasEdgeCase = false;

    void checkLod();
    ELod lodLevelByDistanceTo(bool &isEdgeCase);
    bool isInRange(int maxDistance, bool &isEdgeCase);
    int maxDistanceForLod(ELod lodLevel);
};