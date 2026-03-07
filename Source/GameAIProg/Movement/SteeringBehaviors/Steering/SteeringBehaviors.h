#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	void SetTargetRadius(float radius) { TargetRadius = radius; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
	float TargetRadius;
};

class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	//Seek Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

};


class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() override = default;

	//Flee Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

};

class Arrive : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() override = default;

	//Arrive Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

private:
	float m_MaxSpeed = 0.f;
	bool m_SpeedIsSaved = false;
};

class Face : public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() override = default;

	//Face Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

};

class Pursuit : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() override = default;

	//Pursuit Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

};

class Evade : public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() override = default;

	//Evade Behaviour
	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

private:

	float evadeRadius{ 500.f };
	
};

class Wander : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() override = default;

	virtual SteeringOutput CalculateSteering(float DeltaTime, ASteeringAgent& Agent) override;

protected:
	float Offset = 200.0f;         // Distance to the circle center
	float Radius = 100.0f;         // Radius of the circle
	float MaxAngleChange = 0.5f;   // Max jitter per frame (in radians)
	float WanderAngle = 0.0f;      // Current direction on the circle
};
