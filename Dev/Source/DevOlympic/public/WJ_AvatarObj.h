
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WJ_AvatarObj.generated.h"

UCLASS()
class DEVOLYMPIC_API AWJ_AvatarObj : public AActor
{
	GENERATED_BODY()
	
public:	
	AWJ_AvatarObj();

protected:
	virtual void BeginPlay() override;

public:	

	UPROPERTY(EditAnywhere, Category = "Settings")
	int avatarID;

	// Component
	UPROPERTY(EditAnywhere, Category = "Root")
	class UStaticMeshComponent* rootComp;

	UPROPERTY(EditAnywhere, Category = "Avatar")
	class UCapsuleComponent* collision;
	UPROPERTY(EditAnywhere,Category = "Avatar")
	class UStaticMeshComponent* bodyComp;
	UPROPERTY(EditAnywhere, Category = "Avatar")
	class UStaticMeshComponent* headComp;

	// TEXT
	UPROPERTY(EditAnywhere, Category = "Text")
	class UTextRenderComponent* arrowTxt;
	UPROPERTY(EditAnywhere, Category = "Text")
	class UTextRenderComponent* selectTxt;

	// Offset
	UPROPERTY(EditAnywhere, Category = PickUpSettings)
	FVector grabOffset = FVector(0,0,10);

	UPROPERTY(EditAnywhere, Category = PickUpSettings)
	FVector originalOffset = FVector(0,0,9);

	void HideText();
};
