#pragma once
#include "PropertyValue.h"
#include <string>
#include <vector>

namespace Figure::Editor::Shared
{
    class PropertySource
    {
    public:
        virtual ~PropertySource() = default;
        virtual std::vector<PropertyValue> GetProperties() const = 0;
        virtual bool SetProperty(const std::wstring& propertyName, const std::wstring& value) = 0;
    };
}
