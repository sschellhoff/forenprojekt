#pragma once

#ifndef KEYBOARDMENU_HPP
#define KEYBOARDMENU_HPP

#include "Menu.hpp"

class KeyboardMenu : public Menu
{
private:
    bool m_showKeyboard;
    bool m_shiftKeyPressed;
    int m_lastKey;

    void doDraw(const DrawParameter& params) override;
    void updated(sf::RenderWindow& screen, const float time) override;

public:
    static const int BUTTON_KEYBOARD_OFF = 99;

    KeyboardMenu(sf::RenderWindow& screen, ResourceManager& resourceManager);

    void showKeyboard();

    bool isShown();
};

#endif