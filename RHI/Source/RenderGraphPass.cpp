#include "RHI/RenderGraphPass.hpp"

namespace RHI
{

    const char* Pass::GetName() const
    {
        return m_name.c_str();
    }

    void Pass::Resize(ImageSize2D size)
    {
        size = size;
    }

    ImageSize2D Pass::GetSize() const
    {
        return m_size;
    }

} // namespace RHI