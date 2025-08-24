#pragma once

#include "CoreMinimal.h"


class GAMECORE_API MaterialEnumHelper{

public:
    //all available materials
    static std::vector<materialEnum> materialVector();

    //index for a material in the default order from this class
    static int indexFor(materialEnum type);

    static materialEnum groundMaterialFor(ETerrainType terraintype);

    static FString toString(materialEnum typein);
};