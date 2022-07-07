#pragma once

#include "ui_element.hpp"

/*
    @todo:
    - Actions recording for Undo/Redo
*/

namespace Asuka {


using UiPosition = decltype(UiElement::position);

enum UiEditorActionType
{
    UI_ACTION_NONE = 0,
    UI_ACTION_MOVE,
    UI_ACTION_SELECTION,
};

struct UiEditorActionMove
{
    UiElement *ui_element;
    UiPosition old_position;
    UiPosition new_position;
};

struct UiEditorActionSelection
{
    UiElement *ui_element;
};

struct UiEditorAction
{
    UiEditorActionType type;
    union
    {
        UiEditorActionMove move;
        UiEditorActionSelection selection;
    };
};


struct UiEditor
{
    UiElement *hovered_element;
    UiElement *selection;

    UiEditorAction history[32];
    isize last_action_index;
    isize current_action_index = -1;

    UiElement *graveyard[32];
    usize graveyard_size;
};


INLINE
isize get_next_action_index(UiEditor *editor)
{
    isize result = (editor->last_action_index + 1) % ARRAY_COUNT(editor->history);
    return result;
}


INLINE
UiEditorAction *get_next_action(UiEditor *editor)
{
    isize desired_index = get_next_action_index(editor);
    UiEditorAction *result = editor->history + desired_index;
    return result;
}


INLINE
UiEditorAction *get_last_action(UiEditor *editor)
{
    UiEditorAction *result = editor->history + editor->last_action_index;
    return result;
}


INLINE
UiEditorAction *get_current_action(UiEditor *editor)
{
    UiEditorAction *result = NULL;

    if (editor->current_action_index != -1)
    {
        ASSERT(editor->current_action_index < ARRAY_COUNT(editor->history));
        result = editor->history + editor->current_action_index;
    }

    return result;
}

INLINE
void commit_action(UiEditor *editor, UiEditorAction *action)
{
    isize next_index = get_next_action_index(editor);
    editor->last_action_index = next_index;
    editor->current_action_index = -1;
}


} // namespace Asuka
