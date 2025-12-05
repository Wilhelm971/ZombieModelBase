// Copyright University of Inland Norway

#include "NonPlayerCharacters.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

ANonPlayerCharacters::ANonPlayerCharacters()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 720.f, 0.f);
	GetCharacterMovement()->MaxWalkSpeed = 0.f;
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

	if (!bIsMoving || CurrentWorldPath.Num() < 2) return;

	float Elapsed = GetWorld()->GetTimerManager().GetTimerElapsed(MoveFinishTimer);
	float Progress = FMath::Min(1.f, Elapsed / TurnBasedMoveDuration);

	// Find position at exact progress along path
	FVector NewPos = GetPositionAlongPath(Progress);
	SetActorLocation(NewPos, true);

	// Face direction of movement
	if (Progress < 1.f)
	{
		FVector Dir = (NewPos - GetActorLocation()).GetSafeNormal();
		SetActorRotation(Dir.Rotation());
	}
}

void ANonPlayerCharacters::MoveAlongWorldPath(const TArray<FVector>& WorldPath)
{
	if (bIsMoving || WorldPath.Num() < 2) return;

	bIsMoving = true;
	CurrentWorldPath = WorldPath;
	MoveStartLocation = GetActorLocation();
	MoveTargetGridPos = FIntPoint(-1, -1); // Set in manager call

	// Precompute total path length for constant speed
	PathTotalLength = 0.f;
	for (int32 i = 1; i < WorldPath.Num(); ++i)
	{
		PathTotalLength += FVector::Dist(WorldPath[i - 1], WorldPath[i]);
	}

	if (PathTotalLength <= 0.f)
	{
		FinishPathMove();
		return;
	}

	// Start timer to force finish (safety + exact duration)
	GetWorld()->GetTimerManager().SetTimer(MoveFinishTimer, this, &ANonPlayerCharacters::FinishPathMove, TurnBasedMoveDuration, false);
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

FVector ANonPlayerCharacters::GetPositionAlongPath(float Progress)
{
	if (PathTotalLength <= 0.f) return CurrentWorldPath.Last();

	float DistanceAlongPath = Progress * PathTotalLength;

	// Walk segments cumulatively
	float CumulativeDist = 0.f;
	for (int32 i = 1; i < CurrentWorldPath.Num(); ++i)
	{
		float SegmentLen = FVector::Dist(CurrentWorldPath[i - 1], CurrentWorldPath[i]);
		float NextCumul = CumulativeDist + SegmentLen;

		if (DistanceAlongPath <= NextCumul)
		{
			// Lerp within this segment
			float SegmentProgress = (DistanceAlongPath - CumulativeDist) / SegmentLen;
			return FMath::Lerp(CurrentWorldPath[i - 1], CurrentWorldPath[i], SegmentProgress);
		}

		CumulativeDist = NextCumul;
	}

	return CurrentWorldPath.Last(); // End of path
}

void ANonPlayerCharacters::FinishPathMove()
{
	if (CurrentWorldPath.Num() > 0)
	{
		SetActorLocation(CurrentWorldPath.Last(), true);
	}

	GridPosition = MoveTargetGridPos; // Update grid pos
	bIsMoving = false;
	CurrentWorldPath.Empty();
	PathTotalLength = 0.f;
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
	SetState(EState::Zombie);
}