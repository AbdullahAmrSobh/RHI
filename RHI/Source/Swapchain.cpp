#include "RHI/Swapchain.hpp"

namespace RHI
{
    Swapchain::Swapchain(Device* device)
        : m_device(device)
        , m_name()
        , m_imageSize()
        , m_imageUsage()
        , m_imageFormat()
        , m_presentMode()
        , m_imageCount()
        , m_imageIndex()
        , m_semaphoreIndex()
        , m_waitSemaphore()
        , m_image()
        , m_signalSemaphore()
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
        return m_image[GetCurrentImageIndex()];
    }

    Handle<Image> Swapchain::GetImage(uint32_t index) const
    {
        return m_image[index];
    }

    Handle<Semaphore> Swapchain::GetSignalSemaphore() const
    {
        return m_signalSemaphore[GetCurrentSemaphoreIndex()];
    }

    Handle<Semaphore> Swapchain::GetWaitSemaphore() const
    {
        return m_waitSemaphore[GetCurrentSemaphoreIndex()];
    }

    uint32_t Swapchain::GetCurrentSemaphoreIndex() const
    {
        return m_semaphoreIndex;
    }

    uint32_t Swapchain::GetNextSemaphoreIndex() const
    {
        return (m_semaphoreIndex + 1) % m_imageCount;
    }

    void Swapchain::RotateSemaphores()
    {
        m_semaphoreIndex = GetNextSemaphoreIndex();
    }

} // namespace RHI