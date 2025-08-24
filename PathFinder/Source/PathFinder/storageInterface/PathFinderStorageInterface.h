#pragma once

#include "StoragePlugin/Storage/BaseInterface/StorageInterface.h"

class APathFinder;

class PATHFINDER_API PathFinderStorageInterface : public StorageInterface {

public:
    PathFinderStorageInterface();
    virtual ~PathFinderStorageInterface() override;

    void Save(FString worldName, APathFinder *pathFinder);
    bool Load(FString worldName, APathFinder *pathFinder);

private:
    bool extendedDebugLog = true;

    FString makePath(FString worldLevelString);

    int bytesPerNode();
    int bytesForNodes(int count);

    //appends a node by id and position into array, no neighborhood
    void AppendNode(
        TArray<uint8> &Bytes,
        APathFinder::Node *node
    );

    ///loads a node by id and position, pointer increased, node added to graph
    void LoadNode(
        uint8 *&Ptr, // will modified to next offset!
        APathFinder *pathFinder
    );

    /// write node count at front of array
    /// expects array to be unitiliazed!
    void WriteCountNodes(TArray<uint8> &array, int count);

    /// read node count at Ptr, ptr is Increased to next offset
    int ReadCountNodes(uint8 *&Ptr);

    //appends node data: by id, added: convex a & b, neighbors
    void AppendConvexAndNeighborHood(
        TArray<uint8> &Bytes,
        APathFinder::Node *node
    );

    //reads node data: by id, added: convex a & b, neighbors
    void ReadConvexAndNeighborHood(
        uint8 *&Ptr, // will modified to next offset!
        APathFinder *pathFinder
    );
};