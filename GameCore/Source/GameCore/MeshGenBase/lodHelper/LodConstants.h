#pragma once
#include "CoreMinimal.h"
#include "GameCore/MeshGenBase/ELod.h"

/// @brief holds view distances and the lod vector
class GAMECORE_API LodConstants {

public:
    static const int metersLodNearUntil = 200;
    static const int metersLodMiddleUntil = 400;
    static const int metersLodFarUntil = 600;

    
    static std::vector<ELod> lodVector(){
        std::vector<ELod> types = {
            ELod::lodNear,
            ELod::lodMiddle,
            ELod::lodFar
        };
        return types;
    }


private:
    LodConstants(){}
    ~LodConstants(){}
};