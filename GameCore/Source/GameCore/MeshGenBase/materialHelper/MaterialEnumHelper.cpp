#include "MaterialEnumHelper.h"
#include <map>

FString MaterialEnumHelper::toString(materialEnum typein){
    std::map<materialEnum, FString> map;
    map[materialEnum::grassMaterial] = TEXT("grassMaterial");
    map[materialEnum::wallMaterial] = TEXT("wallMaterial");
    map[materialEnum::glassMaterial] = TEXT("glassMaterial");
    map[materialEnum::stoneMaterial] = TEXT("stoneMaterial");
    map[materialEnum::sandMaterial] = TEXT("sandMaterial");
    map[materialEnum::redsandMaterial] = TEXT("redsandMaterial");
    map[materialEnum::treeMaterial] = TEXT("treeMaterial");
    map[materialEnum::palmLeafMaterial] = TEXT("palmLeafMaterial");
    map[materialEnum::waterMaterial] = TEXT("waterMaterial");
    map[materialEnum::snowMaterial] = TEXT("snowMaterial");
    map[materialEnum::beigeStoneMaterial] = TEXT("beigeStoneMaterial");
    map[materialEnum::_texturedMaterial] = TEXT("_texturedMaterial");
    map[materialEnum::prop_alarmBoxMaterial] = TEXT("prop_alarmBoxMaterial");
    map[materialEnum::grassMaterial] = TEXT("grassMaterial");
    map[materialEnum::wingMaterial] = TEXT("wingMaterial");

    if(map.find(typein) != map.end()){
        return map[typein];
    }
    return TEXT("materialNotFound");
}

std::vector<materialEnum> MaterialEnumHelper::materialVector(){
    std::vector<materialEnum> types = {
        materialEnum::grassMaterial,
        materialEnum::wallMaterial,
        materialEnum::glassMaterial,
        materialEnum::stoneMaterial,
        materialEnum::sandMaterial,
        materialEnum::redsandMaterial,
        materialEnum::treeMaterial,
        materialEnum::palmLeafMaterial,
        materialEnum::waterMaterial,
        materialEnum::snowMaterial,
        materialEnum::beigeStoneMaterial,
        materialEnum::prop_alarmBoxMaterial,
        materialEnum::_texturedMaterial,
        materialEnum::wingMaterial
    };
    return types;
}




materialEnum MaterialEnumHelper::groundMaterialFor(ETerrainType terraintype){
    if(terraintype == ETerrainType::EOcean){
        return materialEnum::sandMaterial;
    }
    if(terraintype == ETerrainType::EDesert){
        return materialEnum::redsandMaterial;
    }
    if(terraintype == ETerrainType::ESnowHill){
        return materialEnum::snowMaterial;
    }
    if(terraintype == ETerrainType::EDesertForest){
        return materialEnum::beigeStoneMaterial;
    }

    return materialEnum::grassMaterial;
}


int MaterialEnumHelper::indexFor(materialEnum type){
    int outIndex = 0;
    std::vector<materialEnum> allMaterials = materialVector();
    for (int i = 0; i < allMaterials.size(); i++){
        if(allMaterials[i] == type){
            return i;
        }
    }
    return outIndex;
}