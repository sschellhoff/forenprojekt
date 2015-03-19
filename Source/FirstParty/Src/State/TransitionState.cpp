#include "TransitionState.hpp"
#include "EnterStateInformation.hpp"
#include "../Utility.hpp"
#include "../rendering/transitions/RandomTransition.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <cmath>

TransitionState::TransitionState(sf::RenderWindow& screen,
                                 ResourceManager& resourceManager, 
                                 AppConfig& config) :
    State(screen, resourceManager, config)
{
    auto desktopMode = sf::VideoMode::getDesktopMode();
    auto size = static_cast<int>(desktopMode.width > desktopMode.height ?
        desktopMode.width : desktopMode.height);
    auto powerOfTwo = static_cast<int>(powf(2.f,
        floorf(logf(static_cast<float>(size))/logf(2.f))));
    if(powerOfTwo < size)
        powerOfTwo *= 2;
    // BUG: the PC might not be able to handle this size!
    // we still need a texture as big as possible to handle screen-size-changes during the gameplay
    bool s = m_sourceImage.create(powerOfTwo, powerOfTwo, true);
    s = m_targetImage.create(powerOfTwo, powerOfTwo, true);
}

TransitionState::~TransitionState()
{
}

void TransitionState::onEnter(const EnterStateInformation* enterInformation, const float time)
{
    State::onEnter(enterInformation, time);
    const EnterTransitionStateInformation* info = dynamic_cast<const EnterTransitionStateInformation*>(enterInformation);
    m_followingState = info->m_followingState;
    m_followingEnterInformation = info->m_onEnterInformation;

    auto r = m_screen.getSize();
    m_sourceImage.setView(utility::getDefaultView(m_sourceImage, r));
    m_sourceImage.clear();
    info->m_source->pause(time);
    info->m_source->draw(m_sourceImage);
    m_sourceImage.display();

    m_targetImage.setView(utility::getDefaultView(m_targetImage, r));
    m_targetImage.clear();
    info->m_onEnterInformation->m_prepareOnly = true;
    info->m_target->onEnter(info->m_onEnterInformation, time);
    info->m_target->update(time);
    info->m_target->draw(m_targetImage);
    m_targetImage.display();

    m_transition.reset(new RandomTransition(&m_sourceImage.getTexture(), &m_targetImage.getTexture(), 0.5f, m_screen.getSize(), static_cast<RandomTransition::TransitionType>(info->m_transitionType)));
}

StateChangeInformation TransitionState::update(const float time)
{
    updateTime(time);
    if(m_transition)
    {
        m_transition->update();
        if(m_transition->isFinished())
            return StateChangeInformation(m_followingState, m_followingEnterInformation);
    }

    return StateChangeInformation::Empty();
}

void TransitionState::draw(const DrawParameter& params)
{
    params.getTarget().setView(utility::getDefaultView(params.getTarget(), params.getTarget().getSize()));
    m_transition->draw(params);
}