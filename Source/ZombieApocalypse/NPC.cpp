// Copyright University of Innland Norway

#include "NPC.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"


ANPC::ANPC()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// Manual root setup
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

	// Manual skeletal mesh setup
	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	SkeletalMeshComponent->SetupAttachment(RootComponent);
}

void ANPC::BeginPlay()
{
	Super::BeginPlay();
	SetState(CurrentState);
	UpdateMesh();
}

void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving || CurrentWorldPath.Num() < 2) return;

	float Elapsed = GetWorldTimerManager().GetTimerElapsed(MoveFinishTimer);
	if (Elapsed < 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC %s: Timer invalid in Tick!"), *GetName());
		return;
	}

	float Progress = FMath::Clamp(Elapsed / TurnBasedMoveDuration, 0.f, 1.f);

	FVector NewPos = GetPositionAlongPath(Progress);
	bool bMoved = SetActorLocation(NewPos, false); // false = teleport
	if (!bMoved)
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC %s: SetActorLocation FAILED! From %s to %s"), *GetName(), *GetActorLocation().ToString(), *NewPos.ToString());
		return;
	}

	// Smooth rotation: Use current path segment direction
	if (CurrentWorldPath.Num() > 1 && CurrentPathSegmentIndex < CurrentWorldPath.Num())
	{
		FVector SegmentStart = CurrentWorldPath[CurrentPathSegmentIndex - 1];
		FVector SegmentEnd = CurrentWorldPath[CurrentPathSegmentIndex];
		FVector SegmentDir = (SegmentEnd - SegmentStart).GetSafeNormal();

		if (!SegmentDir.IsNearlyZero(0.001f))
		{
			FRotator TargetRot = SegmentDir.Rotation();

			// Smooth lerp (no pop)
			FRotator CurrentRot = GetActorRotation();
			FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 8.f); // fast interp
			SetActorRotation(NewRot);
		}

		// Advance segment if past midpoint of it.
		float SegmentProgress = FVector::Dist(GetActorLocation(), SegmentStart) / FVector::Dist(SegmentStart, SegmentEnd);
		if (SegmentProgress > 0.5f && CurrentPathSegmentIndex < CurrentWorldPath.Num() - 1)
		{
			CurrentPathSegmentIndex++;
		}
	}
}

void ANPC::UpdateMesh()
{
	if (!SkeletalMeshComponent) return; // use manual mesh

	USkeletalMesh* TargetMesh = nullptr;
	FVector NewScale = FVector::OneVector;
	FRotator NewRotation = FRotator::ZeroRotator;
	FVector NewLocation = FVector::ZeroVector;

	/*--- Pick State ---*/
	switch (CurrentState)
	{
	case EState::Zombie:
		TargetMesh = ZombieSkin;
		NewScale = FVector(0.3f);
		NewRotation = FRotator(0.f, -90.f, 0.f);
		NewLocation = FVector(0.f, 0.f, 0.f);
		break;

	case EState::Bitten:
		TargetMesh = BittenSkin;
		NewScale = FVector(0.45f);
		NewRotation = FRotator(0.f, 90.f, -90.f);
		NewLocation = FVector(-37.f, 0.f, 5.f);
		break;

	case EState::Human:
	default:
		TargetMesh = HumanSkin;
		NewScale = FVector(0.5f);
		NewRotation = FRotator(0.f, 90.f, 0.f);
		NewLocation = FVector(0.f, 0.f, 0.f);
		break;
	}

	/*--- Apply Mesh ---*/
	if (TargetMesh)
	{
		SkeletalMeshComponent->SetSkeletalMesh(TargetMesh);
	}

	/*--- Apply Transforms ---*/
	SkeletalMeshComponent->SetRelativeScale3D(NewScale);
	SkeletalMeshComponent->SetRelativeRotation(NewRotation);
	SkeletalMeshComponent->SetRelativeLocation(NewLocation);
}

FVector ANPC::GetPositionAlongPath(float Progress)
{
	if (PathTotalLength <= 0.f) return CurrentWorldPath.Last();

	float DistanceAlongPath = FMath::Clamp(Progress, 0.f, 1.f) * PathTotalLength;

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
	return CurrentWorldPath.Last(); // end of path
}

void ANPC::FinishPathMove()
{
	bool bCheck = SetActorLocation(CurrentWorldPath.Last(), true);
	if (!bCheck)
	{
		UE_LOG(LogTemp, Log, TEXT("NPC %s: SetActorLocaiton is FALSE"), *GetName());
	}

	UE_LOG(LogTemp, Log, TEXT("NPC %s: Finish move to %s"), *GetName(), *CurrentWorldPath.Last().ToString());

	bIsMoving = false;
	CurrentWorldPath.Empty();
	PathTotalLength = 0.f;
}

void ANPC::MoveAlongWorldPath(const TArray<FVector>& WorldPath)
{
	if (bIsMoving || WorldPath.Num() < 2) return;

	UE_LOG(LogTemp, Log, TEXT("NPC %s: Starting move! Duration: %f, Points: %d"), *GetName(), TurnBasedMoveDuration, WorldPath.Num());
	for (int32 i = 0; i < WorldPath.Num(); ++i)
	{
		UE_LOG(LogTemp, Log, TEXT(" Point %d: %s"), i, *WorldPath[i].ToString());
	}

	bIsMoving = true;
	CurrentWorldPath = WorldPath;
	CurrentPathSegmentIndex = 1;
	MoveStartLocation = GetActorLocation();
	MoveTargetGridPos = WorldPath[WorldPath.Num() - 1];

	// Precompute total path length for constant speed
	PathTotalLength = 0.f;
	for (int32 i = 1; i < WorldPath.Num(); ++i)
	{
		PathTotalLength += FVector::Dist(WorldPath[i - 1], WorldPath[i]);
	}

	if (PathTotalLength <= 0.f || TurnBasedMoveDuration <= 0.01f)
	{
		UE_LOG(LogTemp, Warning, TEXT("NPC %s: Skipping move - Zero path (%f) or duration (%f)"), *GetName(), PathTotalLength, TurnBasedMoveDuration);
		FinishPathMove();
		return;
	}

	// Start timer to force finish
	GetWorld()->GetTimerManager().SetTimer(MoveFinishTimer, this, &ANPC::FinishPathMove, TurnBasedMoveDuration, false);
}

void ANPC::SetState(EState NewState)
{
	if (CurrentState == NewState) return;

	CurrentState = NewState;
	UpdateMesh();
}

EState ANPC::GetState()
{
	return CurrentState;
}

