// Zombie.cpp
#include "Zombie.h"
#include "Components/CapsuleComponent.h"

AZombie::AZombie()
{
    PrimaryActorTick.bCanEverTick = false;

    GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
    GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
    GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
}