#pragma once
namespace Figure::Editor::Shared
{
    class EditorContext;

    class EditorCommand
    {
    public:
        virtual ~EditorCommand() = default;
        virtual const wchar_t* GetName() const = 0;
        virtual void Execute(EditorContext& context) = 0;
        virtual void Undo(EditorContext& context) = 0;
    };
}
