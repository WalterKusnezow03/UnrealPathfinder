#include "MeshDataMap.h"

MeshDataMap::MeshDataMap(){

}

MeshDataMap::~MeshDataMap(){

}



MeshData &MeshDataMap::meshDataReference(materialEnum type, bool raycast){
    if(raycast){
        return meshDataReferenceRaycast(type);
    }
    return meshDataReferenceNoRaycast(type);
}

MeshData &MeshDataMap::meshDataReferenceRaycast(materialEnum material){
    if(raycastMeshData.find(material) == raycastMeshData.end()){
        MeshData empty;
        raycastMeshData[material] = empty;
    }
    return raycastMeshData[material];
}

MeshData &MeshDataMap::meshDataReferenceNoRaycast(materialEnum material){
    if(noRaycastMeshData.find(material) == noRaycastMeshData.end()){
        MeshData empty;
        noRaycastMeshData[material] = empty;
    }
    return noRaycastMeshData[material];
}


void MeshDataMap::replaceMeshDataRaycast(MeshData &data, materialEnum type){
    raycastMeshData[type] = data;
}

void MeshDataMap::replaceMeshDataNoRaycast(MeshData &data, materialEnum type){
    noRaycastMeshData[type] = data;
}



/**
 * append mesh data
 */
void MeshDataMap::appendMeshDataRaycast(
    MeshData &meshdata,
    materialEnum type
){
    MeshData &found = meshDataReferenceRaycast(type);
    found.append(meshdata);
}

void MeshDataMap::appendMeshDataNoRaycast(
    MeshData &meshdata,
    materialEnum type
){
    MeshData &found = meshDataReferenceNoRaycast(type);
    found.append(meshdata);
}
