#pragma once

#ifndef SCROLLWINDOW_HPP
#define SCROLLWINDOW_HPP

#include "MenuElement.hpp"
#include "MenuPanel.hpp"
#include "MenuTemplate.hpp"

#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include <functional>
#include <vector>
#include <memory>

class Button;
class CheckBox;
class LineLabel;
class MenuSprite;
class Slider;

class SubWindow : public MenuElement
{
public:

    SubWindow(const int id,
              const sf::Vector2f& position,
              const sf::Vector2f& size,
              const sf::Vector2f& offset,
              const int innerHeight,
              MenuElements& elements,
              const SubWindowStyle& style);

    virtual void draw(const DrawParameter& params) override;
    void drawAdditionalForeground(const DrawParameter& params) override;
    virtual void update(const sf::RenderWindow& screen, const float time, const sf::Vector2i& mouseOffset = sf::Vector2i(0, 0)) override;

    MenuPanel* getPanel();
protected:

    virtual void onPositionChanged() override;

private:

    MenuPanel m_panel;
    int m_innerHeight;
    sf::Vector2f m_center;
    sf::Vector2f m_size;
    sf::View m_orginalView;

    int m_startValue;
    int m_endValue;
    bool m_active;

    SubWindowStyle m_style;
    sf::RectangleShape m_windowRect;
    sf::RectangleShape m_sliderRect;
    sf::RectangleShape m_positionRect;

    void on(const DrawParameter& params);
    void off(const DrawParameter& params);

    std::function<void(const Button& sender)> m_clickCallback;

    float sliderPixelToWindowPixel(float pixel);
    float windowPixelToSliderPixel(float pixel);

    sf::Vector2i getMouseOffset(const sf::RenderWindow& screen);
};

#endif