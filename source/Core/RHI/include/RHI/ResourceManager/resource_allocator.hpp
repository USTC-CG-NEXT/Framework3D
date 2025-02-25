#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>

#include "Logger/Logger.h"
#ifndef RESOURCE_ALLOCATOR_STATIC_ONLY
#include "nodes/core/api.hpp"
#endif
#include "RHI/ShaderFactory/shader.hpp"
#include "RHI/api.h"
#include "RHI/internal/nvrhi_equality.hpp"
#include "RHI/internal/nvrhi_hash.hpp"
#include "RHI/internal/nvrhi_sizes.hpp"
#include "RHI/internal/resources.hpp"

#ifdef USTC_CG_BACKEND_NVRHI
#include <nvrhi/nvrhi.h>

#endif

USTC_CG_NAMESPACE_OPEN_SCOPE

MACRO_MAP(DESC_HANDLE_TRAIT, RESOURCE_LIST)
MACRO_MAP(HANDLE_DESC_TRAIT, RESOURCE_LIST)

template<typename RESOURCE>
using desc = typename ResouceDesc<RESOURCE>::Desc;

template<typename DESC>
using resc = typename DescResouce<DESC>::Resource;

class ResourceAllocator {
#define CACHE_NAME(RESOURCE)   m##RESOURCE##Cache
#define INUSE_NAME(RESOURCE)   mInUse##RESOURCE
#define PAYLOAD_NAME(RESOURCE) RESOURCE##CachePayload
#define CACHE_SIZE(RESOURCE)   m##RESOURCE##CacheSize

#define JUDGE_RESOURCE_DYNAMIC(RSC) \
    if (entt::type_hash<RSC##Handle>() == handle.type().id())
#define JUDGE_RESOURCE(RSC) if constexpr (std::is_same_v<RSC##Handle, RESOURCE>)

#define RESOLVE_DESTROY_DYNAMIC(RESOURCE)             \
    RESOURCE##Handle h;                               \
    h = handle.cast<RESOURCE##Handle>();              \
    if (h) {                                          \
        PAYLOAD_NAME(RESOURCE) payload{ h, mAge, 0 }; \
        resolveCacheDestroy(                          \
            h,                                        \
            CACHE_SIZE(RESOURCE),                     \
            payload,                                  \
            CACHE_NAME(RESOURCE),                     \
            INUSE_NAME(RESOURCE));                    \
        return;                                       \
    }

#define RESOLVE_DESTROY(RESOURCE)                       \
    PAYLOAD_NAME(RESOURCE) payload { handle, mAge, 0 }; \
    resolveCacheDestroy(                                \
        handle,                                         \
        CACHE_SIZE(RESOURCE),                           \
        payload,                                        \
        CACHE_NAME(RESOURCE),                           \
        INUSE_NAME(RESOURCE));

   public:
    explicit ResourceAllocator() noexcept;

#define CHECK_EMPTY(RESOURCE)             \
    assert(!CACHE_NAME(RESOURCE).size()); \
    assert(!INUSE_NAME(RESOURCE).size());

    ~ResourceAllocator() noexcept
    {
        {
            terminate();
            MACRO_MAP(CHECK_EMPTY, RESOURCE_LIST)
        }
    }

#define CLEAR_CACHE(RESOURCE)                    \
    assert(!INUSE_NAME(RESOURCE).size());        \
    for (auto it = CACHE_NAME(RESOURCE).begin(); \
         it != CACHE_NAME(RESOURCE).end();       \
         it++) {                                 \
        it->second.handle = nullptr;             \
    }                                            \
    CACHE_NAME(RESOURCE).clear();

    void terminate() noexcept { MACRO_MAP(CLEAR_CACHE, RESOURCE_LIST) }

#define FOREACH_DESTROY_DYNAMIC(RESOURCE) \
    JUDGE_RESOURCE_DYNAMIC(RESOURCE)      \
    {                                     \
        RESOLVE_DESTROY_DYNAMIC(RESOURCE) \
    }

#ifndef RESOURCE_ALLOCATOR_STATIC_ONLY

    void destroy(entt::meta_any handle) noexcept
    {
        if constexpr (mEnabled) {
            // If code runs here, It means some of your output resource is not
            // created;
            MACRO_MAP(FOREACH_DESTROY_DYNAMIC, RESOURCE_LIST)
        }
        else {
            handle = nullptr;
        }
    }
#endif

#define FOREACH_DESTROY(RESOURCE) \
    JUDGE_RESOURCE(RESOURCE)      \
    {                             \
        RESOLVE_DESTROY(RESOURCE) \
    }

    template<typename RESOURCE>
    void destroy(RESOURCE& handle) noexcept
    {
        if constexpr (mEnabled) {
            MACRO_MAP(FOREACH_DESTROY, RESOURCE_LIST)
        }
        else {
            handle = nullptr;
        }
    }

#define GC_TYPE(RSC) gc_type<RSC##Handle>(CACHE_SIZE(RSC), CACHE_NAME(RSC));

    void gc() noexcept { MACRO_MAP(GC_TYPE, RESOURCE_LIST) }

#define RESOLVE_CREATE(RESOURCE) \
    resolveCacheCreate(          \
        handle,                  \
        desc,                    \
        CACHE_SIZE(RESOURCE),    \
        CACHE_NAME(RESOURCE),    \
        INUSE_NAME(RESOURCE),    \
        rest...);

#define FOREACH_CREATE(RESOURCE) \
    JUDGE_RESOURCE(RESOURCE)     \
    {                            \
        RESOLVE_CREATE(RESOURCE) \
    }

    template<typename DESC, typename RESOURCE = resc<DESC>, typename... Args>
    RESOURCE create(const DESC& desc, Args&&... rest)
    {
        RESOURCE handle;

        if constexpr (mEnabled) {
            MACRO_MAP(FOREACH_CREATE, RESOURCE_LIST)
        }
        else {
            handle = create_resource(desc, std::forward<Args>(rest)...);
        }
        assert(handle);
        return handle;
    }
    nvrhi::IDevice* device;
    ShaderFactory* shader_factory;
    void set_device(nvrhi::IDevice* device)
    {
        assert(device);
        this->device = device;
    }

#define DEFINEContainer(RESOURCE)                                     \
    struct PAYLOAD_NAME(RESOURCE) {                                   \
        RESOURCE##Handle handle;                                      \
        size_t age = 0;                                               \
        uint32_t size = 0;                                            \
    };                                                                \
    using RESOURCE##CacheContainer =                                  \
        AssociativeContainer<RESOURCE##Desc, RESOURCE##CachePayload>; \
    using RESOURCE##InUseContainer =                                  \
        AssociativeContainer<RESOURCE##Handle, RESOURCE##Desc>;       \
    RESOURCE##CacheContainer CACHE_NAME(RESOURCE);                    \
    RESOURCE##InUseContainer INUSE_NAME(RESOURCE);                    \
    uint32_t CACHE_SIZE(RESOURCE) = 0;

#define PURGE(RESOURCE)                                \
    RESOURCE##CacheContainer::iterator purge(          \
        const RESOURCE##CacheContainer::iterator& pos) \
    {                                                  \
        pos->second.handle = nullptr;                  \
        m##RESOURCE##CacheSize -= pos->second.size;    \
        return CACHE_NAME(RESOURCE).erase(pos);        \
    }

   private:
#define CREATE_CONCRETE(RESOURCE)                       \
    JUDGE_RESOURCE(RESOURCE)                            \
    {                                                   \
        return device->create##RESOURCE(desc, rest...); \
    }

    template<typename RESOURCE, typename... Args>
    RESOURCE create_resource(const desc<RESOURCE>& desc, Args&&... rest)
    {
        MACRO_MAP(CREATE_CONCRETE, NVRHI_RESOURCE_LIST)
        if constexpr (std::is_same_v<ProgramHandle, RESOURCE>) {
            return shader_factory->createProgram(desc);
        }
        if constexpr (std::is_same_v<PipelineHandle, RESOURCE>) {
            return device->createRayTracingPipeline(desc, rest...);
        }
        if constexpr (std::is_same_v<AccelStructHandle, RESOURCE>) {
            return device->createAccelStruct(desc, rest...);
        }
    }

    template<typename RESOURCE>
    void resolveCacheCreate(
        RESOURCE& handle,
        auto& desc,
        auto& cacheSize,
        auto&& cache,
        auto&& inUseCache,
        auto&&... rest)
    {
        auto it = cache.find(desc);
        if (it != cache.end()) {
            // we do, move the entry to the in-use list, and remove from the
            // cache
            handle = it->second.handle;
            cacheSize -= it->second.size;
            cache.erase(it);
        }
        else {
            handle = create_resource<RESOURCE>(desc, rest...);

            if constexpr (std::is_same_v<BindingSetHandle, RESOURCE>) {
                for (auto& resource : desc.bindings) {
                    mRelatedBindingSets.emplace(resource.resourceHandle, desc);
                }
            }
        }

        inUseCache.emplace(handle, desc);
    }

    template<typename RESOURCE>
    auto calcSize(desc<RESOURCE>& key)
    {
        return gpu_resource_size(key);
    }

    template<typename RESOURCE>
    void resolveCacheDestroy(
        RESOURCE& handle,
        auto& cacheSize,
        auto& cachePayload,
        auto&& cache,
        auto&& inUseCache)
    {
        // find the texture in the in-use list (it must be there!)
        auto it = inUseCache.find(handle);
        if (it == inUseCache.end()) {
            return;
        }

        // move it to the cache
        auto desc = std::move(it->second);

        cachePayload.size = calcSize<RESOURCE>(desc);

        // cache.emplace(key, CachePayload{ handle, mAge, size });
        cache.emplace(std::move(desc), std::move(cachePayload));
        cacheSize += cachePayload.size;

        // remove it from the in-use list
        inUseCache.erase(it);
    }

    template<typename RESOURCE>
    void gc_type(auto& cacheSize, auto&& cache_in)
    {
        if ((cacheSize >= CACHE_CAPACITY)) {
            using ContainerType = std::remove_cvref_t<decltype(cache_in)>;
            using Vector = std::vector<std::pair<
                typename ContainerType::key_type,
                typename ContainerType::mapped_type>>;
            auto cache = Vector();
            std::copy(
                cache_in.begin(),
                cache_in.end(),
                std::back_insert_iterator<Vector>(cache));

            std::sort(
                cache.begin(),
                cache.end(),
                [](const auto& lhs, const auto& rhs) {
                    return lhs.second.age < rhs.second.age;
                });

            auto curr = cache.begin();
            while (cacheSize >= CACHE_CAPACITY) {
                if constexpr (
                    std::is_same_v<TextureHandle, RESOURCE> ||
                    std::is_same_v<BufferHandle, RESOURCE> ||
                    std::is_same_v<SamplerHandle, RESOURCE>) {
                    auto resource = curr->second.handle;

                    auto related_binding_set =
                        mRelatedBindingSets.find(resource);

                    if (related_binding_set != mRelatedBindingSets.end()) {
                        auto pointed_binding_set_desc =
                            related_binding_set->second;

                        auto binding_set_outdated =
                            mBindingSetCache.find(related_binding_set->second);

                        // At least, in the beginning, the binding set should be
                        // in the cache
                        assert(binding_set_outdated != mBindingSetCache.end());
                        auto binding_set = binding_set_outdated->second;

                        while (binding_set_outdated != mBindingSetCache.end()) {
                            mBindingSetCache.erase(binding_set_outdated);
                            binding_set_outdated = mBindingSetCache.find(
                                related_binding_set->second);
                        }

                        // remove the related binding set, whose value is
                        // related_binding_set->second

                        std::erase_if(
                            mRelatedBindingSets,
                            [&pointed_binding_set_desc](const auto& pair) {
                                return pair.second == pointed_binding_set_desc;
                            });
                    }
                }

                purge(cache_in.find(curr->first));
                ++curr;
            }

            size_t oldestAge = cache.front().second.age;
            for (auto& it : cache_in) {
                it.second.age -= oldestAge;
            }
            mAge -= oldestAge;
        }
    }

    static constexpr size_t CACHE_CAPACITY = 1u << 30u;  // 1 GiB

    template<typename T>
    struct Hasher {
        std::size_t operator()(const T& s) const noexcept
        {
            return hash_value(s);
        }
    };

    void dump(bool brief = false, uint32_t cacheSize = 0) const noexcept;

// #define USE_STD_MAP
#ifdef USE_STD_MAP
    template<typename Key, typename Value>
    using AssociativeContainer = std::unordered_multimap<Key, Value>;
#else
    template<typename Key, typename Value, typename Hasher = Hasher<Key>>
    class AssociativeContainer {
        // We use a std::vector instead of a std::multimap because we don't
        // expect many items in the cache and std::multimap generates tons of
        // code. std::multimap starts getting significantly better around 1000
        // items.
        using Container = std::vector<std::pair<Key, Value>>;
        Container mContainer;

       public:
        AssociativeContainer();
        ~AssociativeContainer() noexcept;
        using iterator = typename Container::iterator;
        using const_iterator = typename Container::const_iterator;
        using key_type = typename Container::value_type::first_type;
        using mapped_type = typename Container::value_type::second_type;

        size_t size() const
        {
            return mContainer.size();
        }

        iterator begin()
        {
            return mContainer.begin();
        }

        const_iterator begin() const
        {
            return mContainer.begin();
        }

        iterator end()
        {
            return mContainer.end();
        }

        const_iterator end() const
        {
            return mContainer.end();
        }

        iterator erase(iterator it);
        const_iterator find(const key_type& key) const;
        void clear()
        {
            return mContainer.clear();
        }
        iterator find(const key_type& key);
        template<typename... ARGS>
        void emplace(ARGS&&... args);
    };
#endif

#define CONTAINER_RELATED(RESOURCE) \
    DEFINEContainer(RESOURCE);      \
    PURGE(RESOURCE)

    MACRO_MAP(CONTAINER_RELATED, RESOURCE_LIST);

    size_t mAge = 0;
    static constexpr bool mEnabled = true;

    std::unordered_multimap<nvrhi::IResource*, nvrhi::BindingSetDesc>
        mRelatedBindingSets;
};

#ifndef USE_STD_MAP

template<typename K, typename V, typename H>
ResourceAllocator::AssociativeContainer<K, V, H>::AssociativeContainer()
{
    mContainer.reserve(128);
}

template<typename K, typename V, typename H>

ResourceAllocator::AssociativeContainer<K, V, H>::
    ~AssociativeContainer() noexcept
{
}

template<typename K, typename V, typename H>
typename ResourceAllocator::AssociativeContainer<K, V, H>::iterator
ResourceAllocator::AssociativeContainer<K, V, H>::erase(iterator it)
{
    return mContainer.erase(it);
}

template<typename K, typename V, typename H>
typename ResourceAllocator::AssociativeContainer<K, V, H>::const_iterator
ResourceAllocator::AssociativeContainer<K, V, H>::find(
    const key_type& key) const
{
    return const_cast<AssociativeContainer*>(this)->find(key);
}

template<typename K, typename V, typename H>
typename ResourceAllocator::AssociativeContainer<K, V, H>::iterator
ResourceAllocator::AssociativeContainer<K, V, H>::find(const key_type& key)
{
    return std::find_if(
        mContainer.begin(), mContainer.end(), [&key](const auto& v) {
            return v.first == key;
        });
}

template<typename K, typename V, typename H>
template<typename... ARGS>
void ResourceAllocator::AssociativeContainer<K, V, H>::emplace(ARGS&&... args)
{
    mContainer.emplace_back(std::forward<ARGS>(args)...);
}
#endif

inline ResourceAllocator::ResourceAllocator() noexcept
{
}

USTC_CG_NAMESPACE_CLOSE_SCOPE