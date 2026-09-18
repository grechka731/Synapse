#pragma once

#include <EASTL/string.h>
#include <cstdint>

class IUtilityContext;
class ISystemContext;
class IGameContext;
class IModule;

using createUtilityModuleFn = IModule* (*)(IUtilityContext*);
using createSystemModuleFn  = IModule* (*)(ISystemContext*);
using createGameModuleFn    = IModule* (*)(IGameContext*);
using destroyModuleFn       = void (*)(IModule*);

enum class ModuleCategory : uint8_t { Utility, System, Game };

class IModule {
private:
    eastl::string m_name;
    ModuleCategory m_type;

public:
    destroyModuleFn destroy = nullptr;

    explicit IModule(const eastl::string& name, ModuleCategory type)
        : m_name(name), m_type(type) {}

    virtual ~IModule() = default;

    const eastl::string& getName() const {
        return m_name;
    }

    ModuleCategory getCategory() const {
        return m_type;
    }

    virtual void init() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
};