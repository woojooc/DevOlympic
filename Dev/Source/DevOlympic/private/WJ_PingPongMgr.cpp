

#include "WJ_PingPongMgr.h"
#include "VRGameModeBase.h"
#include "WJ_ObjectPool.h"
#include "Kismet/GameplayStatics.h"
#include "ModeSelect.h"
#include "WJ_PPSingleModeWall.h"
#include "WJ_Point.h"
#include "SJ_PingPongPlayer.h"


UWJ_PingPongMgr::UWJ_PingPongMgr()
{
	PrimaryComponentTick.bCanEverTick = true;
}


void UWJ_PingPongMgr::BeginPlay()
{
	Super::BeginPlay();

	//gameModeBase 캐싱
	gameModeBase = Cast<AVRGameModeBase>(GetWorld()->GetAuthGameMode());

	// # 게임모드 설정
	// 게임모드 셀렉트 월드에서 찾아오기
	modeSelect = Cast<AModeSelect>(UGameplayStatics::GetActorOfClass(GetWorld(), AModeSelect::StaticClass()));
	editMode = modeSelect->m_state;

	// # 점수판 캐싱
	TArray<AActor*> bpPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AWJ_Point::StaticClass(),bpPoints);
	//pointPannel = Cast<AWJ_Point>(UGameplayStatics::GetActorOfClass(GetWorld(),AWJ_Point::StaticClass()));
	if (bpPoints.Num() != 0)
	{
		pointPannel = Cast<AWJ_Point>(bpPoints[0]);
		pointPannel->SetColor(FColor::Blue);
		scorePannel = Cast<AWJ_Point>(bpPoints[1]);
		scorePannel->SetColor(FColor::Red);
	}

	playerActorA = UGameplayStatics::GetActorOfClass(GetWorld(), ASJ_PingPongPlayer::StaticClass());
	if (editMode == EEditMode::Multi)
	{
		// 플레이어 호스트 ( 0, A ) -> 탁구대 사이드 A
		// 플레이어 게스트 ( 1, B ) -> 탁구대 사이드 B 에 소환

		// 첫 서브 랜덤으로 뽑기
	}
	else
	{
		playerActorB = UGameplayStatics::GetActorOfClass(GetWorld(),AWJ_PPSingleModeWall::StaticClass());

		// 벽 활성화
		AActor* wall = UGameplayStatics::GetActorOfClass(GetWorld(), AWJ_PPSingleModeWall::StaticClass());
		if (wall)
		{
			wall->SetActorHiddenInGame(false);
		}

		// 서브 설정
		bServPlayer = 0;
	}
}


void UWJ_PingPongMgr::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (m_state)
	{
	case EPingPongState::Intro:
		// 대기
		Intro();
		break;

	case EPingPongState::Serv:
		// 공 소환
		Serv();
		break;

	case EPingPongState::Rally:
		// 충돌 체크
		//Rally();
		break;

	case EPingPongState::RallyOver:
		// 공 제거
		// 점수(point) 획득
		// 10 : 10 ==> Serv, bIsDeuse = true : 2점차날때까지 반복
		// point < 11 ==> Serv
		// 11 ==> SetOver
		//RallyOver();
		break;

	case EPingPongState::SetOver:
		// set < 5  ==>  Serv
		// 5 ==> MatchOver
		SetOver();
		break;

	case EPingPongState::MatchOver:
		// score 비교 승자 결정
		MatchOver();
		break;

	default:
		break;
	}
}

void UWJ_PingPongMgr::Intro()
{
	SetState(EPingPongState::Serv);
}

void UWJ_PingPongMgr::Serv()
{
	// 공이 스폰되었으면 대기
	if (bSpawnBall)
	{
		return;
	}

	// bServPlayer가 T -> A 위치에 공 스폰
	// else -> B 위치에 공 스폰
	if (bServPlayer == 0)
	{
		//A 공 스폰
		if (playerActorA)
		{
			gameModeBase->objectPool->GetPingPongBall(playerActorA,0,editMode);
			bSpawnBall = true;
		}
	}
	else
	{
		//B 공 스폰
		gameModeBase->objectPool->GetPingPongBall(playerActorB,1,editMode);
		bSpawnBall = true;
	}

	// 플레이어가 공을 치면 이벤트 발생
}

void UWJ_PingPongMgr::Rally()
{
	
}

void UWJ_PingPongMgr::RallyOver()
{

}

void UWJ_PingPongMgr::SetOver()
{
	set++;
	pointA = 0;
	pointB = 0;
	pointPannel->ResetPoint();

	if (set == 5)
	{
		SetState(EPingPongState::MatchOver);
		return;
	}

	SetState(EPingPongState::Serv);
}

void UWJ_PingPongMgr::MatchOver()
{
	if (scoreA > scoreB)
	{
		// A 승리
	}
	else
	{
		// B 승리
	}

	// 게임 종료 로비로 돌아가기 버튼 오브젝트 활성화
}

void UWJ_PingPongMgr::SetState(EPingPongState state)
{
	m_state = state;
}

EPingPongState UWJ_PingPongMgr::GetState()
{
	return m_state;
}

void UWJ_PingPongMgr::StartRally()
{
	SetState(EPingPongState::Rally);
	// 서브 카운트 증가
	servCount++;
}

void UWJ_PingPongMgr::OnCollisionGround(int player, bool in)
{
	// 상태 변화
	// 랠리 오버로 이동

	if (in)
	{
		// player point ++
		if (player == 0)
		{
			pointA++;

			// if Deuce point ++
			if (bIsDeuce)
			{
				pointA++;
			}
		}
		else
		{
			pointB++;
			if (bIsDeuce)
			{
				pointB++;
			}
		}
		pointPannel->SetPoint(0, pointA);
		pointPannel->SetPoint(1, pointB);
	}
	else
	{
		// other player point ++
		// if Deuce point ++
		if (player == 0)
		{
			pointB++;

			if (bIsDeuce)
			{
				pointB++;
			}
		}
		else
		{
			pointA++;
			if (bIsDeuce)
			{
				pointA++;
			}
		}
		pointPannel->SetPoint(0, pointA);
		pointPannel->SetPoint(1, pointB);
	}

	//serv 설정
	if (servCount >= 2)
	{
		bServPlayer = !bServPlayer;
		servCount = 0;
	}

	// 듀스 2점차 이상 세트 종료
	if (bIsDeuce)
	{
		if (pointA - pointB >= 2 || pointA - pointB <= -2)
		{
			// 스코어 계산
			if (pointA > pointB)
			{
				scoreA++;
			}
			else
			{
				scoreB++;
			}

			scorePannel->SetPoint(0,scoreA);
			scorePannel->SetPoint(1,scoreB);

			SetState(EPingPongState::SetOver);
			return;
		}
	}
	else 
	{
		// 듀스 아닌 경우 11점 획득시 세트 종료
		if (pointA == 11 || pointB == 11)
		{
			// 스코어 계산
			if (pointA == 11)
			{
				scoreA++;
			}
			else
			{
				scoreB++;
			}

			scorePannel->SetPoint(0, scoreA);
			scorePannel->SetPoint(1, scoreB);

			SetState(EPingPongState::SetOver);
			return;
		}
	}

	if (pointA == 10 && pointB == 10)
	{
		bIsDeuce = true;
	}

	SetState(EPingPongState::Serv);
}

void UWJ_PingPongMgr::NetServ()
{
	// 기존 공 삭제
	
	SetState(EPingPongState::Serv);
}