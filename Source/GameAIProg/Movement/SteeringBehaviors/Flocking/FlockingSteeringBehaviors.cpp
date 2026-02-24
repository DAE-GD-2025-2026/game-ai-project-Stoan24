#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};

	//Get average position
	const FVector2D avgPos = pFlock->GetAverageNeighborPos();

	if (avgPos.IsNearlyZero())
		return steering;

	//Seek average position
	steering.LinearVelocity = avgPos - pAgent.GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.GetMaxLinearSpeed();

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};
	const auto& neighbors = pFlock->GetNeighbors();
	const int nrOfNeighbors = pFlock->GetNrOfNeighbors();

	for (int i = 0; i < nrOfNeighbors; ++i)
	{
		FVector2D vecToAgent = pAgent.GetPosition() - neighbors[i]->GetPosition();
		const float distance = vecToAgent.Size();

		if (distance > 0)
		{
			float forceStrength = 1.0f / distance;
			vecToAgent.Normalize();
			steering.LinearVelocity += vecToAgent * forceStrength;
		}
	}

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.GetMaxLinearSpeed();

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};

	//Get average velocity
	const FVector2D avgVel = pFlock->GetAverageNeighborVelocity();

	if (avgVel.IsNearlyZero())
		return steering;

	//Match velocity
	steering.LinearVelocity = avgVel;

	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.GetMaxLinearSpeed();

	return steering;
}