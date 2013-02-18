#include "Level.hpp"
#include "../Config.hpp"
#include "Entity.hpp"
#include "../rendering/DrawParameter.hpp"
#include "../resources/ResourceManager.hpp"
#include "../resources/LevelFileLoader.hpp"
#include "../Utility.hpp" // toString, toMeter
#include "Teeter.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/System/Err.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <Box2D/Collision/Shapes/b2PolygonShape.h>
#include <Box2D/Collision/Shapes/b2CircleShape.h>
#include <Box2D/Common/b2Math.h>
#include <Box2D/Dynamics/b2Body.h>
#include <Box2D/Dynamics/b2Fixture.h>
#include <Box2D/Common/b2Draw.h>

#include <tinyxml2.h>

#include <algorithm>
#include <ostream> // endl
#include <string>
#include <unordered_map>
#include <utility> // pair, make_pair, move
#include "collision/ChangePropertyCollisionHandler.hpp"

Level::Level(const unsigned int level, ResourceManager& resourceManager, Config& config) :
    m_number(level),
    m_resourceManager(resourceManager),
    m_world(b2Vec2(0.f, 9.81f)),
    m_timeStep(1.f / 60.f),
    m_velocityIterations(4),
    m_positionIterations(4),
    m_config(config),
    m_soundManager(resourceManager),
    m_totalTarget(0),
    m_remainingTarget(0)
{
    m_world.SetAllowSleeping(false);
    m_debugDraw = false;
    load();

    m_fpsShow = config.get<bool>("ShowFps");

    m_bitmapfont = m_resourceManager.getBitmapFont("gold");
    m_labelFPS.setBitmapFont(m_bitmapfont);
    m_labelFPS.setPosition(10, 10);
    m_labelFPS.setRotation(0);
    m_labelFPS.setText(utility::toString<int>(0));

    m_labelTarget.setBitmapFont(m_bitmapfont);
    m_labelTarget.setBitmapFont(m_bitmapfont);
    m_labelTarget.setPosition(10, 10);
    m_labelTarget.setRotation(0);
    m_labelTarget.setText(utility::toString<int>(0));
}

Level::~Level()
{

}

void Level::restartAt(const float time)
{
    if(m_background != nullptr)
        m_background->restartAt(time);
    TimedObject::restartAt(time);
    for(auto it = begin(m_entities); it != end(m_entities); ++it)
        (*it)->restartAt(time);
    m_lastTime = time;
}

void Level::update(const float elapsedTime, sf::RenderTarget& screen)
{
    auto it = begin(m_entities);
    while(it != end(m_entities))
    {
        if((*it)->killed())
        {
            (*it)->unbindBody();
            it = m_entities.erase(it);
        }
        else
            ++it;
    }

    m_soundManager.update();

    m_timeStep =  elapsedTime - m_lastTime;
    m_velocityIterations = std::max(1, static_cast<int>(4 * m_timeStep * 60.0f));
    m_positionIterations = m_velocityIterations;

    m_world.Step(m_timeStep, m_velocityIterations, m_positionIterations);

    for(auto it = begin(m_entities); it != end(m_entities); ++it)
    {
        if((*it)->getType() == Entity::Target && (*it)->killed())
            m_remainingTarget--;
        (*it)->update(elapsedTime);
    }

    if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        m_debugDraw = !m_debugDraw;

    m_lastTime = elapsedTime;

    m_scrollView.setViewSize(screen.getSize());
    sf::Vector2f ballpos = sf::Vector2f(utility::toPixel(m_ball->getPosition().x), utility::toPixel(m_ball->getPosition().y));
    m_scrollView.adjustView(ballpos, screen);

    if(m_background != nullptr)
        m_background->update(elapsedTime, screen.getView());

    m_fpsCounter.update();
    m_labelFPS.setPosition(m_scrollView.toGlobalCoords(sf::Vector2u(10,10)));
    m_labelFPS.setText(utility::toString<int>(m_fpsCounter.getFPS()));

    m_labelTarget.setPosition(m_scrollView.getGlobalRightCorner() - 100, m_scrollView.getGlobalTopCorner() + 10);
    m_labelTarget.setText("�: " + utility::toString(m_remainingTarget) + "/" + utility::toString(m_totalTarget));
}

void Level::draw(const DrawParameter& param)
{
    if(m_background != nullptr)
        m_background->draw(param);

    for(auto it = begin(m_entities); it != end(m_entities); ++it)
        it->get()->draw(param);

    if(m_fpsShow)
        m_labelFPS.draw(DrawParameter(param));

    m_labelTarget.draw(DrawParameter(param));

    if(m_debugDraw)
    {
        DebugDraw d(param.getTarget());
        d.SetFlags(b2Draw::e_shapeBit | b2Draw::e_centerOfMassBit);
        m_world.SetDebugDraw(&d);
        m_world.DrawDebugData();
    }
}

bool Level::load()
{
    if(m_number == 0) // Level start from 1
        return false;

    tinyxml2::XMLDocument doc;
    doc.LoadFile(filename().c_str());

    if(!validate(doc)) // Validate the XML file
    {
        sf::err() << "XML level file '" << filename() << "' is not valid!" << std::endl;
        return false;
    }

    // Parse templates
    tinyxml2::XMLElement* templates = doc.FirstChildElement("level")->FirstChildElement("templates");

    std::unordered_map<std::string, tinyxml2::XMLElement*> shapes;
    std::unordered_map<std::string, tinyxml2::XMLElement*> physics;
    std::unordered_map<std::string, tinyxml2::XMLElement*> entities;

    if(templates != nullptr)
    {
        if(templates->FirstChildElement("shapes") != nullptr)
             shapes = std::move(LevelFileLoader::parseList(templates->FirstChildElement("shapes"), "shape", "name"));
        if(templates->FirstChildElement("physics") != nullptr)
            physics = std::move(LevelFileLoader::parseList(templates->FirstChildElement("physics"), "physic", "name"));
        if(templates->FirstChildElement("entities") != nullptr)
        {
            // Add use keys 'name' (objects) and 'rep' (grid)
            entities = std::move(LevelFileLoader::parseList(templates->FirstChildElement("entities"), "entity", "rep"));
            auto temp = LevelFileLoader::parseList(templates->FirstChildElement("entities"), "entity", "name");
            entities.insert(begin(temp), end(temp));
        }
    }

    // ==Parse grid==
    tinyxml2::XMLElement* grid = doc.FirstChildElement("level")->FirstChildElement("grid");

    // Get tile size
    unsigned int size = static_cast<unsigned int>(grid->IntAttribute("size"));
    m_height = grid->FloatAttribute("height");
    m_width = grid->FloatAttribute("width");

    // Separate the lines for easier processing
    std::vector<std::string> lines = LevelFileLoader::parseGrid(grid);

    for(unsigned int row = 0; row < lines.size(); ++row)
        for(std::size_t column = 0; column < lines[row].length(); column += 2)
        {
            std::string name = lines[row].substr(column, 2);

            // Ignore empty 'tiles'
            if(name == "  ")
                continue;

            // Identify the needed parts of an entity
            tinyxml2::XMLElement* entity = nullptr;
            tinyxml2::XMLElement* physic = nullptr;
            tinyxml2::XMLElement* shape = nullptr;
            
            // Entity template exists
            if(entities.find(name) != end(entities))
            {
                entity = entities.find(name)->second;

                // Physics tag exists
                physic = entity->FirstChildElement("physics");
                if(physic != nullptr)
                {
                    // Shape template exists
                    if(physic->Attribute("shape") != nullptr &&
                        shapes.find(std::string(physic->Attribute("shape"))) != end(shapes))
                        shape = shapes.find(std::string(physic->Attribute("shape")))->second;
                    // Physics doesn't use a template
                    else
                        shape = physic->FirstChildElement("shape");

                    // Physics template exists other wise no template is used
                    if(physics.find(std::string(physic->Attribute("name"))) != end(physics))
                        physic = physics.find(std::string(physic->Attribute("name")))->second;
                }
                
                // Entity is well defined
                if(entity != nullptr && shape != nullptr && physic != nullptr)
                    m_entities.push_back(std::move(createEntity(entity, sf::Vector2u((column/2)*size, row*size), shape, physic)));
            }
        }

    tinyxml2::XMLElement* objects = doc.FirstChildElement("level")->FirstChildElement("objects");
    tinyxml2::XMLElement* backgroundXml = objects->FirstChildElement("background");
    tinyxml2::XMLElement* world = doc.FirstChildElement("level")->FirstChildElement("world");
    
    tinyxml2::XMLElement* element = nullptr; // Temp object

    // Load background-image
    std::unique_ptr<Background> background(new Background(sf::Vector2u(
        static_cast<unsigned int>(m_width),
        static_cast<unsigned int>(m_height))));
    if(backgroundXml != nullptr)
    {
        for(auto parallax = backgroundXml->FirstChildElement("parallax"); parallax != nullptr;
            parallax = parallax->NextSiblingElement("parallax"))
        {
            std::unique_ptr<ParallaxLayer> layer(new ParallaxLayer(sf::Vector2f(
                parallax->FloatAttribute("width"),
                parallax->FloatAttribute("height"))));
            for(auto anim = parallax->FirstChildElement("animation"); anim != nullptr;
                anim = anim->NextSiblingElement("animation"))
                layer->bindAnimation(std::move(LevelFileLoader::parseAnimation(anim, layer.get(), m_resourceManager)));
            background->bindLayer(std::move(layer));
        }
    }
    m_background = std::move(background);

    for(tinyxml2::XMLElement* entitiesIterator = objects->FirstChildElement("entity");
        entitiesIterator != nullptr; entitiesIterator = entitiesIterator->NextSiblingElement("entity"))
    {
        // Identify the needed parts of an entity
        tinyxml2::XMLElement* entity = nullptr;
        tinyxml2::XMLElement* physic = nullptr;
        tinyxml2::XMLElement* shape = nullptr;

        sf::Vector2u position(entitiesIterator->IntAttribute("x"), entitiesIterator->IntAttribute("y"));

        if(entitiesIterator->Attribute("template") != nullptr)
        {
            std::string name;
            name = std::string(entitiesIterator->Attribute("template"));

            // Entity template exists
            if(entities.find(name) != end(entities))
            {
                entity = entities.find(name)->second;

                // Physics tag exists
                physic = entity->FirstChildElement("physics");
                if(physic != nullptr)
                {
                    // Shape template exists
                    if(physic->Attribute("shape") != nullptr &&
                        shapes.find(std::string(physic->Attribute("shape"))) != end(shapes))
                        shape = shapes.find(std::string(physic->Attribute("shape")))->second;
                    // Physics doesn't use a template
                    else
                        shape = physic->FirstChildElement("shape");

                    // Physics template exists other wise no template is used
                    if(physics.find(std::string(physic->Attribute("name"))) != end(physics))
                        physic = physics.find(std::string(physic->Attribute("name")))->second;
                }
            }
        }

        // Entity is well defined
        if(entity != nullptr && shape != nullptr && physic != nullptr)
            m_entities.push_back(std::move(createEntity(entity, position, shape, physic)));
    }

    // Load world properties
    tinyxml2::XMLElement* gravity = world->FirstChildElement("gravity");
    m_world.SetGravity(b2Vec2(gravity->FloatAttribute("x"), gravity->FloatAttribute("y")));
    m_world.SetContactFilter(&m_contactFilter);
    m_world.SetContactListener(&m_contactListener);

    // setup scrollview
    m_scrollView.setLevelSize(sf::Vector2u(
        static_cast<unsigned int>(getWidth()),
        static_cast<unsigned int>(getHeight())));

    // get the fucking ball
    for(auto it = begin(m_entities); it != end(m_entities); ++it)
        if((*it)->getType() == Entity::Ball)
        {
            m_ball = dynamic_cast<Ball*>((*it).get());
            m_ball->setStartPosition(m_ball->getPosition());
            m_ball->setFieldDimension(b2Vec2(m_width,m_height));
        }

    return true;
}

bool Level::validate(const tinyxml2::XMLDocument& document)
{
    if(document.Error()) // Error while loading file
    {
        document.PrintError();
        return false;
    }

    return true;

    // Check for required tags
    bool tagCheck = true;
    tagCheck &= (document.FirstChildElement("level") != nullptr);
    tagCheck &= (document.FirstChildElement("level")->FirstChildElement("objects") != nullptr);
    tagCheck &= (document.FirstChildElement("level")->FirstChildElement("world") != nullptr);
    tagCheck &= (document.FirstChildElement("level")->FirstChildElement("grid") != nullptr);
    tagCheck &= (document.FirstChildElement("level")->FirstChildElement("objects")
        ->FirstChildElement("grid") != nullptr);

    return tagCheck;
}

std::unique_ptr<Entity> Level::createEntity(tinyxml2::XMLElement* xml, const sf::Vector2u& position,
        tinyxml2::XMLElement* shape, tinyxml2::XMLElement* physic)
{
    std::unique_ptr<Entity> entity;
    tinyxml2::XMLElement* element = nullptr;
    
    if(xml->Attribute("type") != nullptr)
    {
        if(std::string(xml->Attribute("type")) == "teeter")
            entity = std::unique_ptr<Teeter>(new Teeter(m_config.get<float>("MouseScale")));
        else if(std::string(xml->Attribute("type")) == "ball")
            entity = std::unique_ptr<Ball>(new Ball(m_config.get<float>("BallResetTime")));
        else if(std::string(xml->Attribute("type")) == "target")
        {
            entity = std::unique_ptr<Entity>(new Entity(Entity::Target));
            m_totalTarget++;
        }
        else
            entity = std::unique_ptr<Entity>(new Entity(Entity::None));
    }
    else // No type specified == normal Entity
        entity = std::unique_ptr<Entity>(new Entity(Entity::None));

    if(xml->Attribute("collideWithBall") != nullptr)
    {
        entity->setCollideWithBall(xml->BoolAttribute("collideWithBall"));
    }
    else
        entity->setCollideWithBall(true);
    auto collider = xml->FirstChildElement("onCollision");
    if(collider != nullptr)
        parseCollider(entity.get(), collider);

    entity->setName(std::string(xml->Attribute("name")));
    
    auto animations = xml->FirstChildElement("animations");
    // Load animation
    for(auto element = animations->FirstChildElement("animation"); element != nullptr;
        element = element->NextSiblingElement("animation"))
        entity->bindAnimation(std::move(LevelFileLoader::parseAnimation(element, entity.get(), m_resourceManager)));

    auto constants = xml->FirstChildElement("constants");
    if(constants != nullptr)
        LevelFileLoader::parseConstants(constants, entity.get(), entity.get());

    // Load sound
    if(xml->FirstChildElement("sound") != nullptr)
        entity->bindSound(std::string(xml->FirstChildElement("sound")->Attribute("name")), &m_soundManager);

    // Load body
    element = physic->FirstChildElement("body");
    b2BodyDef bodyDef;
    if(std::string(element->Attribute("type")) == "static")
        bodyDef.type = b2_staticBody;
    else if(std::string(element->Attribute("type")) == "kinematic")
        bodyDef.type = b2_kinematicBody;
    else if(std::string(element->Attribute("type")) == "dynamic")
        bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2(static_cast<float>(utility::toMeter(position.x)), static_cast<float>(utility::toMeter(position.y)));
    bodyDef.angle = utility::toRadian<float, float>(element->FloatAttribute("angle"));
    bodyDef.fixedRotation = element->BoolAttribute("fixedRotation");
    bodyDef.angularDamping = element->BoolAttribute("angularDamping");
    LevelFileLoader::parseKinematics(element, entity.get());

    // Load shape
    if(std::string(shape->Attribute("type")) == "polygon") // Load polygon
    {
        std::vector<b2Vec2> vertices;

        // Iterate over the vertices
        for(tinyxml2::XMLElement* vertexIterator = shape->FirstChildElement("vertex");
            vertexIterator != nullptr; vertexIterator = vertexIterator->NextSiblingElement("vertex"))
        {
            vertices.push_back(b2Vec2(utility::toMeter(vertexIterator->FloatAttribute("x")),
                utility::toMeter(vertexIterator->FloatAttribute("y"))));
        }

        // Construct the b2Shape
        std::unique_ptr<b2PolygonShape> ps(new b2PolygonShape);
        ps->Set(vertices.data(), vertices.size());
        m_shapes.push_back(std::move(ps));
    }
    else if(std::string(shape->Attribute("type")) == "circle") // Load circle
    {
        std::unique_ptr<b2CircleShape> cs(new b2CircleShape);
        cs->m_radius = utility::toMeter(shape->FloatAttribute("radius"));
        m_shapes.push_back(std::move(cs));
    }

    // Load fixtures
    element = physic->FirstChildElement("fixture");
    b2FixtureDef fixtureDef;
    fixtureDef.shape = m_shapes.back().get();
    fixtureDef.density = element->FloatAttribute("density");
    fixtureDef.friction = element->FloatAttribute("friction");
    fixtureDef.restitution = element->FloatAttribute("restitution");

    // Create & bind body
    b2Body* body = m_world.CreateBody(&bodyDef);
    body->CreateFixture(&fixtureDef);
    // Save the entity in the body for the contact listener
    body->SetUserData(entity.get());

    entity->bindBody(body);

    m_remainingTarget = m_totalTarget;

    return std::move(entity);
}

const unsigned int Level::number()
{
    return m_number;
}

const std::string Level::filename()
{
    std::string filename("res/level/");
    if(m_number < 10)
        filename.append("00" + utility::toString(m_number) + ".lvl");
    else if(m_number < 100)
        filename.append("0" + utility::toString(m_number) + ".lvl");
    else
        filename.append(utility::toString(m_number) + ".lvl");

    return filename;
}

const float Level::getWidth() const
{
    return m_width;
}

const float Level::getHeight() const
{
    return m_height;
}

void Level::parseCollider(Entity* entity, tinyxml2::XMLElement* xml)
{
    for(auto child = xml->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
    {
        if(std::string(child->Name()) == "changeProperty")
        {
            std::unique_ptr<ChangePropertyCollisionHandler> collider(new ChangePropertyCollisionHandler(child->Attribute("name")));
            std::unique_ptr<ValueProvider> provider(LevelFileLoader::parseProvider(child->FirstChildElement(), collider.get()));
            collider->bindProvider(std::move(provider));
            entity->bindCollisionHandler(std::move(collider));
        }
    }
}