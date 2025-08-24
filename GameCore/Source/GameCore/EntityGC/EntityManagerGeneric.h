// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * Entity Manager -> this class is an individual object pool for a type parameter <T>
 * T must be derived from AActor and will only accept pointers in and out
 */
template <typename T>
class GAMECORE_API EntityManagerGeneric{
	//static_assert(std::is_base_of<AActor, T>::value, "T must be derived from AActor");

public:
	EntityManagerGeneric(){
		
	}
	~EntityManagerGeneric(){
		actorVector.clear();
	}

	
	void add(T *actor){
		if(actor != nullptr){
			for (int i = 0; i < actorVector.size(); i++){
				if(actorVector.at(i) == actor){
					return; //duplicate found
				}
			}
			actorVector.push_back(actor);
	
			//DebugHelper::showScreenMessage("released an entity! ", FColor::Yellow);
		}
	}

	void erase(T *actor){
		if(actor != nullptr){
			for (int i = 0; i < actorVector.size(); i++){
				if(actorVector.at(i) == actor){
					actorVector.erase(actorVector.begin() + i);
					return;
				}
			}
		}
	}

	T *getFirstActor(){
		if(hasActorsLeft()){
			T *actor = actorVector.back(); //get last elements, first would do shifting elements(bad)
			actorVector.pop_back();
			//DebugHelper::showScreenMessage("popped an entity! ", FColor::Yellow);
			return actor;
		}
		return nullptr;
	}

	bool hasActorsLeft(){
		return actorVector.size() > 0;
	}

private:
	/// @brief ector vector T* pointers
	std::vector<T *> actorVector;


};
