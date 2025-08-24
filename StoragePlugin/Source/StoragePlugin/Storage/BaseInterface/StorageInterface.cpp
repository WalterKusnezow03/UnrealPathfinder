#include "StorageInterface.h"
#include "Misc/FileHelper.h"
#include "StoragePlugin/Storage/VertexData/TerrainVertex.h"
#include "StoragePlugin/Storage/VertexData/TerrainNormal.h"
#include "StoragePlugin/Storage/VertexData/TerrainUv.h"
#include "StoragePlugin/Storage/VertexData/TerrainVertexIndex.h"
#include "GameCore/DebugHelper.h"


//-------- SAVING --------

bool StorageInterface::SaveBinaryData(
    FString Path,
    TArray<uint8> &Bytes
){
    // Load
    return FFileHelper::SaveArrayToFile(Bytes, *Path);
}


//-------- LOADING --------



//load binary
bool StorageInterface::LoadBinaryData(
    FString Path,
    TArray<uint8> &Bytes
){
    // Load
    return FFileHelper::LoadFileToArray(Bytes, *Path);
}



// ------ PATH -------
FString StorageInterface::BaseDir(FString worldLevelName){
    FString preString = FPaths::ProjectSavedDir(); //ends with a single "/"
    preString += FString::Printf(TEXT("%s/"), *worldLevelName); //also ends with a single "/"
    return preString;
}