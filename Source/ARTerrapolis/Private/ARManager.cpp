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
    auto TrackedImages = UARBlueprintLibrary::GetAllGeometriesByClass<UARTrackedImage>();

    for (UARTrackedImage* Image : TrackedImages)
    {
        if (Image->GetTrackingState() == EARTrackingState::Tracking)
        {
            DetectedMapImage = Image;
            FTransform ImageTransform = Image->GetLocalToWorldTransform();

            if (!SpawnedBuilding)
            {
                FActorSpawnParameters SpawnParams;
                // On spawn le bâtiment initial au centre de l'image
                SpawnedBuilding = GetWorld()->SpawnActor<AActor>(BuildingClass, ImageTransform, SpawnParams);
            }
            else
            {
                SpawnedBuilding->SetActorTransform(ImageTransform);
            }
        }
    }
}

void AARManager::HandleHoverLogic()
{
    FVector CamLoc;
    FRotator CamRot;
    if (GetWorld()->GetFirstPlayerController())
    {
        GetWorld()->GetFirstPlayerController()->GetPlayerViewPoint(CamLoc, CamRot);
    }

    FVector Start = CamLoc;
    FVector End = Start + (CamRot.Vector() * 2000.0f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
    if (CurrentGhostBuilding) Params.AddIgnoredActor(CurrentGhostBuilding);

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        // On vérifie qu'on touche la map ou qu'on a une image trackée
        if (DetectedMapImage && (Hit.GetActor() == SpawnedBuilding || Hit.GetActor()->GetName().Contains("Plane")))
        {
            // On utilise le transform de l'image comme référence absolue
            FTransform RefTransform = DetectedMapImage->GetLocalToWorldTransform();
            FVector LocalHit = RefTransform.InverseTransformPosition(Hit.Location);

            // Calcul de la grille (80x50, pivot au centre)
            float NormalizedX = LocalHit.X + (MapSize.X / 2.0f);
            float NormalizedY = LocalHit.Y + (MapSize.Y / 2.0f);

            CurrentCol = FMath::FloorToInt(NormalizedX / TileSize);
            CurrentRow = FMath::FloorToInt(NormalizedY / TileSize);

            CurrentCol = FMath::Clamp(CurrentCol, 0, FMath::FloorToInt(MapSize.X / TileSize) - 1);
            CurrentRow = FMath::Clamp(CurrentRow, 0, FMath::FloorToInt(MapSize.Y / TileSize) - 1);

            // Positionnement aimanté
            float SnappedX = (CurrentCol * TileSize) + (TileSize / 2.0f) - (MapSize.X / 2.0f);
            float SnappedY = (CurrentRow * TileSize) + (TileSize / 2.0f) - (MapSize.Y / 2.0f);

            FVector GhostLocalPos = FVector(SnappedX, SnappedY, 1.0f);
            FVector GhostWorldPos = RefTransform.TransformPosition(GhostLocalPos);

            if (!CurrentGhostBuilding && GhostBuildingClass)
            {
                CurrentGhostBuilding = GetWorld()->SpawnActor<AActor>(GhostBuildingClass, GhostWorldPos, RefTransform.GetRotation().Rotator());
            }

            if (CurrentGhostBuilding)
            {
                CurrentGhostBuilding->SetActorLocation(GhostWorldPos);
                CurrentGhostBuilding->SetActorRotation(RefTransform.GetRotation());
                CurrentGhostBuilding->SetActorHiddenInGame(false);
            }
        }
        else if (CurrentGhostBuilding)
        {
            CurrentGhostBuilding->SetActorHiddenInGame(true);
        }
    }
}

void AARManager::ConfirmPlacement()
{
    if (CurrentGhostBuilding && !CurrentGhostBuilding->IsHidden())
    {
        GetWorld()->SpawnActor<AActor>(BuildingClass, CurrentGhostBuilding->GetActorLocation(), CurrentGhostBuilding->GetActorRotation());
        UE_LOG(LogTemp, Warning, TEXT("Placement Valide - Tuile: [%d, %d]"), CurrentCol, CurrentRow);
    }
}