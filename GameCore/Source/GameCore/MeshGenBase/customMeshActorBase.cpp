// Fill out your copyright notice in the Description page of Project Settings.
#include "GameCore/MeshGenBase/customMeshActorBase.h"

#include "CoreMinimal.h"
#include "GameCore/util/FVectorTouple.h"
#include "GameCore/util/AActorUtil.h"
#include "Components/BoxComponent.h"
#include "KismetProceduralMeshLibrary.h"
#include "ELod.h"
#include "GameCore/MeshGenBase/lodHelper/LodCheckContainer.h"
#include "GameCore/MeshGenBase/foliage/ETerrainType.h"
#include "GameCore/util/FVectorUtil.h"
#include "GameCore/PlayerInfo/PlayerInfo.h"
#include "AssetPlugin/gameStart/assetManager.h"
#include "GameCore/MeshGenBase/materialHelper/MaterialEnumHelper.h"

#include "GameCore/MeshGenBase/lodHelper/LodConstants.h"

// Sets default values
AcustomMeshActorBase::AcustomMeshActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

    rootScene = CreateDefaultSubobject<USceneComponent>(TEXT("USceneRoot"));
    RootComponent = rootScene;

    // Create the ProceduralMeshComponent
    Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
    Mesh->SetupAttachment(RootComponent);
    //RootComponent = Mesh; //depreacted


    MeshNoRaycast = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("noRaycastMesh"));
    MeshNoRaycast->SetCollisionEnabled(ECollisionEnabled::NoCollision);  // Kein Raycast für diesen Layer

    // Attach it to the RootComponent (Mesh) so it has the same transform
    MeshNoRaycast->SetupAttachment(RootComponent);

    currentLodLevel = ELod::lodNear;

    /*
    bool asyncCook = true;
    if(asyncCook){
        // Asynchrones Cooking aktivieren
        Mesh->bUseAsyncCooking = true;
        MeshNoRaycast->bUseAsyncCooking = true;

    }*/
}


// Called when the game starts or when spawned
void AcustomMeshActorBase::BeginPlay()
{
	Super::BeginPlay();
    initLodMeshesOnBeginPlay();
    switchToLodOnBeginPlayOrUpdateMesh();
}

void AcustomMeshActorBase::switchToLodOnBeginPlayOrUpdateMesh(){
    switchToLod(ELod::lodNear); //set lod on start to hide other components right away and later update!
}



void AcustomMeshActorBase::disableDistanceListening(){
    distanceListeningBlocked = true;
}


void AcustomMeshActorBase::enableLodListening(){
    LISTEN_FOR_LOD_PLAYER = true;
}

void AcustomMeshActorBase::disableLodListening(){
    LISTEN_FOR_LOD_PLAYER = false;
}




// Called every frame
void AcustomMeshActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
    TickShaderRunningTime(DeltaTime);
    changeLodBasedOnPlayerPosition();



    //debug
    if(false){
        if(currentLodLevel == ELod::lodNear){
            

            FVector locationOfPlayer = PlayerInfo::playerLocation() + FVector(0,0,50);
            FVector thisLocation = GetActorLocation() + FVector(0,0,50);
            DebugHelper::showLineBetween(GetWorld(), locationOfPlayer, thisLocation, FColor::Red, DeltaTime * 3.0f);
        }
    }
    
}

void AcustomMeshActorBase::disablePhysicscollision(){
    if(Mesh){
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}





/// @brief process a 2D map of local coordinates
/// correct position of the chunk must be set before!
/// @param map 2D vector of LOCAL coordinates!
void AcustomMeshActorBase::createTerrainFrom2DMap(
    std::vector<std::vector<FVector>> &map,
    ETerrainType typeIn
){
    thisTerrainType = typeIn;

    int distanceBetweenNodesMin = 300;
    
    
    FVector originVec(0, 0, 0);

    std::vector<ELod> lods = LodConstants::lodVector();
    int prevLodStep = 1; //x++ y++ default as expected
    for (int lodStep = 0; lodStep < lods.size(); lodStep++)
    {
        ELod lodNow = lods[lodStep];


        materialEnum groundMaterial = MaterialEnumHelper::groundMaterialFor(typeIn);
        MeshData &grassLayer = findMeshDataReference(groundMaterial, lodNow, true);
        MeshData &stoneLayer = findMeshDataReference(materialEnum::stoneMaterial, lodNow, true);

        appendLodTerrain(
            map,
            grassLayer,
            stoneLayer,
            prevLodStep // index increase
        );
        grassLayer.calculateNormals();
        stoneLayer.calculateNormals();

        //go to next lod and clamp if needed
        prevLodStep *= 2;
        if(prevLodStep >= map.size()){
            prevLodStep = map.size() - 1;
        }
    }



    ReloadMeshAndApplyAllMaterials();

    enableLodListening();
}

void AcustomMeshActorBase::appendLodTerrain(
    std::vector<std::vector<FVector>> &map,
    MeshData &grassLayer,
    MeshData &stoneLayer,
    int stepSize
){
    int distanceBetweenNodesMin = 300;
    FVector originVec(0, 0, 0);

    int xstep = stepSize;
    for (int x = 0; x < map.size(); x += stepSize)
    { // -1 for inbounce connect to next

        while (x + xstep >= map.size())
        {
            xstep--;
            if(xstep <= 0){
                return;
            }
        }

        int ystep = stepSize;



        for (int y = 0; y < map.at(x).size(); y += ystep)
        {
            bool quit = false;
            while (y + ystep >= map.size())
            {
                ystep--;
                if(ystep <= 0){
                    quit = true;
                    break;
                }
            }
            if(quit){
                break;
            }
            // 
            //    1--2
            //    |  |
            //    0<-3
            //

            if (x + xstep < map.size() && y + ystep < map.at(x + xstep).size())
            {
                
                // get the vertecies
                FVector vzero = map.at(x).at(y);
                FVector vone = map.at(x).at(y + ystep);
                FVector vtwo = map.at(x + xstep).at(y + ystep);
                FVector vthree = map.at(x + xstep).at(y);

                // add to standard output
                // buildQuad(vzero, vone, vtwo, vthree, output, newtriangles);

                FVector normal = FVectorUtil::calculateNormal(vzero, vone, vtwo); // direction obviously
                if (FVectorUtil::directionIsVertical(normal))
                {
                    grassLayer.appendEfficent(vzero, vone, vtwo, vthree);
                }
                else
                {
                    stoneLayer.appendEfficent(vzero, vone, vtwo, vthree);
                }


                    
            }
        }
    }
}









void AcustomMeshActorBase::filterTouplesForVerticalVectors(
    TArray<FVectorTouple> &touples,
    std::vector<FVector> &potentialLocations
){
    // iterate over touples
    // determine normal angle and apply foliage, rocks, trees accordingly
    if (touples.Num() < 1){
        return;
    }

    //if normal faces towards up: flat area, create something
    for(FVectorTouple &t : touples){
        FVector &location = t.first();
        FVector &normal = t.second();
        bool facingUpwards = FVectorUtil::directionIsVertical(normal);
        if(facingUpwards){
            potentialLocations.push_back(location); 
        }
    }
}


/**
 * 
 * 
 * --- base damage api ---
 * 
 * 
 */
/// @brief will allow custom mesh actors such as destructables and terrain react to damage
/// @param d 
void AcustomMeshActorBase::takedamage(int d, bool surpressed){
    //damage owner as this could be a kimb of an actor
    if(damagedOwner != nullptr){
        DebugHelper::showScreenMessage("AcustomMeshActorBase Damage Owner!");
        damagedOwner->takedamage(d);
    }
}

void AcustomMeshActorBase::takedamage(int d){
    takedamage(d, false);
}

/// @brief allows tha ctor to react to damage from a origin
/// @param d 
/// @param hitpoint hitpoint from weapon  
void AcustomMeshActorBase::takedamage(int d, FVector &hitpoint){
    takedamage(d, hitpoint, false);
}

void AcustomMeshActorBase::takedamage(int d, FVector &hitpoint, bool surpressed){
    takedamage(d, surpressed);
}


void AcustomMeshActorBase::setTeam(teamEnum t){
    this->team = t;
}
teamEnum AcustomMeshActorBase::getTeam(){
    return team;
}

void AcustomMeshActorBase::setDamagedOwner(IDamageinterface *damagedOwnerIn){
    damagedOwner = damagedOwnerIn;
}

/**
 * 
 * 
 * ------ NEW LOD LEVEL METHODS ------
 * 
 * 
 */

/// @brief the mesh data reference will by near lod if not specified
/// @param type 
/// @param raycastOnLayer 
/// @return 
MeshData &AcustomMeshActorBase::findMeshDataReference(
    materialEnum type,
    bool raycastOnLayer
){
    
    return findMeshDataReference(
        type,
        ELod::lodNear,
        raycastOnLayer
    );
}

MeshData &AcustomMeshActorBase::findMeshDataReference(
    materialEnum type,
    ELod lodLevel,
    bool raycastOnLayer
){
    if(meshLodContainers.find(lodLevel) == meshLodContainers.end()){
        meshLodContainers[lodLevel] = ProceduralMeshComponentPair(); //should not happen at all!
    }
    return meshLodContainers[lodLevel].meshDataReference(type, raycastOnLayer);
}







///@brief will append the meshdata AND RELOAD THE MESH
void AcustomMeshActorBase::appendMeshDataAndReload(
    MeshData &meshdata,
    materialEnum type,
    ELod lodLevel,
    bool raycastOnLayer
){
    MeshData &found = findMeshDataReference(type, lodLevel, raycastOnLayer);
    found.append(meshdata);
    ReloadMeshForMaterialByLod(lodLevel, type);
    // ReloadMeshAndApplyAllMaterials();
}

void AcustomMeshActorBase::changeLodBasedOnPlayerPosition(){
    if(!LISTEN_FOR_LOD_PLAYER){
        return;
    }

    FVector locationOfPlayer = PlayerInfo::playerLocation();
    this->SetActorHiddenInGame(false);  // macht den Actor sichbat
    
    //set invisble
    //DebugHelper::showScreenMessage("lod test");
    ELod lod = lodLevelByDistanceTo(locationOfPlayer);
    if(lod != currentLodLevel){
        switchToLod(lod);
        DebugHelper::showScreenMessage("Tick Lod Update");
    }
    return;

}







/// @brief reloads all meshes, raycast, no raycast, all meshdata, all materials all lod submeshes!
/// call this method when replacing mesh data or changing the lod!
void AcustomMeshActorBase::ReloadMeshAndApplyAllMaterials(){
    std::vector<ELod> lodLevels = LodConstants::lodVector();
    std::vector<materialEnum> materials = MaterialEnumHelper::materialVector();
    for (int i = 0; i < lodLevels.size(); i++){
        ELod lod = lodLevels[i];

        if(meshLodContainers.find(lod) != meshLodContainers.end()){
            ProceduralMeshComponentPair &ref = meshLodContainers[lod];
            for (int j = 0; j < materials.size(); j++){
                ref.updateMeshAllAndApplyMaterial(materials[j]);
            }
        }
    }
    
}

///@brief reloads the mesh data for a single material, LOD NEAR ONLY
void AcustomMeshActorBase::ReloadMeshForMaterial(materialEnum material){
    ReloadMeshForMaterialByLod(ELod::lodNear, material); //default also for mesh data reference
}

/**
 * LOD REFACTURE SECTION
 */
void AcustomMeshActorBase::initLodMeshesOnBeginPlay(){

    //override lod near
    meshLodContainers[ELod::lodNear] = ProceduralMeshComponentPair();
    meshLodContainers[ELod::lodNear].overrideMeshPair(Mesh, MeshNoRaycast);

    // create others
    std::vector<ELod> lods = LodConstants::lodVector();
    for (int i = 0; i < lods.size(); i++){
        ELod iLod = lods[i];
        if(iLod != ELod::lodNear){
            meshLodContainers[iLod] = ProceduralMeshComponentPair();
            ProceduralMeshComponentPair &ref = meshLodContainers[iLod];
            ref.init(
                TEXT("ProceduralMeshActor"),
                i, // index
                this,
                RootComponent
            );
        }
    }

}




//new: meshes per lod
UProceduralMeshComponent *AcustomMeshActorBase::MeshForLod(ELod lod){
    if(meshLodContainers.find(lod) != meshLodContainers.end()){
        return meshLodContainers[lod].RaycastMesh();
    }
    return nullptr;
}
UProceduralMeshComponent *AcustomMeshActorBase::MeshNoRaycastForLod(ELod lod){
    if(meshLodContainers.find(lod) != meshLodContainers.end()){
        return meshLodContainers[lod].noRaycastMesh();
    }
    return nullptr;
}

/// @brief switch lod level, call on beginplay to init!
/// @param lod 
void AcustomMeshActorBase::switchToLod(ELod lod){
    currentLodLevel = lod;

    FString lodString;
    if(currentLodLevel == ELod::lodNear)
        lodString = TEXT("switch to lod near");
    if(currentLodLevel == ELod::lodMiddle)
        lodString = TEXT("switch to lod middle");
    if(currentLodLevel == ELod::lodFar)
        lodString = TEXT("switch to lod far");

    DebugHelper::showScreenMessage(lodString);

    for(auto &pair : meshLodContainers){
        ELod pairLod = pair.first;
        ProceduralMeshComponentPair &ref = pair.second;
        bool hide = pairLod != lod;
        ref.setHiddenInGame(hide);
    }

}

/// @brief updates mesh section completely and apply material
/// @param lod 
/// @param material 
void AcustomMeshActorBase::ReloadMeshForMaterialByLod(ELod lod, materialEnum material){


    if(meshLodContainers.find(lod) == meshLodContainers.end()){
        meshLodContainers[lod] = ProceduralMeshComponentPair();
    }
    meshLodContainers[lod].updateMeshAll(material);

}

/**
 * LOD REFACTURE END
 */
ELod AcustomMeshActorBase::GetCurrentLodLevel(){
    return currentLodLevel;
}








/// @brief replaces the mesh layer for an mesh component
/// caution: mesh section is recreated because modifying the triangle buffer is not allowed
/// when will to update an mesh!
/// @param meshcomponent 
/// @param otherMesh 
/// @param layer 
/// @param enableCollision 
void AcustomMeshActorBase::updateMesh(
    UProceduralMeshComponent &meshcomponent,
    MeshData &otherMesh, //MUST BE SAVED IN A VALUE CLASS SCOPE SOMEWHERE!
    int layer,
    bool enableCollision
){
    if(otherMesh.getVerteciesRef().Num() == 0){
        return;
    }

    /**
     * example: 
     * 
    Mesh->CreateMeshSection(
        layer, 
        newvertecies, 
        this->triangles, 
        normals, 
        UV0, 
        VertexColors, 
        Tangents, 
        true
    );*/
    //meshcomponent.ClearMeshSection(layer);
    meshcomponent.CreateMeshSection(
        layer, 
        otherMesh.getVerteciesRef(),//newvertecies, 
        otherMesh.getTrianglesRef(),//this->triangles, 
        otherMesh.getNormalsRef(),//normals, 
        otherMesh.getUV0Ref(),//UV0, 
        otherMesh.getVertexColorsRef(),//VertexColors, 
        otherMesh.getTangentsRef(),//Tangents, 
        true
    );

    
    //meshcomponent.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    meshcomponent.SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    meshcomponent.SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);

    if(!enableCollision){
        AActorUtil::enableColliderOnActor(*this, true);
        meshcomponent.SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }else{
        meshcomponent.SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    }
}


/**
 * 
 * --- helper functions for special meshes: rooms ---
 * 
 */

/// @brief adds mesh data to all lod layers (Use on custom mesh creation, for example rooms)
/// @param meshdata meshdata to add and COPY
/// @param type type of material for this layer, will be raycast enabled by default!
void AcustomMeshActorBase::replaceMeshData(MeshData &meshdata, materialEnum type){
    //all lod levels included
    std::vector<ELod> lodvector = LodConstants::lodVector();
    for (int i = 0; i < lodvector.size(); i++){
        replaceMeshData(meshdata, type, lodvector[i]);
    }
}

/// @brief replaces the mesh data for a lod level and a material (layer automatically found),
/// will be raycast enabled by default, not the non collsion mesh!! (Use on custom mesh creation, for example rooms)
/// @param meshdata 
/// @param type 
/// @param lodLevel 
void AcustomMeshActorBase::replaceMeshData(MeshData &meshdata, materialEnum type, ELod lodLevel){
    

    if(meshLodContainers.find(lodLevel) == meshLodContainers.end()){
        meshLodContainers[lodLevel] = ProceduralMeshComponentPair();
    }

    meshLodContainers[lodLevel].replaceMeshDataRaycastAndUpdate(
        meshdata,
        type
    );

}










/// @brief applys a material to the whole component (slot 0 by default)
/// @param ProceduralMeshComponent 
/// @param Material 
void AcustomMeshActorBase::ApplyMaterial(
    UProceduralMeshComponent* ProceduralMeshComponent, 
    UMaterialInterface* Material
) {
    ApplyMaterial(ProceduralMeshComponent, Material, 0);
}

/// @brief applys a material to the whole component at passed index slot
/// @param ProceduralMeshComponent mesh to apply for
/// @param Material material to set
/// @param layer layer to apply for / index
void AcustomMeshActorBase::ApplyMaterial(
    UProceduralMeshComponent* ProceduralMeshComponent, 
    UMaterialInterface* Material,
    int layer
) {
	if (ProceduralMeshComponent != nullptr && Material != nullptr) {
		// Apply the material to the first material slot (index 0) of the procedural mesh
		ProceduralMeshComponent->SetMaterial(layer, Material);

    }
}


/// @brief applies material by enum to the found layer on the passed mesh
/// @param ProceduralMeshComponent 
/// @param type 
void AcustomMeshActorBase::ApplyMaterial(
    UProceduralMeshComponent* ProceduralMeshComponent,
    materialEnum type
){
    int layer = AcustomMeshActorBase::layerByMaterialEnum(type);
    if (assetManager *e = assetManager::instance())
    {
        ApplyMaterial(ProceduralMeshComponent, e->findMaterial(type), layer);
    }
}





/// @brief will automatically apply the layer! but only lod near!
/// @param type material enum type to refresh
void AcustomMeshActorBase::ApplyMaterial(
    materialEnum type
){
    if(meshLodContainers.find(ELod::lodNear) != meshLodContainers.end()){
        meshLodContainers[ELod::lodNear].ApplyMaterialRaycast(type);
    }

    //ApplyMaterial(Mesh, type);
}


/// @brief applies the material to the lod near no raycast layer as expected
/// @param type 
void AcustomMeshActorBase::ApplyMaterialNoRaycastLayer(
    materialEnum type
){
    //ApplyMaterial(MeshNoRaycast, type);


    if(meshLodContainers.find(ELod::lodNear) != meshLodContainers.end()){
        meshLodContainers[ELod::lodNear].ApplyMaterialNoRaycast(type);
    }

}





/// @brief returns the layer by material enum type
/// @param type type of material
/// @return int layer index
int AcustomMeshActorBase::layerByMaterialEnum(materialEnum type){
    std::vector<materialEnum> types = MaterialEnumHelper::materialVector();
    for (int i = 0; i < types.size(); i++)
    {
        if(type == types[i]){
            return i;
        }
    }
    return 0;
}




/**
 * 
 * 
 * -- terrain type helper --
 * 
 */
std::vector<ETerrainType> AcustomMeshActorBase::terrainVector(){
    std::vector<ETerrainType> vector = {
        ETerrainType::EDesert,
        ETerrainType::EOcean,
        ETerrainType::EForest,
        ETerrainType::ETropical,
        ETerrainType::ESnowHill,
        ETerrainType::EDesertForest
    };
    return vector;
}

/**
 * 
 * 
 * --- lod helper methods ---
 * 
 * 
 */




///@brief will find the LOD distance to the given position and disable the actor if lod is far
ELod AcustomMeshActorBase::lodLevelByDistanceTo(FVector &locationOfPlayer){

    FVector a = GetActorLocation();
    LodCheckContainer checkContainer(a, locationOfPlayer);
    if(!distanceListeningBlocked){
        SetActorHiddenInGame(checkContainer.hideActorByLod());
    }


    //DEBUG
    if(false){
        return ELod::lodMiddle;
    }

    return checkContainer.lod();
}

/**
 * 
 * 
 * --- refresh mesh for dynamic actors like water or other moving meshes ---
 * 
 * 
 */
void AcustomMeshActorBase::TickShaderRunningTime(float DeltaTime){
    shaderRunningTime += DeltaTime;
    if(shaderRunningTime > 2 * M_PI){
        shaderRunningTime = 0.0f;
    }
}

void AcustomMeshActorBase::vertexShaderFor(MeshData &data){
    TArray<FVector> &vertecies = data.getVerteciesRef();
    for (int i = 0; i < vertecies.Num(); i++){
        FVector &current = vertecies[i];
        applyShaderToVertex(current);
    }
}

/// @brief apply vertex shader to the given vertex
/// @param vertex vertex to move
void AcustomMeshActorBase::applyShaderToVertex(FVector &vertex){
    //keep empty, is virtaul here.
}

void AcustomMeshActorBase::refreshMesh(
    UProceduralMeshComponent& meshComponent,
    MeshData &other,
    int layer
){
    if(other.verteciesNum() <= 0){
        return;
    }

    meshComponent.UpdateMeshSection(
        layer, 
        other.getVerteciesRef(), 
        other.getNormalsRef(), 
        other.getUV0Ref(),
        other.getVertexColorsRef(), 
        other.getTangentsRef()
    );

    if(false){
        meshComponent.SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        meshComponent.SetCollisionResponseToAllChannels(ECR_Ignore);
        meshComponent.SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }

}





/**
 * 
 * 
 * mesh hit test
 * 
 * 
 */

FVector AcustomMeshActorBase::worldToLocalHit(FVector &worldhit){
    //world hit to local
    FTransform worldTransform = GetActorTransform();
    return worldTransform.InverseTransformPosition(worldhit);
}


bool AcustomMeshActorBase::doesHitWorld(FVector &hitWorld, materialEnum mat){
    FVector hitLocal = worldToLocalHit(hitWorld);
    return doesHitLocal(hitLocal, mat);
}

bool AcustomMeshActorBase::doesHitLocal(FVector &hitLocal, materialEnum mat){
    MeshData &meshFound = findMeshDataReference(
        mat,
        ELod::lodNear,
        true//raycastFlag
    );
    return meshFound.doesHit(hitLocal);
}









/// @brief creates a two sided quad from 4 vertecies and a material
/// expecting the vertecies to be already ordered correctly in clockwise order from a to d!
/// Will apply the mesh layer 0 and material immidiatly for this actor
/// @param a a0
/// @param b b1
/// @param c c2
/// @param d d3
/// @param material material to be applied, must not be nullptr
void AcustomMeshActorBase::createTwoSidedQuad(
    FVector &a, 
    FVector &b,
    FVector &c,
    FVector &d,
    materialEnum material
){
    MeshData meshData;
    meshData.appendDoublesided(a, b, c, d);
    meshData.calculateNormals();
    replaceMeshData(meshData, material);
    ReloadMeshAndApplyAllMaterials();
}