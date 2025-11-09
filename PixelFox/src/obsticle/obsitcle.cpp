#include "obsiticle.h"

#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/utilities/logger/logger.h"


pixel_game::Obsticle::~Obsticle()
{
	Release();
}

bool pixel_game::Obsticle::Init(const INIT_OBSTICLE_DESC& desc)
{
	m_pSprite = std::make_unique<pixel_engine::QuadObject>();
	m_pSprite->SetLayer(pixel_engine::ELayer::Obstacles);
	m_pSprite->SetPosition(desc.position);
	m_pSprite->SetScale(desc.scale);
	m_pSprite->SetVisible(false);
	m_pSprite->SetTexture(desc.baseTexture);
	m_pSprite->Initialize();

	m_pAnimState = std::make_unique<pixel_engine::AnimSateMachine>(m_pSprite.get());
	
	if (desc.obsticleSprites.size() > 3)
	{
		m_pAnimState->AddFrameFromDir("general", desc.obsticleSprites);
		m_pAnimState->SetInitialState("general");
	}
	
	if (!m_pAnimState->Initialize())
	{
		pixel_engine::logger::debug(
			"Failed to initialize {}, anim state!",
			desc.szName);
	}

	m_nScale	= desc.scale;
	m_nPosition = desc.position;
	m_szName	= desc.szName;

	pixel_engine::PhysicsQueue::Instance().AddObject(m_pSprite.get());
	m_bInit = true;
	return true;
}

void  pixel_game::Obsticle::Update(float deltaTime)
{
	if (!m_bInit) return;
	if (!m_pSprite->IsVisible()) return;

	m_pAnimState->OnFrameBegin(deltaTime);
	m_pAnimState->OnFrameEnd();
}

void pixel_game::Obsticle::Draw()
{
	if (!m_bInit) return;
	if (!m_pSprite) return;
	m_pSprite->SetVisible(true); 
}

void pixel_game::Obsticle::Hide()
{
	if (!m_bInit) return;
	if (!m_pSprite) return;
	
	m_pSprite->SetVisible(false);
}

void pixel_game::Obsticle::Release()
{
	if (!m_bInit) return;
	if (!m_pSprite) return;
	pixel_engine::PhysicsQueue::Instance().RemoveObject(m_pSprite.get());
}

FVector2D pixel_game::Obsticle::GetPosition() const
{
	return m_nPosition;
}

FVector2D pixel_game::Obsticle::GetScale() const
{
	return m_nScale;
}

std::string pixel_game::Obsticle::GetName() const
{
	return m_szName;
}

pixel_engine::PEISprite* pixel_game::Obsticle::GetSpirte() const
{
	if (m_pSprite) return m_pSprite.get();
	return nullptr;
}

pixel_engine::BoxCollider* pixel_game::Obsticle::GetCollider() const
{
	if (!m_pSprite) return nullptr;
	return m_pSprite->GetCollider();
}
