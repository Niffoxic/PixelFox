#include "main_menu.h"

#include "pixel_engine/physics_manager/physics_queue.h"
#include "pixel_engine/render_manager/render_queue/render_queue.h"
#include "pixel_engine/utilities/logger/logger.h"

using namespace pixel_game;
using pixel_engine::PhysicsQueue;

MainMenu::MainMenu(const PG_MAIN_MENU_DESC& desc)
{
	m_pKeyboard					 = desc.pKeyboard;
    m_pWindows                   = desc.pWindows;
	m_fnOnEnterPress_FiniteMap	 = desc.OnEnterPress_FiniteMap;
	m_fnOnEnterPress_InfiniteMap = desc.OnEnterPress_InfiniteMap;
	m_fnOnEnterPress_Quit		 = desc.OnEnterPress_Quit;

    m_pBackgroundAnim = std::make_unique<pixel_engine::AnimSateMachine>(&m_menuBackground);
}

pixel_game::MainMenu::~MainMenu()
{
	Hide();
}

void MainMenu::Init()
{
    //~ build
    BuildFonts         ();
    BuildLayout        ();
    BuildControlsLayout();

    //~ config
    ShowMainMenu    ();
    HideControlsMenu();

    //~ defaults
    m_eState         = EMenuState::Main;
    m_bVisible       = true;
    m_nSelectedIndex = m_nItem_Finite;
}

void MainMenu::Update(float deltaTime)
{
    if (!m_bVisible) return;
    HandleInput(deltaTime);

    if (IsVisible()) 
    {
        m_pBackgroundAnim->OnFrameBegin(deltaTime);
        m_pBackgroundAnim->OnFrameEnd();
    }
}

void pixel_game::MainMenu::Show()
{
    m_bVisible = true;
    ShowCommonLayout();

    if (m_eState == EMenuState::Main) ShowMainMenu();
    else                              ShowControlsMenu();
}

void pixel_game::MainMenu::Hide()
{
    m_bVisible = false;
    HideMainMenu();
    HideControlsMenu();
    HideCommonLayout();
}

_Use_decl_annotations_
bool MainMenu::IsVisible() const
{
    return m_bVisible;
}

void MainMenu::HandleInput(float deltaTime)
{
    if (!m_pKeyboard) return;

    if (m_nInputDelay > 0.f)
        m_nInputDelay -= deltaTime;

    if (m_nInputDelay > 0.f)
        return;

    constexpr float InputCooldown = 0.2f;

    if (m_eState == EMenuState::Main)
    {
        //~ navigate main menu options
        if (m_pKeyboard->WasKeyPressed(VK_UP) || m_pKeyboard->IsKeyPressed('W'))
        {
            SelectPrev();
            m_nInputDelay = InputCooldown;
            return;
        }

        if (m_pKeyboard->WasKeyPressed(VK_DOWN) || m_pKeyboard->IsKeyPressed('S'))
        {
            SelectNext();
            m_nInputDelay = InputCooldown;
            return;
        }

        //~ activate current selection
        if (m_pKeyboard->WasKeyPressed(VK_RETURN) ||
            m_pKeyboard->WasKeyPressed(VK_SPACE))
        {
            ActivateSelected();
            m_nInputDelay = InputCooldown;
            return;
        }

        //~ quick quit
        if (m_pKeyboard->WasKeyPressed(VK_ESCAPE))
        {
            if (m_fnOnEnterPress_Quit)
                m_fnOnEnterPress_Quit();
            m_nInputDelay = InputCooldown;
            return;
        }
    }
    else //~ Controls Menu
    {
        if (m_pKeyboard->WasKeyPressed(VK_ESCAPE) ||
            m_pKeyboard->WasKeyPressed(VK_RETURN) ||
            m_pKeyboard->WasKeyPressed(VK_SPACE))
        {
            m_eState = EMenuState::Main;
            HideControlsMenu();
            ShowMainMenu();
            m_nInputDelay = InputCooldown;
            return;
        }
    }
}

void pixel_game::MainMenu::BuildLayout()
{
    //~ defaults
    float screenW = 1280.f;
    float screenH = 720.f;

    if (m_pWindows)
    {
        const int cw = m_pWindows->GetWindowsWidth();
        const int ch = m_pWindows->GetWindowsHeight();

        screenW = static_cast<float>(cw);
        screenH = static_cast<float>(ch);
    }

    constexpr float pxUnit = 32.f;
    auto U = [](float px) { return px / 32.f; };

    const float menuW = U(screenW);
    const float menuH = U(screenH);
    const float pad = U(16.f);

    //~ background
    m_menuBackground.SetPosition({ 0, 0 });
    m_menuBackground.GetCollider()->SetColliderType(pixel_engine::ColliderType::Trigger);
    m_menuBackground.SetScale({ menuW, menuH });
    m_menuBackground.SetLayer(pixel_engine::ELayer::Background);
    m_menuBackground.SetTexture("assets/menu/background/00.png");

    if (m_pBackgroundAnim)
    {
        m_pBackgroundAnim->AddFrameFromDir("city", "assets/menu/background/");
        m_pBackgroundAnim->SetInitialState("city");
        m_pBackgroundAnim->GetCurrentAnim()->SetCycleCompleteDuration(5.0f);
    }

    //~ main screen 
    const float panelW = U(640.f);
    const float panelH = U(480.f);
    m_menuScreen.SetPosition({ 0.f, 0.f });
    m_menuScreen.SetScale({ panelW - 2.f * pad, panelH - 2.f * pad });
    m_menuScreen.SetTexture("assets/menu/panel.png");

    // ~ menu items 
    const float itemW       = U(420.f);
    const float itemH       = U(64.f);
    const float gapY        = U(18.f);
    const float topInset    = U(140.f);
    const float panelTop    = +panelH * 0.5f;
    const float firstCY     = panelTop - topInset - itemH * 0.5f;
    const float cx          = 0.f;

    const float upwardOffset = U(-180.f); 

    //~ finite map option
    auto pos = fox_math::Vector2D<float>(cx, firstCY + (itemH + gapY) * 0.f + upwardOffset);
    auto scl = fox_math::Vector2D<float>(itemW, itemH);
    m_menuFiniteMap.SetPosition(pos);
    m_menuFiniteMap.SetScale(scl);
    m_menuFiniteMap.SetTexture("assets/menu/option.png");

    m_menuHoverFiniteMap.SetPosition(pos);
    m_menuHoverFiniteMap.SetScale(scl);
    m_menuHoverFiniteMap.SetTexture("assets/menu/option_hover.png");

    //~ infinite map option
    pos = fox_math::Vector2D<float>(cx, firstCY + (itemH + gapY) * 1.f + upwardOffset);
    scl = fox_math::Vector2D<float>(itemW, itemH);
    m_menuInfiniteMap.SetPosition(pos);
    m_menuInfiniteMap.SetScale(scl);
    m_menuInfiniteMap.SetTexture("assets/menu/option.png");

    m_menuHoverInfiniteMap.SetPosition(pos);
    m_menuHoverInfiniteMap.SetScale(scl);
    m_menuHoverInfiniteMap.SetTexture("assets/menu/option_hover.png");

    //~ controls option
    pos = fox_math::Vector2D<float>(cx, firstCY + (itemH + gapY) * 2.f + upwardOffset);
    scl = fox_math::Vector2D<float>(itemW, itemH);
    m_menuControlsOption.SetPosition(pos);
    m_menuControlsOption.SetScale(scl);
    m_menuControlsOption.SetTexture("assets/menu/option.png");

    m_menuHoverControlsOption.SetPosition(pos);
    m_menuHoverControlsOption.SetScale(scl);
    m_menuHoverControlsOption.SetTexture("assets/menu/option_hover.png");

    //~ quit option
    pos = fox_math::Vector2D<float>(cx, firstCY + (itemH + gapY) * 3.f + upwardOffset);
    scl = fox_math::Vector2D<float>(itemW, itemH);
    m_menuQuitOption.SetPosition(pos);
    m_menuQuitOption.SetScale(scl);
    m_menuQuitOption.SetTexture("assets/menu/option.png");

    m_menuHoverQuitOption.SetPosition(pos);
    m_menuHoverQuitOption.SetScale(scl);
    m_menuHoverQuitOption.SetTexture("assets/menu/option_hover.png");

    //~ visibility
    const bool inMain = (m_eState == EMenuState::Main);
    m_menuHoverFiniteMap.SetVisible(inMain && m_nSelectedIndex == m_nItem_Finite);
    m_menuHoverInfiniteMap.SetVisible(inMain && m_nSelectedIndex == m_nItem_Infinite);
    m_menuHoverControlsOption.SetVisible(inMain && m_nSelectedIndex == m_nItem_Controls);
    m_menuHoverQuitOption.SetVisible(inMain && m_nSelectedIndex == m_nItem_Quit);

    // buttons visible
    m_menuFiniteMap.SetVisible(true);
    m_menuInfiniteMap.SetVisible(true);
    m_menuControlsOption.SetVisible(true);
    m_menuQuitOption.SetVisible(true);
}


void pixel_game::MainMenu::BuildFonts()
{
    m_menuTexts.clear();

    float screenW = 1280.f;
    float screenH = 720.f;
    if (m_pWindows)
    {
        const int cw = m_pWindows->GetWindowsWidth();
        const int ch = m_pWindows->GetWindowsHeight();
        if (cw > 0) screenW = static_cast<float>(cw);
        if (ch > 0) screenH = static_cast<float>(ch);
    }

    { // title common
        auto title = std::make_unique<pixel_engine::PEFont>();
        title->SetPx(64);
        const float y = 60.f;
        const float x = screenW * 0.3f;
        title->SetPosition({ x, y });
        title->SetText("Pixel Fox");
        m_menuTexts.push_back(std::move(title));
    }

    { // esc hint common
        auto hint = std::make_unique<pixel_engine::PEFont>();
        hint->SetPx(16);
        hint->SetPosition({ screenW - 300.f, 24.f });
        hint->SetText("ESC to quit/back");
        m_menuTexts.push_back(std::move(hint));
    }

    const float baseY = screenH * 0.33f;
    const float stepY = 80.f;          
    const float textX = screenW * 0.35f;

    { // regular map
        m_menuFiniteMapText.SetPx(32);
        m_menuFiniteMapText.SetPosition({ textX + 35, baseY });
        m_menuFiniteMapText.SetText("Regular Map");
    }

    { // hardcore map
        m_menuInfiniteMapText.SetPx(32);
        m_menuInfiniteMapText.SetPosition({ textX, baseY + stepY });
        m_menuInfiniteMapText.SetText("Hardcore Map");
    }

    { // controls option
        m_menuControlsOptionText.SetPx(32);
        m_menuControlsOptionText.SetPosition({ textX + 50, baseY + stepY * 2.f });
        m_menuControlsOptionText.SetText("Controls");
    }

    { // quit option
        m_menuQuitOptionText.SetPx(32);
        m_menuQuitOptionText.SetPosition({ textX + 125, baseY + stepY * 3.f });
        m_menuQuitOptionText.SetText("Quit");
    }
}

void pixel_game::MainMenu::BuildControlsLayout()
{
    //~ window size
    float screenW = 1280.f;
    float screenH = 720.f;
    if (m_pWindows)
    {
        const int cw = m_pWindows->GetWindowsWidth();
        const int ch = m_pWindows->GetWindowsHeight();
        if (cw > 0) screenW = static_cast<float>(cw);
        if (ch > 0) screenH = static_cast<float>(ch);
    }

    //~ world units
    auto U = [](float px) { return px / 32.f; };

    //~ larger controls panel
    const float panelW = U(800.f);
    const float panelH = U(560.f);

    m_controlsPanel.SetPosition({ 0.f, 2.f });
    m_controlsPanel.SetScale({ panelW, panelH });
    m_controlsPanel.SetTexture("assets/menu/panel.png");
    if (auto* colP = m_controlsPanel.GetCollider())
        colP->SetColliderType(pixel_engine::ColliderType::Trigger);

    //~ controls
    m_controlsPanelTexts.clear();

    const float startX = screenW * 0.30f;  
    const float descX  = screenW * 0.49f;  
    const float startY = screenH * 0.25f;  
    const float stepY  = 50.f; 

    struct Row { const char* key; const char* desc; float y; };
    Row rows[] = 
    {
        { "W",      "Move Upward",                  startY + stepY * 0.f },
        { "A",      "Move Left",                    startY + stepY * 1.f },
        { "S",      "Move Downward",                startY + stepY * 2.f },
        { "D",      "Move Right",                   startY + stepY * 3.f },
        { "E",      "Special Attack",               startY + stepY * 4.f },
        { "SPACE",  "Dash",                         startY + stepY * 5.f },
        { "Ctrl+S", "Save Current Game State",      startY + stepY * 6.f },
        { "Ctrl+L", "Load Last Saved State",        startY + stepY * 7.f },
        { "Esc",    "To go back or open main menu", startY + stepY * 8.f },
        { "F/G",    "Show FPS/ Set HP = 10000",                     startY + stepY * 9.f },
    };

    for (const auto& r : rows)
    {
        //~ key
        {
            pixel_engine::PEFont key;
            key.SetPx(32);                      
            key.SetPosition({ startX, r.y });
            key.SetText(r.key);           
            m_controlsPanelTexts.push_back(std::move(key));
        }
        //~ description
        {
            pixel_engine::PEFont desc;
            desc.SetPx(16);                    
            desc.SetPosition({ descX, r.y });
            desc.SetText(r.desc);    
            m_controlsPanelTexts.push_back(std::move(desc));
        }
    }
}

void pixel_game::MainMenu::ShowCommonLayout()
{
    //~ background and fonts
    auto& physicsQ = pixel_engine::PhysicsQueue::Instance();
    physicsQ.AddObject(&m_menuBackground);

    auto& renderQ = pixel_engine::PERenderQueue::Instance();
    for (auto& font : m_menuTexts)
        if (font) renderQ.AddFont(font.get());
}

void pixel_game::MainMenu::HideCommonLayout()
{
    //~ background and fonts
    auto& physicsQ = pixel_engine::PhysicsQueue::Instance();
    physicsQ.RemoveObject(&m_menuBackground);

    auto& renderQ = pixel_engine::PERenderQueue::Instance();
    for (auto& font : m_menuTexts)
        if (font) renderQ.RemoveFont(font.get());
}

void pixel_game::MainMenu::ShowMainMenu()
{
    auto& q = pixel_engine::PhysicsQueue::Instance();

    // inner screen panel
    //q.AddObject(&m_menuScreen);

    // options
    q.AddObject(&m_menuFiniteMap);
    q.AddObject(&m_menuInfiniteMap);
    q.AddObject(&m_menuControlsOption);
    q.AddObject(&m_menuQuitOption);

    // hover overlays
    q.AddObject(&m_menuHoverFiniteMap);
    q.AddObject(&m_menuHoverInfiniteMap);
    q.AddObject(&m_menuHoverControlsOption);
    q.AddObject(&m_menuHoverQuitOption);

    // fonts
    auto& renderQ = pixel_engine::PERenderQueue::Instance();
    renderQ.AddFont(&m_menuFiniteMapText);
    renderQ.AddFont(&m_menuInfiniteMapText);
    renderQ.AddFont(&m_menuControlsOptionText);
    renderQ.AddFont(&m_menuQuitOptionText);
}

void pixel_game::MainMenu::HideMainMenu()
{
    auto& q = pixel_engine::PhysicsQueue::Instance();

    //q.RemoveObject(&m_menuScreen);

    q.RemoveObject(&m_menuFiniteMap);
    q.RemoveObject(&m_menuInfiniteMap);
    q.RemoveObject(&m_menuControlsOption);
    q.RemoveObject(&m_menuQuitOption);

    q.RemoveObject(&m_menuHoverFiniteMap);
    q.RemoveObject(&m_menuHoverInfiniteMap);
    q.RemoveObject(&m_menuHoverControlsOption);
    q.RemoveObject(&m_menuHoverQuitOption);

    // fonts
    auto& renderQ = pixel_engine::PERenderQueue::Instance();
    renderQ.RemoveFont(&m_menuFiniteMapText);
    renderQ.RemoveFont(&m_menuInfiniteMapText);
    renderQ.RemoveFont(&m_menuControlsOptionText);
    renderQ.RemoveFont(&m_menuQuitOptionText);
}

void pixel_game::MainMenu::ShowControlsMenu()
{
    auto& physicsQ = pixel_engine::PhysicsQueue::Instance();
    physicsQ.AddObject(&m_controlsPanel);

    // fonts for control keys
    auto& renderQ = pixel_engine::PERenderQueue::Instance();
    for (auto& font : m_controlsPanelTexts)
        renderQ.AddFont(&font);
}

void pixel_game::MainMenu::HideControlsMenu()
{
    auto& physicsQ = pixel_engine::PhysicsQueue::Instance();
    physicsQ.RemoveObject(&m_controlsPanel);

    // remove control fonts
    auto& renderQ = pixel_engine::PERenderQueue::Instance();
    for (auto& font : m_controlsPanelTexts)
        renderQ.RemoveFont(&font);
}

void pixel_game::MainMenu::SelectNext()
{
    const int prev = m_nSelectedIndex;
    m_nSelectedIndex = (m_nSelectedIndex + 1) % m_ItemCount;

    if (m_eState == EMenuState::Main && prev != m_nSelectedIndex)
    {
        m_menuHoverFiniteMap.SetVisible(m_nSelectedIndex == m_nItem_Finite);
        m_menuHoverInfiniteMap.SetVisible(m_nSelectedIndex == m_nItem_Infinite);
        m_menuHoverControlsOption.SetVisible(m_nSelectedIndex == m_nItem_Controls);
        m_menuHoverQuitOption.SetVisible(m_nSelectedIndex == m_nItem_Quit);
    }
}

void pixel_game::MainMenu::SelectPrev()
{
    const int prev = m_nSelectedIndex;
    m_nSelectedIndex = (m_nSelectedIndex - 1 + m_ItemCount) % m_ItemCount;

    if (m_eState == EMenuState::Main && prev != m_nSelectedIndex)
    {
        m_menuHoverFiniteMap.SetVisible(m_nSelectedIndex == m_nItem_Finite);
        m_menuHoverInfiniteMap.SetVisible(m_nSelectedIndex == m_nItem_Infinite);
        m_menuHoverControlsOption.SetVisible(m_nSelectedIndex == m_nItem_Controls);
        m_menuHoverQuitOption.SetVisible(m_nSelectedIndex == m_nItem_Quit);
    }
}

void pixel_game::MainMenu::ActivateSelected()
{
    if (m_eState != EMenuState::Main)
        return;

    switch (m_nSelectedIndex)
    {
    case m_nItem_Finite:
        if (m_fnOnEnterPress_FiniteMap) m_fnOnEnterPress_FiniteMap();
        break;

    case m_nItem_Infinite:
        if (m_fnOnEnterPress_InfiniteMap) m_fnOnEnterPress_InfiniteMap();
        break;

    case m_nItem_Controls:
        m_eState = EMenuState::Controls;
        HideMainMenu();
        ShowControlsMenu();
        break;

    case m_nItem_Quit:
        if (m_fnOnEnterPress_Quit) m_fnOnEnterPress_Quit();
        break;

    default:
        break;
    }
}
