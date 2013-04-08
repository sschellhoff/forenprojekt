#include "TransitionState.hpp"
#include "EnterStateInformation.hpp"

TransitionState::TransitionState(sf::RenderWindow& screen, ResourceManager& resourceManager, Config& config) :
State(screen, resourceManager, config)
{
}

TransitionState::~TransitionState()
{

}

void TransitionState::onEnter(void *enterInformation)
{
    EnterTransitionStateInformation* info = (EnterTransitionStateInformation*)enterInformation;
    m_transition = std::move(info->m_transition);
    m_followingState = info->m_followingState;
    m_followingEnterInformation = info->m_onEnterInformation;
}

StateChangeInformation TransitionState::update()
{
    m_transition->update();
    if(m_transition->isFinished())
        return StateChangeInformation(m_followingState, m_followingEnterInformation);

    return StateChangeInformation::Empty();
}

void TransitionState::draw()
{
    m_transition->draw(m_screen);
}