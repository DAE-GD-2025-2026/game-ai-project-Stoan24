#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"
#include <vector>

#include "Animation/AnimSequenceHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	Agents.SetNum(FlockSize);

	Neighbors.Reserve(FlockSize);
	Neighbors.SetNum(FlockSize);

	//Behaviors
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pSeparationBehavior = std::make_unique<Separation>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);

	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();

	std::vector<BlendedSteering::WeightedBehavior> blendedBehaviors{};
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pCohesionBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeparationBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pVelMatchBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeekBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pWanderBehavior.get(), 0.5f));

	pBlendedSteering = std::make_unique<BlendedSteering>(blendedBehaviors);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int i = 0; i < FlockSize; i++)
	{
		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{ -500, 0, 90 }, FRotator::ZeroRotator, SpawnInfo);
		Agents[i]->SetSteeringBehavior(pBlendedSteering.get());
	}

	//pPrioritySteering = std::make_unique<PrioritySteering>();

	if (bTrimWorld)
	{
		TrimWorld = pWorld->SpawnActor<AWorldTrimVolume>(FVector{ 0,0,0 }, FRotator::ZeroRotator);
	}
}

Flock::~Flock()
{
	for (auto pAgent : Agents)
	{
		if (pAgent) pAgent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world

	for (ASteeringAgent* pAgent : Agents)
	{
		if (!pAgent) continue;

		RegisterNeighbors(pAgent);
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
	for (ASteeringAgent* pAgent : Agents)
	{
		if(pAgent && DebugRenderSteering)
		{
			FVector loc = pAgent->GetActorLocation();
			FVector vel = pAgent->GetVelocity();

			DrawDebugDirectionalArrow(pWorld, loc, loc + vel,
				50.f, FColor::Green, false, -1.f, 0, 2.f);
		}
	}

	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();
	}
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}

		ImGui::Spacing();

		ImGui::Checkbox("Debug Steering", &DebugRenderSteering);
		ImGui::Checkbox("Debug Neighborhood", &DebugRenderNeighborhood);

		ImGui::Spacing();
		ImGui::Separator();
		

  // TODO: implement ImGUI sliders for steering behavior weights here

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Cohesion",
			pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Separation",
			pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("VelocityMatch",
			pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[2].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
			pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[3].Weight = InVal; }, "%.2f");

		ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
			pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight, 0.f, 1.f,
			[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[4].Weight = InVal; }, "%.2f");

		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
	if (Agents.Num() > 0 && Agents[0])
	{
		FVector center = Agents[0]->GetActorLocation();

		//Neighborhood Radius
		DrawDebugCircle(pWorld, center, NeighborhoodRadius, 72,
			FColor::Cyan, false, -1.f, 0, 2.f, FVector(0, 1, 0), FVector(1, 0, 0), false);

		RegisterNeighbors(Agents[0]);

		//Lines to all current neighbors
		for (int i = 0; i < NrOfNeighbors; ++i)
		{
			if (Neighbors[i])
			{
				DrawDebugLine(pWorld, center, Neighbors[i]->GetActorLocation(),
					FColor::Green, false, -1.f, 0, 1.f);
			}
		}
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING

void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
	NrOfNeighbors = 0;

	for (ASteeringAgent* pOtherAgent : Agents)
	{
		if (pOtherAgent == pAgent) continue;

		//Check if within Neighborhood Radius
		float distance = FVector2D::Distance(pAgent->GetPosition(), pOtherAgent->GetPosition());
		if (distance < NeighborhoodRadius)
		{
			//Overwrite the pointer
			Neighbors[NrOfNeighbors] = pOtherAgent;
			NrOfNeighbors++;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	FVector2D avgPosition = FVector2D::ZeroVector;
	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		avgPosition += Neighbors[i]->GetPosition();
	}
	return avgPosition / static_cast<float>(NrOfNeighbors);
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	FVector2D avgVelocity = FVector2D::ZeroVector;
	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		avgVelocity += FVector2D(Neighbors[i]->GetVelocity());
	}
	return avgVelocity / static_cast<float>(NrOfNeighbors);
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
	if (pSeekBehavior)
	{
		FTargetData targetData;
		targetData.Position = Target.Position;
		pSeekBehavior->SetTarget(targetData);
	}
}

