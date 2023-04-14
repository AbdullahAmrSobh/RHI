#pragma once



class Loop 
{
public:
    virtual ~Loop() = default; 

    virtual void Init() = 0; 

    virtual void FrameUpdate(float delta_time) = 0;

private:
    GLFWwindow* window; 
    std::unique_ptr<RHI::Context> m_context;
    std::unique_ptr<RHI::Swapchain> m_swapchain;
    
};