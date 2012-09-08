#ifndef _GUI_SPELL_H
#define _GUI_SPELL_H

#include "../GUIClasses/guiDocument.h"
#include "../Players/Mana.h"

class Player;
class NetworkData;
class GeometryQuads;
class guiImage;
class LocalClient;
class Spell;

class SpellDlg : public guiDocument
{
public:
    SpellDlg(LocalClient * pLocalClient, bool bHideSpells);
    ~SpellDlg();

    void setCustomData(const char * sKey, const char * sData, DisplayEngine * pDisplay);
    bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    void updateContent(Player * pPlayer, bool bOnlyInstants = false, bool bCantCastSpells = false);
    void cancelCastSpell(Spell * pSpell);
    void updateManaLabels();
    void getCastSpellsData(NetworkData * pData);
    Player * getCurrentCaster()
    {
        return m_pCurrentPlayer;
    };
    int getNumberOfClickedSpells();
    bool takeMana(Mana mana);
    void restituteMana(Mana mana);
    Mana getRemainingMana()
    {
        return m_RemainingMana;
    };
    void disableEOT(bool bDisabled);
    void updateSpellInformation(Spell * pSpell);

protected:
    LocalClient * m_pLocalClient;
    Player * m_pCurrentPlayer;
    Mana m_RemainingMana;
    Spell * m_pCanceledSpell;
    bool m_bOnlyInstants;
    bool m_bCantCastSpells;
};

#endif
