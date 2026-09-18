#pragma once

#include "interfaces/IModule.hpp"
#include <EASTL/vector.h>
#include <EASTL/hash_map.h>
#include <EASTL/string.h>
#include <filesystem>

class Engine;
class UtilityFacade;
class SystemFacade;
class GameFacade;

struct ModuleConfig{
    eastl::string name;
    std::filesystem::path path;
    ModuleCategory category;
    bool isEnabled;
};

class ModuleManager {
private:
    eastl::vector<IModule*> utility;
    eastl::vector<IModule*> system;
    eastl::vector<IModule*> game;

    eastl::hash_map<eastl::string, IModule*> m_moduleLookup;
    eastl::hash_map<eastl::string, void*> m_libraryHandles;
    eastl::vector<eastl::string> m_pendingUnloads;


    UtilityFacade* m_utilityFacade = nullptr;
    SystemFacade*  m_systemFacade  = nullptr;
    GameFacade*    m_gameFacade    = nullptr; 


    void removeModule(eastl::vector<IModule*>& vec, IModule* module);

    void addModule(IModule* module);
    void unloadModuleByName(const eastl::string& name);

    void processPendingUnloads();

    void readManifest(Engine* engine, const std::filesystem::path& manifestPath = "config/boot.json");
    
    public:
    ModuleManager();
    ~ModuleManager() { shutdown(); }

    void update();
    void init(Engine* engine);
    void shutdown();

    IModule* getModuleByName(const eastl::string& name);
    void requestUnloadByName(const eastl::string& name);
    void loadModuleFromDynamicLib(const std::filesystem::path& path, Engine* engine, ModuleCategory category);
};