#include "PCInputEngine.h"
#include "KeyboardListener.h"
#include "../SystemHeaders.h"

// -----------------------------------------------------------------
// Name : PCInputEngine
// Constructor
// -----------------------------------------------------------------
PCInputEngine::PCInputEngine()
{
    m_cZ = 'w'; // TODO : do it better
    m_cX = 'x';
    m_cN = 'n';
    m_cB = 'b';
    m_iModifiers = 0;
}

// -----------------------------------------------------------------
// Name : ~PCInputEngine
// Destructor
// -----------------------------------------------------------------
PCInputEngine::~PCInputEngine()
{
}

// -----------------------------------------------------------------
// Name : onKeyboard
// -----------------------------------------------------------------
void PCInputEngine::onKeyboard(unsigned char key, int x, int y)
{
    m_iModifiers = glutGetModifiers();
    if (m_pKeyboardListener != NULL)
    {
        if (m_pKeyboardListener->onKeyDown(key))
            return;
    }
    InputButton eButton;
    if (key == '\n' || key == 13)
        eButton = ButtonStart;
    else if (key == 27)
        eButton = ButtonBack;
    else if (key == m_cZ)
        eButton = ButtonZ;
    else if (key == m_cX)
        eButton = ButtonX;
    else if (key == m_cN)
        eButton = ButtonNext;
    else if (key == m_cB)
        eButton = ButtonPrev;
    else  // This button isn't used
        return;
    onPressButton(eButton);
    onReleaseButton(eButton);
}

// -----------------------------------------------------------------
// Name : onSpecialKeyboard
// -----------------------------------------------------------------
void PCInputEngine::onSpecialKeyboard(int key, int x, int y)
{
    m_iModifiers = glutGetModifiers();
    InputButton eButton = (InputButton)0;
    int speckey = -1;
    switch (key)
    {
    case GLUT_KEY_LEFT:
        eButton = ButtonLeft;
        speckey = SPECKEY_LEFT;
        break;
    case GLUT_KEY_RIGHT:
        eButton = ButtonRight;
        speckey = SPECKEY_RIGHT;
        break;
    case GLUT_KEY_UP:
        eButton = ButtonUp;
        speckey = SPECKEY_UP;
        break;
    case GLUT_KEY_DOWN:
        eButton = ButtonDown;
        speckey = SPECKEY_DOWN;
        break;
    case GLUT_KEY_HOME:
        speckey = SPECKEY_HOME;
        break;
    case GLUT_KEY_END:
        speckey = SPECKEY_END;
        break;
    }
    if (m_pKeyboardListener != NULL && speckey != -1)
    {
        if (m_pKeyboardListener->onSpecialKeyDown(speckey))
            return;
    }
    if (eButton != (InputButton)0)
    {
        onPressButton(eButton);
        onReleaseButton(eButton);
    }
}

// -----------------------------------------------------------------
// Name : onMouseClick
// -----------------------------------------------------------------
void PCInputEngine::onMouseClick(int button, int state, int x, int y)
{
    m_iModifiers = glutGetModifiers();
    InputButton eButton;
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        eButton = Button1;
        break;
    case GLUT_RIGHT_BUTTON:
        eButton = Button2;
        break;
    case GLUT_WHEEL_UP:
        onPressButton(ButtonZ);
        onReleaseButton(ButtonZ);
        return;
    case GLUT_WHEEL_DOWN:
        onPressButton(ButtonX);
        onReleaseButton(ButtonX);
        return;
    default:  // This button isn't used
        return;
    }
    if (state == GLUT_DOWN)
        onPressButton(eButton);
    else if (state == GLUT_UP)
        onReleaseButton(eButton);
}

// -----------------------------------------------------------------
// Name : onMouseMove
// -----------------------------------------------------------------
void PCInputEngine::onMouseMove(int x, int y)
{
    onAnalogicMove(x, y);
}

// -----------------------------------------------------------------
// Name : isShiftPressed
// -----------------------------------------------------------------
bool PCInputEngine::isShiftPressed()
{
    return ((m_iModifiers & GLUT_ACTIVE_SHIFT) != 0);
}

// -----------------------------------------------------------------
// Name : isCtrlPressed
// -----------------------------------------------------------------
bool PCInputEngine::isCtrlPressed()
{
    return ((m_iModifiers & GLUT_ACTIVE_CTRL) != 0);
}
