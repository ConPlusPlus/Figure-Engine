#include "PropertyInspector.h"

namespace Figure::Editor::Shared
{
    void PropertyInspector::SetSource(const std::shared_ptr<PropertySource>& source) { m_source = source; }
    std::shared_ptr<PropertySource> PropertyInspector::GetSource() const { return m_source; }

    std::vector<PropertyValue> PropertyInspector::GetVisibleProperties() const
    {
        return m_source ? m_source->GetProperties() : std::vector<PropertyValue>{};
    }

    bool PropertyInspector::UpdateProperty(const std::wstring& propertyName, const std::wstring& value)
    {
        return m_source ? m_source->SetProperty(propertyName, value) : false;
    }
}
