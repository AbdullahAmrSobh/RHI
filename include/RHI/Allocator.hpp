#pragma once

namespace RHI
{

enum class AllocationPolicy
{
    Linear,
    FreeAtOnce, 
    Stack,
    DoubleStack, 
    RingBuffer, 
};

class DeviceAddress
{
};

class Resource {};

class MemoryPool
{
public:
    DeviceAddress Map();
    void          Unmap();

    size_t GetCapacity();
    float Defragmentation(); 

    void BeginDefragmentationPass();
    void BeginDefragmentationPass();

private:
    std::vector<Resource*> m_allocatedResources; 
};

}  // namespace RHI