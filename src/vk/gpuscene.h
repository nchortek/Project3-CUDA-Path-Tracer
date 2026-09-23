#pragma once

#include <volk.h>

class GpuScene
{
public:
    GpuScene() = default;
    ~GpuScene()
    {
        destroy();
    }

    // GpuScene should be created once and passed by reference as needed,
    // so we should disable copies/moves
    GpuScene(const GpuScene&) = delete;
    GpuScene& operator=(const GpuScene&) = delete;
    GpuScene(GpuScene&&) = delete;
    GpuScene& operator=(GpuScene&&) = delete;

    bool init();
    void destroy();

private:

};