#ifndef _GUI_POPUP_H
#define _GUI_POPUP_H

#include "guiFrame.h"
#include "guiDocument.h"
#include "guiButton.h"
#include "guiEditBox.h"

#define POPUP_EFFECT_OUTRO  200
#define POPUP_EFFECT_FOCUS  201
#define POPUP_EFFECT_INTRO  202

class InputEngine;

class guiPopupDocument : public guiDocument
{
public:
    guiPopupDocument()
    {
        m_pClickedComponent = NULL;
    };
    virtual bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt)
    {
        m_pClickedComponent = pCpnt;
        return true;
    };
    guiComponent * getClickedComponent()
    {
        guiComponent * pCpnt = m_pClickedComponent;
        m_pClickedComponent = NULL;
        return pCpnt;
    };

protected:
    guiComponent * m_pClickedComponent;
};

class guiPopup : public guiFrame
{
public:
    ~guiPopup();

    void update(double delta);

    // Member access
    guiEditBox * getEditBox()
    {
        return (guiEditBox*) m_pDoc->getComponent("DefaultEditBox");
    };
    guiComponent * getClickedComponent()
    {
        return ((guiPopupDocument*)getDocument())->getClickedComponent();
    };
    guiButton * getButton(int iButton);

    // Static default constructors
    static guiPopup * createEmptyPopup(DisplayEngine * pDisplay);
    static guiPopup * createYesNoPopup(const char * sText, DisplayEngine * pDisplay);
    static guiPopup * createOkAutoclosePopup(const char * sText, DisplayEngine * pDisplay);
    static guiPopup * createOkCancelPopup(const char * sText, DisplayEngine * pDisplay);
    static guiPopup * createTextAndMultiButtonsPopup(const char * sText, int iNbButtons, int iWidth, DisplayEngine * pDisplay);
    static guiPopup * createTextInputPopup(const char * sText, int iNbLines, bool bMultiLines, int iBoxWidth, InputEngine * pInput, DisplayEngine * pDisplay);
    static guiPopup * createTimedPopup(const char * sText, double fTimer, int iWidth, DisplayEngine * pDisplay);

protected:
    guiPopup(); // constructor is private ; use static constructor functions instead, or write subclass
    double m_fTimer;
    bool m_bAutoClose;
};

#endif
