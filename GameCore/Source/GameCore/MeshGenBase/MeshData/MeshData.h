// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <set>
#include "GameCore/MeshGenBase/MeshData/BoundingBox/BoundingBoxSimple.h"
#include "KismetProceduralMeshLibrary.h"
#include "AssetPlugin/gameStart/assetEnums/materialEnum.h"
#include "CoreMath/Matrix/MMatrix.h"


/**
 * The data must be saved here to organize mesh actor layers for different materials 
 * and keeping code clean when applying meshes and handling more layers
 */
class GAMECORE_API MeshData
{	
	// ------ SAVE TO STORAGE SECTION -------
public:


private:
	int internalLayerId = 0;

	// ------ MEMBER VARIABLES NEEDED FOR COPY CONSTRUCTOR ------
protected:
	//mesh data
	TArray<FVector> vertecies;
	TArray<int32> triangles;
	TArray<FVector> normals;
	TArray<FProcMeshTangent> Tangents; 
    TArray<FColor> VertexColors;
	TArray<FVector2D> UV0; //Uv generation custom planar mapping not implemented yet, to come

	//bound data needed for hittesting
	BoundingBoxSimple bounds;

	materialEnum materialPreferred = materialEnum::wallMaterial;



	// ------ CLASS SECTION --------
public:
	MeshData();
	virtual ~MeshData(); //automatischer aufruf aus subklasse

	MeshData(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn);
	MeshData(const MeshData &other);
	MeshData &operator=(const MeshData &other);

	void append(MeshData &other);
	void append(
		FVector &a,
		FVector &b,
		FVector &c
	);
	void append(
		FVector &a,
		FVector &b,
		FVector &c,
		FVector &d
	);

	void appendEfficent(
		FVector &a,
		FVector &b,
		FVector &c
	);
	void appendEfficent(
		FVector &a,
		FVector &b,
		FVector &c,
		FVector &d
	);
	void appendEfficent(MeshData &other);


	void appendDoublesided(
		FVector &a,
		FVector &b,
		FVector &c
	);

	void appendDoublesided(
		FVector &a,
		FVector &b,
		FVector &c,
		FVector &d
	);

	void appendDoubleSidedTriangleBuffer(
		std::vector<FVector> &buffer
	);

	void rebuild(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn);

	void clearMesh();
	
	void setVertecies(TArray<FVector> &&verteciesIn);
	void setTriangles(TArray<int32> &&trianglesIn);

	void calculateNormals();

	TArray<FVector> &getVerteciesRef();
	TArray<int32> &getTrianglesRef();
	TArray<FVector> &getNormalsRef();
	TArray<FVector2D> &getUV0Ref();
	TArray<FProcMeshTangent> &getTangentsRef();
	TArray<FColor> &getVertexColorsRef();

	void offsetAllvertecies(FVector &offset);
	void transformAllVertecies(MMatrix &other);

	void appendVertecies(std::vector<FVector> &vec);
	

	void closeMeshAtCenter(FVector &center, std::vector<FVector> &vec, bool clockWise);
	void closeMeshAtCenter(FVector &center, int bufferSizeToConnect, bool clockWise);

	static std::vector<FVector> create2DQuadVertecies(int xMax, int yMax);


	//new helper
	void seperateMeshIntoAllTrianglesDoubleSided(std::vector<MeshData> &meshDataVectorOutput);
	void splitAllTrianglesInHalfAndSeperateMeshIntoAllTrianglesDoubleSided(
		std::vector<MeshData> &meshDataVectorOutput
	);
	void findLongestSide(
		FVector &a,
		FVector &b,
		FVector &c,
		FVector &startOut,
		FVector &dirOut);

	void splitAndRemoveTrianglesAt(FVector &localHitPoint);
	bool doesHit(FVector &localHitPoint);

	FVector center();
	void centerMesh();

	void appendCube(
		FVector &a,
		FVector &b,
		FVector &c,
		FVector &d,
		FVector &a1,
		FVector &b1,
		FVector &c1,
		FVector &d1
	);

	void appendCube(
		FVector &a,
		FVector &b,
		FVector &c,
		FVector &d,
		FVector orthogonalDir
	);

	void appendCubeAt(
		FVector &pivot,
		int scaleX,
		int scaleY,
		int scaleZ
	);

protected:
	float MIN_SPLITDISTANCE = 50.0f;

	bool canSplit(int v0, int v1, int v2);
	bool canSplit(FVector &a, FVector &b, FVector &c);
	bool canSplit(FVector &a, FVector &b, FVector &c, float mindistanceKept);
	bool canSplit(int v0, int v1, int v2, float mindistanceKept);

	float EPSILON = 5.0f;
	bool isCloseSame(FVector &a, FVector &b);
	bool isCloseSame(FVector &a, int index);

	void fillUpMissingVertecies(int count);

	void clearNormals();

	void buildTriangle(
		FVector &a,
		FVector &b,
		FVector &c,
		TArray<FVector> &output,
		TArray<int32> &trianglesOutput
	);

	

	std::vector<int> findClosestIndexWithVertexDuplicatesTo(FVector &vertex);
	int findClosestIndexTo(FVector &vertex);
	int findClosestIndexToAndAvoid(FVector &vertex, int indexAvoid);
	int findClosestIndexToAndAvoid(FVector &vertex, std::vector<int> &avoid);

	void join(TArray<FVector> &vertecies, TArray<int32> &triangles, TArray<FVector> &normalsin);

	bool isValidVertexIndex(int i);
	bool isValidVertexIndex(int i, int j, int n);
	bool isValidTriangleIndex(int i);
	bool isValidNormalIndex(int index);
	bool isValidNormalIndex(int i, int j, int k);
	FVector createNormal(int v0, int v1, int v2);

	void findTrianglesInvolvedWith(int index, std::vector<int> &trianglesFound);
	bool isPartOfTraingle(int target, int v0, int v1, int v2);

	bool solveIsInTriangle(
		int v0, int v1, int v2, FVector &target
	);
	

	void addTriangle(int v0, int v1, int v2);
	void addTriangleDoublesided(int v0, int v1, int v2);
	void splitTriangleInHalf(int v0, int v1, int v2);
	bool trianglesAreSame(int v0, int v1, int v2, int v00, int v01, int v02);
	int removeTriangleSimilarTo(int v0, int v1, int v2);

	//helper for removing triangles by vertex
	void removeVertex(int index);
	virtual void removeVertex(int index, std::vector<int> &connectedvertecies);
	void removeTrianglesInvolvedWith(int vertexIndex, std::vector<int> &connectedvertecies);
	bool contains(std::vector<int> &ref, int index);


public:
	
	materialEnum targetMaterial();
	void setTargetMaterial(materialEnum inMaterial);

	void generateMatricesPerFaceAndLookDirOfNormal(std::vector<MMatrix> &output);

	void generateMatricesPerFaceAndLookDirOfNormalInterpolated(
		std::vector<MMatrix> &output,
		int stepSize
	);

	int verteciesNum();
	bool hasAnyVertecies();

	//helper for displacement
	void pushInwards(FVector &location, int radius, FVector scaleddirection);

	void debugDrawMesh(MMatrix &transform, UWorld *world);

	void flipAllTriangles();

protected:
	void findConnectedVerteciesTo(int index, std::vector<int> &output);




	void updateBoundsIfNeeded();
	void updateBoundsIfNeeded(FVector &other);

	bool isInsideBoundingbox(FVector &other);



/**
 * 2D section
 */
public:
	void generate(int sizeX, int sizeY, int distanceXY);

	FVector &findIndex(int i, int j);
	void replaceAt(int i, int j, FVector &other);

protected:
	int indexFor(int i, int j);

	FVector noneVertex;
	int umbruch = 0;



/**
 * road section
 */
public:
	void appendParalellLinesClosedAsQuads(
		TArray<FVector> &line1,
		TArray<FVector> &line2
	);



};
