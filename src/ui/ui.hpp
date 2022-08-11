#pragma once

#include <allocator.hpp>
#include "ui_element.hpp"


/*
    @todo: All things what I want to make for UI:

    - @important @fix: Proper position and scale computation for children!
    - Textfields
    - Color Effects
      - Tint
    - Filters
      - Blur
      - Shadow
      - Glow
    - Blending modes
      - Add
      - Multiply
    - Alpha masks
    - Stencil masks
    - Events!
    - Custom responses to Events
    - Animation

*/

struct UiScene {
    UiElement *root;
    UiElement storage[32];
    u32 element_count;

    UiElement *hovered_element;

    // Remember what element is clicked, so that if Cursor is returned to it,
    // it will return to pressed state.
    UiElement *clicked_element;

    // @todo: Focus system
    UiElement *focus;
};
