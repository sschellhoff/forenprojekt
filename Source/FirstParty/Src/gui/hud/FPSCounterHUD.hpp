#ifndef FPSCOUNTERHUD_HPP
#define FPSCOUNTERHUD_HPP

#include "../LineLabel.hpp"
#include "../FPSCounter.hpp"
#include "LabelHUD.hpp"

class ResourceManager;

///
class FPSCounterHUD : public LabelHUD
{
private:
    
    FPSCounter m_fpsCounter;

public:

    FPSCounterHUD(const sf::Vector2f& position,
                  const sf::Vector2f& offset,
                  const BitmapFont* bitmapFont);
    
    void update(const DrawParameter& params);

};

#endif
