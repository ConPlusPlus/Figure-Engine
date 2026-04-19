#pragma once
#include "PropertySource.h"
#include <memory>
#include <vector>

namespace Figure::Editor::Shared
{
    class PropertyInspector
    {
    public:
        void SetSource(const std::shared_ptr<PropertySource>& source);
        std::shared_ptr<PropertySource> GetSource() const;
        std::vector<PropertyValue> GetVisibleProperties() const;
        bool UpdateProperty(const std::wstring& propertyName, const std::wstring& value);

    private:
        std::shared_ptr<PropertySource> m_source;
    };
}
