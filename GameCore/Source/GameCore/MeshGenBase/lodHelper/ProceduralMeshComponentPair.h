#pragma once

#include "CoreMinimal.h"
#include "GameCore/MeshGenBase/MeshData/MeshData.h"
#include "GameCore/MeshGenBase/lodHelper/MeshDataPair/MeshDataMap.h"


/// @brief will store a uprocedural mesh with raycast and without including their mesh
/// data for all layers
class GAMECORE_API ProceduralMeshComponentPair : public MeshDataMap {

public:
    ProceduralMeshComponentPair();
    ~ProceduralMeshComponentPair();

    void init(
        FString name,
        int index,
        AActor *actorOwner,
        USceneComponent *RootComponent
    );

    void overrideMeshPair(UProceduralMeshComponent *raycastMeshIn, UProceduralMeshComponent *noraycastIn);

    UProceduralMeshComponent *RaycastMesh();
    UProceduralMeshComponent *noRaycastMesh();


    void setCollisionEnabled(bool flag);
    void setHiddenInGame(bool flag);

    void updateMeshAllAndApplyMaterial(materialEnum type);

    //All update mesh methods will call CREATE MESH SECTION INTERNALLY - NEW INDEX BUFFER TO GPU
    void updateMeshAll();
    void updateMeshAll(materialEnum type);
    void updateMeshRaycast(materialEnum type);
    void updateMeshNoRaycast(materialEnum type);

    //All refresh map methods are allowed to UPDATE the vertex buffer, for example for cpu side wave shader
    void refreshMeshRaycast(materialEnum type);
    void refreshMeshNoRaycast(materialEnum type);

    void replaceMeshDataRaycastAndUpdate(MeshData &data, materialEnum type);
    void replaceMeshDataNoRaycastAndUpdate(MeshData &data, materialEnum type);

    static int layerByMaterialEnum(materialEnum type);

    //applies materials to all layers, raycast and no raycast!, no mesh update
    void ApplyAllMaterials();

    //applies material to raycast and no raycast mesh
    void ApplyMaterialAll(materialEnum type);

    void ApplyMaterialRaycast(materialEnum type);
    void ApplyMaterialNoRaycast(materialEnum type);

    void appendMeshDataAndReloadRaycast(MeshData &datain, materialEnum type);
    void appendMeshDataAndReloadNoRaycast(
        MeshData &meshdata,
        materialEnum type
    );

    /// @brief overrides the internal meshdata and refreshes the mesh completly
    void overrideMeshDataFromBaseAndUpdateMesh(
        MeshDataMap &other
    );

private:
    //declared in base class
    //std::map<materialEnum, MeshData> raycastMeshData;
    //std::map<materialEnum, MeshData> noRaycastMeshData; //no physics mesh at all

    UProceduralMeshComponent *raycastMesh = nullptr;
    UProceduralMeshComponent *noraycastMesh = nullptr;

    void updateMesh(
        UProceduralMeshComponent &meshcomponent,
        MeshData &otherMesh, // MUST BE SAVED IN A VALUE CLASS SCOPE SOMEWHERE!
        int layer
    );

    void refreshMesh(
        UProceduralMeshComponent &meshComponent,
        MeshData &other,
        int layer
    );

    void ApplyMaterial(UProceduralMeshComponent *ProceduralMeshComponent, UMaterialInterface *Material);
	void ApplyMaterial(
		UProceduralMeshComponent *ProceduralMeshComponent,
		UMaterialInterface *Material,
		int layer
	);
	void ApplyMaterial(
		UProceduralMeshComponent *ProceduralMeshComponent,
		materialEnum type
	);
};