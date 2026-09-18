#pragma once

class IUtilityContext {
public:
    virtual ~IUtilityContext() = default;
    virtual float getDeltaTime() const = 0;
};