#include "ui_editor.hpp"


namespace Asuka
{


struct UiEditorUpdateResult
{
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

            Vec2F mouse_position = V2(input->mouse.position);
            Vec2F mouse_prev_position = V2(input->mouse.previous_position);
            intersection_result res1 = segment_segment_intersection(aabb.min, V2(aabb.max.x, aabb.min.y), mouse_prev_position, mouse_position);
            intersection_result res2 = segment_segment_intersection(aabb.min, V2(aabb.min.x, aabb.max.y), mouse_prev_position, mouse_position);
            intersection_result res3 = segment_segment_intersection(aabb.max, V2(aabb.max.x, aabb.min.y), mouse_prev_position, mouse_position);
            intersection_result res4 = segment_segment_intersection(aabb.max, V2(aabb.min.x, aabb.max.y), mouse_prev_position, mouse_position);

            b32 hover = in_rectangle(aabb, mouse_position)
                     || res1.found == INTERSECTION_FOUND
                     || res2.found == INTERSECTION_FOUND
                     || res3.found == INTERSECTION_FOUND
                     || res4.found == INTERSECTION_FOUND;

            if (hover)
            {
                if (GetPressCount(input->mouse.LMB) > 0)
                {
                    result.ui_element_to_select = ui_element;
                }

                if (GetHoldCount(input->mouse.LMB) > 0)
                {
                    result.dP = V2(input->mouse.position - input->mouse.previous_position);
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

    if (GetPressCount(input->mouse.LMB) > 0)
    {
        editor->selection = update_result.ui_element_to_select;
    }
    if (editor->selection != NULL && length(update_result.dP) > 0.5f) // 0.1f is a deadzone
    {
        editor->selection->position += update_result.dP;
    }
}

} // namespace Asuka
