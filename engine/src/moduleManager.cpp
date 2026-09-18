#include "core/moduleManager.hpp"
#include "core/engineFacades.hpp" // Включаем заголовок с фасадами
#include "interfaces/IModule.hpp"
#include <cstddef>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#if defined(_WIN32)
#define ENGINE_PLATFORM_WINDOWS
constexpr const char *DYNAMIC_LIB_EXT = ".dll";
#elif defined(__linux__)
#define ENGINE_PLATFORM_LINUX
constexpr const char *DYNAMIC_LIB_EXT = ".so";
#else
#error "Unsupported platform!"
#endif

#ifdef ENGINE_PLATFORM_WINDOWS
#include <windows.h>
#elif defined(ENGINE_PLATFORM_LINUX)
#include <dlfcn.h>
#endif

ModuleManager::ModuleManager() { m_moduleLookup.reserve(64); }

void ModuleManager::removeModule(eastl::vector<IModule *> &vec,
                                 IModule *module) {
  for (size_t i = 0; i < vec.size(); ++i) {
    if (vec[i] == module) {
      vec[i] = vec.back();
      vec.pop_back();
      return;
    }
  }
}

void ModuleManager::addModule(IModule *module) {
  if (!module)
    return;

  if (module->getCategory() == ModuleCategory::Utility) {
    utility.push_back(module);
  } else if (module->getCategory() == ModuleCategory::System) {
    system.push_back(module);
  } else {
    game.push_back(module);
  }

  m_moduleLookup[module->getName()] = module;
  std::cout << "[ModuleManager] Registered module: "
            << module->getName().c_str() << std::endl;
}

void ModuleManager::unloadModuleByName(const eastl::string &name) {
  IModule *module = getModuleByName(name);
  if (!module) {
    std::cerr << "[ModuleManager] Cannot unload module, not found: "
              << name.c_str() << std::endl;
    return;
  }

  std::cout << "[ModuleManager] Unloading module: " << name.c_str()
            << std::endl;
  module->shutdown();

  if (module->getCategory() == ModuleCategory::Utility) {
    removeModule(utility, module);
  } else if (module->getCategory() == ModuleCategory::System) {
    removeModule(system, module);
  } else {
    removeModule(game, module);
  }

  m_moduleLookup.erase(name);

  if (module->destroy) {
    module->destroy(module);
  }

  auto libIt = m_libraryHandles.find(name);
  if (libIt != m_libraryHandles.end()) {
#ifdef ENGINE_PLATFORM_WINDOWS
    FreeLibrary((HMODULE)libIt->second);
#elif defined(ENGINE_PLATFORM_LINUX)
    dlclose(libIt->second);
#endif
    m_libraryHandles.erase(libIt);
  }
}

IModule *ModuleManager::getModuleByName(const eastl::string &name) {
  auto it = m_moduleLookup.find(name);
  if (it != m_moduleLookup.end()) {
    return it->second;
  }
  return nullptr;
}

void ModuleManager::update() {
  for (size_t i = 0; i < utility.size(); ++i)
    utility[i]->update();
  for (size_t i = 0; i < system.size(); ++i)
    system[i]->update();
  for (size_t i = 0; i < game.size(); ++i)
    game[i]->update();

  processPendingUnloads();
}

void ModuleManager::init(Engine *engine) {
  m_utilityFacade = new UtilityFacade(engine);
  m_systemFacade = new SystemFacade(engine);
  m_gameFacade = new GameFacade(engine);

  std::cout << "[ModuleManager] Initializing ModuleManager..." << std::endl;
  readManifest(engine);

  for (size_t i = 0; i < utility.size(); ++i)
    utility[i]->init();
  for (size_t i = 0; i < system.size(); ++i)
    system[i]->init();
  for (size_t i = 0; i < game.size(); ++i)
    game[i]->init();
}

void ModuleManager::shutdown() {
  std::cout << "[ModuleManager] Shutting down ModuleManager..." << std::endl;

  for (size_t i = 0; i < game.size(); ++i) {
    game[i]->shutdown();
    if (game[i]->destroy)
      game[i]->destroy(game[i]);
  }
  game.clear();

  for (size_t i = 0; i < system.size(); ++i) {
    system[i]->shutdown();
    if (system[i]->destroy)
      system[i]->destroy(system[i]);
  }
  system.clear();

  for (size_t i = 0; i < utility.size(); ++i) {
    utility[i]->shutdown();
    if (utility[i]->destroy)
      utility[i]->destroy(utility[i]);
  }
  utility.clear();

  m_moduleLookup.clear();
}

void ModuleManager::loadModuleFromDynamicLib(const std::filesystem::path &path,
                                             Engine *engine,
                                             ModuleCategory category) {
  std::filesystem::path fullPath = path;
  fullPath += DYNAMIC_LIB_EXT;

  std::cout << "[ModuleManager] Attempting to load dynamic library: "
            << fullPath.string() << std::endl;

  void *rawSymbolCreate = nullptr;
  destroyModuleFn destroy = nullptr;
  void *hModule = nullptr;

#ifdef ENGINE_PLATFORM_WINDOWS
  HMODULE hWinModule = LoadLibraryA(fullPath.string().c_str());
  if (!hWinModule) {
    std::cerr << "[ModuleManager ERROR] Failed to load library (Win32 Error: "
              << GetLastError() << "): " << fullPath.string() << std::endl;
    return;
  }
  hModule = (void *)hWinModule;
  rawSymbolCreate = (void *)GetProcAddress(hWinModule, "createModule");
  destroy = (destroyModuleFn)GetProcAddress(hWinModule, "destroyModule");
#elif defined(ENGINE_PLATFORM_LINUX)
  hModule = dlopen(fullPath.string().c_str(), RTLD_NOW | RTLD_GLOBAL);
  if (!hModule) {
    std::cerr << "[ModuleManager ERROR] dlopen failed: " << dlerror()
              << std::endl;
    return;
  }

  dlerror();
  rawSymbolCreate = dlsym(hModule, "createModule");
  destroy = (destroyModuleFn)dlsym(hModule, "destroyModule");

  const char *dlsymError = dlerror();
  if (dlsymError) {
    std::cerr << "[ModuleManager ERROR] dlsym details: " << dlsymError
              << std::endl;
  }
#endif

  if (!rawSymbolCreate || !destroy) {
    std::cerr << "[ModuleManager ERROR] Failed to find factory procedures in: "
              << fullPath.string() << std::endl;
#ifdef ENGINE_PLATFORM_WINDOWS
    FreeLibrary((HMODULE)hModule);
#elif defined(ENGINE_PLATFORM_LINUX)
    dlclose(hModule);
#endif
    return;
  }

  IModule *module = nullptr;

  if (category == ModuleCategory::Utility) {
    auto createFn = (createUtilityModuleFn)rawSymbolCreate;
    module = createFn(m_utilityFacade);
  } else if (category == ModuleCategory::System) {
    auto createFn = (createSystemModuleFn)rawSymbolCreate;
    module = createFn(m_systemFacade);
  } else {
    auto createFn = (createGameModuleFn)rawSymbolCreate;
    module = createFn(m_gameFacade);
  }

  if (!module) {
    std::cerr
        << "[ModuleManager ERROR] Factory createModule() returned nullptr for: "
        << fullPath.string() << std::endl;
#ifdef ENGINE_PLATFORM_WINDOWS
    FreeLibrary((HMODULE)hModule);
#elif defined(ENGINE_PLATFORM_LINUX)
    dlclose(hModule);
#endif
    return;
  }

  module->destroy = destroy;
  addModule(module);
  m_libraryHandles[module->getName()] = hModule;
  std::cout << "[ModuleManager] Module loaded successfully: "
            << module->getName().c_str() << std::endl;
}

void ModuleManager::requestUnloadByName(const eastl::string &name) {
  m_pendingUnloads.push_back(name);
}

void ModuleManager::processPendingUnloads() {
  for (size_t i = 0; i < m_pendingUnloads.size(); ++i) {
    unloadModuleByName(m_pendingUnloads[i]);
  }
  m_pendingUnloads.clear();
}

void ModuleManager::readManifest(Engine *engine,
                                 const std::filesystem::path &manifestPath) {
  std::cout << "[ModuleManager] Reading manifest: "
            << std::filesystem::absolute(manifestPath).string() << std::endl;

  std::ifstream file(manifestPath);
  if (!file.is_open()) {
    std::cerr << "[ModuleManager ERROR] Failed to open manifest file at path: "
              << std::filesystem::absolute(manifestPath).string() << std::endl;
    return;
  }

  nlohmann::json jsonData;
  try {
    jsonData = nlohmann::json::parse(file);
  } catch (const nlohmann::json::parse_error &e) {
    std::cerr << "[ModuleManager ERROR] JSON parse error in manifest: "
              << e.what() << std::endl;
    return;
  }

  if (jsonData.contains("modules") && jsonData["modules"].is_array()) {
    for (const auto &item : jsonData["modules"]) {
      ModuleConfig config;

      std::string stdName = item.value("name", "Unknown");
      std::string stdPath = item.value("path", "");
      std::string stdCategory = item.value("category", "Game");

      config.name = stdName.c_str();
      config.path = stdPath;
      config.isEnabled = item.value("enabled", true);

      if (stdCategory == "Utility") {
        config.category = ModuleCategory::Utility;
      } else if (stdCategory == "System") {
        config.category = ModuleCategory::System;
      } else {
        config.category = ModuleCategory::Game;
      }

      if (config.isEnabled && !config.path.empty()) {
        loadModuleFromDynamicLib(config.path, engine, config.category);
      } else {
        std::cout << "[ModuleManager] Skipping disabled or empty module: "
                  << stdName << std::endl;
      }
    }
  }
}