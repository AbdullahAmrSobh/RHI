#pragma once

namespace Engine
{
    class PassBase
    {
    public:
        class PipelineLibrary*    m_pipelineLibrary;
        class GeometryBufferPool* m_geometryBufferPool;
    };
};