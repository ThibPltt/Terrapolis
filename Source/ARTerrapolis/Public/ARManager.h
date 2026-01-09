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

    UPROPERTY(EditAnywhere, Category = "AR Grid")
    float TileSize = 5.0f; // Taille d'un carré sur ta map (en cm)

    UPROPERTY(EditAnywhere, Category = "AR Grid")
    FVector2D MapSize = FVector2D(80.0f, 50.0f); // Dimensions réelles

    UFUNCTION(BlueprintCallable, Category = "AR Interaction")
    void ConfirmPlacement();

private:
    AActor* SpawnedBuilding;
    AActor* CurrentGhostBuilding;

    // Référence à l'image physique détectée
    UARTrackedImage* DetectedMapImage;

    // Stockage de la position de la tuile survolée
    int32 CurrentCol;
    int32 CurrentRow;

    void UpdateImageTracking();
    void HandleHoverLogic();

};
