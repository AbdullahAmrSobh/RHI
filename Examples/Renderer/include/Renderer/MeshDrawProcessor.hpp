#pragma once

#include <RHI/RHI.hpp>
#include <Shaders/GpuCommonStructs.h>

#include "Renderer/Resources.hpp"
#include "Renderer/BindGroup.hpp"

#include "Renderer-Shaders/Cull.hpp"

namespace Engine
{
    class Material;
    class ComputeShader;
    class StaticMeshLOD;

    class DrawList
    {
    public:
        DrawList(uint32_t capacity = 1024);
        ~DrawList();

        uint32_t getCapacity() const { return m_capacity; }

        uint32_t getCount() const { return m_count; }

        /// Push new element into the draw list.
        /// Returns the assigned instanceID.
        uint32_t push(const StaticMeshLOD* mesh,
            const Material*                material,
            glm::mat4x4                    transform,
            uint32_t                       viewMask = 0x1u);

        /// Removes element by instanceID.
        void remove(uint32_t id);

        /// Uploads CPU-side draw data into GPU buffers.
        void onUpdate();

        auto& getTransforms() const { return m_transformAlloc; }

        auto& getDrawRequests() const { return m_drawAlloc; }

    private:
        struct Entry
        {
            glm::mat4x4                          transformCPU;
            GPU::DrawRequest                     drawCPU;
            GPUArrayAllocation<glm::mat4x4>      transformGPU;
            GPUArrayAllocation<GPU::DrawRequest> drawGPU;
        };

        uint32_t m_capacity;
        uint32_t m_count;

        TL::Vector<Entry> m_entries;

        GPUArray<glm::mat4x4>      m_transformAlloc;
        GPUArray<GPU::DrawRequest> m_drawAlloc;
    };
} // namespace Engine
