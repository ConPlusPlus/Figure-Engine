#pragma once
namespace Figure::Editor::Shared
{
    class EditorContext;

    class EditorToolApp
    {
    public:
        virtual ~EditorToolApp() = default;
        virtual const wchar_t* GetToolName() const = 0;
        virtual bool Initialize(EditorContext& context) = 0;
        virtual void Shutdown(EditorContext& context) = 0;
    };
}
