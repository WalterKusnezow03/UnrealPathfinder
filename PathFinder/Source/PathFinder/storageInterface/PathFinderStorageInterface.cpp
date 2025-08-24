#include "PathFinderStorageInterface.h"
#include "PathFinder/pathFinding/PathFinder.h"

PathFinderStorageInterface::PathFinderStorageInterface(){

}

PathFinderStorageInterface::~PathFinderStorageInterface(){

}

// ---- HELPER FUNCTIONS ----

int PathFinderStorageInterface::bytesPerNode(){
    return sizeof(int) + sizeof(FVector);
}

int PathFinderStorageInterface::bytesForNodes(int count){
    if(count >= 0){
        return count * bytesPerNode();
    }
    return 0;
}

FString PathFinderStorageInterface::makePath(FString worldLevelString){
    FString innerPath = FString::Printf(TEXT("PathFinder/graph.bin"));
    FString outString = BaseDir(worldLevelString) + innerPath;
    return outString;
}


// ---- READ / WRITE NODE COUNT (VERY FRONT OF BYTES) SECTION ----

void PathFinderStorageInterface::WriteCountNodes(
    TArray<uint8> &Bytes, //writen to front, expects array to be unitiliazed!
    int count
){
    Bytes.SetNumUninitialized(sizeof(int));
    uint8 *Ptr = Bytes.GetData();
    /*
    FMemory::Memcpy( 
        void* Dest,
        const void* Src,
        SIZE_T Count
    )
    */
    FMemory::Memcpy(Ptr, &count, sizeof(int));
}

int PathFinderStorageInterface::ReadCountNodes(
    uint8 *&Ptr //is increased to next offset
){
    int outCount = 0;
    FMemory::Memcpy(&outCount, Ptr, sizeof(int));
    Ptr += sizeof(int);
    return outCount;
}

// ---- APPEND / READ NODE (id + pos) SECTION

//appends node in array. Data from before is kept.
void PathFinderStorageInterface::AppendNode(
    TArray<uint8> &Bytes,
    APathFinder::Node *node
){
    if(node){
        int idToWrite = node->getId();
        


        FVector pos = node->pos;

        int prevSize = Bytes.Num();
        Bytes.SetNumUninitialized(prevSize + bytesPerNode());
        uint8 *Ptr = Bytes.GetData(); //get ptr, now save, array resized.
        Ptr += prevSize; //move to correct offset

        /*
        FMemory::Memcpy( 
            void* Dest,
            const void* Src,
            SIZE_T Count
        )
        */
        FMemory::Memcpy(Ptr, &idToWrite, sizeof(int));
        Ptr += sizeof(int); //increase pointer adress to next write position

        FMemory::Memcpy(Ptr, &pos, sizeof(FVector));
        //Ptr += sizeof(FVector); //not needed, end of operation here.


        if(extendedDebugLog){
            FString message = FString::Printf(
                TEXT("Storage Interface PathFinder Saved Node (%d %d %d) id(%d)"),
                pos.X, pos.Y, pos.Z, idToWrite
            );
            DebugHelper::logMessage(message);
        }
    }
}


void PathFinderStorageInterface::LoadNode(
    uint8*& Ptr, //will modified to next offset!
    APathFinder *pathFinder
){
    if(Ptr && pathFinder){
        int copiedId = -1;
        FVector copiedPos;

        /// Structure: [int id][FVector pos]

        /*
        FMemory::Memcpy( 
            void* Dest,
            const void* Src,
            SIZE_T Count
        )
        */
        FMemory::Memcpy(&copiedId, Ptr, sizeof(int));
        Ptr += sizeof(int); //increase pointer adress to next write position

        FMemory::Memcpy(&copiedPos, Ptr, sizeof(FVector));
        Ptr += sizeof(FVector); //increase pointer to next node to read

        //add node to graph, without connections, saved seperatlyafter all nodes.
        //is only saved if id is valid
        pathFinder->addNodeFromStorageInterfaceNoConnection(copiedPos, copiedId);

        if(extendedDebugLog){
            FString message = FString::Printf(
                TEXT("Storage Interface PathFinder Loaded Node (%d %d %d) id(%d)"),
                copiedPos.X, copiedPos.Y, copiedPos.Z, copiedId
            );
            DebugHelper::logMessage(message);
        }
    }
}

// ----- READ / WRITE NEIGHBORHOOD SECTION -----
void PathFinderStorageInterface::AppendConvexAndNeighborHood(
    TArray<uint8> &Bytes,
    APathFinder::Node *node
){
    if(!node){
        return;
    }

    //Structure: [int id][id convexA][id convexB][CountNeighbors][ids Neighbors]
    int nodeId = node->getId();
    int IdConvexA = node->IdConvexNeighborA();
    int IdConvexB = node->IdConvexNeighborB();

    TArray<int> neighborIds = node->NeighborsById(); //only valid ids
    int countNeighbors = neighborIds.Num();

    int bytesNeighbors = sizeof(int) * countNeighbors;
    int bytesTotalAppended = sizeof(int) * 4 + bytesNeighbors;

    //set array num, get pointer
    int prevOffset = Bytes.Num();
    Bytes.SetNumUninitialized(prevOffset + bytesTotalAppended);
    uint8 *Ptr = Bytes.GetData();
    Ptr += prevOffset; //move to end where new data starts


    //Structure: [int id][id convexA][id convexB][CountNeighbors][ids Neighbors]
    /*
    FMemory::Memcpy( 
        void* Dest,
        const void* Src,
        SIZE_T Count
    )
    */
    FMemory::Memcpy(Ptr, &nodeId, sizeof(int));
    Ptr += sizeof(int); //increase pointer adress to next write position
    
    FMemory::Memcpy(Ptr, &IdConvexA, sizeof(int));
    Ptr += sizeof(int); //increase pointer adress to next write position

    FMemory::Memcpy(Ptr, &IdConvexB, sizeof(int));
    Ptr += sizeof(int); //increase pointer adress to next write position

    FMemory::Memcpy(Ptr, &countNeighbors, sizeof(int));
    Ptr += sizeof(int); //increase pointer adress to next write position

    FMemory::Memcpy(Ptr, (uint8 *) neighborIds.GetData(), bytesNeighbors);


    if(extendedDebugLog){
        FString message = FString::Printf(
            TEXT("Storage Interface PathFinder Saved Node NeighborData id(%d) convex (%d, %d)connected to: ("),
            nodeId, IdConvexA, IdConvexB
        );
        for (int i = 0; i < neighborIds.Num(); i++){
            message += FString::Printf(TEXT("%d,"), neighborIds[i]);
        }
        message += TEXT(")");

        DebugHelper::logMessage(message);
    }



}

void PathFinderStorageInterface::ReadConvexAndNeighborHood(
    uint8*& Ptr, //will modified to next offset!
    APathFinder *pathFinder
){
    int nodeId = -1;
    int IdConvexA = -1;
    int IdConvexB = -1;
    int countNodes = -1;
    TArray<int> neighborsById;


    /*
    FMemory::Memcpy( 
        void* Dest,
        const void* Src,
        SIZE_T Count
    )
    */
    FMemory::Memcpy(&nodeId, Ptr, sizeof(int));
    Ptr += sizeof(int);
    FMemory::Memcpy(&IdConvexA, Ptr, sizeof(int));
    Ptr += sizeof(int);
    FMemory::Memcpy(&IdConvexB, Ptr, sizeof(int));
    Ptr += sizeof(int);
    FMemory::Memcpy(&countNodes, Ptr, sizeof(int));
    Ptr += sizeof(int);

    int bytesNeighbors = sizeof(int) * countNodes;
    neighborsById.SetNumUninitialized(countNodes);
    FMemory::Memcpy((uint8*) neighborsById.GetData(), Ptr, bytesNeighbors);
    Ptr += bytesNeighbors;

    if(pathFinder){
        pathFinder->addConnectionsFromStorageInterfaceForNodeById(
            nodeId,
            neighborsById,
            IdConvexA,
            IdConvexB
        );
    }

    if(extendedDebugLog){
        FString message = FString::Printf(
            TEXT("Storage Interface PathFinder Loaded Node NeighborData id(%d) convex (%d, %d)connected to: ("),
            nodeId, IdConvexA, IdConvexB
        );
        for (int i = 0; i < neighborsById.Num(); i++){
            message += FString::Printf(TEXT("%d,"), neighborsById[i]);
        }
        message += TEXT(")");

        DebugHelper::logMessage(message);
    }

}

// ----- SAVE SECTION ------

void PathFinderStorageInterface::Save(FString worldName, APathFinder *pathFinder){
    if(!pathFinder){
        DebugHelper::logMessage("Storage Interface Pathfinder: could not save because pathfinder Ptr is invalid");
        return;
    }

    //es ist im allgemeinen einfacher einfach Alle Nodes iterativ anzuhängen,
    //aber nicht unbedingt richtig
    
    //die ganze binary sollte so aufgebaut sein,
    //das erst der ganze graph aufgebaut wird, und dann die verbindungen gesetzt werden!

    //[headerTotatlNodes][DataBlock Nodes (id + pos)] [DataBlock Connections (id, connected neighbors, convex neighbors)]

    //diese werden dann zum pathfinder

    //write node count
    int nodesCount = pathFinder->countNodesTrackedInIdMap();
    TArray<uint8> Bytes;
    WriteCountNodes(Bytes, nodesCount);

    const std::map<int, APathFinder::Node *> &IdMappedNodesReference = pathFinder->IdMapReference();
    //write all graph nodes first!
    for(auto &pair : IdMappedNodesReference){
        AppendNode(Bytes, pair.second);
    }

    //write neighborhood data after graph has been writen completly
    for(auto &pair : IdMappedNodesReference){
        AppendConvexAndNeighborHood(Bytes, pair.second);
    }

    FString path = makePath(worldName);
    SaveBinaryData(path, Bytes);
}

// ----- LOAD SECTION ------

bool PathFinderStorageInterface::Load(FString worldName, APathFinder *pathFinder){
    if(!pathFinder){
        return false;
    }
    FString path = makePath(worldName);
    TArray<uint8> Bytes;
    if(!LoadBinaryData(path, Bytes)){
        DebugHelper::logMessage("Storage Interface PathFinder Failed Loading BIN!");
        return false;
    }

    uint8 *Ptr = Bytes.GetData();
    uint8 *PtrEndData = Ptr + Bytes.Num();

    //return;

    //LOOP FOR EVER - MAC IS FULL RAM STACK FULL ALL FuLL!
    int countNodes = ReadCountNodes(Ptr);
    for (int i = 0; i < countNodes; i++){
        LoadNode(Ptr, pathFinder);
    }

    /*
    while(Ptr < PtrEndData){
        //Ptr will modified to next offset!
        ReadConvexAndNeighborHood(Ptr, pathFinder);
    }*/

    //es gibt genauso viele convex data + neighbor hoods wie nodes.
    for (int i = 0; i < countNodes; i++){
        ReadConvexAndNeighborHood(Ptr, pathFinder);
    }

    return true;
}

