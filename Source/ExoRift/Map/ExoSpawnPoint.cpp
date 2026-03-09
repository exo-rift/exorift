#include "Map/ExoSpawnPoint.h"
#include "Components/BillboardComponent.h"

AExoSpawnPoint::AExoSpawnPoint()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

#if WITH_EDITORONLY_DATA
	EditorSprite = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorSprite"));
	EditorSprite->SetupAttachment(RootComponent);
#endif
}
