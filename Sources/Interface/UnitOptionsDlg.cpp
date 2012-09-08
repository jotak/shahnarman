#include "UnitOptionsDlg.h"
#include "InterfaceManager.h"
#include "SpellDlg.h"
#include "../LocalClient.h"
#include "../Data/LocalisationTool.h"
#include "../GUIClasses/guiToggleButton.h"
#include "../GUIClasses/guiFrame.h"
#include "../Gameboard/Unit.h"
#include "../Gameboard/GameboardManager.h"
#include "../Gameboard/GameboardInputs.h"
#include "../Players/PlayerManager.h"

#define BUTTON_SIZE   50
#define SPACING       4

// -----------------------------------------------------------------
// Name : UnitOptionsDlg
//  Constructor
// -----------------------------------------------------------------
UnitOptionsDlg::UnitOptionsDlg(LocalClient * pLocalClient) : guiDocument()
{
    m_pUnit = NULL;
    m_pLocalClient = pLocalClient;
    setEnabled(false);

    char sTitle[64];
    i18n->getText("UNIT_OPTIONS", sTitle, 64);
    init(sTitle,
         pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:WinBg"),
         0, 0, 3 * BUTTON_SIZE + 2 * SPACING, BUTTON_SIZE, pLocalClient->getDisplay());

    i18n->getText("DEFEND", sTitle, 64);
    guiToggleButton * pBtn = new guiToggleButton();
    pBtn->init("", H2_FONT, H2_COLOR,
               pLocalClient->getDisplay()->getTextureEngine()->findTexture("interface:Selector"),
               BCO_AddTex, -1, BCO_Decal,
               pLocalClient->getDisplay()->getTextureEngine()->loadTexture("defend_icon"),
               "FortifyButton", 0, 0, BUTTON_SIZE, BUTTON_SIZE, pLocalClient->getDisplay());
    pBtn->setTooltipText(sTitle);
    addComponent(pBtn);

    pBtn = (guiToggleButton*) pBtn->clone();
    pBtn->setTooltipText(i18n->getText("MOVE_TO", sTitle, 64));
    pBtn->setNormalTexture(pLocalClient->getDisplay()->getTextureEngine()->loadTexture("moveto_icon"));
    pBtn->setId("MoveToButton");
    pBtn->moveBy(BUTTON_SIZE + SPACING, 0);
    addComponent(pBtn);

    pBtn = (guiToggleButton*) pBtn->clone();
    pBtn->setTooltipText(i18n->getText("ATTACK", sTitle, 64));
    pBtn->setNormalTexture(pLocalClient->getDisplay()->getTextureEngine()->loadTexture("attack_icon"));
    pBtn->setId("AttackButton");
    pBtn->moveBy(BUTTON_SIZE + SPACING, 0);
    addComponent(pBtn);
}

// -----------------------------------------------------------------
// Name : ~UnitOptionsDlg
//  Destructor
// -----------------------------------------------------------------
UnitOptionsDlg::~UnitOptionsDlg()
{
#ifdef DBG_VERBOSE1
    printf("Begin destroy UnitOptionsDlg\n");
#endif
#ifdef DBG_VERBOSE1
    printf("End destroy UnitOptionsDlg\n");
#endif
}

// -----------------------------------------------------------------
// Name : onButtonEvent
// -----------------------------------------------------------------
bool UnitOptionsDlg::onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
{
    guiToggleButton * pBtn = (guiToggleButton*)pCpnt;
    if (strcmp(pBtn->getId(), "FortifyButton") == 0)
    {
        if (pBtn->getClickState())
        {
            resetOtherButtons(pBtn);
            m_pUnit->getMap()->setFortifyGroupOrder(m_pUnit);
            m_pLocalClient->getGameboard()->getInputs()->setMouseMode(ModeNormal);
            m_pLocalClient->getGameboard()->updateUnitOrder(m_pUnit);
        }
        else
            m_pUnit->getMap()->unsetGroupOrder(m_pUnit);
    }
    else if (strcmp(pBtn->getId(), "MoveToButton") == 0)
    {
        if (pBtn->getClickState())
        {
            m_pUnit->getMap()->saveCurrentGroupOrder(m_pUnit);
            resetOtherButtons(pBtn);
            m_pLocalClient->getGameboard()->getInputs()->setMouseMode(ModeMoveToTarget);
        }
        else
        {
            if (m_pLocalClient->getGameboard()->getInputs()->getMouseMode() == ModeMoveToTarget)
                updateOrder();
            else
                m_pUnit->getMap()->unsetGroupOrder(m_pUnit);
            m_pLocalClient->getGameboard()->getInputs()->setMouseMode(ModeNormal);
            m_pLocalClient->getGameboard()->updateUnitOrder(m_pUnit);
        }
    }
    else if (strcmp(pBtn->getId(), "AttackButton") == 0)
    {
        if (pBtn->getClickState())
        {
            m_pUnit->getMap()->saveCurrentGroupOrder(m_pUnit);
            resetOtherButtons(pBtn);
            m_pLocalClient->getGameboard()->getInputs()->setMouseMode(ModeAttackTarget);
        }
        else
        {
            if (m_pLocalClient->getGameboard()->getInputs()->getMouseMode() == ModeAttackTarget)
                updateOrder();  // Order hasn't been changed yet => retrieve order
            else
                m_pUnit->getMap()->unsetGroupOrder(m_pUnit);
            m_pLocalClient->getGameboard()->getInputs()->setMouseMode(ModeNormal);
            m_pLocalClient->getGameboard()->updateUnitOrder(m_pUnit);
        }
    }
    else if (strcmp(pBtn->getId(), "SkillButton") == 0)
    {
        if (pBtn->getClickState())
        {
            // Make sure the player still has enough mana
            ChildEffect * pEffect = (ChildEffect*)pBtn->getAttachment();
            if (m_pLocalClient->getInterface()->getSpellDialog()->takeMana(pEffect->cost))
            {
                m_pUnit->getMap()->unsetGroupOrder(m_pUnit);
                resetOtherButtons(pBtn);
                m_pLocalClient->getPlayerManager()->doSkillEffect(m_pUnit, pEffect);
                m_pLocalClient->getGameboard()->updateUnitOrder(m_pUnit);
            }
            else
                pBtn->setClickState(false);
        }
        else
        {
            extern void clbkSelectTarget_cancelSelection(u32, int);
            clbkSelectTarget_cancelSelection(LUAOBJECT_SKILL, 0);
        }
    }
    return true;
}

// -----------------------------------------------------------------
// Name : resetOtherButtons
// -----------------------------------------------------------------
void UnitOptionsDlg::resetOtherButtons(guiToggleButton * pBtn)
{
    guiToggleButton * btn2 = (guiToggleButton*) getFirstComponent();
    while (btn2 != NULL)
    {
        if (btn2 != pBtn)
            btn2->setClickState(false);
        btn2 = (guiToggleButton*)getNextComponent();
    }
}

// -----------------------------------------------------------------
// Name : updateOrder
// -----------------------------------------------------------------
void UnitOptionsDlg::updateOrder()
{
    assert(m_pUnit != NULL);

    guiToggleButton * pSelBtn = NULL;
    switch (m_pUnit->getOrder())
    {
    case OrderFortify:
        pSelBtn = (guiToggleButton*) getComponent("FortifyButton");
        break;
    case OrderMove:
        pSelBtn = (guiToggleButton*) getComponent("MoveToButton");
        break;
    case OrderAttack:
        pSelBtn = (guiToggleButton*) getComponent("AttackButton");
        break;
    case OrderSkill:
    {
        // Find button associated to this skill effect
        ChildEffect * pEffect = m_pUnit->getSkillOrder();
        assert(pEffect != NULL);
        guiComponent * pCpnt = getFirstComponent();
        while (pCpnt != NULL)
        {
            if (pCpnt->getAttachment() == pEffect)
            {
                pSelBtn = (guiToggleButton*) pCpnt;
                break;
            }
            pCpnt = getNextComponent();
        }
        assert(pSelBtn != NULL);
        break;
    }
    default:
        break;
    }
    if (pSelBtn != NULL)    // Not Order_None
        pSelBtn->setClickState(true);
    resetOtherButtons(pSelBtn);
}

// -----------------------------------------------------------------
// Name : setUnit
// -----------------------------------------------------------------
void UnitOptionsDlg::setUnit(Unit * unit)
{
    // Remove any skill button
    guiToggleButton * btn = (guiToggleButton*) getFirstComponent();
    while (btn != NULL)
    {
        if (strcmp(btn->getId(), "SkillButton") == 0)
            btn = (guiToggleButton*) deleteCurrentComponent(true);
        else
            btn = (guiToggleButton*) getNextComponent();
    }

    if (unit != NULL)
    {
        // Add new skill buttons (loop through each skill and each skill activable effect)
        int xPxl = 3 * BUTTON_SIZE + 3 * SPACING;
        LuaObject * pLua = unit->getFirstEffect(0);
        while (pLua != NULL)
        {
            if (pLua->getType() == LUAOBJECT_SKILL)
            {
                Skill * pSkill = (Skill*) pLua;
                int nbEffects = pSkill->getNbChildEffects();
                for (int i = 0; i < nbEffects; i++)
                {
                    ChildEffect * pEffect = pSkill->getChildEffect(i);

                    // Create toggle button
                    guiToggleButton * pBtn = new guiToggleButton();
                    pBtn->init("", H2_FONT, H2_COLOR,
                               getDisplay()->getTextureEngine()->findTexture("interface:Selector"),
                               BCO_AddTex, -1, BCO_Decal,
                               getDisplay()->getTextureEngine()->loadTexture(pEffect->sIcon),
                               "SkillButton", xPxl, 0, BUTTON_SIZE, BUTTON_SIZE, getDisplay());
                    pBtn->setAttachment(pEffect);
                    pBtn->setTooltipText(pSkill->getLocalizedDescription());
                    addComponent(pBtn);
                    xPxl += BUTTON_SIZE + SPACING;
                }
            }
            pLua = unit->getNextEffect(0);
        }
        setWidth(xPxl - SPACING);
    }

    m_pUnit = unit;
    if (unit != NULL)
    {
        setEnabled(true);
        guiFrame * pFrm = m_pLocalClient->getInterface()->findFrameFromDoc(this);
        pFrm->updateSizeFit();
        pFrm->moveTo(m_pLocalClient->getClientParameters()->screenXSize / 2 - pFrm->getWidth() / 2, pFrm->getYPos());
        updateOrder();
    }
    else
        setEnabled(false);
}

// -----------------------------------------------------------------
// Name : cancelSkillAction
//  Note: pUnit is null when the function is called (directly or not) from UnitOptionsDlg itself.
//  In this case, we also want to update unit order
//  When pUnit is not NULL, it means that the unit's order is going to be modified from somewhere else... then we just want to restitute mana.
// -----------------------------------------------------------------
void UnitOptionsDlg::cancelSkillAction(Unit * pUnit)
{
    ChildEffect * pEffect = NULL;
    if (pUnit == NULL)
        pEffect = m_pUnit->getSkillOrder();
    else
        pEffect = pUnit->getSkillOrder();
    // Restitute mana
    assert(pEffect != NULL);
    m_pLocalClient->getInterface()->getSpellDialog()->restituteMana(pEffect->cost + ((LuaObject*)pEffect->getAttachment())->getExtraMana());
    if (pUnit == NULL)
    {
        m_pUnit->getMap()->unsetGroupOrder(m_pUnit);
        m_pLocalClient->getGameboard()->updateUnitOrder(m_pUnit);
    }
}

// -----------------------------------------------------------------
// Name : redoSkillAction
// -----------------------------------------------------------------
void UnitOptionsDlg::redoSkillAction(Unit * pUnit)
{
    ChildEffect * pEffect = pUnit->getSkillOrder();
    // Take mana again
    if (!m_pLocalClient->getInterface()->getSpellDialog()->takeMana(pEffect->cost))
    {
        m_pUnit->getMap()->unsetGroupOrder(pUnit);
        m_pLocalClient->getGameboard()->updateUnitOrder(pUnit);
    }
}
