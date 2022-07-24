#include "ui_editor.hpp"


namespace Asuka
{


struct UiEditorUpdateResult
{
    UiElement *hovered_element;
    UiElement *ui_element_to_select;
    v2 dP;
};


INTERNAL
void ui_update_editor_impl(
    UiEditor *editor,
    UiScene *scene,
    UiElement *ui_element,
    Game::Input *input,
    UiEditorUpdateResult &result) // @warning: result is an l-value reference
{
    switch (ui_element->type)
    {
        case UI_ELEMENT_SHAPE:
        {
            Rect2 aabb = get_bounding_box(ui_element);

            v2 mouse_position = make_vector2(input->mouse.position);
            v2 mouse_prev_position = make_vector2(input->mouse.previous_position);
            intersection_result res1 = segment_segment_intersection(aabb.min, make_vector2(aabb.max.x, aabb.min.y), mouse_prev_position, mouse_position);
            intersection_result res2 = segment_segment_intersection(aabb.min, make_vector2(aabb.min.x, aabb.max.y), mouse_prev_position, mouse_position);
            intersection_result res3 = segment_segment_intersection(aabb.max, make_vector2(aabb.max.x, aabb.min.y), mouse_prev_position, mouse_position);
            intersection_result res4 = segment_segment_intersection(aabb.max, make_vector2(aabb.min.x, aabb.max.y), mouse_prev_position, mouse_position);

            b32 hover = in_rectangle(aabb, mouse_position)
                     || res1.found == INTERSECTION_FOUND
                     || res2.found == INTERSECTION_FOUND
                     || res3.found == INTERSECTION_FOUND
                     || res4.found == INTERSECTION_FOUND;

            if (hover)
            {
                result.hovered_element = ui_element;
                if (GetPressCount(input->mouse.LMB) > 0)
                {
                    result.ui_element_to_select = ui_element;
                }

                if (GetHoldCount(input->mouse.LMB) > 0)
                {
                    result.dP = make_vector2(input->mouse.position - input->mouse.previous_position);
                }
            }
        }
        break;

        case UI_ELEMENT_GROUP:
        {
            for (u32 child_index = 0; child_index < ui_element->group.children_count; child_index++)
            {
                auto child = ui_element->group.children[child_index];
                ui_update_editor_impl(editor, scene, child, input, result);
                // if (update_result.ui_element_to_select)
                // {
                //     result.ui_element_to_select = update_result.ui_element_to_select;
                // }
            }
        }
        break;

        default:
            ASSERT_FAIL("You should process all UiElement types.");
    }
}


INTERNAL
void commit_action(UiEditor *editor, UiEditorAction action)
{
    ASSERT(action.type != UI_ACTION_NONE);

    auto next = get_action(editor, editor->action_end_index);
    editor->action_end_index += 1;
    auto end = get_action(editor, editor->action_end_index);

    *next = action;
    *end  = ui_action();
}


INTERNAL
void undo_action(UiEditor *editor)
{
    auto last_action = get_last_action(editor);
    switch (last_action->type)
    {
        case UI_ACTION_MOVE:
        {
            last_action->move.ui_element->position = last_action->move.old_position;
        }
        break;

        case UI_ACTION_SELECTION:
        {
            editor->selection = last_action->selection.from;
        }
        break;
    }

    if (last_action->type != UI_ACTION_NONE)
    {
        editor->action_end_index -= 1;
    }

    osOutputDebugString("Ctrl+Z => %lld\n", editor->action_end_index);
}


INTERNAL
void redo_action(UiEditor *editor)
{
    auto next_action = get_action(editor, editor->action_end_index);
    switch (next_action->type)
    {
        case UI_ACTION_MOVE:
        {
            next_action->move.ui_element->position = next_action->move.new_position;
        }
        break;

        case UI_ACTION_SELECTION:
        {
            editor->selection = next_action->selection.to;
        }
        break;
    }

    if (next_action->type != UI_ACTION_NONE)
    {
        editor->action_end_index += 1;
    }

    osOutputDebugString("Ctrl+Shift+Z => %lld\n", editor->action_end_index);
}


void ui_update_editor(UiEditor *editor, UiScene *scene, Game::Input *input)
{
    UiEditorUpdateResult update_result = {};
    ui_update_editor_impl(editor, scene, scene->root, input, update_result);

    editor->hovered_element = update_result.hovered_element;

    if (GetPressCount(input->mouse.LMB) > 0)
    {
        if (editor->selection != update_result.ui_element_to_select)
        {
            auto action = ui_action(UI_ACTION_SELECTION);
            action.selection.from = editor->selection;
            action.selection.to   = update_result.ui_element_to_select;

            commit_action(editor, action);

            editor->selection = update_result.ui_element_to_select;
            osOutputDebugString("commit selection: %lld\n", editor->action_end_index - 1);
        }
    }
    if (editor->selection != NULL && length(update_result.dP) > 0.5f) // 0.5f is a deadzone
    {
        if (editor->current_action.type == UI_ACTION_NONE)
        {
            auto action = get_current_action(editor);
            action->type = UI_ACTION_MOVE;
            action->move.ui_element   = editor->selection;
            action->move.old_position = editor->selection->position;
        }

        editor->selection->position += update_result.dP;
    }
    if (GetReleaseCount(input->mouse.LMB) > 0 && editor->current_action.type != UI_ACTION_NONE)
    {
        auto action = get_current_action(editor);
        if (action && action->type == UI_ACTION_MOVE)
        {
            action->move.new_position = editor->selection->position;
            commit_action(editor, *action);
            *action = ui_action();
            osOutputDebugString("commit move: %lld\n", editor->action_end_index - 1);
        }
        else
        {
            INVALID_CODE_PATH();
        }
    }
    if (GetHoldCount(input->keyboard.Ctrl) > 0 && GetHoldCount(input->keyboard.Shift) > 0 && GetPressCount(input->keyboard.Z) > 0)
    {
        redo_action(editor);
    }
    else if (GetHoldCount(input->keyboard.Ctrl) > 0 && GetPressCount(input->keyboard.Z) > 0)
    {
        undo_action(editor);
    }
}


} // namespace Asuka
