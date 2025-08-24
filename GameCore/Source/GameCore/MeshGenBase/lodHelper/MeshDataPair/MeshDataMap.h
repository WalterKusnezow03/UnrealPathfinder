#pragma once

#include "CoreMinimal.h"
#include "GameCore/MeshGenBase/MeshData/MeshData.h"

class ProceduralMeshComponentPair; // forward declaration


class GAMECORE_API MeshDataMap {
    //Allow ProceduralMeshComponentPair derived from this, using the protected members of another object.
    friend class ProceduralMeshComponentPair;

public:
    MeshDataMap();
    ~MeshDataMap();

    MeshData &meshDataReference(materialEnum type, bool raycast);
    MeshData &meshDataReferenceRaycast(materialEnum material);
    MeshData &meshDataReferenceNoRaycast(materialEnum material);

    void replaceMeshDataRaycast(MeshData &data, materialEnum type);
    void replaceMeshDataNoRaycast(MeshData &data, materialEnum type);

    void appendMeshDataRaycast(MeshData &datain, materialEnum type);
    void appendMeshDataNoRaycast(
        MeshData &meshdata,
        materialEnum type
    );

    
    

protected: 

    std::map<materialEnum, MeshData> raycastMeshData;
    std::map<materialEnum, MeshData> noRaycastMeshData; //no physics mesh at all

    
};