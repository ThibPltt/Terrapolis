// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ARBlueprintLibrary.h"
#include "ARManager.generated.h"

UCLASS()
class ARTERRAPOLIS_API AARManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AARManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

    // La configuration de session (à assigner dans l'éditeur)
    UPROPERTY(EditAnywhere, Category = "AR Setup")
    UARSessionConfig* Config;

    // Le blueprint du bâtiment à faire apparaître
    UPROPERTY(EditAnywhere, Category = "AR Setup")
    TSubclassOf<AActor> BuildingClass;

    // Le blueprint du bâtiment transparent qui suit le regard
    UPROPERTY(EditAnywhere, Category = "AR Setup")
    TSubclassOf<AActor> GhostBuildingClass;

    // Fonction pour confirmer le placement (clic de souris)
    UFUNCTION(BlueprintCallable, Category = "AR Interaction")
    void ConfirmPlacement();

private:
    // Pointeur vers le bâtiment généré
    AActor* SpawnedBuilding;

    // Fonction pour gérer le tracking de l'image
    void UpdateImageTracking();

    // Pointeur vers le bâtiment fantôme actuellement dans la scène
    AActor* CurrentGhostBuilding;

    // La fonction qui va calculer le rayon (Raycast)
    void HandleHoverLogic();
};
