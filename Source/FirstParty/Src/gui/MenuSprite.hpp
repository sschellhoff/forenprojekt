#pragma once

#ifndef MENUSPRITE_HPP
#define MENUSPRITE_HPP

#include "MenuElement.hpp"
#include "ToolTip.hpp"

#include "../rendering/Sprite.hpp"
#include "ScreenSize.hpp"
#include "ScreenLocation.hpp"

class MenuSprite : public MenuElement
{
public:

    MenuSprite(const Sprite& sprite,
               const ScreenLocation& position,
               const ScreenScale& scale,
               const int id);

    void setToolTip(const ToolTip& toolTip);
    void setToolTipText(const std::string& text, const std::string& replacement = "");
    void setTextureRect(const sf::IntRect& textureRect);
    void setTexture(const sf::Texture& texture);
    void setSprite(const Sprite& sprite);

private:
    void onDrawElement(const DrawParameter& params) override;
    void updated(const sf::RenderWindow& screen, const double time, const sf::Vector2i& offset = sf::Vector2i(0, 0)) override;
    void layoutUpdated(const sf::Vector2f& screenSize) override;
    void onDrawAdditionalForeground(const DrawParameter& params) override;

    Sprite m_sprite;
    ScreenScale m_scale;
    ToolTip m_toolTip;
    bool m_showToolTip;

    std::unique_ptr<MenuElement> onClone() const override;
};

#endif // MENUSPRITE_HPP
