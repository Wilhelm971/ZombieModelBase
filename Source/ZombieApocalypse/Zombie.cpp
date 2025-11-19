#include "AZombie.h"

AZombie::AZombie()
{
    PrimaryActorTick.bCanEverTick = false;
    GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
    GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
}