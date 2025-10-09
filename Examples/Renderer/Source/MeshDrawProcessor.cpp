#include "Renderer/MeshDrawProcessor.hpp"
#include "Renderer/PipelineLibrary.hpp"
#include "Renderer/Scene.hpp"

namespace Engine
{
    DrawList::DrawList(uint32_t capacity)
        : m_capacity(capacity)
        , m_count(0)
    {
        auto& pool = RenderContext::ptr->getStructuredBuffersPool();
        m_transformAlloc.init(pool, capacity);
        m_drawAlloc.init(pool, capacity);

        m_entries.reserve(capacity);
    }

    DrawList::~DrawList()
    {
        if (m_capacity == 0)
            return;

        auto& pool = RenderContext::ptr->getStructuredBuffersPool();
        m_transformAlloc.shutdown();
        m_drawAlloc.shutdown();

        m_entries.clear();
        m_capacity = 0;
        m_count    = 0;
    }

    uint32_t DrawList::push(const StaticMeshLOD* mesh,
        const Material*                          material,
        glm::mat4x4                              transform,
        uint32_t                                 viewMask)
    {
        TL_ASSERT(m_count < m_capacity);

        auto transformAlloc = m_transformAlloc.allocate(1);
        auto drawAlloc      = m_drawAlloc.allocate(1);

        TL_ASSERT(transformAlloc.valid() && drawAlloc.valid());

        GPU::DrawRequest req{
            .meshID     = mesh->m_sbDrawArgs.getOffsetIndex(),
            .materialID = 0, // TODO: hook up material->id
            .instanceID = transformAlloc.getOffsetIndex(),
            .viewMask   = viewMask,
        };

        Entry e{
            .transformCPU = transform,
            .drawCPU      = req,
            .transformGPU = transformAlloc,
            .drawGPU      = drawAlloc,
        };

        m_entries.push_back(e);
        return m_count++;
    }

    void DrawList::remove(uint32_t id)
    {
        TL_ASSERT(id < m_count);

        auto& entry = m_entries[id];
        m_transformAlloc.free(entry.transformGPU);
        m_drawAlloc.free(entry.drawGPU);

        if (id != m_count - 1)
        {
            m_entries[id] = m_entries.back();
        }
        m_entries.pop_back();
        --m_count;
    }

    void DrawList::onUpdate()
    {
        auto  device = RenderContext::ptr->m_device;
        auto& pool   = RenderContext::ptr->getStructuredBuffersPool();
        auto* frame  = device->GetCurrentFrame();

        if (m_count == 0)
            return;

        for (auto& e : m_entries)
        {
            m_transformAlloc.update(e.transformGPU, e.transformCPU);
            m_drawAlloc.update(e.drawGPU, e.drawCPU);
        }
    }
} // namespace Engine
