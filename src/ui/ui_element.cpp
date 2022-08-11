#include "ui_element.hpp"


rect2 get_bounding_box(UiElement *element)
{
    rect2 result = {};

    switch (element->type)
    {
        case UI_ELEMENT_SHAPE:
        {
            result.min = element->position - hadamard(element->pivot, element->scale);
            result.max = result.min + hadamard(element->shape.size, element->scale);
        }
        break;

        case UI_ELEMENT_GROUP:
        {
        }
        break;

        default:
            ASSERT_FAIL("You should process all UiElement types.");
    }

    return result;
}


void push_filter(UiElement *element, UiFilter *filter)
{
    ASSERT(element->filter_count < ARRAY_COUNT(element->filters));

    element->filters[element->filter_count++] = filter;
}


matrix4 get_transform(UiElement *element)
{
    matrix4 transform = matrix4::identity;

    // @todo: make 'matrix4 scale(Matrix, Vec4F)' function
    transform._11 = element->scale.x;
    transform._22 = element->scale.y;

    return transform;
}
