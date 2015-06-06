#include "ResourceManager.hpp"
#include "MenuLoader.hpp"
#include "../MacHelper.hpp"

#include <tinyxml2.h>

#include <exception>
#include <functional> // bind

ResourceManager::ResourceManager()
{
    m_soundManager = std::unique_ptr<SoundManager>(new SoundManager(*this));
    // Parse resource information
    tinyxml2::XMLDocument doc;
    doc.LoadFile((resourcePath() + "res/resources.nfo").c_str());

    if(doc.Error()) // Error while loading file
    {
        doc.PrintError();
        return;
    }

    parseSounds(doc);
    parseTextures(doc);
    parseFonts(doc);
    parseBitmapFonts(doc);
    parseMenus(doc);
    parseSpriteSheet(doc);
    parseLevelFileName(doc);
    parsePublicKeys(doc);
    parseHashValues(doc);
}

BitmapFont* ResourceManager::getBitmapFont(const std::string& key)
{
    auto bitmapFont = m_bitmapFontKeys.find(key);
    if(bitmapFont != end(m_bitmapFontKeys) && bitmapFont->first == key)
    {
        std::string name = bitmapFont->first;
        std::string path = bitmapFont->second;
        if(m_bitmapFonts.exists(name))
            return m_bitmapFonts.get(name);
        else
        {
            if(m_bitmapFonts.load(name, [path, this](){ return loadBitmapFont(path); }))
                return m_bitmapFonts.get(name);
        }
    }

    throw std::runtime_error(utility::replace(utility::translateKey("UnknownBitmapFont"), key));
}

MenuTemplate* ResourceManager::getMenuTemplate(const std::string& name)
{
    auto menuEntry = m_menuKeys.find(name);
    if(menuEntry != end(m_menuKeys) && menuEntry->first == name)
    {
        std::string path = menuEntry->second;
        if(m_menus.exists(name))
            return m_menus.get(name);
        else
        {
            if(m_menus.load(name, [path, this](){ return MenuLoader::loadMenuTemplate(std::string("res/menus/") + path, *this); }))
                return m_menus.get(name);
        }
    }

    throw std::runtime_error(utility::replace(utility::translateKey("UnknownMenuTemplate"), name));
}

#ifndef NO_SOUND
sf::SoundBuffer* ResourceManager::getSoundBuffer(const std::string& key)
{
    // Does the key even exist?
    auto sound = m_soundBufferKeys.find(key);
    if(sound != end(m_soundBufferKeys) && sound->first == key)
    {
        std::string name = sound->first;
        std::string path = sound->second;
        // Sound already loaded
        if(m_soundBuffers.exists(name)){
            return m_soundBuffers.get(name);
        }
        else
        {
            if(m_soundBuffers.load(name, [path](){ return loadSoundBuffer(path); }))
                return m_soundBuffers.get(name);
        }
    }
    
    // If the key doesn't exist
    throw std::runtime_error(utility::replace(utility::translateKey("UnknownSound"), key));
}
#endif

sf::Texture* ResourceManager::getTexture(const std::string& key)
{
    // Does the key even exist?
    auto texture = m_textureKeys.find(key);
    if(texture != end(m_textureKeys) && texture->first == key)
    {
        std::string path = texture->second.first;
        bool smooth = texture->second.second;
        // Texture already loaded
        if(m_textures.exists(path))
            return m_textures.get(path);
        else
        {
            if(m_textures.load(path, [path, smooth](){ return loadTexture(path, smooth); }))
                return m_textures.get(path);
        }
    }
    
    // If the key doesn't exist
    throw std::runtime_error(utility::replace(utility::translateKey("UnknownTexture"), key));
}

sf::Font* ResourceManager::getFont(const std::string& key)
{
    // Does the key even exist?
    auto font = m_fontKeys.find(key);
    if(font != end(m_fontKeys) && font->first == key)
    {
        std::string path = font->second;
        // Texture already loaded
        if(m_fonts.exists(path))
            return m_fonts.get(path);
        else
        {
            if(m_fonts.load(path, [path](){ return loadFont(path); }))
                return m_fonts.get(path);
        }
    }
    
    // If the key doesn't exist
    throw std::runtime_error(utility::replace(utility::translateKey("UnknownFont"), key));
}

void ResourceManager::parseTextures(tinyxml2::XMLDocument& doc)
{
    if(auto textures = doc.FirstChildElement("textures"))
    {
        for(auto textureIterator = textures->FirstChildElement("texture");
            textureIterator != nullptr; textureIterator = textureIterator->NextSiblingElement("texture"))
        {
            auto properties = std::make_pair<std::string, bool>(
                std::string(textureIterator->Attribute("path")),
                textureIterator->BoolAttribute("smooth"));

            m_textureKeys.insert(std::make_pair(
                std::string(textureIterator->Attribute("name")), properties));
        }
    }
}

void ResourceManager::parseFonts(tinyxml2::XMLDocument& doc)
{
    if(auto fonts = doc.FirstChildElement("fonts"))
    {
        for(auto fontIterator = fonts->FirstChildElement("font");
            fontIterator != nullptr; fontIterator = fontIterator->NextSiblingElement("font"))
        {
            m_fontKeys.insert(std::make_pair(
                std::string(fontIterator->Attribute("name")), std::string(fontIterator->Attribute("path"))));
        }
    } 
}

void ResourceManager::parseSounds(tinyxml2::XMLDocument& doc)
{
    if(auto sounds = doc.FirstChildElement("sounds"))
    {
        for(auto soundIterator = sounds->FirstChildElement("sound");
            soundIterator != nullptr; soundIterator = soundIterator->NextSiblingElement("sound"))
        {
            m_soundBufferKeys.insert(std::make_pair<std::string, std::string>(
                std::string(soundIterator->Attribute("name")), std::string(soundIterator->Attribute("path"))));
        }
    }
}

void ResourceManager::parseBitmapFonts(tinyxml2::XMLDocument& doc)
{
    if(auto bitmapFonts = doc.FirstChildElement("bitmapfonts"))
    {
        for(auto it = bitmapFonts->FirstChildElement("bitmapfont");
            it != nullptr; it = it->NextSiblingElement("bitmapfont"))
        {
            m_bitmapFontKeys.insert(std::make_pair<std::string, std::string>(
                std::string(it->Attribute("name")), std::string(it->Attribute("path"))));
        }
    }
}

void ResourceManager::parseMenus(tinyxml2::XMLDocument& doc)
{
    if(auto menus = doc.FirstChildElement("menus"))
    {
        for(auto it = menus->FirstChildElement("menu");
            it != nullptr; it = it->NextSiblingElement("menu"))
        {
            m_menuKeys.insert(std::make_pair<std::string, std::string>(
                std::string(it->Attribute("name")), std::string(it->Attribute("path"))));
        }
    }
}

SoundManager& ResourceManager::getSoundManager()
{
    return *m_soundManager;
}

void ResourceManager::parseSpriteSheet(tinyxml2::XMLDocument& doc)
{
    if(auto spriteSheet = doc.FirstChildElement("spritesheets"))
    {
        for(auto it = spriteSheet->FirstChildElement("spritesheet");
            it != nullptr; it = it->NextSiblingElement("spritesheet"))
        {
            m_spriteSheetKeys.insert(std::make_pair<std::string, std::string>(
                std::string(it->Attribute("name")), std::string(it->Attribute("path"))));
        }
    }
}

SpriteSheet* ResourceManager::getSpriteSheet(const std::string& key)
{
    auto spriteSheet = m_spriteSheetKeys.find(key);
    if(spriteSheet != end(m_spriteSheetKeys) && spriteSheet->first == key)
    {
        std::string name = spriteSheet->first;
        std::string path = spriteSheet->second;
        if(m_spriteSheets.exists(name))
            return m_spriteSheets.get(name);
        else
        {
            if(m_spriteSheets.load(name, [path, this](){ return loadSpriteSheet(path); }))
                return m_spriteSheets.get(name);
        }
    }

    throw std::runtime_error(utility::replace(utility::translateKey("UnknownSpriteSheet"), key));
}

void ResourceManager::parseLevelFileName(tinyxml2::XMLDocument& doc)
{
    if(auto levelfile = doc.FirstChildElement("Levels"))
    {
        for(auto it = levelfile->FirstChildElement("Level");
            it != nullptr; it = it->NextSiblingElement("Level"))
        {
            m_levelFileNames.insert(std::make_pair<int, std::string>(it->IntAttribute("number"),
                                                                    std::string(it->Attribute("filename"))));
        }
    }
}

const std::unordered_map<int, std::string>& ResourceManager::getFileNames()
{
    return m_levelFileNames;
}

void ResourceManager::parsePublicKeys(tinyxml2::XMLDocument& doc)
{
    if(auto keys = doc.FirstChildElement("Keys"))
    {
        for(auto it = keys->FirstChildElement("Key");
            it != nullptr; it = it->NextSiblingElement("Key"))
        {
            m_publicKeyKeys.insert(std::make_pair<std::string, std::string>(
                std::string(it->Attribute("name")), std::string(it->Attribute("path"))));
        }
    }
}

CryptoPP::RSA::PublicKey* ResourceManager::getPublicKey(const std::string& key)
{
    auto publicKey = m_publicKeyKeys.find(key);
    if(publicKey != end(m_publicKeyKeys) && publicKey->first == key)
    {
        std::string name = publicKey->first;
        std::string path = publicKey->second;
        if(m_publicKeys.exists(name))
            return m_publicKeys.get(name);
        else
        {
            if(m_publicKeys.load(name, [path, this](){ return loadPublicKey(path); }))
                return m_publicKeys.get(name);
        }
    }

    throw std::runtime_error(utility::replace(utility::translateKey("UnknownPublicKey"), key));
}

void ResourceManager::parseHashValues(tinyxml2::XMLDocument& doc)
{
    if(auto hashValues = doc.FirstChildElement("HashValues"))
    {
        for(auto it = hashValues->FirstChildElement("HashValue");
            it != nullptr; it = it->NextSiblingElement("HashValue"))
        {
            m_hashValues.insert(std::make_pair<std::string, std::string>(
                std::string(it->Attribute("filename")), std::string(it->Attribute("value"))));
        }
    }
}

std::string ResourceManager::getHashValue(const std::string& key)
{
    auto result = m_hashValues.find(key);
    if(result != end(m_hashValues))
        return result->second;

    throw std::runtime_error(utility::replace(utility::translateKey("UnknownHashValue"), key));
}