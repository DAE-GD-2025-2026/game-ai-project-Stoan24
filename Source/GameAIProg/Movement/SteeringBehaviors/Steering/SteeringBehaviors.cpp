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
SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput output{};
	output.IsValid = true;

	FVector const AgentPos = Agent.GetActorLocation();
	FVector2D const Direction = FVector2D(Target.Position.X - AgentPos.X, Target.Position.Y - AgentPos.Y).GetSafeNormal();

	FVector const Forward = Agent.GetActorForwardVector();
	FVector2D const CurrentDirection = FVector2D(Forward.X, Forward.Y).GetSafeNormal();

	float const Dot = FVector2D::DotProduct(CurrentDirection, Direction);
	float const Cross = FVector2D::CrossProduct(CurrentDirection, Direction);

	if (FMath::IsNearlyEqual(Dot, 1.0f))
	{
		output.AngularVelocity = 0.0f;
	}
	else
	{
		output.AngularVelocity = (Cross > 0) ? 1.0f : -1.0f;
	}

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
	output.IsValid = false;

	const FVector2D directionToTarget = Target.Position - Agent.GetPosition();
	const float distance = directionToTarget.Size();

	//Only evade if the seeker is in the circle
	if (distance > evadeRadius)
	{
		return output;
	}

	float agentMaxSpeed = Agent.GetMaxLinearSpeed();
	float time = (agentMaxSpeed > 0) ? (distance / agentMaxSpeed) : 0.f;
	time = FMath::Min(time, 5.0f);

	const FVector2D predictedPosition = Target.Position + (Target.LinearVelocity * time);

	FVector2D escapeDirection = Agent.GetPosition() - predictedPosition;

	if (!escapeDirection.IsNearlyZero())
	{
		escapeDirection.Normalize();
		output.LinearVelocity = escapeDirection * agentMaxSpeed;
		output.IsValid = true;
	}

	return output;
}

//Wander
//*******
SteeringOutput Wander::CalculateSteering(float DeltaTime, ASteeringAgent& Agent)
{
	WanderAngle += FMath::FRandRange(-MaxAngleChange, MaxAngleChange);

	FVector const AgentForward = Agent.GetActorForwardVector();
	FVector2D const Forward2D{ AgentForward.X, AgentForward.Y };

	FVector2D const CircleCenter = Agent.GetPosition() + (Forward2D * Offset);

	FVector2D const Displacement{ Radius * FMath::Cos(WanderAngle), Radius * FMath::Sin(WanderAngle) };

	Target.Position = CircleCenter + Displacement;

	auto output = Seek::CalculateSteering(DeltaTime, Agent);
	output.IsValid = true;
	return output;
}