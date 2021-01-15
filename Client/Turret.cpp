#include "stdafx.h"
#include "Turret.h"
#include "DataStore.h"
#include "ObjectManager.h"
#include "TurretRing.h"
#include "RingBox.h"
#include "GameInfo.h"
#include "BaseTurret.h"
#include "Decoration.h"
#include "CollisionManager.h"
#include "CollisionHelper.h"
#include "CollisionComponent.h"
#include "ClickableComponent.h"
#include "GraphicsComponent.h"

CTurret::CTurret()
{
}


CTurret::~CTurret()
{
	Release();
}

void CTurret::Initialize(void)
{
	//ID 할당
	m_objID = OBJID::TURRET;
	m_dataID = DATAID::TURRET;

	m_pTarget = nullptr;

	//Object에서 매니저에 등록, objectKey+stateKey 할당, 위치 크기 회전 초기화.
	__super::Initialize();
	
	CTurret::ReadDataFromStore();

	MakeRingBoxInfo();
	MakeTurretRing();
	MakeRangeCircle();

	//그래픽/클릭 컴포넌트 등록
	AddComponent<CCollisionComponent>();
	AddComponent<CGraphicsComponent>();
	AddComponent<CClickableComponent>()->SetPlayFunc(std::bind(&CTurret::Selected, this));
}

void CTurret::Update(void)
{
	__super::Update();


	//포커싱된 오브젝트가 나라면, 내 터렛링 활성화
	if (CGameInfo::GetInstance()->GetFocusedObject() == this)
	{
		m_pRangeCircle->SetActivated(true);
		m_pTurretRing->SetActivated(true);
	}
	//이제 포커싱이 넘어갔다면.
	else //if(m_pTurretRing->GetLastFrameActivated() == true)
	{
		m_pRangeCircle->SetActivated(false);

		m_pTurretRing->SetSize(m_pTurretRing->GetDefaultSize() * 0.7f);
		m_pTurretRing->SetActivated(false);
		m_pTurretRing->SetLastFrameActivated(false);
		for (auto& ringBox : m_pTurretRing->GetRingBoxes())
		{
			ringBox->SetStateKey("Idle");
			ringBox->SetActivated(false);
		}
	}

	FindTarget();
}

void CTurret::LateUpdate(void)
{
}

void CTurret::Release(void)
{
	for (auto& element : m_vRingBoxInfo)
	{
		SAFE_DELETE(element);
	}

	m_vRingBoxInfo.clear();
	
	m_pRangeCircle->SetNeedToBeDeleted(true);
	m_pTurretRing->SetNeedToBeDeleted(true);
}

void CTurret::UpgradeTurret(int increase)
{
	CTurret::Release();

	m_level += increase;
	m_stateKey = "Lv" + std::to_string(m_level) +"_Idle";

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_position", m_position);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_rotation", m_rotation);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_size", m_size);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_numOfSkill", m_numOfSkill);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_numOfRingBox", m_numOfRingBox);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_sellable", m_sellable);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_sellPrice", m_sellPrice);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_rallyable", m_rallyable);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_rallyRange", m_rallyRange);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_attackRange", m_attackRange);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_attackSpeed", m_attackSpeed);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_dmg", m_dmg);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_magicDmg", m_magicDmg);


	for (int i = 0; i < m_numOfRingBox; ++i)
	{
		RingBoxInfo* pNewRingBoxInfo = new RingBoxInfo;

		GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_ringBox" + std::to_string(i) + "_name", pNewRingBoxInfo->name);
		GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_ringBox" + std::to_string(i) + "_price", pNewRingBoxInfo->price);
		GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_ringBox" + std::to_string(i) + "_angle", pNewRingBoxInfo->angle);

		m_vRingBoxInfo.push_back(pNewRingBoxInfo);
	}
	//RangedCircle 세팅
	m_pRangeCircle = new CDecoration("RangeCircle", "Green");
	m_pRangeCircle->SetParent(this);
	m_pRangeCircle->SetSize(D3DXVECTOR3(m_attackRange, m_attackRange * 0.6f, 0));

	//터렛링 세팅
	m_pTurretRing = new CTurretRing(this);
	m_pTurretRing->SetParent(this);
}

void CTurret::SellTurret(void)
{
	CGameInfo::GetInstance()->SetGold(CGameInfo::GetInstance()->GetGold() + m_sellPrice);

	m_needToBeDeleted = true;

	CBaseTurret* pNewTurret = new CBaseTurret;
	pNewTurret->SetPosition(m_parentPosition);
}

void CTurret::ReadDataFromStore(void)
{
	//iniFile에서 정보 불러오기.
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_level", m_level);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_numOfSkill", m_numOfSkill);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_numOfRingBox", m_numOfRingBox);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_sellable", m_sellable);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_sellPrice", m_sellPrice);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_rallyable", m_rallyable);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_rallyRange", m_rallyRange);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_attackRange", m_attackRange);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_attackSpeed", m_attackSpeed);

	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_dmg", m_dmg);
	GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_m_magicDmg", m_magicDmg);
}

void CTurret::MakeRingBoxInfo(void)
{
	//RingBoxInfo 심기
	for (int i = 0; i < m_numOfRingBox; ++i)
	{
		RingBoxInfo* pNewRingBoxInfo = new RingBoxInfo;

		GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_ringBox" + std::to_string(i) + "_name", pNewRingBoxInfo->name);
		GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_ringBox" + std::to_string(i) + "_price", pNewRingBoxInfo->price);
		GET_VALUE(m_dataID, m_objectKey, m_stateKey + "_ringBox" + std::to_string(i) + "_angle", pNewRingBoxInfo->angle);

		m_vRingBoxInfo.push_back(pNewRingBoxInfo);
	}
}

void CTurret::MakeTurretRing(void)
{
	//터렛링 세팅
	m_pTurretRing = new CTurretRing(this);
	m_pTurretRing->SetParent(this);
}

void CTurret::MakeRangeCircle(void)
{
	//RangeCircle 심기
	m_pRangeCircle = new CDecoration("RangeCircle", "Green");
	m_pRangeCircle->SetParent(this);
	m_pRangeCircle->SetSize(D3DXVECTOR3(m_attackRange, m_attackRange * 0.6f, 0));
}

void CTurret::Selected(void)
{
	CGameInfo::GetInstance()->SetFocusedObject(this);
}

void CTurret::FindTarget(void)
{
	if (m_pTarget == nullptr && m_attackRange > 0)
	{
		bool findTarget = false;
		for (auto& monsterCC : CCollisionManager::GetInstance()->GetCollisionVector(OBJID::MONSTER))
		{
			if (CollisionHelper::PointEclipseCollision(monsterCC->GetPosition(), m_position + m_parentPosition, m_pRangeCircle->GetSize()))
			{
				m_pRangeCircle->SetStateKey("Blue");
				m_pRangeCircle->StateChangeInit();
				m_pTarget = monsterCC->GetOwner();
				findTarget = true;
				break;
			}
		}
	}
	else if (m_pTarget != nullptr && !m_pTarget->GetNeedToBeDeleted())
	{
		if (!CollisionHelper::PointEclipseCollision(m_pTarget->GetPosition(), m_position + m_parentPosition, m_pRangeCircle->GetSize()))
			m_pTarget = nullptr;
	}
	else
		m_pTarget = nullptr;

	if (m_pTarget == nullptr && m_pRangeCircle->GetStateKey() != "Green")
	{
		m_pRangeCircle->SetStateKey("Green");
		m_pRangeCircle->StateChangeInit();
	}
}

