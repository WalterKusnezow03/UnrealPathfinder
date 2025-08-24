// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DrawDebugHelpers.h"
#include <cmath>
#include <algorithm>
#include "CoreMinimal.h"
#include <limits>
#include "GameCore/DebugHelper.h"
#include "priorityList.h"
#include "PathFinder/pathFinding/priorityQueue.h"
#include "PathFinder/pathFinding/raycastTasks/raycastTask.h"
#include "PathFinder/storageInterface/PathFinderStorageInterface.h"

//---- STATIC VARS ----
APathFinder* APathFinder::pathFinderInstance = nullptr; //very imporntant, do not delete!
int APathFinder::countNodes = 0;

void APathFinder::resetPathFinderPointer(){
    APathFinder::pathFinderInstance = nullptr;
}

bool APathFinder::alreadyHasInstance(){
    return APathFinder::pathFinderInstance != nullptr;
}

bool APathFinder::IdIsValid(int id){
    return id >= 0;
}

//---- Launch API ----
APathFinder::APathFinder(){
    PrimaryActorTick.bCanEverTick = true;
}

void APathFinder::makeInstance(UWorld *world, FString worldLevelName){
    if (world != nullptr)
    {
        UClass *toSpawn = APathFinder::StaticClass();
        if(toSpawn){
            FActorSpawnParameters SpawnParams;
            FVector Location;
            AActor *spawned = world->SpawnActor<AActor>(toSpawn, Location, FRotator::ZeroRotator, SpawnParams);   
            if(spawned){
                APathFinder *casted = Cast<APathFinder>(spawned);
                if(casted){
                    APathFinder::pathFinderInstance = casted;
                    casted->Setup(worldLevelName);
                }
            }
        }
    }
}

void APathFinder::Setup(FString worldname){
    worldLevelNameSaved = worldname;
    TopRight = new APathFinder::Quadrant(1, 1);
    BottomRight = new APathFinder::Quadrant(1,-1);
	TopLeft = new APathFinder::Quadrant(-1,1);
	BottomLeft = new APathFinder::Quadrant(-1,-1);

    DebugHelper::logMessage("PATHFINDER LAUNCHED");

    PathFinderStorageInterface interface;
    if(interface.Load(worldLevelNameSaved, this)){
        FString message = FString::Printf(
            TEXT("Storage Interface PathFinder Loaded Nodes from storage (%d)"),
            countNodesTrackedInIdMap()
        );
        DebugHelper::logMessage(message);
    }else{
        DebugHelper::logMessage("Storage Interface PathFinder failed load from storage");
    }
}

void APathFinder::BeginPlay(){
    Super::BeginPlay();
}

void APathFinder::EndPlay(const EEndPlayReason::Type EndPlayReason){

    //Todo: Complete all tasks before saving to storage 


    //save nodes to storage
    PathFinderStorageInterface interface;
    interface.Save(worldLevelNameSaved, this);

    //reset pointer
    resetPathFinderPointer();

    //delete all
    delete (TopRight); 
    delete (BottomRight); 
    delete (BottomLeft); 
    delete (TopLeft);
    clearDelegates();


    Super::EndPlay(EndPlayReason);
}

void APathFinder::Tick(float deltatime){
    Super::Tick(deltatime);

    if(rayTasksVec.size() > 0){
        int tasksPerTick = 10000;
        for (int i = 0; i < tasksPerTick; i++){
            if(rayTasksVec.size() <= 0){
                return;
            }
            raycastTask &current = rayTasksVec.back();
            current.execute();
            rayTasksVec.pop_back();
            if(i == tasksPerTick-1){
                DebugHelper::showScreenMessage("NEW SYNC TRACE BATCH MADE!",rayTasksVec.size(), FColor::Orange);
            }
        }
    }
}



APathFinder::Node::Node(FVector posIn){
    gx = std::numeric_limits<float>::max(); //set to max for unknown status by default
    fx = gx;
    pos = posIn;
    camefrom = nullptr;
    closedFlag = false;
    nA = nullptr;
    nB = nullptr;


    if(APathFinder *ptr = APathFinder::instance()){
        id = ptr->staticId++; //copy id and post increment.
        ptr->TrackNodeInIdMap(this); //track in id map for later save
    }
}
APathFinder::Node::Node(int idIn, FVector posIn){
    gx = std::numeric_limits<float>::max(); //set to max for unknown status by default
    fx = gx;
    pos = posIn;
    camefrom = nullptr;
    closedFlag = false;
    nA = nullptr;
    nB = nullptr;

    //Save Id!
    id = idIn; 
}

APathFinder::Node::Node(Node &other){
    *this = other;
}
APathFinder::Node &APathFinder::Node::operator=(Node &other){
    if(this != &other){
        gx = other.gx; //set to max for unknown status by default
        fx = other.fx;
        pos = other.pos;
        camefrom = other.camefrom;
        closedFlag = other.closedFlag;
        nA = other.nA;
        nB = other.nB;
    }
    return *this;
}

APathFinder::Node::~Node(){
    camefrom = nullptr;
    visible_tangential_Neighbors.clear();
}

APathFinder::Quadrant::Quadrant(int xSampleIn, int ySampleIn){
    xSample = xSampleIn;
    ySample = ySampleIn;
}

APathFinder::Quadrant::~Quadrant(){
    for (int i = 0; i < map.size(); i++){
        for (int j = 0; j < map.at(i).size(); j++){
            APathFinder::Chunk *c = map.at(i).at(j);
            if(c != nullptr){
                delete (c);
                map.at(i).at(j) = nullptr;
            }
        }
    }
}

APathFinder::Chunk::Chunk(){}
APathFinder::Chunk::~Chunk(){
    for (int i = 0; i < nodes.size(); i++){
        if(nodes.at(i) != nullptr){
            delete (nodes.at(i));
            nodes.at(i) = nullptr;
        }
    }
}

//constructors end

//clear functions

/// @brief clears all nodes from the whole navmesh but doesnt delete the chunks
void APathFinder::clear(){
    TArray<APathFinder::Quadrant*> array = {TopLeft, BottomLeft, TopRight, BottomRight};
    for (int i = 0; i < array.Num(); i++){
        if(array[i] != nullptr){
            array[i]->clear();
        }
    }
    clearDelegates();
}

void APathFinder::Quadrant::clear(){
    for (int i = 0; i < map.size(); i++){
        for (int j = 0; j < map.at(i).size(); j++){
            APathFinder::Chunk *c = map.at(i).at(j);
            if(c != nullptr){
                c->clear();
            }
        }
    }
}

void APathFinder::Chunk::clear(){
    for (int i = 0; i < nodes.size(); i++){
        if(nodes.at(i) != nullptr){
            delete (nodes.at(i));
            nodes.at(i) = nullptr;
        }
    }
    nodes.clear(); //finally clear the nodes
}


void APathFinder::clearDelegates(){
    //delete all delegates
    for (int i = 0; i < released.size(); i++){
        FTraceDelegate *delegate = released.at(i);
        if (delegate != nullptr)
        {
            //DebugHelper::logMessage("debug deleted delegate");
            delete delegate;
            
        }
    }
    released.clear();
}

//clear functions end

//PATH FINDER METHODS



/// @brief returns the instance of the poath finder. DO NOT DELETE
/// CAUTION PATH FINDER MUST BE LAUNCHED THROUGH PATHFINDER MODULE
APathFinder* APathFinder::instance(UWorld *worldIn){
    /*if(alreadyHasInstance()){
        return pathFinderInstance;
    }
    APathFinder::makeInstance(worldIn);
    if(alreadyHasInstance()){
        return pathFinderInstance;
    }
    return nullptr;*/
    return instance();
}

/// @brief acces the pathFinder instance, WILL NOT CREATE ONE
/// BECAUSE WORLD CONTEXT IS NOT PROVIDED, CAN RETURN NULLPTR
/// DO NOT DELETE
/// @return
APathFinder* APathFinder::instance(){
    return pathFinderInstance;
}





//debug drawing
void APathFinder::showPos(FVector e){
    if(debugDrawNodes){
        showPos(e, FColor::Green);
    }
}

void APathFinder::showPos(FVector e, FColor c){
    if(debugDrawNodes){
        FVector End = e + FVector(0, 0, 10000);
        DebugHelper::showLineBetween(GetWorld(), e, End, c);
    }
}


/// @brief adds a new node vector (of locations) to the graph and applies an offset to each node
/// all nodes are independant
/// if wanted
/// @param vec vector to add to graph 
/// @param offset offset to apply to each node
void APathFinder::addNewNodeVector(std::vector<FVector> &vec, FVector &offset){
    for (int i = 0; i < vec.size(); i++){
        addNewNode(vec.at(i) + offset);
    }
}


void APathFinder::addNewNodeVector(std::vector<FVector> &vec, std::vector<FVector> &offsets){
    for (int i = 0; i < offsets.size(); i++){
        FVector current = offsets[i];
        addNewNodeVector(vec, current);
    }
}



/// @brief add nodes to the graph, all independant
/// @param vec vector to push completly
void APathFinder::addNewNodeVector(std::vector<FVector>& vec){
    for (int i = 0; i < vec.size(); i++){
        addNewNode(vec.at(i));
    }

}



/// @brief expects the vector to be a convex hull of an object / grounded nodes! Do not ignore!
/// @param vector vector of positions, convex hull!
void APathFinder::addConvexHull(std::vector<FVector> &vec){
    
    //create all nodes
    std::vector<Node *> outNodes;
    for (int i = 0; i < vec.size(); i++){
        APathFinder::Node *n = new APathFinder::Node(vec.at(i));
        outNodes.push_back(n);
    }

    // add the konvex neighbors
    for (int i = 0; i < vec.size(); i++)
    {
        APathFinder::Node *prev = nullptr;
        APathFinder::Node *next = nullptr;

        if (i == 0)
        {
            prev = outNodes.at(outNodes.size() - 1);
        }else{
            prev = outNodes.at(i - 1);
        }


        if(i == outNodes.size() - 1){
            next = outNodes.at(0);
        }
        else{
            next = outNodes.at(i + 1);
        }


        APathFinder::Node *current = outNodes.at(i);
        if(prev != nullptr && current != nullptr && next != nullptr){
            //current->nA = prev;
            //current->nB = next;
            current->setConvexNeighborA(prev); //es wird davon ausgegangen das sich nodes auf der hülle sehen
            current->setConvexNeighborB(next);
        }

    }



    //alle sofort in graphen ballern
    for (int i = 0; i < outNodes.size(); i++){
        if(outNodes.at(i) != nullptr){
            addNode(outNodes.at(i));

            //NEW:
            //convex hull index setzten für faster path on hull
            //outNodes.at(i)->hullindex = i;
        }
    }
}



/// @brief adds a single node to the graph
/// @param a node to add
void APathFinder::addNewNode(FVector a){
    APathFinder::Quadrant *q = askforQuadrant(a.X, a.Y);
    if(q != nullptr){
        //FScopeLock Lock(&delegate_CriticalSection_a); //new lock added
        q->add(a);
    }
}

void APathFinder::addNode(Node * node){
    if(node != nullptr){
        FVector posCopy = node->pos;
        APathFinder::Quadrant *q = askforQuadrant(posCopy.X, posCopy.Y);
        if(q != nullptr){
            //FScopeLock Lock(&delegate_CriticalSection_a); //new lock added
            q->add(node);
        }
    }
}



// --- STORAGE INTERFACE METHODS ---

const std::map<int, APathFinder::Node *> &APathFinder::IdMapReference(){
    return idMappedNodes;
}

void APathFinder::addNodeFromStorageInterfaceNoConnection(
    FVector &position,
    int id
){
    if(!IdIsValid(id)){
        return;
    }
    if(NodeIsTrackedInIdMap(id)){
        return;
    }

    APathFinder::Quadrant *q = askforQuadrant(position.X, position.Y);
    if(q != nullptr){

        APathFinder::Node *nodeNew = new Node(id, position);
        q->addNoConnect(nodeNew);
        TrackNodeInIdMap(nodeNew);
    }
}

void APathFinder::addConnectionsFromStorageInterfaceForNodeById(
    int id,
    TArray<int> connected,
    int convexA,
    int convexB
){
    //connect to all neighbors if possible!
    if(NodeIsTrackedInIdMap(id)){
        Node *current = idMappedNodes[id];
        if(current){

            //add connected neighbors
            for (int i = 0; i < connected.Num(); i++){
                int neighborId = connected[i];
                if(NodeIsTrackedInIdMap(neighborId)){
        
                    //special convex hull ignored for now. / is not used at all.
                    Node *other = idMappedNodes[neighborId];
        
                    other->addTangentialNeighbor(current);
                    current->addTangentialNeighbor(other);
                }
            }

            //set convex neighbors
            if(NodeIsTrackedInIdMap(convexA)){
                current->setConvexNeighborA(idMappedNodes[convexA]);
            }
            if(NodeIsTrackedInIdMap(convexB)){
                current->setConvexNeighborB(idMappedNodes[convexB]);
            }
        }
    }
}




void APathFinder::TrackNodeInIdMap(APathFinder::Node *node){
    if(node){
        int idRead = node->getId();
        if(idMappedNodes.find(idRead) == idMappedNodes.end()){
            idMappedNodes[idRead] = node;
        }
    }
}

bool APathFinder::NodeIsTrackedInIdMap(int id){
    return idMappedNodes.find(id) != idMappedNodes.end();
}

int APathFinder::countNodesTrackedInIdMap(){
    return idMappedNodes.size();
}

// --- Storage interface methods end ---





APathFinder::Quadrant* APathFinder::askforQuadrant(int xIndex, int yIndex){
    //top left
    if(xIndex < 0 && yIndex >= 0){
        return this->TopLeft;
    }
    //top right
    if(xIndex >= 0 && yIndex >= 0){
        return this->TopRight; 
    }
    //bottom left
    if(xIndex < 0 && yIndex < 0){
        return this->BottomLeft;
    }
    //bottom right
    if(xIndex >= 0 && yIndex < 0){
        return this->BottomRight;
    }
    return nullptr;
}



/// @brief finds a node from the correct quadrant
/// @param node 
/// @return 
APathFinder::Node* APathFinder::findNode(FVector node){
    int x = (int) node.X;
    int y = (int) node.Y;
    APathFinder::Quadrant *q = askforQuadrant(x, y);
    if(q != nullptr){
        APathFinder::Node *nodeFound = q->findNode(node);
        if(nodeFound != nullptr){
            DebugHelper::showScreenMessage("node found path finder", FColor::Green);
            return nodeFound;
        }
    }else{
        DebugHelper::showScreenMessage("QUADRANT IS NULL!!!!!", FColor::Red);
    }
    
    return nullptr;
}

APathFinder::Node* APathFinder::findNodeInDirection(FVector &node, FVector &dir){
    int x = (int) node.X;
    int y = (int) node.Y;
    APathFinder::Quadrant *q = askforQuadrant(x, y);
    if(q != nullptr){
        APathFinder::Node *nodeFound = q->findNodeInDirection(node, dir);
        if(nodeFound != nullptr){
            DebugHelper::showScreenMessage("node found path finder", FColor::Green);
            return nodeFound;
        }
    }else{
        DebugHelper::showScreenMessage("QUADRANT IS NULL!!!!!", FColor::Red);
    }
    
    return nullptr;
}





std::vector<APathFinder::Node *> APathFinder::getSubGraph(FVector a, FVector b){

    std::vector<APathFinder::Node *> nodes;
    std::vector<APathFinder::Node *> asked;

    //why is this just iterating over all quadrants:
    //the "askForArea(a,b)" method is clamping the coordinates by it self
    //to the correct values to properly get all nodes in the correct area.
    TArray<APathFinder::Quadrant*> array = {TopLeft, BottomLeft, TopRight, BottomRight};
    
    
    
    for (int i = 0; i < array.Num(); i++){
        if(array[i] != nullptr){

            asked = array[i]->askForArea(a, b);
            if(asked.size() > 0){
                nodes.insert(nodes.end(), asked.begin(), asked.end());
            }

            
        }
    }
    return nodes;
}


void APathFinder::debugCountNodes(){

    FVector a = FVector(-99999999999, 0, -99999999999);
    FVector b = FVector(99999999999, 0, 99999999999);

    std::vector<APathFinder::Node *> nodes = getSubGraph(a, b);

    FString string = FString::Printf(TEXT("collected COUNT %d"), APathFinder::countNodes);
    FString string2 = FString::Printf(TEXT("collected SUBGRAPH %d"), nodes.size());

    if (GEngine && false)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, string);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, string2);
    }



    //draw nodes
    for (int i = 0; i < nodes.size(); i++){
        //each neighbor, draw line
        APathFinder::Node *current = nodes.at(i);
        if (current != nullptr)
        {
            for (int j = 0; j < current->visible_tangential_Neighbors.size(); j++){
                APathFinder::Node *currNeighbor = current->visible_tangential_Neighbors.at(j);
                if(currNeighbor != nullptr){
                    DebugHelper::showLineBetween(
                        GetWorld(),
                        current->pos,
                        currNeighbor->pos,
                        FColor::Red,
                        100.0f
                    );

                }
            }
        }
    }
}



/// @brief returns a path from a given position a to an given position b
/// @param a fvector start point targeted
/// @param b fvector end point targeted
/// @return a path or an emtpy vector if no path was found
std::vector<FVector> APathFinder::getPath(FVector a, FVector b){

    //check if is last path
    if(prevPath.size() > 0){
        FVector s = prevPath.front();
        FVector e = prevPath.back();
        if(
            FVector::Dist(s, a) <= 500 && 
            FVector::Dist(e, b) <= 500
        ){
            return prevPath;
        }
    }

    APathFinder::Node *start = nullptr;
    APathFinder::Node *end = nullptr;

    FVector dir = b - a;
    start = findNodeInDirection(a, dir);
    //start = findNode(a);
    if(start == nullptr){
        DebugHelper::showScreenMessage("START NODE NOT FOUND", FColor::Red);
    }

    end = findNode(b);
    if(end == nullptr){
        DebugHelper::showScreenMessage("END NODE NOT FOUND", FColor::Red);
    }



    if(start != nullptr && end != nullptr){
        
        //PREBUILD EDGES
        if(PREBUILD_EDGES_ENABLED){
            if(false){
                showPos(start->pos, FColor::Blue);
            }
            
            showPos(end->pos, FColor::Purple);
            return findPath_prebuildEdges(start, end);
        }

        //LIVE CREATED EGDES

        //find path
        std::vector<APathFinder::Node *> graph = getSubGraph(a, b);
        
        if(debugDrawNodes && false){
            showPos(start->pos, FColor::Blue);
            showPos(end->pos, FColor::Red);
            DebugHelper::showLineBetween(GetWorld(), start->pos, end->pos, FColor::Yellow);
        }
        
        return findPath(start, end, graph);
    }



    return std::vector<FVector>();
}


/// @brief calculates the distance between 2 nodes and their positions
/// @param A 
/// @param B 
/// @return 
float APathFinder::distance(APathFinder::Node* A, APathFinder::Node *B){
    if(A != nullptr && B != nullptr){
        float d = FVector::Dist(A->pos, B->pos);
        return d;
    }
    return std::numeric_limits<float>::max();
}

float APathFinder::distance(FVector A, FVector B){
    return FVector::Dist(A, B);
}




/// @brief finds the path between 2 given nodes in the subgraph
/// @param start node from the graph
/// @param end node from the graph
/// @param subgraph graph enclosed by start and end node. Use Subgraph method
/// @return vector of positions: path
std::vector<FVector> APathFinder::findPath(
    Node *start, 
    Node *end, 
    std::vector<APathFinder::Node*> &subgraph
){
    //screenMessage(FString::Printf(TEXT("subgraph size %d"), subgraph.size()));
    

    for (int i = 0; i < subgraph.size(); i++){
        APathFinder::Node *n = subgraph.at(i);
        if(n != nullptr){
            n->reset();
            n->closedFlag = false;
        }
    }

    start->camefrom = nullptr;
    start->closedFlag = false;
    start->gx = 0;
    start->fx = 0;

    end->camefrom = nullptr;
    end->closedFlag = false;

    std::vector<APathFinder::Node*> openList;

    
    priorityQueue openList_;
    openList_.add(start);

    while(openList_.hasNodes()){
    
        APathFinder::Node *current = openList_.popLowestFx();

        if (current != nullptr)
        {
            if(reached(current, end)){
                //path found
                //screenMessage("found path");
                return constructPath(end);
            }

            //show opened nodes: debugging
            if(debugDrawNodes){
                showPos(current->pos, FColor::Blue);
            }
            

            current->close();
            for (int i = 0; i < subgraph.size(); i++)
            {
                APathFinder::Node *n = subgraph.at(i);
                if(n != nullptr){
                    //bool wasClosed = n->closedFlag;
                    if(!n->isClosed()){
                        
                        //create edge at runtime: 
                        //all edges are rechecked during runtime 
                        //because otherwise it could not operate on subgraphs
                        //but is nesecarry to eliminate many nodes at once
                        if(canSeeTangential(current, n)){
                            

                            //gxnew = gx + hx, potentially smaller then prev of other
                            //is shorter path.
                            float gxNew = current->gx + distance(current->pos, n->pos); 
                            if(gxNew < n->gx){
                                //screenMessage(300);
                                float hxEnd = distance(n->pos, end->pos);
                                n->updateCameFrom(gxNew, hxEnd, *current);

                                
                            }
                            //ADD TO OPEN LIST!!
                            openList_.add(n); 
                        }

                        
                    }
                }
            }
        }
    }

    /*
    if (GEngine)
    {
        FString string = FString::Printf(TEXT("out of nodes"));
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, string);
    }*/

    return std::vector<FVector>();
}


bool APathFinder::reached(Node *a, Node *b){
    if(a == nullptr || b == nullptr){
        return false;
    }
    if(a == b){
        return true;
    }
    //neighbor

    //distance
    if(FVector::Dist(a->pos, b->pos) <= 500){
        return true;
    }

    //can see
    if (PREBUILD_EDGES_ENABLED == false && canSee(a->pos, b->pos)){
        return true;
    }
    return false;
}

void APathFinder::screenMessage(int s){
    if (GEngine)
    {
        FString string = FString::Printf(TEXT("text %d"), s);
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, string);
    }
}

void APathFinder::screenMessage(FString s) {
    if (GEngine) {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, s);
    }
}



/// @brief checks if 2 nodes can see each other, damagable interfaces are excluded for now.
/// @param A position A
/// @param B position B
/// @return can see without interrupt
bool APathFinder::canSeeTangential(APathFinder::Node *A, APathFinder::Node*B){
    if(A && B){

        // if edge is too vertical and to high: ignore, cant climb walls.
        // AB = B - A;
        if(isCloseAndTooVertical(A, B)){
            return false;
        }

        //pass tangental check to check for raycast or not
        if (passTangentailCheck(A,B) == false){
            return false; //not tangential, do not check
        }

        // vision checking
        FVector Start = A->pos;
        FVector End = B->pos;
        //testing with more raycasts to ensure realibilty
        if (canSee(Start, End)){
            return true;
        }
        



        //adjustments if any entity might be in way
        Start.Z += ONE_METER * 1.7f;
        End.Z += ONE_METER * 1.7f;
        if (canSee(Start, End)){
            return true;
        }

    }
    return false; //issue: can see false.
}

/// @brief checks with a simple raycasts if nodes can see each other
/// @param Start start node
/// @param End end node
/// @return edge of interest to pass / existent, efficent
bool APathFinder::canSee(FVector &Start, FVector &End){
    if(UWorld *worldPointer = GetWorld()){
        FHitResult HitResult;
        FCollisionQueryParams Params = collsionParamsLowDetailAndFast();
        //Params.bTraceComplex = false; // HIER LOWER RAYCAST DETAIL-> FASTER

        //add params from entity manager (contains all bots for example, which can be ignored)
        //part of a bigger context im working on, comment out or provide your own params
        /*if(EntityManager *e = worldLevel::entityManager()){
            Params = e->getIgnoredRaycastParams();
        }*/
        Params = collisionIgnoreParams;

        //default casting
        bool hittedAnything = worldPointer->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);
        bool canSee = !hittedAnything;
        return canSee;
    }
    return false;
}

std::vector<FVector> APathFinder::constructPath(APathFinder::Node *end){
    std::vector<FVector> list;
    if (end != nullptr)
    {
        while(end != nullptr){
            list.push_back(end->pos);
            end = end->camefrom;
        }

        std::reverse(list.begin(), list.end());
    }
    prevPath = list; //save copy
    return list;
}

/// @brief if edge is too vertical and to high: ignore, cant climb walls.
/// @param a node a 
/// @param b node b
/// @return too close and vertical true or false
bool APathFinder::isCloseAndTooVertical(Node *a, Node *b){

    if(a != nullptr && b != nullptr){
        // AB = B - A;
        FVector AB = b->pos - a->pos;
        float upZ = std::abs(AB.GetSafeNormal().Z); //if the normalized vector, Z is close to 1 its paralell to up axis
        float scale = std::abs(AB.Z);
        if (upZ > 0.8f)
        {
            // skalarproduct up is near one / paralell to z axis
            if (AB.Z > ONE_METER)
            {
                return true;
            }
        }
    }
    return false;
}










/***
 * ---- QUADRANT METHODS ----
 */

/// @brief adds a node to the quadrant
/// @param n position to add
void APathFinder::Quadrant::add(FVector n){
    //std::abs for flipping negatives obviosuly
    int x = std::abs(n.X / CHUNKSIZE); //create new chunks?
    int y = std::abs(n.Y / CHUNKSIZE);
    

    fillMapTo(x, y);

    // Add the node to the appropriate chunk
    map[x][y]->add(n);   
}


/// @brief adds a node to the quadrant (node node)
/// @param n position to add
void APathFinder::Quadrant::add(Node *n){
    if(n != nullptr){
        //std::abs for flipping negatives obviosuly
        int x = std::abs(n->pos.X / CHUNKSIZE); //create new chunks?
        int y = std::abs(n->pos.Y / CHUNKSIZE);
        
        fillMapTo(x, y);

        // Add the node to the appropriate chunk
        map[x][y]->add(n);
    }   
}


void APathFinder::Quadrant::addNoConnect(Node *node){
    if(node){
        int x = std::abs(node->pos.X / CHUNKSIZE); //create new chunks?
        int y = std::abs(node->pos.Y / CHUNKSIZE);
        
        fillMapTo(x, y);

        // Add the node to the appropriate chunk
        map[x][y]->addNoConnect(node);
    }
}




void APathFinder::Quadrant::fillMapTo(int xIndex, int yIndex){
    APathFinder *instance = APathFinder::instance();
    if(instance != nullptr){
        FScopeLock Lock(&instance->delegate_CriticalSection_a); //new lock added from oath finder instance 
        while(map.size() <= xIndex) {
            map.push_back(std::vector<APathFinder::Chunk * >());
        }

        // Ensure all lists up to map.Count have enough chunks
        for (int i = 0; i < map.size(); i++) {
            while (map[i].size() <= yIndex) {
                map[i].push_back(new APathFinder::Chunk());
            }
        }
    }
}




void APathFinder::draw(FVector &pos){
    FVector up = pos + FVector(0, 0, 10000);
    DebugHelper::showLineBetween(GetWorld(), pos, up, FColor::Red, 5.0f);
}

//finds a node from a quadrant
APathFinder::Node* APathFinder::Quadrant::findNode(FVector pos){
    int x1 = std::abs(pos.X / CHUNKSIZE);
    int y1 = std::abs(pos.Y / CHUNKSIZE);

    if(map.size() > x1 && map.at(x1).size() > y1){
        if(map.at(x1).at(y1) != nullptr){
            APathFinder::Node* n = map.at(x1).at(y1)->findNode(pos);
            if(n != nullptr){
                //DebugHelper::showScreenMessage("quadrant found node return!", FColor::Green);
                return n;
            }
        }
    }
    //DebugHelper::showScreenMessage("quadrant not found node return!", FColor::Red);
    return nullptr;
}



APathFinder::Node *APathFinder::Quadrant::findNodeInDirection(FVector &pos, FVector &dir){
    int x1 = std::abs(pos.X / CHUNKSIZE);
    int y1 = std::abs(pos.Y / CHUNKSIZE);

    if(map.size() > x1 && map.at(x1).size() > y1){
        if(map.at(x1).at(y1) != nullptr){
            APathFinder::Node* n = map.at(x1).at(y1)->findNodeInDirection(pos, dir);
            if(n != nullptr){
                //DebugHelper::showScreenMessage("quadrant found node return!", FColor::Green);
                return n;
            }
        }
    }
    //DebugHelper::showScreenMessage("quadrant not found node return!", FColor::Red);
    return nullptr;
}




/// @brief returns all lists from all chunks enclosed by an area
/// @param xA x value of first pos
/// @param yA z value of first pos
/// @param xB x value of second pos
/// @param zB z value of second pos
/// @return 
std::vector<APathFinder::Node*> APathFinder::Quadrant::nodesEnClosedBy(
    float xA, float yA, float xB, float yB
){
    std::vector<APathFinder::Node *> nodes;

    //abs for flipping neg values for the quadrants
    int x1 = std::abs(xA / CHUNKSIZE); //implicit conversion is allowed
    int y1 = std::abs(yA / CHUNKSIZE);
    int x2 = std::abs(xB / CHUNKSIZE); //implicit conversion is allowed
    int y2 = std::abs(yB / CHUNKSIZE);

    int fromX = std::min(x1, x2);
    int toX = std::max(x1, x2);
    //toX = std::min(tg btoX, map.size());
    

    int fromY = std::min(y1, y2);
    int toY = std::max(y1, y2);
    //toY = std::min(toY, map.at(0).size());

    for(int i = fromX; i <= toX; i++){
        for(int j = fromY; j <= toY; j++){
            if(map.size() > i && map.at(i).size() > j){ //hier mit punkt weil call by value
                std::vector<APathFinder::Node *> &read = map.at(i).at(j)->getNodes();
                if(read.size() > 0){
                    nodes.insert(nodes.end(), read.begin(), read.end());
                }

                FString string = FString::Printf(TEXT("read enclosed num size %d"), read.size());
                if(GEngine){
                    //GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, string);
                }
            }
        }
    }

    return nodes;
}



std::vector<APathFinder::Node*> APathFinder::Quadrant::askForArea(FVector a, FVector b){
    
    float lowerX = 0;
    float lowerY = 0;
    float higherX = 0;
    float higherY = 0;
    lowerX = std::min(a.X, b.X); //-CHUNKSIZE; //+ extension
    lowerY = std::min(a.Y, b.Y); //- CHUNKSIZE;
    higherX = std::max(a.X, b.X); //+ CHUNKSIZE;
    higherY = std::max(a.Y, b.Y); //+ CHUNKSIZE;

    bool extendBounds = false;
    if(extendBounds){
        lowerX += - CHUNKSIZE; //+ extension
        lowerY += - CHUNKSIZE;
        higherX += CHUNKSIZE;
        higherY += CHUNKSIZE;
    }

    float inf = std::numeric_limits<float>::infinity();

    if(xSample == 1 && ySample == 1){
        // Bottom-left quadrant
        lowerX = std::clamp(lowerX, 0.0f, inf); //(val, lowerclamp, higherclamp)
        lowerY = std::clamp(lowerY, 0.0f, inf);
        return nodesEnClosedBy(lowerX, lowerY, higherX, higherY);
    }

    if(xSample == -1 && ySample == -1){
        // Top-right quadrant
        higherX = std::clamp(higherX, -inf, 0.0f); 
        higherY = std::clamp(higherY, -inf, 0.0f);
        return nodesEnClosedBy(lowerX, lowerY, higherX, higherY);
    }

    if(xSample == -1 && ySample == 1){
        // Bottom-right quadrant
        higherX = std::clamp(higherX, -inf, 0.0f);
        lowerY = std::clamp(lowerY, 0.0f, inf);
        return nodesEnClosedBy(lowerX, lowerY, higherX, higherY);
    }

    if(xSample == 1 && ySample == -1){
        // Top-left quadrant
        lowerX = std::clamp(lowerX, 0.0f, inf);
        higherY = std::clamp(higherY, -inf, 0.0f);
        return nodesEnClosedBy(lowerX, lowerY, higherX, higherY);
    }

    std::vector<APathFinder::Node*> vec;
    // returns an empty list if none found
    return vec;
}











/**
 * 
 * 
 * ---- CHUNK METHODS -----
 * 
 * 
 */

/// @brief adds a new node to the vector with a position
/// @param vec position of the node to be added
void APathFinder::Chunk::add(FVector vec){
    //find closest node near by
    

    if(!hasNode(vec)){
        Node *node = new Node(vec);
        nodes.push_back(node);
        //connects to all nodes if enabled in header
        if(APathFinder *p = APathFinder::instance()){
            p->connect(node);
        }

        APathFinder::countNodes += 1;
    }
}

/// @brief adds a new node to the node with a position
/// @param vec position of the node to be added
void APathFinder::Chunk::add(Node *node){
    if(node != nullptr){

        addNoConnect(node);
        // connects to all nodes if enabled in header
        if(PREBUILD_EDGES_ENABLED){
            if(APathFinder *p = APathFinder::instance()){
                p->connect(node);
            }
        }
        
    }
}

void APathFinder::Chunk::addNoConnect(Node *node){
    if(node){
        //will only check for duplicate nodes by adress
        for (int i = 0; i < nodes.size(); i++){
            if(nodes.at(i) == node){
                return;
            }
        }
        nodes.push_back(node);
    }
}






/// @brief returns the chunk nodes as reference
/// @return vector<Node> nodes as &ref
std::vector<APathFinder::Node*> &APathFinder::Chunk::getNodes(){
    return nodes;
}

/// @brief tries to find a node from a chunk, if not found: created
/// @param pos position of the targetet node
/// @return returns the closest node near by
APathFinder::Node* APathFinder::Chunk::findNode(FVector pos){
    FVector dir(0.0f, 0.0f, 0.0f); //none
    return findNodeInDirection(pos, dir); //will be ignored
}

APathFinder::Node* APathFinder::Chunk::findNodeInDirection(FVector &pos, FVector &dir){
    dir.Z = 0.0f;
    dir = dir.GetSafeNormal();

    bool checkDirNone = dir.X == 0.0f && dir.Y == 0.0f;
    bool checkDir = !checkDirNone;

    //add node if didnt had any yet
    if(nodes.size() <= 0){
        DebugHelper::showScreenMessage("RETURNED ASYNC CONNECT NODE ", FColor::Yellow);
        return lateadd(pos);
    }

    //find the closest node 
    float closest = std::numeric_limits<float>::max();
    APathFinder::Node *closestNode = nullptr;
    float prevDotProduct = -2.0f; //worst is -1.0f

    for (int i = 0; i < nodes.size(); i++)
    {
        APathFinder::Node *current = nodes.at(i);
        if (current != nullptr)
        {
            if(current->hasAnyNeighbors()){ //no neighbors makes no sense.
                float Difference = FVector::Dist(pos, current->pos);

                if(Difference < closest){

                    //direction check
                    if(checkDir){
                        FVector dirFromBotLocation = current->pos - pos; // AB = B - A
                        dirFromBotLocation.Z = 0.0f;
                        dirFromBotLocation = dirFromBotLocation.GetSafeNormal();

                        //dot product similar: ok
                        float dotProduct = FVector::DotProduct(dir, dirFromBotLocation);
                        //wenn das skalarprodukt zweier normalisierter
                        //vektoren 1 ergibt sind sie paralell zu einander
                        if(dotProduct > prevDotProduct){
                            closest = Difference;
                            closestNode = current;
                            prevDotProduct = dotProduct;
                        }
                    }else{
                        closest = Difference;
                        closestNode = current;
                    }

                    
                }
            }
            
        }
    }

    //connect node or not.
    if(APathFinder::PREBUILD_EDGES_ENABLED && closestNode == nullptr){
        DebugHelper::showScreenMessage("ASYNC LATE CONNECT NODE ", FColor::Yellow);
        return lateadd(pos);
    }

    return closestNode;





}











/// @brief will add a new node to the chunk and connect the edges if the prebuild mode is enabled / forced
/// @param pos position to create the new node at
/// @return created node, may not be fully connected yet if operation is async.
APathFinder::Node* APathFinder::Chunk::lateadd(FVector pos){
    APathFinder::Node *s = new APathFinder::Node(pos);
    nodes.push_back(s);
    if(APathFinder::PREBUILD_EDGES_ENABLED){
        APathFinder *p = APathFinder::instance();
        if(s != nullptr && p != nullptr){
            p->connect(s);
        }
    }
    return s;
}





/// @brief tries to find a node from a chunk as BOOL
/// @param pos position of the targetet node
/// @return returns the closest node near by
bool APathFinder::Chunk::hasNode(FVector pos){
    if(nodes.size() <= 0){
        return false;
    }

    float closest = ONE_METER * 2.0f;
    APathFinder::Node *closestNode = nodes.at(0);

    for (int i = 0; i < nodes.size(); i++)
    {
        APathFinder::Node *current = nodes.at(i);
        if (current != nullptr)
        {

            float Difference = FVector::Dist(pos, current->pos);

            if(Difference < closest){
                closest = Difference;
                closestNode = current;
            }
        }
    }
    if(closest <= (ONE_METER / 3.0f)){
        return true;
    }
    return false;
}




/**
 * 
 * --- NODE METHODS ---
 * 
 */
void APathFinder::Node::setId(int inId){
    id = inId;
}

int APathFinder::Node::getId(){
    return id;
}

TArray<int> APathFinder::Node::NeighborsById(){
    TArray<int> outIds;
    for (int i = 0; i < visible_tangential_Neighbors.size(); i++){
        APathFinder::Node *current = visible_tangential_Neighbors[i];
        if(current){
            int currentId = current->getId();
            if(APathFinder::IdIsValid(currentId)){
                outIds.Add(currentId);
            }
        }
    }
    return outIds;
}

int APathFinder::Node::IdConvexNeighborA(){
    if(nA){
        return nA->getId();
    }
    return -1;
}

int APathFinder::Node::IdConvexNeighborB(){
    if(nB){
        return nB->getId();
    }
    return -1;
}




/// @brief resets the nodes: gx, fx, camefrom neighbor and the closed flag
void APathFinder::Node::reset(){
    camefrom = nullptr;
    gx = std::numeric_limits<float>::max(); //is set to max for unknown status
    fx = gx;
    closedFlag = false;
}

/// @brief will update the came from node and gx, and fx value for this node
/// @param gxIn 
/// @param hxEnd 
/// @param came 
void APathFinder::Node::updateCameFrom(float gxIn, float hxEnd, APathFinder::Node &came){
    this->camefrom = &came;

    oldfx = fx; //copy for no reason

    gx = gxIn;
    fx = gxIn + hxEnd;
}


void APathFinder::Node::close(){
    this->closedFlag = true;
}


bool APathFinder::Node::isClosed(){
    return closedFlag;
}

/// @brief returns if has convex hull neighbors
/// @return 
bool APathFinder::Node::hasNeighbors(){
    return nA != nullptr && nB != nullptr;
}

bool APathFinder::Node::hasAnyNeighbors(){
    bool hasN = hasNeighbors();
    return hasN || visible_tangential_Neighbors.size() > 0;
}

/// @brief will set the a neighbor and also add the other convex node to tangential neighbors
/// because IT IS CONVEX!
/// @param n node 
void APathFinder::Node::setConvexNeighborA(Node *n){
    if(n != nullptr){
        nA = n;
        addTangentialNeighbor(n);
    }
}
/// @brief will set the a neighbor and also add the other convex node to tangential neighbors
/// because IT IS CONVEX!
/// @param n node 
void APathFinder::Node::setConvexNeighborB(Node *n){
    if(n != nullptr){
        nB = n;
        addTangentialNeighbor(n);
    }
}
/// @brief adds a node to the tangential connected neighbors, will allow duplicate add. Node is closed means
/// its closed. No need for searching in O(n) when adding. 
/// will add thread safely
/// @param n must not be nullptr
void APathFinder::Node::addTangentialNeighbor(Node *n){
    if(n != nullptr){
        if(n->getId() == getId()){
            return;
        }

        FScopeLock Lock(&CriticalSection); //lock because of async raycasting

        for (int i = 0; i < visible_tangential_Neighbors.size(); i++){
            Node *current = visible_tangential_Neighbors[i];
            if(current){
                if(current->getId() == n->getId()){
                    return;
                }
            }
        }

        visible_tangential_Neighbors.push_back(n);
    }
}




/**
 * 
 * 
 * CONNECT SECTION
 * 
 * 
 */



/// @brief connects a node in all quadrants IF ENABLED BOOL IN HEADER FILE
///
/// --- ISSUE I AM AWARE OF : ---
/// if 2 nodes are compared to each other which are part of 1 convex hull the condition
/// of tangentiality wont be ever true. 
/// since it still brings some computanional overhead
/// the plottet nodes will maybe tracked in polygonal shape-objects some time in future.
///
/// it is unclear for me whether i will implement that because searching for the node
/// in a list brings with O(n) rougly the same overhead as checking the tangential edges for
/// all points, which is also already greatly reduced by limiting a max distance and 
/// the subgraph functionality.
/// @param node node to connect
void APathFinder::connect(Node *node){
    if(node != nullptr && APathFinder::PREBUILD_EDGES_ENABLED){

        //find min max x and y for distance
    
        int lowerX = node->pos.X - PREBUILD_MAXDISTANCE;
        int lowerY = node->pos.Y - PREBUILD_MAXDISTANCE;
        int higherX = node->pos.X + PREBUILD_MAXDISTANCE;
        int higherY = node->pos.Y + PREBUILD_MAXDISTANCE;

        FVector a(lowerX, lowerY, 0);
        FVector b(higherX, higherY, 0);

        std::vector<Node *> enclosedByMaxDistance = getSubGraph(a, b);



        for (int i = 0; i < enclosedByMaxDistance.size(); i++){
            Node *compare = enclosedByMaxDistance.at(i);
            if(compare != nullptr && compare != node){
                // includes tangential check if possible!
                if(APathFinder *p = APathFinder::instance()){

                    

                    if(traceMode == APathFinder::PathTraceMode::AsyncTrace){
                        asyncCanSee(node, enclosedByMaxDistance.at(i));
                    }
                    if(traceMode == APathFinder::PathTraceMode::SyncTraceByTick){
                        addNewRaytask(node, enclosedByMaxDistance.at(i));
                    }
                    if(traceMode == APathFinder::PathTraceMode::SyncTrace){
                        if (p->canSeeTangential(node, enclosedByMaxDistance.at(i))) 
                        {
                            node->addTangentialNeighbor(compare);
                            compare->addTangentialNeighbor(node);
                        }
                    }



                    
                }
            }
            
        }



    }
}

/// @brief connects two nodes asynchronosly if those are tangential
/// @param a node a
/// @param b node b
/// both nodes must not be nullptr,
/// dont delete the nodes, they are passed into the lambda!
void APathFinder::asyncCanSee(Node *a, Node *b){
    if(a != nullptr && b != nullptr){

        //needs still to pass the tangential check. Remember: if no convex hull, automaically passes the test
        if(!passTangentailCheck(a,b)){
            return;
        }

        if(UWorld *worldPointer = GetWorld()){

            //async cast if prebuild
            if(PREBUILD_EDGES_ENABLED){

                FHitResult HitResult;
                FCollisionQueryParams Params = collsionParamsLowDetailAndFast();

                //add params from entity manager (contains all bots for example, which can be ignored)
                //part of a bigger context im working on, comment out or provide your own params
                /*if(EntityManager *e = worldLevel::entityManager()){
                    Params = e->getIgnoredRaycastParams();
                }*/
                Params = collisionIgnoreParams;
                Params.bTraceComplex = false;

                //bool result = false;
                FVector start = a->pos;
                FVector end = b->pos;

                /**
                 * EAsyncTraceType::Single: Use this for a single line trace (just one raycast).
                 * EAsyncTraceType::Multi: Use this if you want to collect multiple hits along the ray
                 */
                

                FTraceDelegate *MyTraceDelegate = requestDelegate(a, b);
                if(MyTraceDelegate != nullptr){
                    // Now pass the delegate by reference
                    worldPointer->AsyncLineTraceByChannel(
                        EAsyncTraceType::Single,    // Or Multi, depending on what you need
                        start,                      // Start point (FVector)
                        end,                        // End point (FVector)
                        ECC_Visibility,             // Collision channel
                        Params,            // Collision query parameters
                        FCollisionResponseParams(), // Response parameters
                        //&MyTraceDelegate //call back 
                        MyTraceDelegate //call back 
                    );
                }else{
                    DebugHelper::showScreenMessage("issue with delegate occured");
                }

                
            }
        }
    }
}



/// @brief request trace delegate to connect nodes a and b on 
/// @param a 
/// @param b 
/// @return 
FTraceDelegate *APathFinder::requestDelegate(Node *a, Node *b){

    if(a != nullptr && b != nullptr){
        FScopeLock Lock(&delegate_CriticalSection_a);

        FTraceDelegate *delegate = nullptr;
        if (released.size() > 0)
        {
            delegate = released.back();
            released.pop_back();
        }
        if(delegate == nullptr){
            delegate = new FTraceDelegate();
        }
        if(delegate != nullptr){
            delegate->BindLambda(
                [a, b, delegate]
                (const FTraceHandle &TraceHandle, FTraceDatum &TraceData){

                // Lambda logic for handling the trace result
                bool bHit = TraceData.OutHits.Num() > 0;

                if(bHit){
                    FHitResult hitP = TraceData.OutHits[0];
                    FVector hitPos = hitP.ImpactPoint;

                    //if distance close enough to target, forgive the hit.
                    if(FVector::Dist(hitPos, b->pos) <= 25){ //50cm
                        bHit = false; //testing needed
                        //DebugHelper::showScreenMessage("async trace false positive", FColor::Purple);
                    }

                    
                }

                //no hit, can see.
                if(!bHit){
                    a->addTangentialNeighbor(b);
                    b->addTangentialNeighbor(a);

                    //DebugHelper::showScreenMessage("async trace made new", FColor::Yellow);
                    /*
                    if(this->GetWorld() ){
                        DebugHelper::showLineBetween(
                            GetWorld(),
                            a->pos,
                            b->pos,
                            FColor::Blue,
                            1.0f
                        );
                    }*/
                }

                

                if(APathFinder *i = APathFinder::instance()){
                    i->freeDelegate(delegate);
                }        
            });
        }
        
        return delegate;
    }
    return nullptr;
}

/// @brief synchronously releases the ftrace delegate for re usal
/// @param d ftracedelegate to realease
void APathFinder::freeDelegate(FTraceDelegate *d){
    if(d != nullptr){

        d->Unbind();
        FScopeLock Lock(&delegate_CriticalSection_b); //other lock so the locks dont block each other
        released.push_back(d);
    }
}







/// @brief custom path finding method for graph with prebuild edges
/// @param start start node
/// @param end end node
/// @return the path if the minimal one found
std::vector<FVector> APathFinder::findPath_prebuildEdges(
    Node *start,
	Node *end
){
    if(start == nullptr || end == nullptr){
        std::vector<FVector> o;
        return o;
    }
    //DebugHelper::showScreenMessage("ask path");
    start->camefrom = nullptr;
    start->closedFlag = false;
    start->gx = 0;
    start->fx = 0;
    end->reset();
    end->camefrom = nullptr;
    end->closedFlag = false;


    //bounding box for traversed nodes
    /*
    FVector center = (start->pos + end->pos) / 2;
    int lowerX = std::min(start->pos.X, end->pos.X);
    int lowerY = std::min(start->pos.Y, end->pos.Y);
    int higherX = std::max(start->pos.X, end->pos.X);
    int higherY = std::max(start->pos.Y, end->pos.Y);

    FVector lower(lowerX, lowerY, 0);
    FVector higher(higherX, higherY, 0);
    */

    //int boundingBoxIncreaseFrac = 2;
    //lower += (center - lower) * boundingBoxIncreaseFrac; // AB = B - A
    //higher += (center - higher) * boundingBoxIncreaseFrac; // AB = B - A



    std::vector<Node *> markedForCleanUp; //must be cleaned before returning path
    priorityQueue open;
    open.add(start);

    while(open.hasNodes()){
        APathFinder::Node *current = open.popLowestFx();
        

        if (current != nullptr)
        {
            //debugDraw
            APathFinder::Node *prevNode = current->camefrom;
            if(prevNode != nullptr && false){
                DebugHelper::showLineBetween(
                    GetWorld(),
                    current->pos,
                    prevNode->pos,
                    FColor::Cyan,
                    5.0f
                );
            }

        
            markedForCleanUp.push_back(current); // dont forget to push back for cleaning up later
            
            if (current == end){

                std::vector<FVector> outputPath = constructPath(end);
                
                //draw
                for (int i = 1; i < outputPath.size(); i++){
                    FVector nodeA = outputPath.at(i);
                    FVector nodeB = outputPath.at(i-1);
                    
                    if(false){
                        DebugHelper::showLineBetween(
                            GetWorld(),
                            nodeA,
                            nodeB,
                            FColor::Red,
                            5.0f
                        );
                    }
                    
                }

                // clean all status to prevent issues, everything must be cleared
                for (int i = 0; i < markedForCleanUp.size(); i++)
                {
                    Node *n = markedForCleanUp.at(i);
                    if (n != nullptr)
                    {
                        n->reset();
                    }
                }
                DebugHelper::showScreenMessage("Path Found", FColor::Purple);
                return outputPath;
            }

            //not opened yet
            current->close(); //close node

            //DebugHelper::showScreenMessage("neighbors", current->visible_tangential_Neighbors.size(), FColor::Red);
            
            //traverse NOT opened neighbors and add to open if not added yet (will be handeld by queue automatically)
            for (int i = 0; i < current->visible_tangential_Neighbors.size(); i++){
                Node *neighbor = current->visible_tangential_Neighbors[i];
                if(neighbor != nullptr){
                    if(!neighbor->isClosed()){ //open only if not closed

                        float gxNew = current->gx + distance(current->pos, neighbor->pos);
                        //DebugHelper::showScreenMessage("try gx ", gxNew, FColor::Purple);
                        //DebugHelper::showScreenMessage("compare to gx ", neighbor->gx, FColor::Purple);
                        if(gxNew < neighbor->gx){
                            //DebugHelper::showScreenMessage("update gx ", gxNew, FColor::Green);
                            float hxEnd = distance(neighbor->pos, end->pos);
                            neighbor->updateCameFrom(gxNew, hxEnd, *current);
                        }
                        //ADD TO OPEN LIST!! //if readded is bubbled up automatically!
                        open.add(neighbor);

                        //even if a node wasnt the lowest it must be cleaned later!
                        markedForCleanUp.push_back(neighbor);
                    }else{
                        //DebugHelper::showScreenMessage("is closed", FColor::Purple);
                    }
                }
            }
        }else{
            //issue
            break;
        }
    }

    //no path found, make sure to reset all nodes flag status
    for (int i = 0; i < markedForCleanUp.size(); i++){
        Node *n = markedForCleanUp.at(i);
        if(n != nullptr){
            n->reset();
        }
    }

    DebugHelper::showScreenMessage("no path found");


    std::vector<FVector> placeholder;
    return placeholder;
}


/// @brief checks if a node is within the bounding box enclosed by a and b (ordering is not important)
/// @param a pos a
/// @param b pos b
/// @param check node to check, must not be nullptr
/// @return within box or not. If no node provided, default is false
bool APathFinder::isInBounds(FVector &a, FVector &b, APathFinder::Node *check){
    if(check != nullptr){
        FVector c = check->pos;
        return (a.X <= c.X && a.Y <= c.Y && c.X <= b.X && c.Y <= b.Y);
    }
    return false;
}

/**
 * 
 * --- TANGENTIAL CHECKING ---
 * 
 */


/// @brief needs to pass the tangential check before being a node of interest
/// requires both passed nodes to have their very next neighbors and both neighbors being
/// part of the convex hull of the polygon the nodes / edges were collected from
/// ---> if a or b dont have neighbors (not part of a convex hull, true is automatically returned, test passed.)
/// @param a node a
/// @param b node b
/// @return is tangential: no intersection of polygons or not
bool APathFinder::passTangentailCheck(Node *a, Node *b){
    if(a != nullptr && b != nullptr)
    {
        //must be tangential no intersect on both sides to be an connection of intersect
        if(a->hasNeighbors() && b->hasNeighbors())
        {
            bool dirAB_ok = false;
            FVector v0 = b->pos - a->pos;
            FVector vN(v0.Y, -v0.X, 0); // 90 grad drehen: - für eine var, xy tauschen, einen component negieren

            FVector v1 = b->nA->pos - b->pos; // AB = B - A connect to first neighbor
            FVector v2 = b->nB->pos - b->pos; //connect to second neighbor

            //(vN dot v1) * (vN dot v2) > 0 damit beide vektoren auf der selben seite sind. tangentiale konvexe verbindung
            float dotProduct1 = (vN.X * v1.X) + (vN.Y * v1.Y);
            float dotProduct2 = (vN.X * v2.X) + (vN.Y * v2.Y);
            if(dotProduct1 * dotProduct2 >= 0) // >=
            {//vorzeichen gleich, >0 selbe seite
                dirAB_ok = true;
            }


            bool dirBA_ok = false;
            v0 = a->pos - b->pos;

            v1 = (a->nA->pos - a->pos); // AB = B - A connect to first neighbor
            v2 = (a->nB->pos - a->pos); //connect to second neighbor

            //(vN dot v1) * (vN dot v2) > 0 damit beide vektoren auf der selben seite sind. tangentiale konvexe verbindung
            dotProduct1 = (vN.X * v1.X) + (vN.Y * v1.Y);
            dotProduct2 = (vN.X * v2.X) + (vN.Y * v2.Y);
            if(dotProduct1 * dotProduct2 >= 0){//vorzeichen gleich, >0 selbe seite
                dirBA_ok = true;
            }



            
            if(false && dirAB_ok && dirBA_ok){
                
                DebugHelper::showLineBetween(GetWorld(), a->pos, b->pos, FColor::Green, 100.0f);
                
            }

            return dirAB_ok && dirBA_ok;
        }

        if(false){
            DebugHelper::showLineBetween(GetWorld(), a->pos, b->pos, FColor::Yellow, 100.0f);
        }
        return true; //if is not part of a convexx hull, true by default
    }
    
    return false; //issue, not valid nodes passed!
}


/// @brief collision params with trace complex false: Faster
/// @return 
FCollisionQueryParams APathFinder::collsionParamsLowDetailAndFast(){
    FCollisionQueryParams params;
    params.bTraceComplex = false;
    return params;
};

// ------ bot helper -------
FVector APathFinder::findFurthestConnectedNodeFrom(FVector &other){
    Node *foundTargetNode = findNode(other);
    if(foundTargetNode != nullptr){
        std::vector<Node *> &neighborRef = foundTargetNode->visible_tangential_Neighbors;
        if (neighborRef.size() > 0)
        {
            Node *farthest = neighborRef[0];
            FVector farthestPos;
            float dist = 99999999;
            if(farthest != nullptr){
                dist = FVector::Dist(farthest->pos, foundTargetNode->pos);
                farthestPos = farthest->pos;
            }
            for (int i = 1; i < neighborRef.size(); i++){
                Node *current = neighborRef[i];
                if(current != nullptr){
                    float newdist = FVector::Dist(current->pos, farthestPos);
                    if(newdist > dist){
                        dist = newdist;
                        farthestPos = current->pos;
                    }
                }
            }

            
            DebugHelper::showLineBetween(
                GetWorld(),
                farthestPos,
                foundTargetNode->pos,
                FColor::Cyan,
                2.0f
            );
            

            return farthestPos;
        }
    }
    return other;
}

/**
 * 
 * 
 * ----- TICKED SYNCHORNONIZED CONNECTOR, WORKS WORSE THAN ASYNC TRACE, DO NOT USE ------
 * 
 * 
 */


void APathFinder::addNewRaytask(Node *a, Node *b){
    
    if(a != nullptr && b != nullptr){
        raycastTask newTask;
        newTask.setup(GetWorld(), a, b);
        rayTasksVec.push_back(newTask);
    }
}





//debug
void APathFinder::debugShowAllNodes(UWorld *world){
    int count = 0;
    if (TopRight)
    {
        TopRight->debugShowAllNodes(world);
        count += TopRight->chunkCount();
    }
    if(BottomRight){
        BottomRight->debugShowAllNodes(world);
        count += TopRight->chunkCount();
    }
    if(TopLeft){
        TopLeft->debugShowAllNodes(world);
        count += TopRight->chunkCount();
    }
    if(BottomLeft){
        BottomLeft->debugShowAllNodes(world);
        count += TopRight->chunkCount();
    }

    DebugHelper::logMessage("debugPathfinder chunk count", count);
}

void APathFinder::Quadrant::debugShowAllNodes(UWorld *world){
    for (int i = 0; i < map.size(); i++){
        std::vector<Chunk *> &current = map[i];
        for (int j = 0; j < current.size(); j++){
            Chunk *currentChunk = current[j];
            if(currentChunk){
                currentChunk->debugShowAllNodes(world);
            }
        }
    }
}

int APathFinder::Quadrant::chunkCount(){
    int count = 0;
    for (int i = 0; i < map.size(); i++)
    {
        std::vector<Chunk *> &current = map[i];
        for (int j = 0; j < current.size(); j++){
            Chunk *currentChunk = current[j];
            if(currentChunk){
                count++;
            }
        }
    }
    return count;
}

void APathFinder::Chunk::debugShowAllNodes(UWorld *world){
    for (int i = 0; i < nodes.size(); i++){
        APathFinder::Node *node = nodes[i];
        if(node){
            node->show(world);
        }
    }
}

void APathFinder::Node::show(UWorld *world){
    if(world){
        FVector posCopy = pos;
        FVector offset = posCopy + FVector(0, 0, 10000);
        DebugHelper::showLineBetween(world, posCopy, offset, FColor::Red, 100.0f);
    }
}




//ignore params
void APathFinder::addActorToIgnoreRaycastParams(AActor *actor){
    if(actor != nullptr){
        collisionIgnoreParams.AddIgnoredActor(actor);
    }
}

FCollisionQueryParams APathFinder::getIgnoredRaycastParams(){
    return collisionIgnoreParams;
}