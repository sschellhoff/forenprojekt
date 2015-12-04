#pragma once

#ifndef RESOURCE_CACHE_HPP
#define RESOURCE_CACHE_HPP

#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include "../Utility.hpp"

/// This class is a light header-only cache
/// and is capabale of handling all sorts of resources,
/// including the ones from SFML.
template <typename T>
class ResourceCache
{
public:

    typedef std::string Key;
    typedef std::unique_ptr<T> Resource;
    typedef std::function<std::unique_ptr<T>()> Functor;

public:

    T* load(const Key& key, const Functor& func)
    {
        auto it = get(key);
        if(it != nullptr)
            return it;

        auto resource = func();
        if(resource == nullptr)
            throw std::runtime_error(utility::replace(utility::translateKey("LoadFail"), key));

        auto ptr = resource.get();
        m_resources.insert(std::make_pair(key, std::move(resource)));
        return ptr;
    }

    bool exists(const Key& key)
    {
        return (m_resources.find(key) != end(m_resources)) || (m_externalResources.find(key) != end(m_externalResources));
    }

    /// Add an external resource to the pool. This resource will not be handled by this manager, but
    /// still will be returned to everyone asking for it by its name.
    void put(const Key& key, T& resource)
    {
        m_externalResources.insert(std::make_pair(key, &resource));
    }

    T* get(const Key& key)
    {
        auto it = m_resources.find(key);
        if(it != end(m_resources))
            return it->second.get();

        auto ext = m_externalResources.find(key);
        if(ext != end(m_externalResources))
            return ext->second;

        return nullptr;
    }

private:

    std::map<Key, Resource> m_resources;
    std::map<Key, T*> m_externalResources;
};

#endif // RESOURCE_CACHE_HPP
