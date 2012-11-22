#pragma once

#ifndef STATE_HPP
#define STATE_HPP

class ResourceManager;

namespace sf
{
    class RenderWindow;
}

/// This class will be used to discribe a State
/// and to be managed by the StateManager
class State
{
public:
    
    State(sf::RenderWindow& screen, ResourceManager& resourceManager) :
        m_screen(screen),
        m_resourceManager(resourceManager),
        m_pause(false)
    { }

    virtual ~State()
    { }

    virtual void update() = 0;
    virtual void draw() = 0;

    void pause()
    {
         m_pause = true;
    }

    void resume()
    {
         m_pause = false;
    }

    const bool isPaused()
    {
        return m_pause;
    }

protected:
    
    sf::RenderWindow& m_screen;
    ResourceManager& m_resourceManager;

private:

    bool m_pause;

};

#endif // STATE_HPP
