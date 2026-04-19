#pragma once
#include <memory>
#include <vector>

namespace Figure::Editor::Shared
{
    class EditorContext;
    class EditorCommand;

    class CommandHistory
    {
    public:
        void Execute(EditorContext& context, std::unique_ptr<EditorCommand> command);
        bool CanUndo() const;
        bool CanRedo() const;
        void Undo(EditorContext& context);
        void Redo(EditorContext& context);
        void Clear();

    private:
        std::vector<std::unique_ptr<EditorCommand>> m_undoStack;
        std::vector<std::unique_ptr<EditorCommand>> m_redoStack;
    };
}
