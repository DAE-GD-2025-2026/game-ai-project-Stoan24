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

#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.Reserve(FlockSize);
	Neighbors.SetNum(FlockSize);
#endif

	//Behaviors
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pSeparationBehavior = std::make_unique<Separation>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);

	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();

	//Blended
	std::vector<BlendedSteering::WeightedBehavior> blendedBehaviors{};
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pCohesionBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeparationBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pVelMatchBehavior.get(), 0.5f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeekBehavior.get(), 0.0f));
	blendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pWanderBehavior.get(), 1.0f));

	pBlendedSteering = std::make_unique<BlendedSteering>(blendedBehaviors);

	//Priority
	std::vector<ISteeringBehavior*> priotrityBehaviors{};
	priotrityBehaviors.push_back(pEvadeBehavior.get());
	priotrityBehaviors.push_back(pBlendedSteering.get());

	pPrioritySteering = std::make_unique<PrioritySteering>(priotrityBehaviors);


#ifdef GAMEAI_USE_SPACE_PARTITIONING
	float fullSize = WorldSize;

	pPartitionedSpace = std::make_unique<CellSpace>(
		pWorld,
		fullSize, fullSize,
		NrOfCellsX, NrOfCellsX,
		FlockSize
	);
#endif

	//Spawning
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	for (int i = 0; i < FlockSize; i++)
	{
		float randomX = FMath::RandRange(-WorldSize, WorldSize);
		float randomY = FMath::RandRange(-WorldSize, WorldSize);
		FVector spawnLocation = FVector{ randomX, randomY, 90.f };

		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, spawnLocation, FRotator::ZeroRotator, SpawnInfo);
		if (Agents[i]) // Safety check
		{
			Agents[i]->SetSteeringBehavior(pPrioritySteering.get());

#ifdef GAMEAI_USE_SPACE_PARTITIONING
			pPartitionedSpace->AddAgent(*Agents[i]);
			OldPositions.Add(Agents[i]->GetPosition());
#endif
		}
	}

	if (bTrimWorld)
	{
		TrimWorld = pWorld->SpawnActor<AWorldTrimVolume>(FVector{ 0,0,0 }, FRotator::ZeroRotator);
	}
}

Flock::~Flock()
{
	for (const auto pAgent : Agents)
	{
		if (pAgent) pAgent->Destroy();
	}
}

void Flock::Tick(float DeltaTime)
{
	if (pAgentToEvade && pEvadeBehavior)
	{
		FTargetData targetData;
		targetData.Position = pAgentToEvade->GetPosition();
		targetData.LinearVelocity = FVector2D(pAgentToEvade->GetVelocity());
		pEvadeBehavior->SetTarget(targetData);
	}

	for (int i = 0; i < Agents.Num(); ++i)
	{
		ASteeringAgent* pAgent = Agents[i];
		if (!pAgent) continue;

		//Trim
		FVector position = pAgent->GetActorLocation();
		const float limit = TrimWorld ? TrimWorld->GetTrimWorldSize() : 1500.f;

		bool bMoved = false;

		if (position.X > limit) { position.X = -limit; bMoved = true; }
		else if (position.X < -limit) { position.X = limit; bMoved = true; }


		if (position.Y > limit) { position.Y = -limit; bMoved = true; }
		else if (position.Y < -limit) { position.Y = limit; bMoved = true; }

		if (bMoved)
		{
			pAgent->SetActorLocation(position);
		}

#ifdef GAMEAI_USE_SPACE_PARTITIONING

		pPartitionedSpace->UpdateAgentCell(*pAgent, OldPositions[i]);

		pPartitionedSpace->RegisterNeighbors(*pAgent, NeighborhoodRadius);

		OldPositions[i] = pAgent->GetPosition();

#else

		RegisterNeighbors(pAgent);

#endif
	}
}

void Flock::RenderDebug()
{
	for (ASteeringAgent* pAgent : Agents)
	{
		if(pAgent && DebugRenderSteering)
		{
			FVector location = pAgent->GetActorLocation();
			FVector velocity = pAgent->GetVelocity();

			DrawDebugDirectionalArrow(pWorld, location, location + velocity,
				50.f, FColor::Green, false, -1.f, 0, 2.f);
		}
	}

	if (DebugRenderNeighborhood)
	{
		RenderNeighborhood();
	}

	if (pAgentToEvade)
	{
		FVector evaderLocation = pAgentToEvade->GetActorLocation();

		DrawDebugCircle(pWorld, evaderLocation, 400.f, 72,
			FColor::Red, false, -1.f, 0, 3.f, FVector(0, 1, 0), FVector(1, 0, 0), false);

		DrawDebugDirectionalArrow(pWorld, evaderLocation, evaderLocation + pAgentToEvade->GetVelocity(),
			50.f, FColor::Red, false, -1.f, 0, 2.f);
	}

#ifdef GAMEAI_USE_SPACE_PARTITIONING
	if (DebugRenderPartitions && pPartitionedSpace)
	{
		pPartitionedSpace->RenderCells();
	}
#endif

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
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		ImGui::Checkbox("Debug Partitions", &DebugRenderPartitions);
#endif

		ImGui::Spacing();
		ImGui::Separator();

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
	if (Agents.Num() > 0 && Agents[0])
	{
		FVector center = Agents[0]->GetActorLocation();

		//Neighborhood Radius
		DrawDebugCircle(pWorld, center, NeighborhoodRadius, 72,
			FColor::Cyan, false, -1.f, 0, 2.f, FVector(0, 1, 0), FVector(1, 0, 0), false);

#ifdef GAMEAI_USE_SPACE_PARTITIONING

		pPartitionedSpace->RegisterNeighbors(*Agents[0], NeighborhoodRadius);

#else
		
		RegisterNeighbors(Agents[0]);

#endif

		//Lines to all current neighbors
		for (int i = 0; i < NrOfNeighbors; ++i)
		{
			if (GetNeighbors()[i])
			{
				DrawDebugLine(pWorld, center, GetNeighbors()[i]->GetActorLocation(),
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
	const int count = GetNrOfNeighbors();
	if (count == 0) return FVector2D::ZeroVector;

	FVector2D avgPosition = FVector2D::ZeroVector;
	for (int i = 0; i < count; ++i)
	{
		avgPosition += GetNeighbors()[i]->GetPosition();
	}
	return avgPosition / static_cast<float>(count);
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;

	FVector2D avgVelocity = FVector2D::ZeroVector;
	for (int i = 0; i < NrOfNeighbors; ++i)
	{
		avgVelocity += FVector2D(GetNeighbors()[i]->GetVelocity());
	}
	return avgVelocity / static_cast<float>(NrOfNeighbors);
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
	if (pSeekBehavior)
	{
		FTargetData targetData;
		targetData.Position = Target.Position;
		pSeekBehavior->SetTarget(targetData);
	}
}

