// Copyright University of Inland Norway

#include "NonPlayerCharacters.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

ANonPlayerCharacters::ANonPlayerCharacters()
{
	PrimaryActorTick.bCanEverTick = true;

	AIControllerClass = AAIController::StaticClass();
}

void ANonPlayerCharacters::BeginPlay()
{
	Super::BeginPlay();
	SetState(CurrentState);
	UpdateMesh(); // if the starter state does not change...
}

void ANonPlayerCharacters::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANonPlayerCharacters::UpdateMesh()
{

	// Pick mesh according to state
	USkeletalMesh* TargetMesh = HumanSkin; // safe fallback

	if (CurrentState == EState::Zombie && ZombieSkin)
	{
		TargetMesh = ZombieSkin;
	}
	else if (CurrentState == EState::Bitten && BittenSkin)
	{
		TargetMesh = BittenSkin;
	}
	else if (CurrentState == EState::Human && HumanSkin)
	{
		TargetMesh = HumanSkin;
	}

	// safe check
	if (!TargetMesh) return;

	GetMesh()->SetSkeletalMesh(TargetMesh);
}

void ANonPlayerCharacters::SetState(EState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	UpdateMesh();
}

EState ANonPlayerCharacters::GetState()
{
	return CurrentState;
}

void ANonPlayerCharacters::TestStateLogic()
{
	SetState(EState::Human);
}