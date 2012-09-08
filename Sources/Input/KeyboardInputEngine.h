#ifndef _KEYBOARD_INPUT_ENGINE_H
#define _KEYBOARD_INPUT_ENGINE_H

#include "InputEngine.h"

#define SPECKEY_HOME    1
#define SPECKEY_END     2
#define SPECKEY_UP      3
#define SPECKEY_DOWN    4
#define SPECKEY_LEFT    5
#define SPECKEY_RIGHT   6

class KeyboardListener;

class KeyboardInputEngine : public InputEngine
{
public:
    // Constructor / destructor
    KeyboardInputEngine() : InputEngine(true)
    {
        m_pKeyboardListener = NULL;
    };

    // Data providing functions
    virtual bool isShiftPressed() = 0;
    virtual bool isCtrlPressed() = 0;

    // Listeners
    void setKeyboardListener(KeyboardListener * pKeyboardListener)
    {
        m_pKeyboardListener = pKeyboardListener;
    };
    void unsetKeyboardListener(KeyboardListener * pKeyboardListener)
    {
        if (m_pKeyboardListener == pKeyboardListener) m_pKeyboardListener = NULL;
    };

protected:
    KeyboardListener * m_pKeyboardListener;
};

#endif
