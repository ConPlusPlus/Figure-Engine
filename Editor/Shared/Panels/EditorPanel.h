#pragma once
namespace Figure::Editor::Shared
{
    class EditorContext;

    class EditorPanel
    {
    public:
        virtual ~EditorPanel() = default;
        virtual const wchar_t* GetPanelId() const = 0;
        virtual const wchar_t* GetDisplayName() const = 0;
        virtual void Draw(EditorContext& context) = 0;

        bool IsVisible() const { return m_visible; }
        void SetVisible(bool value) { m_visible = value; }

    private:
        bool m_visible = true;
    };
}
