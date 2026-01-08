// Fill out your copyright notice in the Description page of Project Settings.


#include "ARManager.h"
#include "ARBlueprintLibrary.h"
#include "ARTrackable.h"

// Sets default values
AARManager::AARManager()
{
    PrimaryActorTick.bCanEverTick = true;
    SpawnedBuilding = nullptr;
    CurrentGhostBuilding = nullptr;
}

void AARManager::BeginPlay()
{
    Super::BeginPlay();

    // On démarre la session AR au lancement
    if (Config)
    {
        UARBlueprintLibrary::StartARSession(Config);
    }
}

void AARManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateImageTracking();
    HandleHoverLogic();
}

void AARManager::UpdateImageTracking()
{
    // On récupère toutes les images trackées par le téléphone
    auto TrackedImages = UARBlueprintLibrary::GetAllGeometriesByClass<UARTrackedImage>();

    for (UARTrackedImage* Image : TrackedImages)
    {
        // On vérifie si l'image est actuellement vue par la caméra
        if (Image->GetTrackingState() == EARTrackingState::Tracking)
        {
            FTransform ImageTransform = Image->GetLocalToWorldTransform();

            if (!SpawnedBuilding)
            {
                // Si le bâtiment n'existe pas encore, on le crée
                FActorSpawnParameters SpawnParams;
                SpawnedBuilding = GetWorld()->SpawnActor<AActor>(BuildingClass, ImageTransform, SpawnParams);
            }
            else
            {
                // Si il existe déjà, on met à jour sa position pour qu'il "colle" à la map
                SpawnedBuilding->SetActorTransform(ImageTransform);
            }
        }
    }
}

void AARManager::HandleHoverLogic()
{
    // 1. On récupère la vue du joueur (caméra du téléphone)
    FVector CamLoc;
    FRotator CamRot;
    if (GetWorld()->GetFirstPlayerController())
    {
        GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(CamLoc, CamRot);
    }

    FVector Start = CamLoc;
    FVector End = Start + (CamRot.Vector() * 2000.0f); // Rayon de 20 mètres

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (CurrentGhostBuilding) Params.AddIgnoredActor(CurrentGhostBuilding);

    // 2. On lance le rayon
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        // 3. Si on touche quelque chose (la map), on affiche le fantôme
        if (!CurrentGhostBuilding && GhostBuildingClass)
        {
            CurrentGhostBuilding = GetWorld()->SpawnActor<AActor>(GhostBuildingClass, Hit.Location, FRotator::ZeroRotator);
        }

        if (CurrentGhostBuilding)
        {
            CurrentGhostBuilding->SetActorLocation(Hit.Location);
            CurrentGhostBuilding->SetActorHiddenInGame(false);
        }
    }
    else
    {
        // 4. Si on ne regarde plus la map, on cache le fantôme
        if (CurrentGhostBuilding)
        {
            CurrentGhostBuilding->SetActorHiddenInGame(true);
        }
    }
}

void AARManager::ConfirmPlacement()
{
    // On vérifie si le fantôme est actuellement visible
    if (CurrentGhostBuilding && !CurrentGhostBuilding->IsHidden())
    {
        FVector SpawnLocation = CurrentGhostBuilding->GetActorLocation();
        FRotator SpawnRotation = CurrentGhostBuilding->GetActorRotation();

        // On fait apparaître le vrai bâtiment définitif
        GetWorld()->SpawnActor<AActor>(BuildingClass, SpawnLocation, SpawnRotation);
    }
}