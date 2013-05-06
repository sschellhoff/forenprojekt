#pragma once

#ifndef INPUT_HPP
#define INPUT_HPP

#include <SFML/Window/Mouse.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <vector>

namespace utility
{
    class KeyboardWrapper
    {
    public:
        KeyboardWrapper()
        { }

        void progress();

        void notifyKeyPressed(const sf::Keyboard::Key key);

        void notifyKeyReleased(const sf::Keyboard::Key key);

        bool isKeyDown(const sf::Keyboard::Key key);

        bool isKeyPressed(const sf::Keyboard::Key key);

    private:
        std::vector<sf::Keyboard::Key> m_newlyPressed;
        std::vector<sf::Keyboard::Key> m_stillPressed;
        std::vector<sf::Keyboard::Key> m_released;
    };
    extern KeyboardWrapper Keyboard;

    class MouseWrapper
    {
    public:
        MouseWrapper() : m_leftPressed(false)
        {
            capture();
        }

        const sf::Vector2f& getPosition() const;

        void interpolate(int steps, int current);

        void capture();

        bool leftButtonPressed() const;

        bool leftButtonReleased() const;

    private:

        bool m_leftPressed;
        bool m_leftReleased;
        sf::Vector2f m_position;
        sf::Vector2i m_lastPosition;
    };
    extern MouseWrapper Mouse;
} // namespace utility

#endif // INPUT_HPP
