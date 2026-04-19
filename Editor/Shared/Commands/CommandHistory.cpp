#include "CommandHistory.h"
#include "EditorCommand.h"

namespace Figure::Editor::Shared
{
    void CommandHistory::Execute(EditorContext& context, std::unique_ptr<EditorCommand> command)
    {
        if (!command) { return; }
        command->Execute(context);
        m_undoStack.push_back(std::move(command));
        m_redoStack.clear();
    }

    bool CommandHistory::CanUndo() const { return !m_undoStack.empty(); }
    bool CommandHistory::CanRedo() const { return !m_redoStack.empty(); }

    void CommandHistory::Undo(EditorContext& context)
    {
        if (!CanUndo()) { return; }
        auto command = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        command->Undo(context);
        m_redoStack.push_back(std::move(command));
    }

    void CommandHistory::Redo(EditorContext& context)
    {
        if (!CanRedo()) { return; }
        auto command = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        command->Execute(context);
        m_undoStack.push_back(std::move(command));
    }

    void CommandHistory::Clear()
    {
        m_undoStack.clear();
        m_redoStack.clear();
    }
}
