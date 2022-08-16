#pragma once

#include <math.hpp>


/*
    Good to know terminology:

    1. Ui Coordinates are top-down 2D coordinates, where 1 unit corresponds to one
    pixel on the render target.

    2. Local Coordinates are natural local coordinates of the object's coordinate sytstem,
    which can be scaled or offseted by the pivot's position;

    3. Pivot is the point in the local coordinate system, where the center of
    this coordinates are.

       ┌─────────┐
       │  o─→x   │
       │  ↓y     │
       │         │
       └─────────┘

    4.
*/


using EventT = void(void);

enum UiFilterType
{
    UI_FILTER_BLUR,
    UI_FILTER_SHADOW,
    UI_FILTER_TINT,
};

struct UiFilterBlur
{
    v2 radius;
};

struct UiFilterShadow
{
    f32 angle;
    f32 distance;
};

struct UiFilterTint
{
    v4 multiply;
    v4 add;
};

struct UiFilter
{
    UiFilterType type;

    union
    {
        UiFilterBlur blur;
        UiFilterShadow shadow;
        UiFilterTint tint;
    };
};

struct UiElement;

enum UiElementType
{
    UI_ELEMENT_SHAPE,
    UI_ELEMENT_GROUP,
};

struct UiShape
{
    v2 size;     // @note: in Local coordinates
    color32 color;  // Natural element's color, but not be present for UiBitmaps, for example

    // @debug: Diagonal squares
    int32 n;

    // @debug Button
    b32 is_pressed;
};

struct UiGroup
{
    UiElement *children[32];
    u32 children_count;
};

struct UiElement
{
    UiElementType type;
    v2 position; // @note: in Parent coordinates
    v2 pivot;    // @note: in Local coordinates
    v2 scale;    // @note: default should be (1, 1) equivalent to "no scale"

    union
    {
        UiShape shape;
        UiGroup group;
    };

    EventT *on_click;

    UiFilter *filters[3];
    u32 filter_count;
};

template <typename Allocator>
UiElement *allocate_ui_element(Allocator *allocator, UiElementType type)
{
    auto ui_element = ALLOCATE_STRUCT(allocator, UiElement);
    ui_element->type = type;
    ui_element->scale = make_vector2(1, 1);

    return ui_element;
}

template <typename Allocator>
UiElement *allocate_ui_shape(Allocator *allocator)
{
    auto ui_element = allocate_ui_element(allocator, UI_ELEMENT_SHAPE);

    ui_element->shape.size = make_vector2(100, 100);
    ui_element->shape.color = color32::White;
    ui_element->shape.n = 1;

    return ui_element;
}

template <typename Allocator>
UiElement *allocate_ui_group(Allocator *allocator)
{
    auto ui_element = allocate_ui_element(allocator, UI_ELEMENT_GROUP);
    return ui_element;
}

rect2 get_bounding_box(UiElement *element);
void push_filter(UiShape *element, UiFilter *filter);
matrix4 get_transform(UiElement *element);

#if UNITY_BUILD
#include "ui_element.cpp"
#endif
