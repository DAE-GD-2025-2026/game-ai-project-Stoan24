#pragma once
#include "Movement/SteeringBehaviors/Steering/SteeringBehaviors.h"
class Flock;

//COHESION - FLOCKING
//*******************
class Cohesion final : public Seek
{
public:
	explicit Cohesion(Flock* const pFlock) :pFlock(pFlock) {};

	//Cohesion Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* pFlock = nullptr;
};

//SEPARATION - FLOCKING
//*********************
class Separation final : public Flee
{
public:
	explicit Separation(Flock* const pFlock) :pFlock(pFlock) {};

	//Cohesion Behavior
	virtual SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent) override;

private:
	Flock* pFlock = nullptr;
};
//VELOCITY MATCH - FLOCKING
//************************
class VelocityMatch final
{
public:
	explicit VelocityMatch(Flock* const pFlock) :pFlock(pFlock) {};

	//Cohesion Behavior
	SteeringOutput CalculateSteering(float deltaT, ASteeringAgent& pAgent);

private:
	Flock* pFlock = nullptr;
};
