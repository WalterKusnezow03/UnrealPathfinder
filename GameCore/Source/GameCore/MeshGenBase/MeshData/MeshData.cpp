// Fill out your copyright notice in the Description page of Project Settings.


#include "GameCore/MeshGenBase/MeshData/MeshData.h"
#include "CoreMath/Matrix/MMatrix.h"
#include "GameCore/DebugHelper.h"
#include "KismetProceduralMeshLibrary.h"
#include "AssetPlugin/gameStart/assetEnums/materialEnum.h"
#include "GameCore/MeshGenBase/MathHelp/baryCentricInterpolator.h"


#include <algorithm>
#include <set>

MeshData::MeshData()
{
}


/// @brief creates the mesh from triangles and vertecies AND calculates the normals!
/// @param verteciesIn 
/// @param trianglesIn 
MeshData::MeshData(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn){
    setVertecies(MoveTemp(verteciesIn));
    setTriangles(MoveTemp(trianglesIn));
    calculateNormals();
}

void MeshData::rebuild(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn){
    clearMesh();
    setVertecies(MoveTemp(verteciesIn));
    setTriangles(MoveTemp(trianglesIn));
    calculateNormals();
}



MeshData::~MeshData()
{
    clearMesh();
}


/// @brief copy constructor gets called if passing by value
/// @param other 
MeshData::MeshData(const MeshData &other){
    *this = other;
}

/// @brief abusing the assign operator to create a deepcopy of the data
/// @param other 
/// @return 
MeshData& MeshData::operator=(const MeshData &other){
    if(&other != this){
        clearMesh(); //clear mesh data before adding any data!

        
        //broken?
        
        vertecies = other.vertecies;
        triangles = other.triangles;
        normals = other.normals;
        UV0 = other.UV0;
        Tangents = other.Tangents;
        VertexColors = other.VertexColors;


        materialPreferred = other.materialPreferred;
        bounds = other.bounds; //very important for mesh push in

        /*
        for (int i = 0; i < other.vertecies.Num(); i++){
            vertecies.Add(other.vertecies[i]);
        }
        for (int i = 0; i < other.triangles.Num(); i++){
            triangles.Add(other.triangles[i]);
        }
        for (int i = 0; i < other.normals.Num(); i++){
            normals.Add(other.normals[i]);
        }
        for (int i = 0; i < other.UV0.Num(); i++){
            UV0.Add(other.UV0[i]);
        }
        for (int i = 0; i < other.Tangents.Num(); i++){
            Tangents.Add(other.Tangents[i]);
        }
        for (int i = 0; i < other.VertexColors.Num(); i++){
            VertexColors.Add(other.VertexColors[i]);
        }
        */
        

        
    }
    return *this;
}


/// @brief clean up data completely
void MeshData::clearMesh(){
    vertecies.Empty();
    triangles.Empty();
    normals.Empty();

    UV0.Empty();
    Tangents.Empty();
    VertexColors.Empty();
}

/// @brief clears the normals 
void MeshData::clearNormals(){
    normals.Empty();
}

/**
 * --- apply data ---
 */

/// @brief calculates the normals and applies it to the vertecies
void MeshData::calculateNormals(){
    
    
    clearNormals();
    normals.SetNum(vertecies.Num());
    for (int i = 2; i < triangles.Num(); i += 3) {
        int32 Index0 = triangles[i - 2];
        int32 Index1 = triangles[i - 1];
        int32 Index2 = triangles[i];

        if(isValidVertexIndex(Index0, Index1, Index2)){
            FVector Edge1 = vertecies[Index1] - vertecies[Index0];
            FVector Edge2 = vertecies[Index2] - vertecies[Index0];
            FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

            normals[Index0] = Normal;
            normals[Index1] = Normal;
            normals[Index2] = Normal;
        }

        
    }

    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(vertecies, triangles, UV0, normals, Tangents);
    
    
}

/// @brief sets the data for all vertecies, pass by r value reference
/// @param vertecies veretecies to set, be carefull with overriding
void MeshData::setVertecies(TArray<FVector> &&verteciesIn){
    vertecies = MoveTemp(verteciesIn);  // Move the data instead of copying, creating an r value
}
/// @brief sets the data for all triangles, pass by r value reference
/// @param trianglesIn triangles to set for the mesh
void MeshData::setTriangles(TArray<int32> &&trianglesIn){
    triangles = MoveTemp(trianglesIn);
}

/// @brief join another mesh, vertecies add, triangles added with offset added to index
/// all data will be COPIED, regardless of duplicated vertecies, normals and triangles
/// use appendEfficent(Meshdata) to get more data efficent results
/// but with computational overhead!
/// @param other 
void MeshData::append(MeshData &other)
{
    TArray<FVector> &verteciesRef = other.getVerteciesRef();
    TArray<int32> &trianglesRef = other.getTrianglesRef();
    TArray<FVector> &normalsRef = other.getNormalsRef();
    join(verteciesRef, trianglesRef, normalsRef);

    updateBoundsIfNeeded();
}

void MeshData::join(TArray<FVector> &verteciesRef, TArray<int32> &trianglesRef, TArray<FVector> &normalsin){
    int triangleOffset = vertecies.Num(); //beim vertex count offset starten!

    //copy triangles, apply offset
    for (int i = 0; i < trianglesRef.Num(); i++){
        int32 copy = trianglesRef[i];
        copy += triangleOffset;
        //--> offset index weil buffer sich verschiebt insgesamt um size des aktuellen
        //    vertex buffers
        triangles.Add(copy);
    }

    //copy vertecies
    for(int i = 0; i < verteciesRef.Num(); i++){
        FVector &ref = verteciesRef[i];
        vertecies.Add(ref);
    }

    //copy normals
    for(int i = 0; i < normalsin.Num(); i++){
        FVector &ref = normalsin[i];
        normals.Add(ref);
    }
}



/*

---- manual adding section ----

*/
void MeshData::append(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    TArray<FVector> _vertecies;
    TArray<int32> _triangles;
    buildTriangle(a, b, c, _vertecies, _triangles);
    MeshData appendMesh(
        MoveTemp(_vertecies),
        MoveTemp(_triangles)
    );
    append(appendMesh);
}


void MeshData::append(
    FVector &a, 
    FVector &b, 
    FVector &c,
    FVector &d
){
    append(a, b, c);
    append(a, c, d);
}

void MeshData::appendDoublesided(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    /*
    1 2
    0 
    */
   //kann nicht weiter vereinfacht werden damit jene duplikat vertecies auch andere normalen haben
    append(a, b, c); 
    append(a, c, b); 
}

void MeshData::appendDoublesided(
    FVector &a,
    FVector &b,
    FVector &c,
    FVector &d
){
    /*
    b c
    a d
    */
    appendDoublesided(a, b, c);
    appendDoublesided(a, c, d);
}


void MeshData::appendDoubleSidedTriangleBuffer(
    std::vector<FVector> &buffer
){  
    if(buffer.size() < 3){
        return;
    }

    for (int i = 0; i < buffer.size() - 3; i += 3)
    {
        appendDoublesided(
            buffer[i],
            buffer[i + 1],
            buffer[i + 2]
        );
    }
}




void MeshData::buildTriangle(
    FVector &a, 
    FVector &b, 
    FVector &c,
    TArray<FVector> &output,
    TArray<int32> &trianglesOutput
){
    //add triangles (new more readable)
    int32 offset = output.Num();
    trianglesOutput.Add(0 + offset); // 0th vertex in the first triangle
    trianglesOutput.Add(1 + offset); // 1st vertex in the first triangle
    trianglesOutput.Add(2 + offset); // 2nd vertex in the first triangle

    output.Add(a);
    output.Add(b);
    output.Add(c);
}



/// @brief offsets all vertecies in a given direction
/// @param offset 
void MeshData::offsetAllvertecies(FVector &offset){
    for (int i = 0; i < vertecies.Num(); i++){
        vertecies[i] += offset;
    }

    updateBoundsIfNeeded();
}

/// @brief transforms all vertecies with a given matrix
/// @param other 
void MeshData::transformAllVertecies(MMatrix &other){

    //matrix für vertecies: M
    for (int i = 0; i < vertecies.Num(); i++){
        vertecies[i] = other * vertecies[i];
    }

    //matrix für normalen: (M^-1)^T !!!! NICHT VERGESSEN!
    MMatrix M_inverse = other.createInverse();
    M_inverse.transpose();
    for (int i = 0; i < normals.Num(); i++){
        normals[i] = M_inverse * normals[i];
    }

    updateBoundsIfNeeded();
}





/// @brief appends the vertecies to the meshbuffer and creates triangles with n latest vertecies
/// to connect each vertex of the passed vector (allows for smooth 
/// shading and not duplicating vertecies)
/// caution: vec.size() should not be greater than the current vertex count, unless
/// this mesh data vertexcount is 0
/// will connect to the latest vec.size() vertecies, gaps cannot be detected
/// @param vec vector to connect, expected in some correct order 

void MeshData::appendVertecies(std::vector<FVector> &vec){
    if(vec.size() <= 0){
        return;
    }
    if(vertecies.Num() == 0){
        for (int i = 0; i < vec.size(); i++)
        {
            vertecies.Add(vec[i]);
        }
        return;
    }
    

    //soll sich an die letzten vertecies anheften
    //aber ohne vertecies zu duplizieren

    int32 startingIndex = vertecies.Num() - vec.size(); // 0
    if(vertecies.Num() < vec.size()){
        fillUpMissingVertecies(startingIndex); //difference abs
        startingIndex = 0;
    }

    int32 offset = vertecies.Num(); //stimmt so, getestet
    for (int i = 0; i < vec.size(); i++)
    {
        vertecies.Add(vec[i]);
    }

    //für jeden vertex den man hinzugefügt hat, aus 3 (+2) dreiecke basteln
    for (int i = 0; i < vec.size(); i++){

        
        // so.
        /*
        1 2
        0 3
        */
        // 1 triangle
        if(true){
            //das hier sieht eher nicht aus wie die 012 023 winding order, aber sie ist korrekt
            //nach aussen gedreht
            int next = (i + 1) % vec.size();
            triangles.Add(startingIndex + i); //lower(0)
            triangles.Add(startingIndex + next); //lower(3)
            triangles.Add(offset + i); //upper(1)
            
            //2 triangle
            triangles.Add(startingIndex + next); //lower (3)
            triangles.Add(offset + next); //upper(2)
            triangles.Add(offset + i); //upper (1)
        }
        
    }

    updateBoundsIfNeeded();
}

/**
 * caution: this function is not tested
 */
/// @brief will fill up vertecies between the last two ones
/// @param count 
void MeshData::fillUpMissingVertecies(int count){
    if(count <= 0){
        return;
    }
    int sizeOfVertexbuffer = vertecies.Num();
    count = std::abs(count);
    if(sizeOfVertexbuffer == 1){
        FVector copy = vertecies[0];
        for (int i = 0; i < count; i++)
        {
            vertecies.Add(copy);
        }
        return;
    }

    //DebugHelper::logMessage("meshData_debug inserted vertecies");

    FVector last = vertecies[sizeOfVertexbuffer - 1];
    FVector prev = vertecies[sizeOfVertexbuffer - 2];
    FVector connect = last - prev; //AB = B - A
    connect /= count;
    vertecies.RemoveAt(vertecies.Num() - 1);
    for (int i = 0; i < count; i++)
    {
        FVector inner = prev + connect * i;
        vertecies.Add(inner);
    }
    vertecies.Add(last);

    //fix triangle buffer offset because of insert
    int vertexIndexTargeted = sizeOfVertexbuffer - 1;
    for (int i = 0; i < triangles.Num(); i++)
    {
        if(triangles[i] == vertexIndexTargeted){
            triangles[i] = vertecies.Num() - 1;
        }
    }
}



void MeshData::closeMeshAtCenter(FVector &center, std::vector<FVector> &vec, bool clockWise){
    //vertecies.Add(center); //jetzt offset index valid
    for (int i = 0; i < vec.size(); i++){
        vertecies.Add(vec[i]);
    }
    closeMeshAtCenter(center, vec.size(), clockWise);
    return;

}

void MeshData::closeMeshAtCenter(FVector &center, int bufferSizeToConnect, bool clockWise){
    int limit = bufferSizeToConnect; //vertex count um an mitte zu binden
    if(limit > vertecies.Num()){
        limit = vertecies.Num();
    }
    int offset = vertecies.Num() - bufferSizeToConnect;
    if(offset < 0){
        offset = 0;
    }
    vertecies.Add(center);

    for (int i = 0; i < limit; i++)
    {
        int next = (i + 1) % limit;
        if(clockWise){
            /*
            1 2
            0
            */
            triangles.Add(offset); //0
            triangles.Add(offset + i); //1
            triangles.Add(offset + next); //2
        }else{
            /*
            2 1
            0
            */
           triangles.Add(offset); //0
           triangles.Add(offset + next); //2
           triangles.Add(offset + i); //1
        }
    }
}












/**
 * --- helper function ---
 */

///@brief searches for the closest index to a vertex and explicitly looks for duplcate vertecies
std::vector<int> MeshData::findClosestIndexWithVertexDuplicatesTo(
    FVector &vertex
){
    std::vector<int> output;

    int indexFound = findClosestIndexTo(vertex);
    if(isValidVertexIndex(indexFound)){
        output.push_back(indexFound);

        FVector comparePos = vertecies[indexFound];

        //search vertex buffer for similar vertecies
        for (int v = 0; v < vertecies.Num(); v++){
            if(
                v != indexFound && 
                FVector::Dist(comparePos, vertecies[v]) <= 1.0f
            ){
                output.push_back(v);
            }
        }
    }

    return output;
}

/// @brief finds the closest index to a vertex, -1 if the vertex buffer is clear
/// @param vertex position to find
/// @return index in vertecies array
int MeshData::findClosestIndexTo(FVector &vertex){
    if(vertecies.Num() == 0){
        return -1;
    }
    int closestIndex = 0;
    float dist = FVector::Dist(vertex, vertecies[0]);

    for (int i = 1; i < vertecies.Num(); i++){
        float newDist = FVector::Dist(vertex, vertecies[i]);
        if(newDist < dist){ //debug keep distance
            dist = newDist;
            closestIndex = i;
        }
    }
    return closestIndex;
}

bool MeshData::isCloseSame(FVector &a, int index){
    if(index < 0 || index >= vertecies.Num()){
        return false;
    }
    return isCloseSame(a, vertecies[index]);
}

bool MeshData::isCloseSame(FVector &a, FVector &b){
    return std::abs(a.X - b.X) <= EPSILON &&
           std::abs(a.Y - b.Y) <= EPSILON &&
           std::abs(a.Z - b.Z) <= EPSILON;
}



int MeshData::findClosestIndexToAndAvoid(FVector &vertex, int indexAvoid){
    std::vector<int> vec = {indexAvoid};
    return findClosestIndexToAndAvoid(vertex, vec);
}

int MeshData::findClosestIndexToAndAvoid(FVector &vertex, std::vector<int> &avoid){
    if(vertecies.Num() == 0){
        return -1;
    }
    int closestIndex = 0;
    float dist = FVector::Dist(vertex, vertecies[0]);

    for (int i = 1; i < vertecies.Num(); i++){

        if(!contains(avoid, i)){
            float newDist = FVector::Dist(vertex, vertecies[i]);
            if(newDist < dist){ //debug keep distance
                dist = newDist;
                closestIndex = i;
            }
        }
    }
    return closestIndex;
}






/// @brief creates a vertex buffer sample of 0123 clockwise dir on XY Pane
/// @param xMax xMax size
/// @param yMax yMax size
/// @return starting from (0,0,0) 0123 clockwise buffer quad
std::vector<FVector> MeshData::create2DQuadVertecies(int xMax, int yMax){
    std::vector<FVector> shape;
    shape.push_back(FVector(0,0,0));
    shape.push_back(FVector(0,yMax,0));
    shape.push_back(FVector(xMax,yMax,0));
    shape.push_back(FVector(xMax,0,0));
    return shape;
}


/**
 * 
 * is tested!
 * 
 */

void MeshData::appendEfficent(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    int indexA = findClosestIndexTo(a);
    int indexB = findClosestIndexTo(b);
    int indexC = findClosestIndexTo(c);

    int debugEfficentAdded = 3;

    //add if not found correctly
    if(!isCloseSame(a, indexA)){
        vertecies.Add(a);
        indexA = vertecies.Num() - 1; //0
        debugEfficentAdded--;
    }
    if(!isCloseSame(b, indexB)){
        vertecies.Add(b);
        indexB = vertecies.Num() - 1; //1
        debugEfficentAdded--;
    }
    if(!isCloseSame(c, indexC)){
        vertecies.Add(c);
        indexC = vertecies.Num() - 1; //2
        debugEfficentAdded--;
    }
    //add to triangle buffer
    triangles.Add(indexA);
    triangles.Add(indexB);
    triangles.Add(indexC);

    if(debugEfficentAdded > 0 && false){
        FString message = FString::Printf(TEXT("meshdata efficentAdded %d"), debugEfficentAdded);
        DebugHelper::logMessage(message);
    }

    updateBoundsIfNeeded();
}


void MeshData::appendEfficent(
    FVector &a, 
    FVector &b, 
    FVector &c,
    FVector &d
){
    appendEfficent(a, b, c);
    appendEfficent(a, c, d);
}



/// @brief appends all mesh data, eliminates duplicated vertecies,
/// and recalculates all normals
/// @param other some mesh data
void MeshData::appendEfficent(MeshData &other){

    TArray<int32> &trianglesRef = other.getTrianglesRef();
    for (int i = 0; i < trianglesRef.Num() - 3; i += 3){ //-2 ? (no, -3 is correct)
        int32 v0 = trianglesRef[i];
        int32 v1 = trianglesRef[i + 1];
        int32 v2 = trianglesRef[i + 2];

        if(
            other.isValidVertexIndex(v0) &&
            other.isValidVertexIndex(v1) &&
            other.isValidVertexIndex(v2)
        ){
            FVector &v0vertex = other.vertecies[v0];
            FVector &v1vertex = other.vertecies[v1];
            FVector &v2vertex = other.vertecies[v2];

            appendEfficent(v0vertex, v1vertex, v2vertex);
        }
    }

    calculateNormals();
}



FVector MeshData::createNormal(int v0, int v1, int v2){
    if(isValidVertexIndex(v0) && isValidVertexIndex(v1) && isValidVertexIndex(v2)){
        FVector &a = vertecies[v0];
        FVector &b = vertecies[v1];
        FVector &c = vertecies[v2];

        FVector ab = b - a;
        FVector ac = c - a;
        return FVector::CrossProduct(ab, ac);
    }
    return FVector(0,0,0);
}





bool MeshData::isValidVertexIndex(int i){
    return i >= 0 && i < vertecies.Num();
}

bool MeshData::isValidVertexIndex(int i, int j, int n){
    return isValidVertexIndex(i) && isValidVertexIndex(j) && isValidVertexIndex(n);
}

bool MeshData::isValidTriangleIndex(int i){
    return i >= 0 && i < triangles.Num();
}

bool MeshData::isValidNormalIndex(int i){
    return i >= 0 && i < normals.Num();
}

bool MeshData::isValidNormalIndex(int i, int j, int k){
    return isValidNormalIndex(i) && isValidNormalIndex(j) && isValidNormalIndex(k);
}

/**
 * --- Data references ---
 */

/// @brief returns the reference to the mesh data vertecies, be carefull with modifying
/// @return mesh data vertecies by reference
TArray<FVector> &MeshData::getVerteciesRef(){
    return vertecies;
}

/// @brief returns the reference to the mesh data triangles, be carefull with modifying
/// @return mesh data triangles by reference
TArray<int32> &MeshData::getTrianglesRef(){
    return triangles;
}

TArray<FVector> &MeshData::getNormalsRef(){
    return normals;
}

TArray<FVector2D> &MeshData::getUV0Ref(){
    return UV0;
}
TArray<FProcMeshTangent> &MeshData::getTangentsRef(){
    return Tangents;
}
TArray<FColor> &MeshData::getVertexColorsRef(){
    return VertexColors;
}










/**
 * 
 * --- special split mesh function ---
 * 
 */
void MeshData::seperateMeshIntoAllTrianglesDoubleSided(std::vector<MeshData> &meshDataVectorOutput){
    for (int i = 0; i < triangles.Num() - 3; i += 3) {
        int32 Index0 = triangles[i];
        int32 Index1 = triangles[i + 1];
        int32 Index2 = triangles[i + 2];

        if(
            isValidVertexIndex(Index0, Index1, Index2)
        ){
            FVector &v0 = vertecies[Index0];
            FVector &v1 = vertecies[Index1];
            FVector &v2 = vertecies[Index2];
            MeshData newTriangleMesh;
            newTriangleMesh.appendDoublesided(v0, v1, v2); //normals will be calculated in the constructor
            meshDataVectorOutput.push_back(newTriangleMesh);
        }
    }
}



/// @brief will create all triangles possible to split and append it into the vector
/// traingles being lower than the min distance will not be added!
/// @param meshDataVectorOutput 
void MeshData::splitAllTrianglesInHalfAndSeperateMeshIntoAllTrianglesDoubleSided(
    std::vector<MeshData> &meshDataVectorOutput
){
    for (int i = 0; i < triangles.Num() - 3; i += 3) {
        int32 Index0 = triangles[i];
        int32 Index1 = triangles[i + 1];
        int32 Index2 = triangles[i + 2];

        FVector &v0 = vertecies[Index0];
        FVector &v1 = vertecies[Index1];
        FVector &v2 = vertecies[Index2];

        if(canSplit(v0,v1,v2)){
            FVector middle = v1 + 0.5f * (v2 - v1); // gx = A + r(B-A)

            MeshData newTriangleMeshA;
            newTriangleMeshA.appendDoublesided(v0, v1, middle);
            newTriangleMeshA.calculateNormals();
            meshDataVectorOutput.push_back(newTriangleMeshA);

            MeshData newTriangleMeshB;
            newTriangleMeshB.appendDoublesided(v0, middle, v2);
            newTriangleMeshB.calculateNormals();
            meshDataVectorOutput.push_back(newTriangleMeshB);
        }else{
            //DebugHelper::logMessage("DEBUGSPLIT could not split");
        }
    }

}

bool MeshData::canSplit(FVector &a, FVector &b, FVector &c, float mindistanceKept){
    if(FVector::Dist(a,b) < mindistanceKept){
        return false;
    }
    if(FVector::Dist(a,c) < mindistanceKept){
        return false;
    }
    if(FVector::Dist(b,c) < mindistanceKept){
        return false;
    }
    return true;
}


bool MeshData::canSplit(FVector &a, FVector &b, FVector &c){
    return canSplit(a, b, c, MIN_SPLITDISTANCE);
}

bool MeshData::canSplit(int v0, int v1, int v2){
    return canSplit(v0, v1, v2, MIN_SPLITDISTANCE);
}

bool MeshData::canSplit(int v0, int v1, int v2, float mindistanceKept){
    if(isValidVertexIndex(v0,v1,v2)){
        return canSplit(
            vertecies[v0],
            vertecies[v1],
            vertecies[v2],
            mindistanceKept
        );
    }
    return false;
}

void MeshData::addTriangle(int v0, int v1, int v2){
    if(isValidVertexIndex(v0,v1,v2)){
        triangles.Add(v0);
        triangles.Add(v1);
        triangles.Add(v2);
    }
}

void MeshData::addTriangleDoublesided(int v0, int v1, int v2){
    addTriangle(v0, v1, v2);
    addTriangle(v0, v2, v1);
}

///@brief removes the triangle targeted and splits it if possible
void MeshData::splitTriangleInHalf(int v0, int v1, int v2){
    if(isValidVertexIndex(v0,v1,v2)){
        //remove triangle
        int removedTriangles = removeTriangleSimilarTo(v0, v1, v2);
        //DebugHelper::logMessage("debugTriangle -> triangles removed: ", removedTriangles);

        //create new triangles
        float maxDistance = 200.0f;
        if (canSplit(v0, v1, v2, maxDistance))
        {
        
            FVector &A = vertecies[v0];
            FVector &B = vertecies[v1];
            FVector &C = vertecies[v2];

            //FVector middle = A + 0.5f * (B - A); // gx = A + r(B-A)

            FVector start;
            FVector dir;
            findLongestSide(A, B, C, start, dir);
            FVector middle = start + 0.5f * dir;

            //append new vertex and normal
            int newIndex = vertecies.Num();
            if(normals.Num() == vertecies.Num()){
                FVector cross = FVector::CrossProduct((middle - A), (C - A));
                cross = cross.GetSafeNormal();
                normals.Add(cross);
            }
            vertecies.Add(middle);


            if(removedTriangles == 1){
                //append sub triangles
                addTriangle(v0, newIndex, v2);
                addTriangle(newIndex, v1, v2);
            }else{

                //not needed basically. But if a triangle was double sided the splittet
                //will be too.
                addTriangleDoublesided(v0, newIndex, v2);
                addTriangleDoublesided(newIndex, v1, v2);
            }

            //DebugHelper::logMessage("debugTriangle added");

            //calculateNormals();
        }
    }
}


void MeshData::findLongestSide(
    FVector &a, 
    FVector &b, 
    FVector &c,
    FVector &startOut,
    FVector &dirOut
){
    FVector dirAB = b - a;
    FVector dirBC = c - b;
    FVector dirCA = a - c;

    std::vector<FVector> sides = {
        dirAB,
        dirBC,
        dirCA
    };
    std::vector<FVector> startPoints = {a, b, c};

    float largestSide = -1.0f; //so the start is set anyway
    for (int i = 0; i < sides.size(); i++){
        FVector &current = sides[i];
        if (current.Size() > largestSide){
            startOut = startPoints[i];
            dirOut = current;
        }
    }
}

//new split function, update mesh, do not rip apart
void MeshData::splitAndRemoveTrianglesAt(FVector &localHitPoint){
    
    //find closest vertex
    //enclosing the point
    //do not check all triangles
    std::vector<int> indices = findClosestIndexWithVertexDuplicatesTo(localHitPoint);
    for (int indexCurrent = 0; indexCurrent < indices.size(); indexCurrent++){
        //int index = findClosestIndexTo(localHitPoint);

        int index = indices[indexCurrent];
        if(isValidVertexIndex(index)){

            //wenn die eckpunkte des dreiecks
            //A * r + B * s + C * t = F, und r + s + t = 1.0f ist, dann ist es im dreieck

            std::vector<int> foundTriangles;
            findTrianglesInvolvedWith(index, foundTriangles);
            //DebugHelper::logMessage("debugTriangle -> check triangles", (foundTriangles.size() / 3));

            for (int i = 2; i < foundTriangles.size(); i += 3)
            {
                int v0 = foundTriangles[i - 2];
                int v1 = foundTriangles[i - 1];
                int v2 = foundTriangles[i];

                //check if inside
                //DebugHelper::logMessage("debugTriangle -> try solve");
                if(solveIsInTriangle(v0,v1,v2, localHitPoint)){

                    //DebugHelper::logMessage("debugTriangle -> is in triangle");

                    //try split
                    splitTriangleInHalf(v0, v1, v2);

                    //split still needed!!!
                    //new vertecies must be added internally to make a hole for example.
                }
            }
        }
    }
}


///@brief returns whether the mesh is hit or not
bool MeshData::doesHit(FVector &localHitPoint){
    if(vertecies.Num() < 3 || triangles.Num() == 0){
        return false;
    }

    if(isInsideBoundingbox(localHitPoint)){
        //find closest vertex
        //enclosing the point
        //do not check all triangles
        std::vector<int> indices = findClosestIndexWithVertexDuplicatesTo(localHitPoint);
        for (int indexCurrent = 0; indexCurrent < indices.size(); indexCurrent++){
            //int index = findClosestIndexTo(localHitPoint);

            int index = indices[indexCurrent];
            if(isValidVertexIndex(index)){

                //wenn die eckpunkte des dreiecks
                //A * r + B * s + C * t = F, und r + s + t = 1.0f ist, dann ist es im dreieck

                std::vector<int> foundTriangles;
                findTrianglesInvolvedWith(index, foundTriangles);
                //DebugHelper::logMessage("debugTriangle -> check triangles", (foundTriangles.size() / 3));

                for (int i = 2; i < foundTriangles.size(); i += 3)
                {
                    int v0 = foundTriangles[i - 2];
                    int v1 = foundTriangles[i - 1];
                    int v2 = foundTriangles[i];

                    //check if inside
                    //DebugHelper::logMessage("debugTriangle -> try solve");
                    if(solveIsInTriangle(v0,v1,v2, localHitPoint)){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

///@brief finds all triangles where an index is contained
void MeshData::findTrianglesInvolvedWith(
    int index,
    std::vector<int> &trianglesFound
){
    if(isValidVertexIndex(index)){
        for (int i = 2; i < triangles.Num(); i+=3){
            int v0 = triangles[i - 2];
            int v1 = triangles[i - 1];
            int v2 = triangles[i];
    
            if(isPartOfTraingle(index, v0, v1, v2)){
                trianglesFound.push_back(v0);
                trianglesFound.push_back(v1);
                trianglesFound.push_back(v2);
            }
        }
    }
}

bool MeshData::isPartOfTraingle(int target, int v0, int v1, int v2){
    return target == v0 || target == v1 || target == v2;
}

bool MeshData::solveIsInTriangle(
    int v0, int v1, int v2, FVector &target
){

    
    //left/ right off test ----> can work for any polygonal shape, 
    //the point tested for polygon hit must be moved in the edge relative space though as you can see
    //below.
    //O(3) + cos / sin in rotator maker to project the triangle
    if(isValidVertexIndex(v0,v1,v2)){
        FVector A = vertecies[v0];
        FVector B = vertecies[v1];
        FVector C = vertecies[v2];

        FVector AB = B - A;
        FVector AC = C - A;
        FVector BC = C - B;
        FVector CA = A - C;

        FVector normal = FVector::CrossProduct(AB, AC);
        normal = normal.GetSafeNormal();
        // create rotator, x is forward, yz pane projected so we can use the 2D normal correctly.
        MMatrix rotInv = MMatrix::createRotatorFrom(normal); //R
        rotInv.transpose(); //R^-1 = R^T

        FVector localA = target - A;
        FVector localB = target - B;
        FVector localC = target - C;

        

        //remove rotation, project to yz pane, x is 0.0 after inv usuage
        localA = rotInv * localA;
        localB = rotInv * localB;
        localC = rotInv * localC;
        AB = rotInv * AB;
        BC = rotInv * BC;
        CA = rotInv * CA;

        

        // target = rotInv * target
        // ab hier nurnoch y-z-pane, x ist default forward in meiner klasse.
        // wenn das skalar produkt der normalen
        // überall das selbe vorzeichen hat liegt es im dreieck
        FVector ABnormal = FVector(0, AB.Z * -1.0f, AB.Y);
        FVector BCnormal = FVector(0, BC.Z * -1.0f, BC.Y);
        FVector CAnormal = FVector(0, CA.Z * -1.0f, CA.Y);

        /*
        ABnormal = ABnormal.GetSafeNormal();
        BCnormal = BCnormal.GetSafeNormal();
        CAnormal = CAnormal.GetSafeNormal();
        localA = localA.GetSafeNormal();
        localB = localB.GetSafeNormal();
        localC = localC.GetSafeNormal();
        */

        //since the triangle is projected on the yz pane, x is not needed
        float dot0 = localA.Y * ABnormal.Y + localA.Z * ABnormal.Z;
        float dot1 = localB.Y * BCnormal.Y + localB.Z * BCnormal.Z;
        float dot2 = localC.Y * CAnormal.Y + localC.Z * CAnormal.Z;

        /*
        FString message = FString::Printf(
            TEXT("debugTriangle dot products {%.2f, %.2f, %2.f}"),
            dot0, dot1, dot2
        );
        DebugHelper::logMessage(message);*/

        //check dot products
        return (dot0 < 0.0f && dot1 < 0.0f && dot2 < 0.0f) ||
               (dot0 >= 0.0f && dot1 >= 0.0f && dot2 >= 0.0f);
    }else{
        return false;
    }
    return false;
}

///@brief removed triangle can be in incorrect order
int MeshData::removeTriangleSimilarTo(int v0, int v1, int v2){
    int removed = 0;
    if (isValidVertexIndex(v0, v1, v2))
    {
        int i = 2;
        int limit = triangles.Num();
        while(i < limit){
            int &v0Current = triangles[i - 2];
            int &v1Current = triangles[i - 1];
            int &v2Current = triangles[i];

            if(trianglesAreSame(v0, v1, v2, v0Current, v1Current, v2Current)){
                int lastIndex = triangles.Num() - 1;
                if(lastIndex - 2 >= 0){
                    v2Current = triangles[lastIndex]; //copied into reference.
                    v1Current = triangles[lastIndex - 1];
                    v0Current = triangles[lastIndex - 2];

                    triangles.Pop();
                    triangles.Pop();
                    triangles.Pop();
                    limit = triangles.Num();

                    DebugHelper::logMessage("debugTriangle removed, new size: ", triangles.Num());

                    removed++;
                }
                //return;
            }
            i+=3;
        }
    }
    return removed;
}

bool MeshData::trianglesAreSame(int v0, int v1, int v2, int v00, int v01, int v02){
    std::set<int> set;
    set.insert(v0);
    set.insert(v1);
    set.insert(v2);
    set.insert(v00);
    set.insert(v01);
    set.insert(v02);

    return set.size() == 3;
}

/// @brief calculates the center of the vertecies
/// @return center
FVector MeshData::center(){
    FVector zeroVec(0, 0, 0);
    if(vertecies.Num() <= 0){
        return zeroVec;
    }

    for (int i = 0; i < vertecies.Num(); i++){
        zeroVec += vertecies[i];
    }
    zeroVec /= vertecies.Num();
    return zeroVec;
}

/// @brief centers the mesh
void MeshData::centerMesh(){
    FVector thiscenter = center();
    for (int i = 0; i < vertecies.Num(); i++){
        vertecies[i] -= thiscenter;
    }
}

/// @brief flips all triangle surfaces but doesnt refresh the normals!
void MeshData::flipAllTriangles(){
    for (int i = 2; i < triangles.Num(); i += 3){
        //int32 v0 = triangles[i - 2];
        int32 v1 = triangles[i - 1];
        int32 v2 = triangles[i];

        //spaw
        triangles[i] = v1;
        triangles[i - 1] = v2;
    }
}



/**
 * 
 * -- mesh cutting helpers --
 * 
 */


///@brief removes out a vertex from the mesh
void MeshData::removeVertex(int index){
    std::vector<int> ignored;
    removeVertex(index, ignored);
}

///@brief removes out a vertex from the mesh data by index and:
///@param connectedvertecies saves the connected vertecies defined by the triangle buffer
void MeshData::removeVertex(int index, std::vector<int>& connectedvertecies){

    if(isValidVertexIndex(index) && vertecies.Num() > 0){
        removeTrianglesInvolvedWith(index, connectedvertecies);

        //remove vertex by swapping with end and change the triangle buffer as needed
        int oldEnd = vertecies.Num() - 1;
        vertecies[index] = vertecies[oldEnd]; //index der removed wird nach hinten tauschen
        vertecies.Pop(); // pop back end which is in new pos now
        if(isValidNormalIndex(index) && isValidNormalIndex(oldEnd)){
            normals[index] = normals[oldEnd];
            normals.Pop();
        }

        //(update the indices in the triangle buffer because the vertex has changed)
        for (int i = 0; i < triangles.Num(); i++){
            if(triangles[i] == oldEnd){
                triangles[i] = index; //update weil swap
            }
        }

        //find old end in connected vertecies and replace the index
        for (int i = 0; i < connectedvertecies.size(); i++){
            int vertexIndexNow = connectedvertecies[i];
            if(oldEnd == vertexIndexNow){
                connectedvertecies[i] = index; //update weil swap
            }
        }
    }
}

///@brief removes all triangles from the triangle buffer by vertex index and
///@param connectedvertecies saves the connected vertecies for processing
void MeshData::removeTrianglesInvolvedWith(int vertexIndex, std::vector<int> &connectedvertecies){
    if(!isValidVertexIndex(vertexIndex)){
        return;
    }
    TArray<int32> triangleBufferCopy;
    for (int i = 0; i < triangles.Num() - 3; i += 3)
    {
        int32 v0 = triangles[i];
        int32 v1 = triangles[i+1];
        int32 v2 = triangles[i+2];

        bool v0ok = (v0 != vertexIndex);
        bool v1ok = (v1 != vertexIndex);
        bool v2ok = (v2 != vertexIndex);

        if(v0ok && v1ok && v2ok){
            triangleBufferCopy.Add(v0);
            triangleBufferCopy.Add(v1);
            triangleBufferCopy.Add(v2);
        }else{
            //connected vertecies are added to the buffer
            //dont copy the vertex which is removed
            if(v0ok && !contains(connectedvertecies, v0)){
                connectedvertecies.push_back(v0);
            }
            if(v1ok && !contains(connectedvertecies, v1)){
                connectedvertecies.push_back(v1);
            }
            if(v2ok && !contains(connectedvertecies, v2)){
                connectedvertecies.push_back(v2);
            }
            
        }
    }
    triangles = triangleBufferCopy;
}


bool MeshData::contains(std::vector<int> &ref, int index){
    for (int i = 0; i < ref.size(); i++){
        if(ref[i] == index){
            return true;
        }
    }
    return false;
}



/**
 * 
 * 
 * mesh deform no remove helper
 * 
 * 
 */
void MeshData::pushInwards(FVector &location, int radius, FVector scaleddirection){
    if(vertecies.Num() == 0){
        DebugHelper::logMessage("MeshData PushInwards: Empty Vertex Count");
        return;
    }

    radius = std::abs(radius);

    std::vector<int> connected;
    int index = findClosestIndexTo(location);
    if(!isValidVertexIndex(index)){
        DebugHelper::logMessage("MeshData PushInwards: No Index Found Closest");
        return;
    }
    FVector foundLocation = vertecies[index];

    findConnectedVerteciesTo(index, connected);

    //recursivly find all connected vertecies from the triangle buffer
    int i = 0;
    int size = connected.size();
    while(i < size){
        if(i < connected.size()){
            int currentIndex = connected[i];
            if(isValidVertexIndex(currentIndex)){
                FVector &currentVertex = vertecies[currentIndex];
                float dist = FVector::Dist(currentVertex, foundLocation);
                if(dist < radius){
                    findConnectedVerteciesTo(currentIndex, connected);
                    size = connected.size();
                }
            }
        }

        i++;
    }

    DebugHelper::logMessage("MeshData PushInwards: Connected Vertecies", connected.size());

    // apply scaled offset direction
    for (int j = 0; j < connected.size(); j++)
    {
        int currentIndex = connected[j];
        if (isValidVertexIndex(currentIndex))
        {
            DebugHelper::logMessage("MeshData Vertex Push in Index", currentIndex);
            FVector &vertex = vertecies[currentIndex];
            if(isInsideBoundingbox(vertex)){
                vertex += scaleddirection;


                //debug
                DebugHelper::logMessage("MeshData Vertex Push in", vertex);
            }
            
        }
    }

}

void MeshData::findConnectedVerteciesTo(int index, std::vector<int> &output){
    
    for (int i = 0; i < triangles.Num() - 3; i += 3)
    {
        int v0 = triangles[i];
        int v1 = triangles[i+1];
        int v2 = triangles[i+2];
        if(v0 == index || v1 == index || v2 == index){
            if (!contains(output, v0)){
                output.push_back(v0);
            }
            if (!contains(output, v1)){
                output.push_back(v1);
            }
            if (!contains(output, v2)){
                output.push_back(v2);
            }
        }
    }
}




/**
 * 
 * foliage helper
 * 
 */
materialEnum MeshData::targetMaterial(){
    return materialPreferred;
}
void MeshData::setTargetMaterial(materialEnum inMaterial){
    materialPreferred = inMaterial;
}



/// @brief generates the matricies to move an object to the vertex position
/// and rotate in look dir of normal. CAUTION: X axis is forward! The rotation block is
/// in yaw and pitch rotation relative to the XAxis! (1,0,0), orient your mesh accordingly
/// if you are using this methods to place a mesh on a surface for example!
/// @param output 
void MeshData::generateMatricesPerFaceAndLookDirOfNormal(
    std::vector<MMatrix> &output
){
    for (int i = 0; i < triangles.Num() - 3; i+= 3){
        int v0 = triangles[i];
        int v1 = triangles[i+1];
        int v2 = triangles[i+2];
        if(isValidVertexIndex(v0, v1, v2)){
            FVector &vertex0 = vertecies[v0];
            FVector &vertex1 = vertecies[v1];
            FVector &vertex2 = vertecies[v2];

            FVector v0v1 = vertex1 - vertex0; // AB = B - A;
            FVector v0v2 = vertex2 - vertex0;
            FVector normal = FVector::CrossProduct(v0v1, v0v2);
            normal = normal.GetSafeNormal();


            MMatrix rotator = MMatrix::createRotatorFrom(normal);
            //debug
            if(false){
                FVector axis(1, 0, 0);
                axis = rotator * axis;
                DebugHelper::logMessage("debug comparenormal", axis, normal); //works correct!
            }
            // debug end

            MMatrix translation;
            FVector center = (vertex0 + vertex1 + vertex2) / 3;
            translation.setTranslation(center);

            //translation.setTranslation(vertex0); //debug

            MMatrix TR = translation * rotator; //<-- lese richtung --
    
            output.push_back(TR);
        }
    }
}



void MeshData::generateMatricesPerFaceAndLookDirOfNormalInterpolated(
    std::vector<MMatrix> &output,
    int stepSize
){


    baryCentricInterpolator interpolator;
    for (int i = 0; i < triangles.Num() - 3; i+= 3){
        int v0 = triangles[i];
        int v1 = triangles[i+1];
        int v2 = triangles[i+2];
        if(isValidVertexIndex(v0, v1, v2)){
            FVector &vertex0 = vertecies[v0];
            FVector &vertex1 = vertecies[v1];
            FVector &vertex2 = vertecies[v2];

            
            interpolator.setup(vertex0, vertex1, vertex2);
            std::vector<MMatrix> newMatrices;
            interpolator.interpolateAllAsMatrixTransformToVertexAndLookOfNormal(
                stepSize,
                newMatrices
            );

            for (int j = 0; j < newMatrices.size(); j++){
                output.push_back(newMatrices[j]);
            }
        }
    }

}





/**
 * 
 * room mesh data helper 
 * 
 */

///@brief appends a cube using the efficent method
void MeshData::appendCube(
    FVector &a, 
    FVector &b,
    FVector &c,
    FVector &d,
    FVector &a1, 
    FVector &b1,
    FVector &c1,
    FVector &d1
){
    appendEfficent(a, d, c, b);
    appendEfficent(a1, b1, c1, d1);

    appendEfficent(b, b1, a1, a);
    appendEfficent(c, c1, b1, b);
    appendEfficent(d, d1, c1, c);
    appendEfficent(a, a1, d1, d);

    //calculateNormals();
}

void MeshData::appendCube(
    FVector &a,
    FVector &b,
    FVector &c,
    FVector &d,
    FVector orthogonalDir
){
    FVector a1 = a + orthogonalDir;
    FVector b1 = b + orthogonalDir;
    FVector c1 = c + orthogonalDir;
    FVector d1 = d + orthogonalDir;
    appendCube(a, b, c, d, a1, b1, c1, d1);
}


void MeshData::appendCubeAt(
    FVector &pivot,
    int scaleX,
    int scaleY,
    int scaleZ
){
    /*
    1->2
    |  |
    0<-3
    */
    FVector a = pivot;
    FVector b = pivot + FVector(0, scaleY, 0);
    FVector c = pivot + FVector(scaleX, scaleY, 0);
    FVector d = pivot + FVector(scaleX, 0, 0);

    FVector updir(0, 0, scaleZ);

    appendCube(a,b,c,d,updir);
}

int MeshData::verteciesNum(){
    return vertecies.Num();
}

void MeshData::updateBoundsIfNeeded(){
    bounds.updateBoundsIfNeeded(vertecies);
}

void MeshData::updateBoundsIfNeeded(FVector &other){
    bounds.updateBoundsIfNeeded(other);
}

///@brief checks if a vertex is inside the bounding box, NOT allowing edge cases!
bool MeshData::isInsideBoundingbox(FVector &other){
    return bounds.isInsideBoundingbox(other);
}

bool MeshData::hasAnyVertecies(){
    return verteciesNum() > 0;
}


/**
 * 
 * 
 * ----- 2D DATA -----
 * 
 * 
 */

/// @brief 
/// @param sizeX 
/// @param sizeY 
/// @param distanceXY 
void MeshData::generate(int sizeX, int sizeY, int distanceXY){

    sizeX = std::abs(sizeX);
    sizeY = std::abs(sizeY);
    if(sizeX <= 0 || sizeY <= 0){
        return;
    }

    umbruch = sizeX;

    clearMesh();

    //generate array von vertecies mit x,y und abstand, height auf 0
    for (int i = 0; i < sizeY; i++){
        for (int j = 0; j < sizeX; j++){
            FVector newPos(
                j * distanceXY,
                i * distanceXY,
                0.0f
            );
            vertecies.Add(newPos);
        }
    }

    //generate triangles for 2D grid
    for (int i = 0; i < sizeY; i++){
        for (int j = 0; j < sizeX; j++){
            //create 2 triangles
            int nextI = i + 1;
            int nextJ = j + 1;
            if(nextI < sizeY && nextJ < sizeX){

                /*
                1>2
                | |
                0<3

                i = y
                j = x
                */

                int v0 = indexFor(i, j);
                int v1 = indexFor(nextI, j);
                int v2 = indexFor(nextI, nextJ);
                int v3 = indexFor(i, nextJ);

                /*
                
                1 2
                0
                  2
                0 3
                */
                if(
                    isValidVertexIndex(v0) && 
                    isValidVertexIndex(v1) && 
                    isValidVertexIndex(v2) && 
                    isValidVertexIndex(v3)
                ){
                    triangles.Add(v0);
                    triangles.Add(v1);
                    triangles.Add(v2);

                    triangles.Add(v0);
                    triangles.Add(v2);
                    triangles.Add(v3);
                }

            }

            

        }
    }

}


int MeshData::indexFor(int i, int j){
    i = std::abs(i);
    j = std::abs(j);

    int oneD = j * umbruch + i;
    return oneD;
}


FVector &MeshData::findIndex(int i, int j){
    int oneD = indexFor(i, j);
    if (oneD < vertecies.Num()){
        return vertecies[oneD];
    }
    return noneVertex;
}


void MeshData::replaceAt(int i, int j, FVector &other){
    int oneD = indexFor(i, j);
    if (oneD < vertecies.Num()){
        vertecies[oneD] = other;
    }
}



// ---- road section ----
void MeshData::appendParalellLinesClosedAsQuads(
    TArray<FVector> &line0,
    TArray<FVector> &line1
){
    
    int limit = std::min(line0.Num(), line1.Num()); //safety, should be same size
    for(int i = 1; i < limit; i++){
        /*
        1->2 //line1
        |  |
        0<-3 //line0
        */
        FVector &v0 = line0[i-1];
        FVector &v1 = line1[i-1];
        FVector &v2 = line1[i];
        FVector &v3 = line0[i];

        appendEfficent(v0, v1, v2, v3);
    }



}







// ---- debug -----
void MeshData::debugDrawMesh(MMatrix &transform, UWorld *world){
    if(world != nullptr){
        //durch triangle buffer laufen, alle zeichnen
        for (int i = 2; i < triangles.Num(); i+=3){
            int v0 = triangles[i - 2];
            int v1 = triangles[i - 1];
            int v2 = triangles[i];
            if(isValidVertexIndex(v0,v1,v2)){
                FVector v0Vertex = vertecies[v0]; //is copy
                FVector v1Vertex = vertecies[v1];
                FVector v2Vertex = vertecies[v2];

                v0Vertex = transform * v0Vertex;
                v1Vertex = transform * v1Vertex;
                v2Vertex = transform * v2Vertex;

                float time = 2.0f;
                DebugHelper::showLineBetween(world, v0Vertex, v1Vertex, FColor::Red, time);
                DebugHelper::showLineBetween(world, v1Vertex, v2Vertex, FColor::Red, time);
                DebugHelper::showLineBetween(world, v2Vertex, v0Vertex, FColor::Red, time);
            }
        }
    }
}





