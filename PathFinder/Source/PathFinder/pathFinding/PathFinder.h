// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <map>
#include "PathFinder.generated.h"

class raycastTask;

/**
 * a modified a* version with tangential checks along convex polygons for minimzed graphs.
 * Nodes are found in O(1)!
 * 
 * 2 modes:
 * 
 * 1)
 * which operates on subgraphs and rechecks edges on runtime
 * the edges are build and checked on runtime because we want to operate on subgraphs
 * efficently. Also it adds supports for dynamic actors like vehicles blocking paths temporarily
 * 
 * it also eliminates all non tangential edges during runtime because they are never part 
 * of a shortest path!
 * 
 * 2)
 * A node which will prebuild all tangential edges when adding a node
 * toggle the according boolean in this header file:
 * will automatically connect nodes, reduces runtime overhead because minimal tangential graph is already build
 * 
 */

UCLASS()
class PATHFINDER_API APathFinder : public AActor
{
	GENERATED_BODY()

public:
	//---- Launch API ----
	APathFinder();
	static void makeInstance(UWorld *world, FString worldLevelName); // CALL TO LAUNCH
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float deltatime) override;

private:
	static void resetPathFinderPointer();
	static bool alreadyHasInstance();
	void Setup(FString worldname);

	static class APathFinder *pathFinderInstance;

	FString worldLevelNameSaved;

	std::vector<FVector> prevPath;
	static constexpr int CHUNKSIZE = 2000; // 1m = 100, 20m = 2000
	static constexpr int ONE_METER = 70; //distance to keep between nodes
	static int countNodes;

public:



	static const bool debugDrawNodes = true; //false

	

	FCollisionQueryParams collsionParamsLowDetailAndFast();

	static APathFinder *instance(UWorld *worldIn);
	static APathFinder *instance();
	//static void deleteInstance();

	void debugShowAllNodes(UWorld *world);

	void clear(); //clears ALL NODES

	void addNewNodeVector(std::vector<FVector> &vec, FVector &offset);
	void addNewNodeVector(std::vector<FVector> &vec, std::vector<FVector> &offsets);
	void addNewNodeVector(std::vector<FVector> &vec);
	void addNewNode(FVector a);

	void addConvexHull(std::vector<FVector> &vec);

	std::vector<FVector> getPath(FVector a, FVector b);

	FVector findFurthestConnectedNodeFrom(FVector &other);

	enum class PathTraceMode
	{
		SyncTrace,
		SyncTraceByTick,
		AsyncTrace
	};

	class Node
	{
		public:
			static const int noneFx = -1;

			/// @brief will tell if the node is closed (on the closed list) or not
			bool closedFlag;
			/// @brief came from neighbor
			APathFinder::Node *camefrom = nullptr;
			float fx;
			float gx;
			FVector pos;

			
			

			Node(FVector posIn); //no id: setup needed
			Node(int id, FVector posIn); //id from storage

			Node(Node &other);
			Node &operator=(Node &other);

			~Node();
			void reset();
			void updateCameFrom(float gxIn, float hxEnd, Node &came);
			void close();
			bool isClosed();

			float oldfx;

			void setConvexNeighborA(Node *n);
			void setConvexNeighborB(Node *n);
			void addTangentialNeighbor(Node *n);

			//convex neighbors A and B
			APathFinder::Node *nA = nullptr;
			APathFinder::Node *nB = nullptr;

			bool hasNeighbors(); //convex hull neighbors
			bool hasAnyNeighbors(); //any visible neighbors 

			std::vector<Node *> visible_tangential_Neighbors;

			// new: hull index
			int hullindex = -1;
			bool sameHull(Node *other);

			void show(UWorld *world);

			//storage interface helpers
			void setId(int id);
			int getId();

			//only returns valid ids
			TArray<int> NeighborsById();

			//returns -1 if not valid
			int IdConvexNeighborA();
			int IdConvexNeighborB();

		private:
			FCriticalSection CriticalSection;

			//id system for saving
			int id = -1; //is invalid by default, not tracked in storage.
	};

	void addNode(APathFinder::Node *node);
	
	void debugCountNodes();

	void draw(FVector &pos);

	bool passTangentailCheck(Node *a, Node *b);

private:
	

	bool reached(APathFinder::Node *a, APathFinder::Node *b);
	

	
	void screenMessage(int s);
	void screenMessage(FString s);

	class Chunk{
		public:
			/// @brief is a vector of pointers in case the vector is copied internally
			/// and nodes must stay active while path finding
			std::vector<APathFinder::Node*> nodes;
			Chunk();
			~Chunk();
			void add(FVector vec);
			void add(Node *node);

			//add a node without connecting it.
			//designed to be called when using storage
			void addNoConnect(Node *node);

			std::vector<APathFinder::Node *> &getNodes();
			APathFinder::Node *findNode(FVector pos);
			APathFinder::Node *findNodeInDirection(FVector &node, FVector &dir);

			bool hasNode(FVector pos);

			void clear();

			APathFinder::Node *lateadd(FVector pos);

			void debugShowAllNodes(UWorld *world);

			// new:
			//std::vector<PathFinder::ConvexPolygon *> polygons;
	};

	class Quadrant{
		private:
			int xSample;
			int ySample;

		public:
			std::vector<std::vector<APathFinder::Chunk*>> map;
			Quadrant(int xSampleIn, int zSampleIn);
			~Quadrant();

			Node *findNode(FVector pos);
			Node *findNodeInDirection(FVector &node, FVector &dir);
			std::vector<APathFinder::Node *> nodesEnClosedBy(float xA, float zA, float xB, float zB);

			std::vector<APathFinder::Node *> askForArea(FVector a, FVector b);

			void add(FVector n);
			void add(Node *node);

			//add a new node without connection
			//designed to be called when using storage
			void addNoConnect(Node *node);

			void clear();

			void fillMapTo(int xIndex, int yIndex);

			void debugShowAllNodes(UWorld *world);

			int chunkCount();
	};

	class Quadrant *TopRight;
	class Quadrant *BottomRight;
	class Quadrant *TopLeft;
	class Quadrant *BottomLeft;

	Quadrant *askforQuadrant(int xIndex, int zIndex);

	//A node is Found in O(1)!
	Node *findNode(FVector pos);
	Node *findNodeInDirection(FVector &node, FVector &dir);

	void showPos(FVector e);
	void showPos(FVector e, FColor c);

	
	

	float distance(Node *A, Node *B);
	float distance(FVector A, FVector B);

	

	std::vector<APathFinder::Node *> getSubGraph(FVector a, FVector b);

	std::vector<FVector> findPath(
		Node *start,
		Node *end,
		std::vector<APathFinder::Node *> &subgraph
	);

	std::vector<FVector> constructPath(
		Node *end
	);

	bool canSeeTangential(APathFinder::Node *A, APathFinder::Node *B);
	bool canSee(FVector &a, FVector &b);

	bool isCloseAndTooVertical(Node *a, Node *b);

	int traceCount;

	/// @brief will tell whether the graph is build if nodes are added at any time
	static constexpr bool PREBUILD_EDGES_ENABLED = true;

	/// @brief will tell whether the raycast for adding nodes will be async or an synchron operation
	static constexpr bool ASYNC_EDGE_PREBUILDING = true;
	static constexpr int PREBUILD_MAXDISTANCE = 5000; // 10000 / 100 = 100 meter, keep to 50.
	void connect(APathFinder::Node *node);
	void asyncCanSee(Node *a, Node *b);

	std::vector<FVector> findPath_prebuildEdges(
		Node *start,
		Node *end
	);

	bool isInBounds(FVector &a, FVector &b, APathFinder::Node *check);


	FCriticalSection fillQuadrant_CriticalSection;
	FCriticalSection delegate_CriticalSection_a; 
	FCriticalSection delegate_CriticalSection_b;
	
	std::vector<FTraceDelegate *> released;
	FTraceDelegate *requestDelegate(Node *a, Node *b);

	void clearDelegates();

public:
	void freeDelegate(FTraceDelegate *d);

	PathTraceMode traceMode = PathTraceMode::AsyncTrace;



	void addActorToIgnoreRaycastParams(AActor *actor);
	FCollisionQueryParams getIgnoredRaycastParams();

private:
	void addNewRaytask(Node *a, Node *b);
	std::vector<raycastTask> rayTasksVec;


	//ignored actors
	FCollisionQueryParams collisionIgnoreParams;




	// --- STORAGE INTERFACE ---

	//map to save by id: load and save
	//static id must start from 0, -1 is a non available node as convex partner
	int staticId = 0;

	//save nodes by id to save adjacent nodes relation by id.
	std::map<int, Node *> idMappedNodes;

	//will track the node inside the id map
	void TrackNodeInIdMap(APathFinder::Node *node);

	//will return whether a node is tracked in the id map
	bool NodeIsTrackedInIdMap(int id);

public:
	static bool IdIsValid(int id);

	//adds a node to the graph without connecting it at all
	//is only saved if id is valid
	void addNodeFromStorageInterfaceNoConnection(FVector &pos, int id);

	//adds all connections and convex neighbors to a given node by id - if id is valid
	void addConnectionsFromStorageInterfaceForNodeById(
		int id,
		TArray<int> connected,
		int convexA,
		int convexB
	);

	//returns the count of nodes which supposed to be saved
	int countNodesTrackedInIdMap();

	const std::map<int, APathFinder::Node *> &IdMapReference();

	// --- STORAGE INTERFACE Section End ---


};
