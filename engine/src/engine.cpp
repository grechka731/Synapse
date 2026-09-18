#include "core/engine.hpp"
#include "interfaces/IModule.hpp"

void Engine::init() {
    engineState.deltaTime    = 0.0f;
    engineState.timeScale    = 1.0f;
    engineState.isRunning    = true;
    engineState.isPause      = false;
    engineState.currentFrame = 0;

    moduleManager.init(this);
}

void Engine::update() {
    using clock = std::chrono::steady_clock;
    auto last = clock::now();

    while (engineState.isRunning) {
        auto now = clock::now();
        std::chrono::duration<float> diff = now - last;
        last = now;

        engineState.deltaTime = diff.count() * engineState.timeScale;
        engineState.currentFrame++;

        moduleManager.update();
    }
}
void Engine::shutdown(){
    engineState.isRunning=false;
    moduleManager.shutdown();
}

void Engine::loadModule(std::filesystem::path path, ModuleCategory category){
    moduleManager.loadModuleFromDynamicLib(path, this, category);
}

float Engine::getDeltaTime() const {
    return engineState.deltaTime;
}

void Engine::requestShutdown() {
    engineState.isRunning = false;
}

void Engine::requestUnloadModule(const eastl::string& name) {
    moduleManager.requestUnloadByName(name);
}

IModule* Engine::getModule(const eastl::string& name) {
    return moduleManager.getModuleByName(name);
}