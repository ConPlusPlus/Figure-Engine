#pragma once
namespace Figure::Editor::Shared
{
    class EditorContext;

    class IEditorTool
    {
    public:
        virtual ~IEditorTool() = default;
        virtual const wchar_t* GetToolId() const = 0;
        virtual const wchar_t* GetDisplayName() const = 0;
        virtual void OnActivated(EditorContext& context) = 0;
        virtual void OnDeactivated(EditorContext& context) = 0;
    };
}
