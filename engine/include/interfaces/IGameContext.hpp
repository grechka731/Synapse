#pragma once


#include <EASTL/string.h>
#include "IModule.hpp"
#include "IUtilityContext.hpp"

class IGameContext : public IUtilityContext{
public:
    virtual ~IGameContext() = default;
    virtual IModule* getSystemModule(const eastl::string& name) = 0;
};