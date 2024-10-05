#include "RHI/Swapchain.hpp"

namespace RHI
{
    Swapchain::Swapchain(Context* context)
        : m_context(context)
    {
    }

    Swapchain::~Swapchain() = default;

    uint32_t Swapchain::GetCurrentImageIndex() const
    {
        return m_imageIndex;
    }

    uint32_t Swapchain::GetImagesCount() const
    {
        return m_imageCount;
    }

    Handle<Image> Swapchain::GetImage() const
    {
        return m_ringBuffer[m_imageIndex].m_image;
    }

    Handle<Image> Swapchain::GetImage(uint32_t index) const
    {
        return m_ringBuffer[index].m_image;
    }

    Handle<Semaphore> Swapchain::GetSignalSemaphore() const
    {
        return m_ringBuffer[m_imageIndex].m_signalSemaphore;
    }

    Handle<Semaphore> Swapchain::GetSignalSemaphore(uint32_t index) const
    {
        return m_ringBuffer[index].m_signalSemaphore;
    }

    Handle<Semaphore> Swapchain::GetWaitSemaphore() const
    {
        return m_ringBuffer[m_imageIndex].m_waitSemaphore;
    }

    Handle<Semaphore> Swapchain::GetWaitSemaphore(uint32_t index) const
    {
        return m_ringBuffer[index].m_waitSemaphore;
    }

} // namespace RHI