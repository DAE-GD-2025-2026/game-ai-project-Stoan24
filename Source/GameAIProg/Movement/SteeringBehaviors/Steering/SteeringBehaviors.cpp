#include "SteeringBehaviors.h"

#include <ThirdParty/Imath/Deploy/Imath-3.1.12/include/Imath/ImathMath.h>

#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	const FVector2D displacement = Target.Position - Agent.GetPosition();
	output.LinearVelocity = displacement / DeltaTime;

	const float angle = Target.Orientation - Agent.GetRotation();
	output.AngularVelocity = angle / DeltaTime;

	return output;
}

//FLEE
//*******
SteeringOutput Flee::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	const FVector2D displacement = Target.Position - Agent.GetPosition();
	output.LinearVelocity = -displacement / DeltaTime;

	const float angle = Target.Orientation - Agent.GetRotation();
	output.AngularVelocity = -angle / DeltaTime;

	return output;
}

//Arrive
//*******
SteeringOutput Arrive::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	
	return output;
}

//FACE
//*******
SteeringOutput Face::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;
	

	return output;
}

//Pursuit
//*******
SteeringOutput Pursuit::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;


	return output;
}

//EVADE
//*******
SteeringOutput Evade::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;


	return output;
}

//Wander
//*******
SteeringOutput Wander::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;


	return output;
}