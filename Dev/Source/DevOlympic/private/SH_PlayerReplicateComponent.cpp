// Fill out your copyright notice in the Description page of Project Settings.


#include "SH_PlayerReplicateComponent.h"
#include <GameFramework/Pawn.h>
#include <Camera/CameraComponent.h>
#include <MotionControllerComponent.h>

// 언리얼 네트워크 헤더 추가
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USH_PlayerReplicateComponent::USH_PlayerReplicateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USH_PlayerReplicateComponent::BeginPlay()
{
	Super::BeginPlay();

	// 플레이어 컴포넌트 받아오기
	player = Cast<APawn>(GetOwner());
	// 카메라 컴포넌트 받아오기
	playerCam = Cast<UCameraComponent>(player->GetDefaultSubobjectByName(TEXT("MainCamera")));
	// 왼손, 오른손 컨트롤러 받아오기
	leftController = Cast<UMotionControllerComponent>(player->GetDefaultSubobjectByName(TEXT("LeftMotionController")));
	rightController = Cast<UMotionControllerComponent>(player->GetDefaultSubobjectByName(TEXT("RightMotionController")));
}


// Called every frame
void USH_PlayerReplicateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 위치 동기화 매 프레임마다 실행
	UpdateTransform();
}

void USH_PlayerReplicateComponent::UpdateTransform()
{

	// 로컬플레이어라면
	if (player->IsLocallyControlled())
	{
		player_Tr = player->GetActorTransform();
		playerCam_Tr = playerCam->GetComponentTransform();
		leftController_Tr = leftController->GetComponentTransform();
		rightController_Tr = rightController->GetComponentTransform();

		// 로컬플레이어가 서버가 아니라면
		if (!player->HasAuthority())
		{
			// 서버 함수 실행
			Server_UpdateTransform(player_Tr, playerCam_Tr, leftController_Tr, rightController_Tr);
		}
		// 서버라면
		else
		{
			// 곧바로 멀티캐스트 함수 실행
			Multi_UpdateTransform(player_Tr, playerCam_Tr, leftController_Tr, rightController_Tr);
		}
	}
}

bool USH_PlayerReplicateComponent::Server_UpdateTransform_Validate(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	return true;
}

void USH_PlayerReplicateComponent::Server_UpdateTransform_Implementation(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	Multi_UpdateTransform_Implementation(playerTr, cameraTr, lcontTr, rcontTr);
}

bool USH_PlayerReplicateComponent::Multi_UpdateTransform_Validate(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	return true;
}

void USH_PlayerReplicateComponent::Multi_UpdateTransform_Implementation(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	if (!player->IsLocallyControlled())
	{
		// 클라이언트에서 받은 손 위치 함수를 전달
		// 플레이어 액터 위치
		player->SetActorTransform(playerTr);
		// 카메라 컴포넌트
		playerCam->SetWorldTransform(cameraTr);
		// 왼손, 오른손 컨틀롤러
		leftController->SetWorldTransform(lcontTr);
		rightController->SetWorldTransform(rcontTr);
	}
}