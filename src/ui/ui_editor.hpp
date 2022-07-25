#pragma once

#include "ui_element.hpp"


/*
    @todo:
    - Actions recording for Undo/Redo
*/

struct UiScene;
namespace Game { struct Input; }

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


UiEditorAction ui_action(UiEditorActionType type);
UiEditorAction ui_action();


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


UiEditorAction *get_action(UiEditor *editor, isize index);
UiEditorAction *get_current_action(UiEditor *editor);
UiEditorAction *get_last_action(UiEditor *editor);
void ui_update_editor(UiEditor *editor, UiScene *scene, Game::Input *input);
