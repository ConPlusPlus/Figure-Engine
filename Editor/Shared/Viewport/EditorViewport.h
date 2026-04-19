#pragma once
namespace Figure::Editor::Shared
{
    class EditorContext;

    class EditorViewport
    {
    public:
        virtual ~EditorViewport() = default;
        virtual const wchar_t* GetViewportId() const = 0;
        virtual const wchar_t* GetDisplayName() const = 0;
        virtual void Draw(EditorContext& context) = 0;
    };
}
