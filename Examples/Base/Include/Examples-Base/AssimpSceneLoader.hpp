#pragma once

namespace Examples
{
    class Renderer;
    class Scene;

    class AssimpScenneLoader
    {
    public:
        static void LoadScene(Renderer& renderer, Scene& scene, const char* sceneFileLocation, const char* sceneTextureLocation);
    };
} // namespace Examples