#pragma once

#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "provider/ValueProvider.hpp"

#include <memory> // unique_ptr

#include "VariableHolder.hpp"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>

class Animation : sf::Drawable, public VariableHolder
{
public:

    Animation(std::unique_ptr<ValueProvider> provider,
        const unsigned int frames,
        const unsigned int frameWidth,
        const unsigned int frameHeight,
        const bool applyRotation,
        const sf::Vector2f& origin,
        const sf::Vector2f& drawOffset);
    ~Animation();

    void update();

    void setPosition(const float x, const float y);
    void setRotation(const float radians);
    void bindTexture(const sf::Texture& texture, const sf::Vector2f& sourceOffset);
    void bindPositionController(std::unique_ptr<ValueProvider> x, std::unique_ptr<ValueProvider> y);
    void bindRotationController(std::unique_ptr<ValueProvider> provider);

    void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:

    const sf::IntRect Animation::getTextureRect() const;

    std::unique_ptr<ValueProvider> m_xPositionProvider;
    std::unique_ptr<ValueProvider> m_yPositionProvider;
    std::unique_ptr<ValueProvider> m_rotationProvider;
    std::unique_ptr<ValueProvider> m_frameProvider;
    bool m_applyRotation;
    unsigned int m_frames;
    unsigned int m_frame;
    unsigned int m_frameWidth;
    unsigned int m_frameHeight;
    sf::Sprite m_sprite;
    sf::Vector2f m_sourceOffset;
    sf::Vector2f m_externalPosition;
    float m_externalRotation;
    sf::Vector2f m_drawOffset;
};

#endif // ANIMATION_HPP