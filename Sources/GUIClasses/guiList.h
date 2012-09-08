#ifndef _GUI_LIST_H
#define _GUI_LIST_H

#include "guiContainer.h"
#include "guiLabel.h"
#include "../Input/EventListener.h"
#include "../Input/KeyboardListener.h"
#include "../Common/SortInterface.h"

class KeyboardInputEngine;
class guiList;

class guiList : public guiContainer, public KeyboardListener, public SortInterface, public EventListener
{
public:
    // class guiListLabel
    class guiListLabel : public guiLabel
    {
    public:
        guiListLabel()
        {
            m_bSelected = false;
        };
        virtual guiObject * onButtonEvent(ButtonAction * pEvent);
        virtual bool isAt(int xPxl, int yPxl)
        {
            int w = m_iWidth;
            m_iWidth = m_iBoxWidth;
            bool b = guiLabel::isAt(xPxl, yPxl);
            m_iWidth = w;
            return b;
        };
        bool isSelected()
        {
            return m_bSelected;
        };
        void setSelected(bool bSel)
        {
            m_bSelected = bSel;
        };

    protected:
        bool m_bSelected;
    };
    // class guiListDocument
    class guiListDocument : public guiDocument
    {
    public:
        guiListDocument(guiList * pList)
        {
            m_pOwner = pList;
        };
        virtual bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);
    protected:
        guiList * m_pOwner;
    };

    // Constructor / destructor
    guiList(KeyboardInputEngine * pInputs);
    ~guiList();

    // Inherited functions
    virtual u32 getType()
    {
        return guiComponent::getType();
    };
    virtual bool onKeyDown(unsigned char c);
    virtual void displayAt(int iXOffset, int iYOffset, F_RGBA cpntColor = F_RGBA_NULL, F_RGBA docColor = F_RGBA_NULL);

    // Input functions
    virtual guiObject * onButtonEvent(ButtonAction * pEvent);
    virtual bool onCatchButtonEvent(ButtonAction * pEvent);

    // Events
    virtual void onFocusLost();
    virtual bool onButtonEvent(ButtonAction * pEvent, guiComponent * pCpnt);

    // Member access
    FontId getFontId()
    {
        return m_FontId;
    };
    void setFontId(FontId id)
    {
        m_FontId = id;
    };
    void setTextColor(F_RGBA textColor)
    {
        m_TextColor = textColor;
    };
    F_RGBA getTextColor()
    {
        return m_TextColor;
    };
    InputButton getActionOnSelection()
    {
        InputButton action = m_ActionOnSelection;
        m_ActionOnSelection = (InputButton)0;
        return action;
    };
    guiListLabel * getLastSelectedLabel()
    {
        return m_pLastSelectedLabel;
    };
    bool hasFocus()
    {
        return m_bHasFocus;
    };

    // Clone / init
    virtual void init(FontId fontId, F_RGBA textColor, int * iMainTexs, const char * sCpntId, int xPxl, int yPxl, int wPxl, int hPxl, DisplayEngine * pDisplay);
    virtual guiObject * clone();

    // Other
    void setFocus();
    guiListLabel * addItem(char * sText, char * sId);
    void clear();
    guiListLabel * getFirstSelectedItem();
    guiListLabel * getNextSelectedItem();
    void removeSelection();
    int count()
    {
        return getDocument()->getComponentsList()->size;
    };

    // Sort
    void sort();
    bool sortCompare(BaseObject * A, BaseObject * B);

    // Static default constructors
    static guiList * createDefaultList(int width, int height, const char * sId, KeyboardInputEngine * pInputs, DisplayEngine * pDisplay);

protected:
    void updateSelectionGeometry();
    void shiftSelect(guiListLabel * pClickedLabel);

    GeometryQuads * m_pSelectionGeometry;
    FontId m_FontId;
    F_RGBA m_TextColor;
    bool m_bHasFocus;
    KeyboardInputEngine * m_pInputs;
    guiListLabel * m_pLastSelectedLabel;
    bool m_bCatchMouseUp;
    InputButton m_ActionOnSelection;
};

#endif
