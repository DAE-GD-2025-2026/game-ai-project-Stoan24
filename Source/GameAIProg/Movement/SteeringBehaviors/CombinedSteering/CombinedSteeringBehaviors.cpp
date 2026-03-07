
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{}

BlendedSteering::~BlendedSteering()
{
}

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput blendedOutput = {};
	float totalWeight = 0.f;

	for (const auto& wb : WeightedBehaviors)
	{
		if (wb.pBehavior)
		{
			auto output = wb.pBehavior->CalculateSteering(DeltaT, Agent);

			if (output.IsValid)
			{
				output.LinearVelocity.Normalize();

				blendedOutput.LinearVelocity += output.LinearVelocity * wb.Weight;
				totalWeight += wb.Weight;
			}
		}
	}

	if (totalWeight > 0.f)
	{
		blendedOutput.LinearVelocity.Normalize();
		blendedOutput.LinearVelocity *= Agent.GetMaxLinearSpeed();
		blendedOutput.IsValid = true;
	}
	else
	{
		blendedOutput.IsValid = false;
	}

	return blendedOutput;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			break;
	}

	//If none of the behaviors return a valid output, last behavior is returned
	return Steering;
}