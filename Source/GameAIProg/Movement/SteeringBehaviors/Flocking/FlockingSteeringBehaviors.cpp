#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	if (pFlock->GetNrOfNeighbors() == 0) return {};

	//Get average position
	const FVector2D avgPosition = pFlock->GetAverageNeighborPos();
	Target.Position = avgPosition;


	auto output = Seek::CalculateSteering(deltaT, pAgent);
	output.IsValid = true;
	return output;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering = {};
	const auto& neighbors = pFlock->GetNeighbors();
	const int nrOfNeighbors = pFlock->GetNrOfNeighbors();

	if (nrOfNeighbors == 0)
		return steering;

	for (int i = 0; i < nrOfNeighbors; ++i)
	{
		FVector2D ToAgent = pAgent.GetPosition() - neighbors[i]->GetPosition();
		const float distance = ToAgent.Size();

		if (distance > 0)
		{
			//Inversely proportional
			float pushStrength = 1.0f / distance;
			ToAgent.Normalize();
			steering.LinearVelocity += ToAgent * pushStrength;
		}
	}

	if (!steering.LinearVelocity.IsNearlyZero())
	{
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent.GetMaxLinearSpeed();
		steering.IsValid = true;
	}

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	if (pFlock->GetNrOfNeighbors() == 0) return {};

	const FVector2D avgVelocity = pFlock->GetAverageNeighborVelocity();

	SteeringOutput steering = {};
	steering.LinearVelocity = avgVelocity;

	if (!steering.LinearVelocity.IsNearlyZero())
	{
		steering.LinearVelocity.Normalize();
		steering.LinearVelocity *= pAgent.GetMaxLinearSpeed();
		steering.IsValid = true;
	}

	return steering;
}