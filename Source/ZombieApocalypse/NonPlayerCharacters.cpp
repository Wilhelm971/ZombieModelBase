// Copyright University of Inland Norway


#include "NonPlayerCharacters.h"

// Sets default values
ANonPlayerCharacters::ANonPlayerCharacters()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ANonPlayerCharacters::SetState(EState NewState)
{
}

// Called when the game starts or when spawned
void ANonPlayerCharacters::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANonPlayerCharacters::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


