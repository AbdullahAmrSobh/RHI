#include <iostream>
#include <RHI/RHI.hpp>

int main(int argc, const char** argv)
{

    // Create an instance of the RHI
    RHI::Instance instance = RHI::Instance::createInstance();
    
    // enumerate the physical devices, and select one.
    
    PhysicalDevice* pPhysicalDevice = nullptr;
    for (uint32_t i = 0; i < instance.getPhysicalDeviceCount(); ++i)
    {
        auto physicalDevice = instance.getPhysicalDevice(i); 
        
        if (physicalDevices.isDiscrete())
        {
            pPhysicalDevice = &physicalDevice;
        }
    }
    
    // Create a device
    Device device = Device::createDevice();

}