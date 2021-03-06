#pragma once

#ifndef VERTICALHALVES_TRANSITION_HPP
#define VERTICALHALVES_TRANSITION_HPP

#include "Transition.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>
#include <vector>

class VerticalHalvesTransition : public Transition
{
public:
    VerticalHalvesTransition(
        const sf::Texture* sourceTexture,
        const sf::Texture* targetTexture,
        const int stripeCount,
        const double duration,
        const sf::Vector2u& size);


private:
    sf::Sprite m_targetSprite;
    std::vector<std::unique_ptr<sf::Sprite>> m_sourceSprites;
    int m_stripeWidth;
    int m_stripeHeight;

    void doDraw(const DrawParameter& param) override;
    void updated() override;
};

#endif // VERTICALHALVES_TRANSITION_HPP
