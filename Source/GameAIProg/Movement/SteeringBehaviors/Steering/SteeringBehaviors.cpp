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

	//Save max speed
	if (!m_SpeedIsSaved)
	{
		m_MaxSpeed = Agent.GetMaxLinearSpeed();
		m_SpeedIsSaved = true;
	}

	const FVector2D direction = Target.Position - Agent.GetPosition();
	output.LinearVelocity = direction / DeltaTime;


	const float slowRadius = 500.f;
	const float TargetRadius = 100.f;

	float distance = direction.Size();

	
	if (distance > slowRadius)
	{
		Agent.SetMaxLinearSpeed(m_MaxSpeed);
	}
	else if (distance <= TargetRadius)
	{
		Agent.SetMaxLinearSpeed(0.f);
	}
	else
	{
		//Calculate where we are between the 2 radii
		float alpha = (distance - TargetRadius) / (slowRadius - TargetRadius);

		alpha = FMath::Clamp(alpha, 0.f, 1.f);

		float speed = m_MaxSpeed * alpha;
		Agent.SetMaxLinearSpeed(speed);
	}


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

	//Calculate distance
	const FVector2D direction = Target.Position - Agent.GetPosition();
	const float distance = direction.Size();

	//Calculate time
	float time = distance / Agent.GetMaxLinearSpeed();
	time = FMath::Min(time, 5.0f);

	//Calculate where the will be after the time
	const FVector2D predictedPosition = Target.Position + (Target.LinearVelocity * time);

	//Use that position to calculate where to 'SEEK' to
	const FVector2D desiredPosition = predictedPosition - Agent.GetPosition();

	output.LinearVelocity = desiredPosition / DeltaTime;

	return output;
}

//EVADE
//*******
SteeringOutput Evade::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	//Calculate distance
	const FVector2D direction = Target.Position - Agent.GetPosition();
	const float distance = direction.Size();

	//Calculate time
	float time = distance / Agent.GetMaxLinearSpeed();
	time = FMath::Min(time, 5.0f);

	//Calculate where the will be after the time
	const FVector2D predictedPosition = Target.Position + (Target.LinearVelocity * time);

	//Use that position to calculate where to 'FLEE' from
	const FVector2D desiredPosition = predictedPosition - Agent.GetPosition();

	output.LinearVelocity = -desiredPosition / DeltaTime;

	return output;
}

//Wander
//*******
SteeringOutput Wander::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	SteeringOutput output;

	

	return output;
}