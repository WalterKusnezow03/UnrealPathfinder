
#pragma once
#include "CoreMinimal.h"
#include "PathFinder/pathFinding/PathFinder.h"


class PATHFINDER_API raycastTask{

public:
    raycastTask();
    ~raycastTask();

    raycastTask(const raycastTask &other);
    raycastTask &operator=(const raycastTask & other);

    void setup(
        UWorld *world,
        APathFinder::Node *aIn,
        APathFinder::Node *bIn
    );

    void execute();

private:
    bool passTangentialCheck();
    bool syncRaycastCanSee();

    UWorld *worldPointer = nullptr;
    APathFinder::Node *a = nullptr;
    APathFinder::Node *b = nullptr;

    bool wasExecuted = false;

    FCriticalSection locker;
};