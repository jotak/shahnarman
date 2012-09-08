#ifndef _GUI_TOGGLEBUTTON_H
#define _GUI_TOGGLEBUTTON_H

#include "guiButton.h"

class guiToggleButton : public guiButton
{
public:
    guiToggleButton();
    ~guiToggleButton();

    virtual u32 getType()
    {
        return guiButton::getType() | GOTYPE_TOGGLEBUTTON;
    };
    virtual guiObject * onButtonEvent(ButtonAction * pEvent);
    virtual void setClickState(bool bClickState)
    {
        m_bClickState = bClickState;
    };
    bool getClickState()
    {
        return m_bClickState;
    };

    // Clone / init
    virtual guiObject * clone();

    // Static default constructors
    static guiToggleButton * createDefaultTexturedToggleButton(int iTex, int iSize, const char * sId, DisplayEngine * pDisplay);
    static guiToggleButton * createDefaultCheckBox(const char * sId, DisplayEngine * pDisplay);
};

#endif
