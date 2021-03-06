// Fill out your copyright notice in the Description page of Project Settings.


#include "SH_PlayerReplicateComponent.h"
#include <GameFramework/Pawn.h>
#include <Camera/CameraComponent.h>
#include <MotionControllerComponent.h>
#include "WJ_PingPongMgr.h"

// 언리얼 네트워크 헤더 추가
#include "Net/UnrealNetwork.h"
#include "SJ_PingPongPlayer.h"
#include "VRGameModeBase.h"

// Sets default values for this component's properties
USH_PlayerReplicateComponent::USH_PlayerReplicateComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
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
	leftController = Cast<UMotionControllerComponent>(player->GetDefaultSubobjectByName(TEXT("LeftMotionContoller")));
	rightController = Cast<UMotionControllerComponent>(player->GetDefaultSubobjectByName(TEXT("RightMotionContoller")));

	// 플레이어 인덱스를 가져오기 위한 플레이어 스크립트 캐스팅
	class ASJ_PingPongPlayer* temp = Cast<ASJ_PingPongPlayer>(GetOwner());


	gameState = Cast<AVRGameModeBase>(GetWorld()->GetGameState());
	pingpongStateMgr = gameState->pingpongStateMgr;

	// 서버라면
	if(gameState->HasAuthority())
	{
		//  서버 방의 서버 플레이어
		if (player->IsLocallyControlled())
		{
			//pingpongStateMgr->playerActorA = Cast<AActor>(GetOwner());
			player->SetActorLocation(FVector(-198, 0, 112));
			player->SetActorRotation(FRotator(0, 0, 0));
		}
		// 서버 방의 클라이언트 플레이어
		else
		{
			//pingpongStateMgr->playerActorB = Cast<AActor>(GetOwner());
			player->SetActorLocation(FVector(198, 0, 112));
			player->SetActorRotation(FRotator(180, 0, -180));
			// 클라이언트가 입장하면 게임 실행
			gameState->SetLevelState(EPPLevelState::Setting);
			//pingpongStateMgr->SetState(EPingPongState::Serv);
		}
	}
	// 클라이언트 방이라면
	else
	{
		//  클라이언트 방의 클라이언트 플레이어
		if (player->IsLocallyControlled())
		{
			//pingpongStateMgr->playerActorB = Cast<AActor>(GetOwner());
			player->SetActorLocation(FVector(198, 0, 112));
			player->SetActorRotation(FRotator(180, 0, -180));
			// 클라이언트가 입장하면 게임 실행
			gameState->SetLevelState(EPPLevelState::Setting);

			//pingpongStateMgr->SetState(EPingPongState::Serv);
		}
		// 클라이언트 방의 서버 플레이어
		else
		{
			//pingpongStateMgr->playerActorA = Cast<AActor>(GetOwner());
			//player->SetActorLocation(FVector(-198, 0, 112));
			//player->SetActorRotation(FRotator(0, 0, 0));
		}
	}
}


// Called every frame
void USH_PlayerReplicateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 위치 동기화 매 프레임마다 실행
	UpdateReplicate();
}

void USH_PlayerReplicateComponent::UpdateReplicate()
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
			Server_UpdateReplicate(player_Tr, playerCam_Tr, leftController_Tr, rightController_Tr);
		}
		// 서버라면
		else
		{
			// 곧바로 멀티캐스트 함수 실행
			Multi_UpdateReplicate(player_Tr, playerCam_Tr, leftController_Tr, rightController_Tr);
		}
	}
}

bool USH_PlayerReplicateComponent::Server_UpdateReplicate_Validate(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	return true;
}

void USH_PlayerReplicateComponent::Server_UpdateReplicate_Implementation(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	Multi_UpdateReplicate_Implementation(playerTr, cameraTr, lcontTr, rcontTr);
}

bool USH_PlayerReplicateComponent::Multi_UpdateReplicate_Validate(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	return true;
}

void USH_PlayerReplicateComponent::Multi_UpdateReplicate_Implementation(FTransform playerTr, FTransform cameraTr, FTransform lcontTr, FTransform rcontTr)
{
	// 멀티캐스트 적용할 컴포넌트 예외 처리
	if (!player)
	{
		player = Cast<APawn>(GetOwner());
	}
	// 멀티캐스트 적용할 컴포넌트 예외 처리
	if (!playerCam)
	{
		playerCam = Cast<UCameraComponent>(player->GetDefaultSubobjectByName(TEXT("MainCamera")));
	}
	// 멀티캐스트 적용할 컴포넌트 예외 처리
	if (!leftController)
	{
		leftController = Cast<UMotionControllerComponent>(player->GetDefaultSubobjectByName(TEXT("LeftMotionContoller")));
	}
	// 멀티캐스트 적용할 컴포넌트 예외 처리
	if (!rightController)
	{
		rightController = Cast<UMotionControllerComponent>(player->GetDefaultSubobjectByName(TEXT("RightMotionContoller")));
	}
	// 로컬 조작 플레이어가 아니라면
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