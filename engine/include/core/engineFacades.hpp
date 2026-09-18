#pragma once

#include "./engine.hpp"
#include "interfaces/IModule.hpp"
#include "interfaces/IUtilityContext.hpp"
#include "interfaces/ISystemContext.hpp"
#include "interfaces/IGameContext.hpp"

class UtilityFacade : public IUtilityContext {
private:
    Engine* m_engine;

public:
    explicit UtilityFacade(Engine* engine) : m_engine(engine) {}

    float getDeltaTime() const override {
        return m_engine->getDeltaTime();
    }
};

class SystemFacade : public ISystemContext {
private:
    Engine* m_engine;

public:
    explicit SystemFacade(Engine* engine) : m_engine(engine) {}

    float getDeltaTime() const override {
        return m_engine->getDeltaTime();
    }
    
    IModule* getModule(const eastl::string& name) override {
        return m_engine->getModule(name);
    }
};

class GameFacade : public IGameContext {
private:
    Engine* m_engine;

public:
    explicit GameFacade(Engine* engine) : m_engine(engine) {}

    float getDeltaTime() const override {
        return m_engine->getDeltaTime();
    }

    IModule* getSystemModule(const eastl::string& name) override {
        IModule* module = m_engine->getModule(name);

        if (module != nullptr && module->getCategory() == ModuleCategory::System) {
            return module;
        }
        
        return nullptr;
    }
};