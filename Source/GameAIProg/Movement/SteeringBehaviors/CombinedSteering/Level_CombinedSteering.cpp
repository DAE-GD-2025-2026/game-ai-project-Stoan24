#include "Level_CombinedSteering.h"

#include "imgui.h"


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	Seek* pSeek = new Seek();
	Wander* pWanderBlend = new Wander();
	Wander* pWanderPrio = new Wander();
	Evade* pEvade = new Evade();

	BlendedBehaviors.clear();
	BlendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pSeek, 0.5f));
	BlendedBehaviors.push_back(BlendedSteering::WeightedBehavior(pWanderBlend, 0.5f));

	pBlendedSteering = new BlendedSteering(BlendedBehaviors);

	PriorityBehaviors.clear();
	PriorityBehaviors.push_back(pEvade);
	PriorityBehaviors.push_back(pWanderPrio);


	DrunkAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{ -500, 0, 90 }, FRotator::ZeroRotator);
	pBlendedSteering = new BlendedSteering(BlendedBehaviors);
	DrunkAgent->SetSteeringBehavior(pBlendedSteering);
	DrunkAgent->SetDebugRenderingEnabled(true);


	EvadingAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector{ 500, 0, 90 }, FRotator::ZeroRotator);
	pPrioritySteering = new PrioritySteering(PriorityBehaviors);
	EvadingAgent->SetSteeringBehavior(pPrioritySteering);
	EvadingAgent->SetDebugRenderingEnabled(true);
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();
}



// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
			if (DrunkAgent) DrunkAgent->SetDebugRenderingEnabled(CanDebugRender);
			if (EvadingAgent) EvadingAgent->SetDebugRenderingEnabled(CanDebugRender);
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();


		 ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
		 	pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
		 	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		
		 ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		 pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		 [this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
	
		//End
		ImGui::End();
	}

#pragma endregion

	if (DrunkAgent && EvadingAgent)
	{
		FTargetData evaderData;
		evaderData.Position = FVector2D(EvadingAgent->GetActorLocation());
		evaderData.LinearVelocity = FVector2D(EvadingAgent->GetVelocity());

		auto& blendedBehaviors = pBlendedSteering->GetWeightedBehaviorsRef();
		if (!blendedBehaviors.empty() && blendedBehaviors[0].pBehavior)
		{
			blendedBehaviors[0].pBehavior->SetTarget(evaderData);
		}

		FTargetData drunkData;
		drunkData.Position = FVector2D(DrunkAgent->GetActorLocation());
		drunkData.LinearVelocity = FVector2D(DrunkAgent->GetVelocity());

		if (!PriorityBehaviors.empty() && PriorityBehaviors[0])
		{
			PriorityBehaviors[0]->SetTarget(drunkData);
		}
	}

	const APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->WasInputKeyJustPressed(EKeys::LeftMouseButton))
	{
		FHitResult Hit;
		if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
		{
			FTargetData mouseTarget;
			mouseTarget.Position = FVector2D(Hit.Location);
		}
	}

	if (CanDebugRender)
	{
		if (DrunkAgent)
		{
			const FVector location = DrunkAgent->GetActorLocation();
			const FVector velocity = DrunkAgent->GetVelocity();

			//Draw Current Velocity
			DrawDebugDirectionalArrow(GetWorld(), location, location + velocity,
				50.f, FColor::Green, false, -1.f, 0, 3.f);

			//Draw Wander
			const FVector forward = DrunkAgent->GetActorForwardVector();
			constexpr float wanderOffset = 200.f;
			constexpr float wanderRadius = 100.f;
			const FVector circleCenter = location + (forward * wanderOffset);

			DrawDebugCircle(GetWorld(), circleCenter, wanderRadius, 32,
				FColor::White, false, -1.f, 0, 2.f, FVector(0, 1, 0), FVector(1, 0, 0), false);
		}

		if (EvadingAgent)
		{
			//Draw PanicCircle
			float panicRadius = 500.f;

			float dist = FVector::Dist(DrunkAgent->GetActorLocation(), EvadingAgent->GetActorLocation());

			FColor debugColor = (dist < panicRadius) ? FColor::Red : FColor::Green;
			DrawDebugCircle(GetWorld(), EvadingAgent->GetActorLocation(), panicRadius, 64,
				debugColor, false, -1.f, 0, 2.f, FVector(0, 1, 0), FVector(1, 0, 0), false);
		}
	}
}
