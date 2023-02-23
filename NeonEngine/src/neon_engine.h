#pragma once

#include "camera.h"

#include <mutex>

class UserInterface;

class NeonEngine {
public:
    static NeonEngine* get_instance();
    NeonEngine(NeonEngine& other) = delete;
    void operator=(const NeonEngine&) = delete;

    int run();

    Camera camera_viewport;

private:
    NeonEngine();
    ~NeonEngine();

    void initialize();

    UserInterface* user_interface;

    static NeonEngine* instance;
    static std::mutex neon_engine_mutex;
};