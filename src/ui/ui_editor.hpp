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
    UiElement *from;
    UiElement *to;
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


UiEditorAction ui_action(UiEditorActionType type)
{
    UiEditorAction action = {};
    action.type = type;

    return action;
}


UiEditorAction ui_action()
{
    UiEditorAction result = ui_action(UI_ACTION_NONE);
    return result;
}


struct UiEditor
{
    UiElement *hovered_element;
    UiElement *selection;

    // @note: current_action is like a scratchpad where you
    // record your action and then commit to history later.
    UiEditorAction current_action;
    UiEditorAction history[32];
    isize action_end_index;

    UiElement *graveyard[32];
    usize graveyard_size;
};


INLINE
UiEditorAction *get_action(UiEditor *editor, isize index)
{
    // Wrapping to the positive modulo
    index = (index % ARRAY_COUNT(editor->history) + ARRAY_COUNT(editor->history)) % ARRAY_COUNT(editor->history);
    UiEditorAction *result = editor->history + index;
    return result;
}


INLINE
UiEditorAction *get_current_action(UiEditor *editor)
{
    UiEditorAction *result = &editor->current_action;
    return result;
}


INLINE
UiEditorAction *get_last_action(UiEditor *editor)
{
    UiEditorAction *result = get_action(editor, editor->action_end_index - 1);
    return result;
}


} // namespace Asuka
