#include "SteeringBehaviors.h"

#include <ThirdParty/Imath/Deploy/Imath-3.1.12/include/Imath/ImathMath.h>

#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
SteeringOutput Seek::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	const FVector2D direction = Target.Position - Agent.GetPosition();
	output.LinearVelocity = direction / DeltaTime;

	return output;
}

//FLEE
//*******
SteeringOutput Flee::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	const FVector2D direction = Target.Position - Agent.GetPosition();
	output.LinearVelocity = -direction / DeltaTime;

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
	const FVector2D direction = Target.Position - Agent.GetPosition();

	float targetAngle = FMath::RadiansToDegrees((FMath::Atan2(direction.Y, direction.X)));

	float currentAngle = Agent.GetActorRotation().Yaw;

	output.AngularVelocity = FMath::FindDeltaAngleDegrees(currentAngle, targetAngle) / DeltaTime;

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