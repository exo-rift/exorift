#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ExoSpawnPoint.generated.h"

UCLASS()
class EXORIFT_API AExoSpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	AExoSpawnPoint();

	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bIsDropZone = true;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	bool bIsLobbySpawn = false;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	class UBillboardComponent* EditorSprite;
#endif
};
