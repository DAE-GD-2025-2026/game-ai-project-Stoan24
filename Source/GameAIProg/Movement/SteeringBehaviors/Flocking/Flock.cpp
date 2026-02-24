#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"
#include <vector>


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

	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>());

	for (int i = 0; i < FlockSize; i++)
	{

		Agents.Add(pWorld->SpawnActor<ASteeringAgent>(AgentClass, FVector{ -500, 0, 90 }, FRotator::ZeroRotator));
	}

	pPrioritySteering = std::make_unique<PrioritySteering>();

	// Add behaviors to BlendedSteering (Cohesion, Separation, Alignment/VelMatch)
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
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
		//Register neighbors
		RegisterNeighbors(pAgent);

		//Calculate steering
		auto steeringOutput = pPrioritySteering->CalculateSteering(DeltaTime, pAgent);

		pAgent->Update(DeltaTime, steeringOutput);

		// Step 3: Enforce world boundaries
		//pAgent->TrimToWorld(WorldSize, bTrimWorld);
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
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

		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
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
		avgVelocity += Neighbors[i]->GetVelocity();
	}
	return avgVelocity / static_cast<float>(NrOfNeighbors);
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
}

