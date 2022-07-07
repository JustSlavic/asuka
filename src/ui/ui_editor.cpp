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

            Vec2F mouse_position = make_vector2(input->mouse.position);
            Vec2F mouse_prev_position = make_vector2(input->mouse.previous_position);
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


void ui_update_editor(UiEditor *editor, UiScene *scene, Game::Input *input)
{
    UiEditorUpdateResult update_result = {};
    ui_update_editor_impl(editor, scene, scene->root, input, update_result);

    editor->hovered_element = update_result.hovered_element;

    if (GetPressCount(input->mouse.LMB) > 0)
    {
        if (editor->selection != update_result.ui_element_to_select)
        {
            auto action = get_next_action(editor);
            action->type = UI_ACTION_SELECTION;
            action->selection.ui_element = editor->selection;
            commit_action(editor, action);

            editor->selection = update_result.ui_element_to_select;
            osOutputDebugString("commit selection: %lld\n", editor->last_action_index);
        }
    }
    if (editor->selection != NULL && length(update_result.dP) > 0.5f) // 0.5f is a deadzone
    {
        if (editor->current_action_index == -1)
        {
            auto action = get_next_action(editor);
            action->type = UI_ACTION_MOVE;
            action->move.ui_element   = editor->selection;
            action->move.old_position = editor->selection->position;

            editor->current_action_index = editor->last_action_index;
        }

        editor->selection->position += update_result.dP;
    }
    if (GetReleaseCount(input->mouse.LMB) > 0 && editor->current_action_index != -1)
    {
        auto action = get_current_action(editor);
        if (action)
        {
            action->move.new_position = editor->selection->position;
            commit_action(editor, action);
            osOutputDebugString("commit move: %lld\n", editor->last_action_index);
        }
    }
    if (GetHoldCount(input->keyboard.Ctrl) > 0 && GetPressCount(input->keyboard.Z) > 0)
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
                editor->selection = last_action->selection.ui_element;
            }
            break;
        }

        editor->last_action_index -= 1;
        if (editor->last_action_index < 0)
        {
            editor->last_action_index += ARRAY_COUNT(editor->history);
        }

        osOutputDebugString("Ctrl+Z => %lld\n", editor->last_action_index);
    }
}

} // namespace Asuka
