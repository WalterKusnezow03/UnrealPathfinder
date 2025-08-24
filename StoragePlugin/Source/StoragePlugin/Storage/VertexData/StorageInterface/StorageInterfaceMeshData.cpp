#include "StorageInterfaceMeshData.h"

#include "Misc/FileHelper.h"
#include "StoragePlugin/Storage/VertexData/TerrainVertex.h"
#include "StoragePlugin/Storage/VertexData/TerrainNormal.h"
#include "StoragePlugin/Storage/VertexData/TerrainUv.h"
#include "StoragePlugin/Storage/VertexData/TerrainVertexIndex.h"
#include "GameCore/DebugHelper.h"


//-------- SAVING --------


void StorageInterfaceMeshData::SaveMeshData(
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles,
    int chunkId,
    int layer,
    int lod
){
    SaveMeshData(
        Vertecies,
        Normals,
        UV0,
        Triangles,
        makePath(chunkId, layer, lod)
    );
}

void StorageInterfaceMeshData::SaveMeshData(
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles,
    FString path
){
    
    TArray<uint8> Bytes;
    AppendIntoByteBuffer(Bytes, Vertecies, Normals, UV0, Triangles);

    //debug
    PrintBinary(Bytes);

    // Save
    SaveBinaryData(
        path,
        Bytes
    );
}

/// @brief will append the given data into the byte buffer, including the header info!
/// If writing later to the buffer: Use Ptr = bytes.GetData() + bytes.Num()
void StorageInterfaceMeshData::AppendIntoByteBuffer(
    TArray<uint8> &Bytes, //buffer size is increased after append!
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles
){

    int32 vertexCount = Vertecies.Num();
    int32 normalCount = Normals.Num();
    int32 uvCount = UV0.Num();
    int32 triangleCount = Triangles.Num();

    if(debugLog){
        FString message = FString::Printf(
            TEXT("Storage Interface MeshData write buffer sizes (%d), (%d), (%d), (%d)"),
            vertexCount, normalCount, uvCount, triangleCount
        );
        DebugHelper::logMessage(message);
    }
    

    //int32 is 4 bytes
    //MUST BE THE SAME IN LOADING METHOD!
    int infoBytesSize = getInfoBytesSize(); 
    int verteciesByteSize = getVertexBytesSize(Vertecies.Num());
    int normalsByteSize = getNormalsBytesSize(Normals.Num());
    int uv0ByteSize = getUVBytesSize(UV0.Num());
    int trianglesByteSize = getTrianglesBytesSize(Triangles.Num());

    int completeSize =
        infoBytesSize +
        verteciesByteSize +
        normalsByteSize +
        uv0ByteSize +
        trianglesByteSize;

    //write info and data size
    int previousSize = Bytes.Num();
    Bytes.SetNumUninitialized(previousSize + completeSize); // SET SIZE VERY IMPORTANT, previous data is NOT overriden.
    
    //pointer erst nach set num machen, da Array UMKOPIERT WIRD!
    uint8* Ptr = Bytes.GetData(); //get data pointer to work with (starts at front)
    Ptr += previousSize;

    //write info data
    writeInfoData(
        Vertecies.Num(), 
        Normals.Num(),
        UV0.Num(),
        Triangles.Num(),
        Ptr
    ); //at offset of previous bytes!
    

    // ---- write mesh data ---- 
    
    //Ptr += infoBytesSize; // get to offset adress after info data!

    /*
    FMemory::Memcpy ( 
        void* Dest,
        const void* Src,
        SIZE_T Count (in bytes if using unit8*)
    )
    */
    FMemory::Memcpy(Ptr, (uint8 *)Vertecies.GetData(), verteciesByteSize); //copy casted data
    Ptr += verteciesByteSize; //increase pointer adress

    FMemory::Memcpy(Ptr, (uint8 *)Normals.GetData(), normalsByteSize); //copy
    Ptr += normalsByteSize; //increase pointer adress

    FMemory::Memcpy(Ptr, (uint8 *)UV0.GetData(), uv0ByteSize); //copy
    Ptr += uv0ByteSize; //increase pointer adress

    FMemory::Memcpy(Ptr, (uint8 *)Triangles.GetData(), trianglesByteSize); //copy
    //Ptr += trianglesByteSize; //increase pointer adress not be needed, end of data!!
    





}














//immidiate byte offset for data Ptr, is increased!, POINTER MUST BE VALID! MUST BE REFERENCE TO TAKE EFFECT OF +=
void StorageInterfaceMeshData::writeInfoData(
    int32 vertexCount,
    int32 normalCount,
    int32 uvCount,
    int32 triangleCount,
    uint8*& Ptr
){
    TArray<int32> info = {
        vertexCount,
        normalCount,
        uvCount,
        triangleCount
    };
    for (int i = 0; i < info.Num(); i++){
        int32 *infoCurrent = &info[i];
        /*
        FMemory::Memcpy ( 
            void* Dest,
            const void* Src,
            SIZE_T Count
        )
        */
        FMemory::Memcpy(Ptr, infoCurrent, sizeof(int32)); //copy
        Ptr += sizeof(int32); //increase pointer adress
    }

    //how it works in theory:
    /*
    FMemory::Memcpy(Ptr, &vertexCount, sizeof(int32));
    Ptr += sizeof(int32);
    
    FMemory::Memcpy(Ptr, &normalCount, sizeof(int32));
    Ptr += sizeof(int32);

    FMemory::Memcpy(Ptr, &uvCount, sizeof(int32));
    Ptr += sizeof(int32);

    FMemory::Memcpy(Ptr, &triangleCount, sizeof(int32));*/
}


//------- SIZE OF HELPER ---------
int StorageInterfaceMeshData::getInfoBytesSize(){
    return sizeof(int32) * 4;
}

int StorageInterfaceMeshData::getVertexBytesSize(int count){
    return sizeof(FTerrainVertex) * count;
}

int StorageInterfaceMeshData::getNormalsBytesSize(int count){
    return sizeof(FTerrainNormal) * count;
}

int StorageInterfaceMeshData::getUVBytesSize(int count){
    return sizeof(FTerrainUV) * count;
}

int StorageInterfaceMeshData::getTrianglesBytesSize(int count){
    return sizeof(FTerrainVertexIndex) * count;
}




//-------- LOADING --------

void StorageInterfaceMeshData::LoadMeshData(
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles,
    int chunkId,
    int layer,
    int lod
){
    FString path = makePath(chunkId, layer, lod);
    LoadMeshData(
        Vertecies,
        Normals,
        UV0,
        Triangles,
        path
    );
}

void StorageInterfaceMeshData::LoadMeshData(
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles,
    FString path
){
    /*
    FMemory::Memcpy ( 
        void* Dest,
        const void* Src,
        SIZE_T Count
    )
    */
    TArray<uint8> bytes;
    if(!LoadBinaryData(path, bytes)){
        DebugHelper::logMessage("Storage Interface mesh data ERROR LOADING BIN DATA");
    }
    PrintBinary(bytes);

    bool endReachedIgnore = false;
    uint8 *Ptr = bytes.GetData(); // at 0 offset.
    LoadIntoMeshBuffers(
        bytes,
        Ptr,
        Vertecies,
        Normals,
        UV0,
        Triangles,
        endReachedIgnore
    );
}


void StorageInterfaceMeshData::LoadIntoMeshBuffers(
    TArray<uint8> &Bytes, //buffer size is increased after append!
    uint8*& Ptr, //is increased after append, must be at correct offset starting with header bytes!
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles,
    bool &endReached
){
    //load info data
    int32 vertexCount = 0;
    int32 normalCount = 0;
    int32 uvCount = 0;
    int32 triangleCount = 0;

    loadInfoData(Ptr, vertexCount, normalCount, uvCount, triangleCount);

    //debug
    if(debugLog){
        FString message = FString::Printf(
            TEXT("Storage Interface MeshData loaded buffer sizes (%d), (%d), (%d), (%d)"),
            vertexCount, normalCount, uvCount, triangleCount
        );
        DebugHelper::logMessage(message);
    }
    

    //SET SIZE VERY IMPORTANT
    Vertecies.SetNumUninitialized(vertexCount);
    Normals.SetNumUninitialized(normalCount);
    UV0.SetNumUninitialized(uvCount);
    Triangles.SetNumUninitialized(triangleCount);

    //copy mesh data based on infoData
    int infoBytesSize = getInfoBytesSize();


    int verteciesByteSize = getVertexBytesSize(vertexCount);
    int normalsBytesSize = getNormalsBytesSize(normalCount);
    int uvBytesSize = getUVBytesSize(uvCount);
    int trianglesByteSize = getTrianglesBytesSize(triangleCount);

    //verify size
    int totalSize =
        infoBytesSize +
        verteciesByteSize +
        normalsBytesSize +
        uvBytesSize +
        trianglesByteSize;

    //if bytes exceeded, mesh data is broken.
    //if(totalSize != sizeof(uint8) * Bytes.Num()){
    if(totalSize > sizeof(uint8) * Bytes.Num()){ //check if reaching out of bounds with Pointer.
        DebugHelper::logMessage("Storage Interface MeshData Exceeded byte size");
        endReached = true;
        return;
    }
    if(totalSize == sizeof(uint8) * Bytes.Num()){
        endReached = true;
    }

    /*
    FMemory::Memcpy (
        void* Dest,
        const void* Src,
        SIZE_T Count
    )
    */

    //TODO:
    //if Data is completly empty, dont even copy.




    FMemory::Memcpy((uint8 *)Vertecies.GetData(), Ptr, verteciesByteSize); // copy casted data
    Ptr += verteciesByteSize;                                             // increase pointer adress


    FMemory::Memcpy((uint8 *)Normals.GetData(), Ptr, normalsBytesSize); // copy casted data
    Ptr += normalsBytesSize;                                            // increase pointer adress

    FMemory::Memcpy((uint8 *)UV0.GetData(), Ptr, uvBytesSize); // copy casted data
    Ptr += uvBytesSize;                                        // increase pointer adress

    FMemory::Memcpy((uint8 *)Triangles.GetData(), Ptr, trianglesByteSize); // copy casted data
    Ptr += trianglesByteSize; // increase pointer adress (if next data loading needed)



}










void StorageInterfaceMeshData::loadInfoData(
    uint8 *& Ptr, //Ptr already at given offset
    int32 &vertexCount,
    int32 &normalCount,
    int32 &uvCount,
    int32 &triangleCount
){
    TArray<int32 *> infoData = {
        &vertexCount,
        &normalCount,
        &uvCount,
        &triangleCount
    };

    /*
    FMemory::Memcpy ( 
        void* Dest,
        const void* Src,
        SIZE_T Count
    )
    */
    for (int i = 0; i < infoData.Num(); i++){
        FMemory::Memcpy(infoData[i], Ptr, sizeof(int32));
        Ptr += sizeof(int32);
    }
}









// ------- MAKE PATH ------
FString StorageInterfaceMeshData::makePath(
    int chunkId,
    int layer,
    int lod
){
    FString fileName = FString::Printf(TEXT("Terrain/Chunk_%.d_Layer_%.d_Lod_%d.bin"), chunkId, layer, lod);
    FString Path = BaseDir(TEXT("debugPathfromMeshData")) + fileName;
    return Path;
}


// ------- TEST ---------
void StorageInterfaceMeshData::Test(){
    TArray<FVector> vertexBuffer = {
        FVector(0, 0, 1),
        FVector(1, 0, 2),
        FVector(2, 0, 3)
    };

    TArray<FVector> normalBuffer = {
        FVector(0, 2, 0),
        FVector(0, 3, 0),
        FVector(0, 4, 0)
    };

    TArray<FVector2D> uvBuffer = {
        FVector2D(0,0),
        FVector2D(0,1),
        FVector2D(1,0)
    };

    TArray<int32> triangleBuffer = {
        0,1,2,
        0,2,1
    };

    //before save
    PrintBuffers(vertexBuffer, normalBuffer, uvBuffer, triangleBuffer);

    //save
    SaveMeshData(
        vertexBuffer,
        normalBuffer,
        uvBuffer,
        triangleBuffer,
        0,
        0,
        0
    );
    

    vertexBuffer.Empty();
    normalBuffer.Empty();
    uvBuffer.Empty();
    triangleBuffer.Empty();

    //debug
    if(false){
        return;
    }

    //load
    LoadMeshData(
        vertexBuffer,
        normalBuffer,
        uvBuffer,
        triangleBuffer,
        0,
        0,
        0
    );

    //CAUTION: EMPTY

    //print
    PrintBuffers(vertexBuffer, normalBuffer, uvBuffer, triangleBuffer);

}













void StorageInterfaceMeshData::PrintBuffers(
    TArray<FVector> &Vertecies,
    TArray<FVector> &Normals,
    TArray<FVector2D> &UV0,
    TArray<int32> &Triangles
){
    FString output = TEXT("StorageInterfaceMeshData: ");
    output += TEXT("VertexBuffer: ");
    for (int i = 0; i < Vertecies.Num(); i++)
    {
        FVector &vertex = Vertecies[i];
        FString part = FString::Printf(TEXT("(%.2f %.2f %2f)"), vertex.X, vertex.Y, vertex.Z);
        output += part;
    }
    output += TEXT("NormalBuffer: ");
    for (int i = 0; i < Normals.Num(); i++)
    {
        FVector &normal = Normals[i];
        FString part = FString::Printf(TEXT("(%.2f %.2f %2f)"), normal.X, normal.Y, normal.Z);
        output += part;
    }

    output += TEXT("UVBuffer: ");
    for (int i = 0; i < UV0.Num(); i++)
    {
        FVector2D &uv = UV0[i];
        FString part = FString::Printf(TEXT("(%.2f %.2f)"), uv.X, uv.Y);
        output += part;
    }

    output += TEXT("triangleBuffer: ");
    for (int i = 0; i < Triangles.Num(); i++)
    {
        int32 &t = Triangles[i];
        FString part = FString::Printf(TEXT("(%d)"), t);
        output += part;
    }

    DebugHelper::logMessage(output);
}


void StorageInterfaceMeshData::PrintBinary(TArray<uint8>&bytes){
    FString byteString = TEXT("Storage Interface Meshdata bin: ");
    for (int i = 0; i < bytes.Num(); i++){
        byteString += FString::FromInt((int32)bytes[i]);
    }

    DebugHelper::logMessage(byteString);
}

/// ----- nachschlage werk -------
/*
void StorageInterface::SaveVertexData(
    FString Path, 
    TArray<FVector> &Vertices
){
    // Save
    int byteSize = Vertices.Num() * sizeof(FTerrainVertex);
    FFileHelper::SaveArrayToFile(
        TArrayView<uint8>((uint8 *)Vertices.GetData(), byteSize),
        *Path
    );
}

void StorageInterface::LoadVertexData(
    FString Path,
    TArray<FVector> &Vertices
){
    // Load
    TArray<uint8> Bytes;
    FFileHelper::LoadFileToArray(Bytes, *Path);
    int sizeAsFVector = Bytes.Num() / sizeof(FTerrainVertex);
    Vertices.SetNum(sizeAsFVector); // zurück zu FVector size
    FMemory::Memcpy(Vertices.GetData(), Bytes.GetData(), Bytes.Num());
}*/