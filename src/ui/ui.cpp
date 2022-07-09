#include "ui.hpp"


namespace Asuka {


UiScene *ui_allocate_scene(memory::arena_allocator *allocator)
{
    UiScene *result = memory::allocate_struct<UiScene>(allocator);
    return result;
}


void push_child(UiGroup *group, UiElement *child)
{
    ASSERT(group->children_count < ARRAY_COUNT(group->children));

    group->children[group->children_count++] = child;
}


void ui_update_element(UiScene *scene, UiElement *ui_element, Game::Input *input)
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
                scene->hovered_element = ui_element;

                if (GetPressCount(input->mouse.LMB) > 0)
                {
                    scene->clicked_element = ui_element;
                }

                if (GetHoldCount(input->mouse.LMB) > 0)
                {
                    if (scene->clicked_element == ui_element)
                    {
                    }
                    else
                    {
                        // @todo: Decide should it play animation of hover when
                        // hovered with mouse button holded or not
                        // ui_element->color = Color32::Red;
                    }
                }
                else
                {
                }

                if (GetReleaseCount(input->mouse.LMB) > 0)
                {
                    if (scene->clicked_element == ui_element)
                    {
                        if (ui_element->on_click)
                        {
                            ui_element->shape.n += GetReleaseCount(input->mouse.LMB);
                            ui_element->on_click();
                        }
                    }
                }
            }
            else
            {
                if (GetHoldCount(input->mouse.LMB) > 0)
                {
                }
                else
                {
                }
            }
        }
        break;

        case UI_ELEMENT_GROUP:
        {
            for (u32 child_index = 0; child_index < ui_element->group.children_count; child_index++)
            {
                auto child = ui_element->group.children[child_index];
                ui_update_element(scene, child, input);
            }
        }
        break;

        default:
            ASSERT_FAIL("You should process all UiElement types.");
    }

    if (GetReleaseCount(input->mouse.LMB) > 0)
    {
        scene->clicked_element = NULL;
    }
}

void ui_update_scene(UiScene *scene, Game::Input *input)
{
    scene->hovered_element = NULL;
    ui_update_element(scene, scene->root, input);
}

} // namespace Asuka
