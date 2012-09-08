#ifndef _GAMEROOT_H
#define _GAMEROOT_H

class LocalClient;
class Server;

class GameRoot
{
public:
    // Constructor / destructor
    GameRoot();
    ~GameRoot();

    // Main functions
    void Init();
    void Update(double delta);
    void Display();

    LocalClient * m_pLocalClient;
};

#endif
