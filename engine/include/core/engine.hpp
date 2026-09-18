#pragma once

#include "core/moduleManager.hpp"
#include "interfaces/IModule.hpp"
#include <cstdint>
#include <filesystem>

struct EngineState{
    float deltaTime;
    float timeScale;
    bool isRunning;
    bool isPause;
    uint64_t currentFrame;
};

class Engine {
    EngineState engineState;
    ModuleManager moduleManager;

    public:
    void init();
    void update();
    void shutdown();

    void loadModule(std::filesystem::path path, ModuleCategory category);

    float getDeltaTime() const;
    void requestShutdown();
    void requestUnloadModule(const eastl::string& name);
    IModule* getModule(const eastl::string& name);
};