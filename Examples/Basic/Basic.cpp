#include <RHI/RHI.hpp>
#include <RHI-Vulkan/Loader.hpp>

#include "GLFW/glfw3.h"

int main()
{
    auto context = RHI::CreateVulkanContext({
        .applicationName = "app-name",
        .applicationVersion = { 0, 1, 1 },
        .engineName = "engine-name",
        .engineVersion = { 0, 1, 1 },
    });

    uint8_t bufferContent[16] = {};
    auto [buffer, _] = context->CreateBuffer<uint8_t>(RHI::BufferUsage::Storage, bufferContent);

    auto bufferPtr = (uint8_t*)context->MapBuffer(buffer);
    bufferPtr[0] = 255;
    bufferPtr[15] = 255;
    context->UnmapBuffer(buffer);

    context->DestroyBuffer(buffer);

    /////////////////////////////

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    ////////////////////////////

    // create swapchain
    // create shader module
    // create pipeline object
    // create sampler
    // draw simple triangle
}