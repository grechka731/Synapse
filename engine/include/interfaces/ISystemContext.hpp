#pragma once

#include <EASTL/string.h>
#include "IModule.hpp"
#include "IUtilityContext.hpp"


class ISystemContext : public IUtilityContext{
public:
    virtual ~ISystemContext() = default;
    virtual IModule* getModule(const eastl::string& name) = 0;
};