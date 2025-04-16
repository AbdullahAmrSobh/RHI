#pragma once

#include "RHI/Common.hpp"
#include "RHI/Result.hpp"

#include <TL/Assert.hpp>
#include <TL/Containers.hpp>
#include <TL/Stacktrace.hpp>

#include <type_traits>
#include <utility>

#define RHI_DECLARE_OPAQUE_RESOURCE(name) \
    struct name                           \
    {                                     \
    }

namespace RHI
{
    struct NullHandle_T final
    {
    };

    inline constexpr NullHandle_T NullHandle = {};

    template<typename Resource>
    class Handle final
    {
        template<typename U>
        friend class HandlePool;
        template<typename U>
        friend class Handle;

    public:
        Handle() { m_handle = UINT64_MAX; }

        Handle(NullHandle_T) { m_handle = UINT64_MAX; }

        template<typename BaseResource>
            requires(std::is_base_of_v<BaseResource, Resource> || std::is_base_of_v<Resource, BaseResource>)
        Handle(Handle<BaseResource> baseHandle)
        {
            m_rawHandle.index = baseHandle.m_rawHandle.index;
            m_rawHandle.genId = baseHandle.m_rawHandle.genId;
        }

        inline bool operator==(Handle other) const { return m_handle == other.m_handle; }

        inline bool operator!=(Handle other) const { return m_handle != other.m_handle; }

        explicit    operator uint64_t() const { return m_handle; }

        inline bool operator==(const NullHandle_T&) const { return m_handle == UINT64_MAX; }

        inline      operator bool() const { return *this != NullHandle; }

    private:
        Handle(uint64_t id, uint16_t genId)
        {
            m_rawHandle.index = id;
            m_rawHandle.genId = genId;
        }

        struct RawHandle
        {
            uint64_t index : 48;
            uint64_t genId : 16;
        };

        union
        {
            RawHandle m_rawHandle;
            uint64_t  m_handle;
        };
    };

    /// @brief Represents a storage where RHI objects live.
    template<typename Resource>
    class HandlePool final
    {
    public:
        RHI_INTERFACE_BOILERPLATE(HandlePool);

        using HandleType   = Handle<Resource>;
        using ResourceType = Resource;

        TL::String              ReportLiveResources() const;

        uint32_t                ReportLiveResourcesCount() const;

        // Clears all resources in the pool
        inline void             Clear();

        // Gets the resource associated with handle
        inline const Resource*  Get(Handle<Resource> handle) const;

        // Gets the resource associated with handle
        inline Resource*        Get(Handle<Resource> handle);

        // Inserted a zerod resource and returns its handle
        inline Handle<Resource> Emplace(Resource&& resource);

        // Removes a resource from the owner
        inline void             Release(Handle<Resource> handle);

        template<typename... Args>
        inline Result<HandleType> Create(Args&&... args)
        {
            Resource resource{};
            auto     result = resource.Init(std::forward<Args>(args)...);
            if (IsSuccess(result)) return Emplace(std::move(resource));
            return result;
        }

        template<typename... Args>
        void Destroy(HandleType handle, Args&&... args)
        {
            Get(handle)->Shutdown(std::forward<Args>(args)...);
            Release(handle);
        }

    private:
        TL::Vector<Resource>                m_resources;
        TL::Vector<uint16_t>                m_genIds;
        TL::Vector<size_t>                  m_freeSlots;

        TL::Map<HandleType, TL::Stacktrace> m_liveResources;
    };

} // namespace RHI

#include "RHI/Handle.inl" // IWYU pragma: export