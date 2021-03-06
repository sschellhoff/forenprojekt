#pragma once

#ifndef SPRITESHEET_HPP
#define SPRITESHEET_HPP

#include "../rendering/Blending.hpp"

#include <unordered_map>
#include <iostream>

namespace tinyxml2
{
    class XMLDocument;
}

class SpriteSheet
{
public:
    struct SpriteData
    {
        int x;
        int y;
        int width;
        int height;
        float originX;
        float originY;
        int offsetX;
        int offsetY;
        Blending::Mode blendMode;

        SpriteData() : x(0), y(0), width(0), height(0), originX(0), originY(0), offsetX(0), offsetY(0)
        { }
    };

private:
    std::unordered_map<std::string, SpriteData> m_spriteKeys;
    std::string m_textureName;
    std::string m_fileName;
    Blending::Mode m_blendMode;

    void insert(const std::string& key, const SpriteData& data);

public:
    SpriteSheet(const std::string& fileName);
    SpriteSheet();
    SpriteData get(const std::string& key) const;
    std::string getTextureName() const;
    bool loadFromFile(const std::string& fileName);
};

#endif