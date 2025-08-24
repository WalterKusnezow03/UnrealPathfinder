// Fill out your copyright notice in the Description page of Project Settings.


#include "CoreMinimal.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "StaticMeshResources.h" 
#include "GameFramework/Actor.h"

#include "Engine/StaticMeshActor.h"
#include "MeshDescription.h"

#include "GameCore/DebugHelper.h"
#include "PathFinder.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h"
#include <cmath>
#include "EdgeCollector.h"

EdgeCollector::EdgeCollector()
{
    //readEdges = new std::vector<FVector>();
    //edgeDataEdges = new std::vector<edgeData>();
    worldIn = nullptr;
}

EdgeCollector::~EdgeCollector()
{
    /*
    if(readEdges != nullptr){
        delete (readEdges);
    }
    
    if(edgeDataEdges != nullptr){
        delete (edgeDataEdges);
    }*/
}


//inner class constructor
EdgeCollector::edgeData::edgeData(FVector bottomIn, FVector topIn){
    if(topIn.Z < bottomIn.Z){
        top = bottomIn;
        bottom = topIn;
    }else{
        top = topIn;
        bottom = bottomIn;
    }
}

//detsructor of inner class
EdgeCollector::edgeData::~edgeData(){
}





// --- methods ---



/// @brief finds all UActorComponents from a given actor
/// @tparam T UActorComponents or derived, specified by the passed list
/// @param a actor to get from
/// @param items container to save in, list to prevent useless copying of vector
template <typename T>
void EdgeCollector::findAllOfType(AActor &actor , std::list<T*> & items)
{
	//upper type bound must be set inside here if the class is not generic!
	static_assert(std::is_base_of<UActorComponent, T>::value, "must be UActorComponent component");

    //find other actors first and search inside
    TArray<UChildActorComponent *> actors;
    actor.template GetComponents<UChildActorComponent>(actors);
    if(actors.Num() > 0){
        for (int i = 0; i < actors.Num(); i++){
            AActor *a1 = actors[i]->GetChildActor(); //gets the aactor of an child actor
            if (a1)
            {
                findAllOfType<T>(*a1, items);
            }
        }
    }

    //then layer own
	TArray<T *> array;
	actor.template GetComponents<T>(array); //only provided by aactor
	if(array.Num() > 0){
		for (int i = 0; i < array.Num(); i++){
			items.push_back(array[i]);
		}
	}
}



//Function to process static meshes in the world
//returns a reference to the edges, copy needed because object will destroy them on destruct!

/// @brief will get all edges from the given world and add it to the nav mesh
/// @param World world to get from, must not be nullptr
void EdgeCollector::getAllEdges(UWorld* World)
{
    worldIn = World;
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid World context."));
        return;
        // return readEdges;
    }

    // Iterate over all actors in the world
    TArray<AActor*> AllActors;
    //alle actors in das allactors array
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (int32 i = 0; i < AllActors.Num(); ++i)
    {
        AActor* Actor = AllActors[i];
        if (Actor)
        {

            //new method gets all children
            std::list<UStaticMeshComponent*> list;
            findAllOfType<UStaticMeshComponent>(*Actor, list);
            for(UStaticMeshComponent *component : list){
                getEdgesFromSingleMeshComponent(component);
                //, edgeDataEdges);
            }


            //deprecated, is in plguin now
            /*
            //exclude enteties
            IDamageinterface *damageInterface = Cast<IDamageinterface>(Actor);
            if(damageInterface == nullptr){

            }*/
        }

    }

   
}


//collect adges for an single actor passed per pointer
//data will get append to array
void EdgeCollector::getEdgesForActor(AActor* actor){
    if(actor){
        
        //container to save in
        std::vector<edgeData> vectorEdges;

        //new method gets all children not just the first layer
        std::list<UStaticMeshComponent*> list;
        findAllOfType<UStaticMeshComponent>(*actor, list);
        for(UStaticMeshComponent *component : list){
            getEdgesFromSingleMeshComponent(component);//, vectorEdges);
        }

    
    }
}



// reads static mesh from static mesh component
void EdgeCollector::getEdgesFromSingleMeshComponent(
    UStaticMeshComponent* MeshComponent
    //,std::vector<edgeData> &vector
)
{
    if (MeshComponent && MeshComponent->GetStaticMesh())
    {
        //needs the transform to revert the world space
        const FTransform LocalToWorldTransform = MeshComponent->GetComponentTransform();

        getEdgesFromSingleMesh(
            MeshComponent->GetStaticMesh(), 
            MeshComponent->GetComponentLocation(),
            LocalToWorldTransform
            //,vector
        );
    }

    return;
}


// Function to get all vertical edges of a mesh as copy
void EdgeCollector::getEdgesFromSingleMesh(
    UStaticMesh* StaticMesh, 
    FVector debugPos, 
    FTransform LocalToWorldTransform
){
    if (!StaticMesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("StaticMesh is null."));
        return;
    }

    // Ensure we have the render data for the mesh
    const FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
    if (!RenderData) 
    {
        UE_LOG(LogTemp, Warning, TEXT("RenderData is null."));
        return;
    }

    //showPos(worldIn, debugPos);

    std::vector<edgeData> currentEdges = std::vector<edgeData>();

    // Iterate through all LODs (Levels of Detail)
    for (int32 LODIndex = 0; LODIndex < RenderData->LODResources.Num(); LODIndex++)
    {
        const FStaticMeshLODResources& LODResources = RenderData->LODResources[LODIndex];

        // Iterate through all the vertex buffers
        const FPositionVertexBuffer& PositionVertexBuffer = LODResources.VertexBuffers.PositionVertexBuffer;
        //const FStaticMeshVertexBuffer& StaticMeshVertexBuffer = LODResources.VertexBuffers.StaticMeshVertexBuffer;



        // Iterate through each vertex
        for (uint32 VertexIndex = 1; VertexIndex < PositionVertexBuffer.GetNumVertices(); VertexIndex++)
        {
            
            //matrix transformation durchführen um vector korrekt zu lesen
            FVector A = LocalToWorldTransform.TransformPosition(
                FVector(PositionVertexBuffer.VertexPosition(VertexIndex - 1))
            );

            FVector B = LocalToWorldTransform.TransformPosition(
                FVector(PositionVertexBuffer.VertexPosition(VertexIndex))
            );

            
            //Wenn vertikale kante: interessiert daran im sichtbarkeits navigations graphen
            if(isVertical(A, B)){

                bool extended = false;
                // check ob vorherige edge vertikal erweitert
                if(currentEdges.size() > 0){

                    extended = false;

                    edgeData &prev = currentEdges.back(); //reference the back

                    //close enough to prev edge, 10cm 
                    FVector lower = A.Z < B.Z ? A : B;
                    FVector higher = A.Z > B.Z ? A : B;
                    if (FVector::Dist(prev.top, lower) <= 10)
                    { 
                        prev.top = higher;
                        extended = true;
                    }
                }

                //if not extended vertically, just add
                if(!extended){
                    currentEdges.push_back(edgeData(A, B)); //constructor will sort autmatically top and bottom
                }
                
            }
        }
    }

    //VERY IMPORTANT
    ComputeConvexHull(currentEdges);

    //causes issues currently
    CleanUpParalellEdges(currentEdges); //convex hull needed! -- this is not nesecarry because the hull is convex!


    //caluclate raycast hits and apply to all edges aligning them properly
    scaleUpConvexHullShape(currentEdges);
    collectRaycasts(currentEdges, worldIn);
    showEdges(currentEdges, worldIn); //debug

    //feed to nav mesh
    std::vector<FVector> toPosVec;
    for (int i = 0; i < currentEdges.size(); i++){
        toPosVec.push_back(currentEdges.at(i).bottom);
    }
    if(APathFinder *f = APathFinder::instance(worldIn)){
        f->addConvexHull(toPosVec);
    }
    return;

}


/// @brief checks if an given egde (from A to B) is vertical
/// @param A start
/// @param B end
/// @return is nearly vertical compared to Z axis / up direction
bool EdgeCollector::isVertical(FVector A, FVector B){

    FVector connect = B - A; //AB = B - A
    if(A.Z > B.Z){
        connect = A - B;
    }

    FVector connectNormal = connect.GetSafeNormal(); //einheitsvektor / normalisieren 

    float skalarProduktUp = std::abs(connectNormal.Z); //Up component

    return skalarProduktUp > 0.8f; //edge of interest wenn er weit genug nach oben zeigt.
}




//will compute the convex hull and write into the passed array
void EdgeCollector::ComputeConvexHull(std::vector<edgeData> &points) { //passed by reference also kann ich reinkritzeln
    std::vector<edgeData> convexHull; //value, will be removed from stack

    if (points.size() < 3) {
        return;
    }

    // Sort points to calculate the lower hull
    //Z is up and ignored here.
    std::sort(points.begin(), points.end(), [](const edgeData& a, const edgeData& b) {
        if (a.top.X == b.top.X) {
            return a.top.Y < b.top.Y;
        }
        return a.top.X < b.top.X;
    });

    // Calculate the lower hull
    for (const edgeData& point : points) {
        while (convexHull.size() >= 2 && !IsClockwise(convexHull[convexHull.size() - 2], convexHull.back(), point)) {
            convexHull.pop_back();
        }
        convexHull.push_back(point);
    }

    // Calculate the upper hull
    int lowerHullCount = convexHull.size();
    for (int i = points.size() - 2; i >= 0; --i) {
        edgeData point = points[i];
        while (
            convexHull.size() > lowerHullCount && 
            !IsClockwise(convexHull[convexHull.size() - 2], convexHull.back(), point)
        ) {
            convexHull.pop_back();//den geprüften punkt poppen
        }
        convexHull.push_back(point); //neuen punkt pushen
    }

    // Remove the last point which might be duplicated
    if (!convexHull.empty()) {
        convexHull.pop_back();
    }


    //copy data to passed array
    int cSize = convexHull.size();

    points = convexHull;
    
    /*
    //Overwrite elements
    for (int i = 0; i < cSize; i++) {
        if (i < points.size()) {
            points[i] = convexHull[i];
        }
    }

    //Remove the excess elements
    if (cSize < points.size()) {
        points.erase(points.begin() + cSize, points.end());
    }*/

    return;
    //convexHull;
}

/// @brief checks if the point is clockwise rotated for the graham scan
/// @param a 
/// @param b 
/// @param c 
/// @return 
bool EdgeCollector::IsClockwise(const edgeData& a, const edgeData& b, const edgeData& c) {
    //return (b.top.X - a.top.X) * (c.top.Y - a.top.Y) - (b.top.Y - a.top.Y) * (c.top.Y - a.top.Y) < 0; //wrong
    return (b.top.X - a.top.X) * (c.top.Y - a.top.Y) - (b.top.Y - a.top.Y) * (c.top.X - a.top.X) < 0;
    // < 0 means, kolliniear vectors are kept and ignored.
}




//shows the edges for debugging
void EdgeCollector::showEdges(std::vector<edgeData> &edges, UWorld *world){
    for (int i = 0; i < edges.size(); i++){
        FVector Start = edges.at(i).top;
        FVector End = edges.at(i).bottom;
        DrawDebugLine(world, Start, End, FColor::Green, false, 10.0f, 0, 5.0f);
    }
}


void EdgeCollector::showPos(UWorld *world, FVector e){
    FVector End = e + FVector(0, 0, 50000);
    DrawDebugLine(world, e, End, FColor::Green, true, 10.0f, 100, 5.0f);
}

//draws a line from point a to b
void EdgeCollector::showLine(FVector e, FVector g){
    if(worldIn != nullptr){
        DrawDebugLine(worldIn, e, g, FColor::Yellow, true, 10.0f, 100, 5.0f);
    }
    
}







void EdgeCollector::scaleUpConvexHullShape(std::vector<edgeData> &edges){
    std::vector<FVector> apply;
    int pushAwaycm = 100;
    //SCALE UP SHAPE
    for (int i = 0; i < edges.size(); i++){
        
        //get edges (prev, current, next)
        edgeData *prev = &edges.at(i);
        if(i == 0){
            prev = &edges.back();
        }
        else
        {
            prev = &edges.at(i - 1);
        }

        edgeData *current = &edges.at(i);
        edgeData *next = &edges.at(i);
        if(i == edges.size() - 1){
            next = &edges.front();
        }
        else
        {
            next = &edges.at(i + 1);
        }

        //calculate connect, add up, invert * -1, normalize, scale
        FVector dirA = prev->top - current->top; // AB = B - A
        FVector dirB = next->top - current->top;
        dirA.Z = 0;
        dirB.Z = 0;
        FVector pushOutDirectionScaled = (dirA + dirB).GetSafeNormal() * -1 * pushAwaycm; //get half weight
        apply.push_back(pushOutDirectionScaled);
    }

    //override data after calculating to keep pushout consistent and not immidiatly maniupulating
    for (int i = 0; i < apply.size(); i++){
        if(i < edges.size()){
            FVector applied = apply.at(i);
            edges.at(i).top += applied;
            edges.at(i).bottom += applied;
        }
    }
}








/// @brief applies the raycast bottom position to the edges
/// @param edges 
/// @param world 
void EdgeCollector::collectRaycasts(std::vector<edgeData> &edges, UWorld *world){


    // create raycasts
    for (int i = 0; i < edges.size(); i++)
    {
        collectRaycast(edges.at(i), world);
    }
}

/// @brief gets the hitpoint of an edge in the world ground (nesecarry if objects are overlapping)
/// @param edge edge to check
/// @param world world to check in 
void EdgeCollector::collectRaycast(edgeData &edge, UWorld *world){
    if(world){
        FVector Start = edge.top;
        //Start.Z += 500;
        FVector End = edge.bottom;
        End.Z -= 200;

        FHitResult HitResult;
		FCollisionQueryParams Params;

        
		bool bHit = world->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);


		
		if (bHit)
		{
            
            //distance from top must be greater than bottom from hitpoint
            //to make the hit point valid
            //(if you stick your meshes into the ground thats not my issue.)
            //float completeDistance = FVector::Dist(edge.bottom, edge.top);
            //float hitDistanceFromTop = FVector::Dist(edge.top, HitResult.ImpactPoint);
            //float hitDistanceFromBottom = FVector::Dist(edge.bottom, HitResult.ImpactPoint);
            
            //refacturing for z distance
            float completeDistance = std::abs(edge.bottom.Z - edge.top.Z);
            float hitDistanceFromTop = std::abs(edge.top.Z - HitResult.ImpactPoint.Z);
            float hitDistanceFromBottom = std::abs(edge.bottom.Z - HitResult.ImpactPoint.Z);


            //prevent nodes in ground and too far up
            if(hitDistanceFromTop > hitDistanceFromBottom * 1.5f){

                FVector hitPos = HitResult.ImpactPoint;
                hitPos.Z += GROUND_OFFSET; //offset fix above ground
                //at any point use the hitpos
                edge.bottom = hitPos;


                //debug testing
                if(EdgeCollector::DEBUG_DRAW_EDGES){
                    DebugHelper::showLineBetween(world, edge.top, hitPos, FColor::Cyan);
                }
                
            }
        }
    }
    //return nullptr;
}










/// @brief keep in mind, the edges MUST be sorted to a convex hull before!
/// @param currentEdges to simplyfy
void EdgeCollector::CleanUpParalellEdges(std::vector<edgeData> &currentEdges){

    int size = currentEdges.size() - 1;
    int i = 1;
    while(i < size - 1){
        FVector &a = currentEdges.at(i - 1).top;
        FVector &b = currentEdges.at(i ).top;
        FVector &c = currentEdges.at(i + 1).top;

        if (xyExtension(a,b,c))
        {
            currentEdges.erase(currentEdges.begin() + i);
        }

        i++;
        size = currentEdges.size() - 1;
    }
}

bool EdgeCollector::xyExtension(FVector &a, FVector &b, FVector &c){
    FVector ab = (b - a).GetSafeNormal();
    FVector bc = (c - b).GetSafeNormal();
    //if ab and bc are almost paralell, they almost creating one line, making b redundant in a path over a b and c
    if(xyDotProduct(ab,bc) >= 0.9f){
        return true;
    }
    return false;
}
float EdgeCollector::xyDotProduct(FVector &A, FVector &B){
    return (A.X * B.X) + (A.Y * B.Y);
}






/// @brief is not tested
/// @param vector 
void EdgeCollector::clean(std::vector<edgeData> &vector){
    int a = 1;
    int size = vector.size();
    while (a > 0 && a < size)
    {
        if(checkZExtension(vector.at(a - 1), vector.at(a))){
            vector.erase(vector.begin() + a - 1);
            size = vector.size();
        }
        a++;
    }
}
/// @brief is also not tested
/// @param p 
/// @param update 
/// @return 
bool EdgeCollector::checkZExtension(edgeData &p, edgeData &update){
    
    FVector lower = p.bottom.Z < update.bottom.Z ? p.bottom : update.bottom;
    FVector higher = p.top.Z > update.top.Z ? p.top : update.top;

    FVector xyConnect = lower - higher;
    xyConnect.Z = 0;
    if(FVector::Dist(FVector(0,0,0), xyConnect) <= 100){
        if(isVertical(lower, higher)){
            update.bottom = lower;
            update.top = higher;
            return true;
        }
    }
    return false;
}
