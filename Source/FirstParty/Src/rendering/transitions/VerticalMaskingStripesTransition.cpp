#include "VerticalMaskingStripesTransition.hpp"
#include "../../Utility.hpp"

VerticalMaskingStripesTransition::VerticalMaskingStripesTransition(
    const sf::Texture* sourceTexture,
    const sf::Texture* targetTexture,
    const int stripeCount,
    const double duration,
    const sf::Vector2u& size) :
    Transition(sourceTexture, targetTexture, duration, size)
{
    if(targetTexture != nullptr)
    {
        m_stripeHeight = size.y;
        m_stripeWidth = size.x / stripeCount;

        if(static_cast<unsigned int>(m_stripeWidth * stripeCount) < size.x)
            m_stripeWidth++;

        for(int i = 0; i < stripeCount; ++i)
        {
            auto sprite = std::unique_ptr<sf::Sprite>(new sf::Sprite(*targetTexture, sf::IntRect(i * m_stripeWidth, 0, m_stripeWidth, 0)));
            sprite->setPosition(static_cast<float>(i * m_stripeWidth), 0);
            m_targetSprites.push_back(std::move(sprite));
        }
    }
    else
        throw std::runtime_error(utility::replace(utility::translateKey("@TargetTexture"), "VerticalMaskingStripesTransition"));

    if(sourceTexture != nullptr)
    {
        m_sourceSprite.setTexture(*sourceTexture, true);
        m_sourceSprite.setPosition(0, 0);
    }
}

void VerticalMaskingStripesTransition::updated()
{
    auto scale = getProgress();

    int count = 0;
    for(auto it = m_targetSprites.begin(); it != m_targetSprites.end(); ++it)
    {
        if(count % 2 == 0)
        {
            auto currentRect = (*it)->getTextureRect();
            currentRect.height = static_cast<int>(m_stripeHeight * scale);
            (*it)->setTextureRect(currentRect);
        }
        else
        {
            (*it)->setTextureRect(sf::IntRect(static_cast<int>((*it)->getPosition().x),
                                              m_stripeHeight - static_cast<int>(m_stripeHeight * scale),
                                              m_stripeWidth,
                                              static_cast<int>(m_stripeHeight * scale)));

            (*it)->setPosition((*it)->getPosition().x, 
                               static_cast<float>(m_stripeHeight - static_cast<int>(m_stripeHeight * scale)));
        }
        count++;
    }
}

void VerticalMaskingStripesTransition::doDraw(const DrawParameter& param)
{
    if(getSourceTexture() != nullptr)
        param.getTarget().draw(m_sourceSprite);
    if(getTargetTexture() != nullptr)
        for(auto it = m_targetSprites.begin(); it != m_targetSprites.end(); ++it)
            param.getTarget().draw(**it);
}
