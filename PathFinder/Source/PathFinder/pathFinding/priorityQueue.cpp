// Fill out your copyright notice in the Description page of Project Settings.


#include "PathFinder/pathFinding/PathFinder.h"
#include <map>
#include "GameCore/DebugHelper.h"
#include "priorityQueue.h"

priorityQueue::priorityQueue()
{
}

priorityQueue::~priorityQueue()
{
}

/// @brief will freshly add a node OR bubble up the node if existent!
/// @param node node to add or refresh
void priorityQueue::add(APathFinder::Node *node){
    if(node != nullptr){
        //(am letzten index einfügen, bubble up)
        // solange das über mir grösser ist, swap

        if(indexMap.find(node) != indexMap.end()){
            int index = indexMap[node];
            bubbleUpfrom(index); //refresh
            //DebugHelper::showScreenMessage("udpate node", node->fx, FColor::Blue);
        }
        else
        {
            //DebugHelper::showScreenMessage("add node", node->fx, FColor::Blue);
            nodes.push_back(node);
            indexMap[node] = nodes.size() - 1; //add with new index
            bubbleUpfrom(nodes.size() - 1);
        }
    }

    //show();
}

void priorityQueue::bubbleUpfrom(int index){
    if (nodes.size() > 1 && index > 0 && index < nodes.size()){

        float copyfx = nodes.at(index)->fx;

        
        int parentIndex = (int)((index - 1) / 2); // abrunden default

        
        while (parentIndex >= 0){
            APathFinder::Node *parentNode = nodes.at(parentIndex);
            if(parentNode != nullptr){
                if(parentNode->fx > copyfx){
                    //swap
                    APathFinder::Node *tmp = parentNode;
                    nodes.at(parentIndex) = nodes.at(index);
                    nodes.at(index) = tmp;

                    //replot in index map, updated positions
                    indexMap[nodes.at(parentIndex)] = parentIndex;
                    indexMap[nodes.at(index)] = index;

                    //next level
                    index = parentIndex; //one up
                    parentIndex = (int)((parentIndex - 1) / 2); //one up

                    

                }else{
                    return;
                }
            }else{
                return;
            }
        }
    }
}








APathFinder::Node *priorityQueue::popLowestFx(){
    if(hasNodes()){
        APathFinder::Node *front = nodes.front();
        //remove from index map
        if(front != nullptr){
            indexMap.erase(front);
        }
        downheap();

        if(front != nullptr){
            if(showScreenLog) DebugHelper::showScreenMessage("popped fx", front->fx);
        }

        return front;
    }
    return nullptr;
}

void priorityQueue::downheap(){
    //special cases
    if(nodes.size() == 1){
        //only one node
        nodes.erase(nodes.begin());
        return;
    }

    //default
    if(nodes.size() > 1){
        nodes.at(0) = nodes.back(); //replace first
        nodes.erase(nodes.begin() + nodes.size() - 1); //rmv last
    }

    

    int parentIndex = 0;
    int left = 1;
    int right = left + 1;

    while(true){
        left = 1 + (parentIndex * 2);
        right = left + 1;

        //index out of bounds
        if(parentIndex >= nodes.size() || left >= nodes.size()){
            return;
        }

        // bubble down swap mit left or right (0 based bin heap)
        // child left = 1 + i * 2 
        // child right = left + 1;

        int lowerIndex = left;
        if (right < nodes.size()) //if right exists
        {
            //better fx child
            lowerIndex = nodes.at(left)->fx < nodes.at(right)->fx ? left : right;
        }

        //check the lower to get up
        //parent fx is larger than smaller childs fx
        if(nodes.at(parentIndex)->fx > nodes.at(lowerIndex)->fx){
            //swap
            APathFinder::Node *parentCopy = nodes.at(parentIndex);
            nodes.at(parentIndex) = nodes.at(lowerIndex);
            nodes.at(lowerIndex) = parentCopy;

            //update index map
            indexMap[nodes.at(parentIndex)] = parentIndex;
            indexMap[nodes.at(lowerIndex)] = lowerIndex;

            //next level
            parentIndex = lowerIndex;
            
        }else{
            return;
        }
    }

    
}

bool priorityQueue::hasNodes(){
    return nodes.size() > 0;
}




void priorityQueue::show(){
    if(hasNodes()){
        FString output = TEXT("nodes: \n");
        FString breakN = TEXT(" - ");
        for (int i = 0; i < nodes.size(); i++)
        {
            APathFinder::Node *n = nodes.at(i);
            if(n != nullptr){
                output += FString::Printf(TEXT("%.2f"), nodes.at(i)->fx);
                output += breakN;
            }
        }

        DebugHelper::showScreenMessage(output, FColor::Green);
    }else{
        DebugHelper::showScreenMessage("no nodes in queue! ", FColor::Green);
    }
}