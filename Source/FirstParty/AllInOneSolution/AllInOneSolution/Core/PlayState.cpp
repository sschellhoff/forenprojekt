#include "PlayState.hpp"
#include "resources/ResourceManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Keyboard.hpp>

PlayState::PlayState(sf::RenderWindow& screen, ResourceManager& resourceManager, Config& config) :
    State(screen, resourceManager, config),
    m_level(nullptr) // Make sure
{
    screen.setFramerateLimit(60);
    m_level = std::unique_ptr<Level>(new Level(1, m_resourceManager, m_config));
    float time = m_frametime.getElapsedTime().asSeconds();
    m_level->restartAt(time);
}

PlayState::~PlayState()
{

}

void PlayState::update()
{
    m_level->update(m_frametime.getElapsedTime().asSeconds());

	if(sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		m_level.reset();
		m_level = std::unique_ptr<Level>(new Level(1, m_resourceManager, m_config));
	}
}

void PlayState::draw()
{
    m_level->draw(m_screen);
}
