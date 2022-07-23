#include "asuka.hpp"
#include <png.hpp>


#define ASUKA_DEBUG_FOLLOWING_CAMERA 1


namespace Game {


f32 uniform_real(f32 from, f32 to)
{
    ASSERT(to > from);

    f32 r = (f32)rand()/(f32)(RAND_MAX/to); // Uniform [0, 1]
    return r + from;
}


INTERNAL
void DrawBitmap(
    OffscreenBuffer* buffer,
    f32 left, f32 top,
    Bitmap *image,
    f32 c_alpha = 1.0f)
{
    // @note: Top-down coordinate system.
    Vector2i tl = round_to_v2i(make_vector2(left, top));
    Vector2i br = tl + round_to_v2i(make_vector2(image->width, image->height));

    Vector2i image_tl = make_vector2i(0, 0);
    Vector2i image_br = make_vector2i(image->width, image->height);

    if (tl.x < 0) {
        image_tl.x = -tl.x;
        tl.x = 0;
    }
    if (tl.y < 0) {
        image_tl.y = -tl.y;
        tl.y = 0;
    }
    if (br.x > buffer->Width) {
        br.x = buffer->Width;
    }
    if (br.y > buffer->Height) {
        br.y = buffer->Height;
    }

    Vector2i dimensions = br - tl;
    Vector2i image_dims = image_br - image_tl;

    Vector2i dims {
        (dimensions.x < image_dims.x) ? dimensions.x : image_dims.x,
        (dimensions.y < image_dims.y) ? dimensions.y : image_dims.y,
    };

    u8* Row = (u8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;
    u8* image_pixel_row = (u8 *) image->pixels + image_tl.y * image->width * image->bytes_per_pixel + image_tl.x * image->bytes_per_pixel;

    for (int y = 0; y < dims.y; y++) {
        u32* Pixel = (u32*) Row;
        u8 * image_pixel = image_pixel_row;

        for (int x = 0; x < dims.x; x++) {
            u8 blue = image_pixel[0];
            u8 green = image_pixel[1];
            u8 red = image_pixel[2];
            u8 alpha = image_pixel[3];

            if (image->bytes_per_pixel == 1) {
                u8 chroma = *image_pixel;

                f32 c = chroma / 255.f;
                f32 a = alpha / 255.f;

                f32 back_r = (*Pixel & 0xFF) / 255.f;
                f32 back_g = ((*Pixel & 0xFF00) >> 8) / 255.f;
                f32 back_b = ((*Pixel & 0xFF0000) >> 16) / 255.f;
                f32 back_a = ((*Pixel & 0xFF000000) >> 24) / 255.f;

                f32 new_r = (1.0f - a) * back_r + a * c;
                f32 new_g = (1.0f - a) * back_g + a * c;
                f32 new_b = (1.0f - a) * back_b + a * c;

                *Pixel = (((u32)(new_r * 255.f)) << 0) |
                         (((u32)(new_g * 255.f)) << 8) |
                         (((u32)(new_b * 255.f)) << 16);
            } else if (image->bytes_per_pixel == 3) {
                *Pixel = (red) | (green << 8) | (blue << 16);
            } else if (image->bytes_per_pixel == 4) {
                f32 r = red / 255.f;
                f32 g = green / 255.f;
                f32 b = blue / 255.f;
                f32 a = (alpha / 255.f) * c_alpha;

                f32 back_r = (*Pixel & 0xFF) / 255.f;
                f32 back_g = ((*Pixel & 0xFF00) >> 8) / 255.f;
                f32 back_b = ((*Pixel & 0xFF0000) >> 16) / 255.f;
                f32 back_a = ((*Pixel & 0xFF000000) >> 24) / 255.f;

                // @todo: it seems I messed up red and blue channels here
                f32 new_r = (1.0f - a) * back_r + a * r;
                f32 new_g = (1.0f - a) * back_g + a * g;
                f32 new_b = (1.0f - a) * back_b + a * b;

                *Pixel = (((u32)(new_r * 255.f)) << 0) |
                         (((u32)(new_g * 255.f)) << 8) |
                         (((u32)(new_b * 255.f)) << 16);
            }

            Pixel++;
            image_pixel += image->bytes_per_pixel;
        }

        Row += buffer->Pitch;
        image_pixel_row += image->width * image->bytes_per_pixel;
    }
}


INTERNAL
void DrawRectangle(
    OffscreenBuffer* buffer,
    v2 top_left, v2 bottom_right,
    Color24 color,
    b32 stroke = false)
{
    Vector2i tl = round_to_v2i(top_left);
    Vector2i br = round_to_v2i(bottom_right);

    if (tl.x < 0) tl.x = 0;
    if (tl.y < 0) tl.y = 0;
    if (br.x > buffer->Width)  br.x = buffer->Width;
    if (br.y > buffer->Height) br.y = buffer->Height;

    Vector2i dimensions = br - tl;

    u8* Row = (u8*)buffer->Memory + tl.y*buffer->Pitch + tl.x*buffer->BytesPerPixel;

    for (int y = 0; y < dimensions.y; y++) {
        u32* Pixel = (u32*) Row;

        for (int x = 0; x < dimensions.x; x++) {
            if (stroke && (x == 0 || y == 0)) {
                *Pixel = 0;
            } else {
                *Pixel = pack_to_uint32(color);
            }
            Pixel++;
        }

        Row += buffer->Pitch;
    }
}


#if ASUKA_PLAYBACK_LOOP
INTERNAL
void DrawBorder(OffscreenBuffer* Buffer, u32 Width, Color24 Color)
{
    DrawRectangle(Buffer, make_vector2(0, 0), make_vector2(Buffer->Width, Width), Color);
    DrawRectangle(Buffer, make_vector2(0, Width), make_vector2(Width, Buffer->Height - Width), Color);
    DrawRectangle(Buffer, make_vector2(Buffer->Width - Width, Width), make_vector2(Buffer->Width, Buffer->Height - Width), Color);
    DrawRectangle(Buffer, make_vector2(0, Buffer->Height - Width), make_vector2(Buffer->Width, Buffer->Height), Color);
}
#endif


INTERNAL
void draw_empty_rectangle_in_meters(OffscreenBuffer *buffer, Rectangle2 rect, u32 width, Color24 color, v2 offset, f32 pixels_per_meter)
{
    f32 rect_width = get_width(rect);
    f32 rect_height = get_height(rect);

    v2 screen_center_in_pixels = 0.5f * make_vector2(buffer->Width, buffer->Height);

    offset.y = -offset.y;

    v2 min_corner_in_pixels = screen_center_in_pixels - (get_center(rect) - rect.min - offset) * pixels_per_meter;
    v2 max_corner_in_pixels = screen_center_in_pixels + (rect.max - get_center(rect) + offset) * pixels_per_meter;

    DrawRectangle(buffer, min_corner_in_pixels, make_vector2(max_corner_in_pixels.x, min_corner_in_pixels.y + width), color);
    DrawRectangle(buffer, make_vector2(min_corner_in_pixels.x, min_corner_in_pixels.y + width), make_vector2(min_corner_in_pixels.x + width, max_corner_in_pixels.y - width), color);
    DrawRectangle(buffer, make_vector2(max_corner_in_pixels.x - width, min_corner_in_pixels.y + width), make_vector2(max_corner_in_pixels.x, max_corner_in_pixels.y - width), color);
    DrawRectangle(buffer, make_vector2(min_corner_in_pixels.x, max_corner_in_pixels.y - width), max_corner_in_pixels, color);
}


INTERNAL
void ui_draw_element(UiScene *scene, UiElement *ui_element, OffscreenBuffer *buffer)
{
    switch (ui_element->type)
    {
        case UI_ELEMENT_SHAPE:
        {
            i32 n = ui_element->shape.n;
            Rectangle2 aabb = get_bounding_box(ui_element);
            for (i32 i = 0; i < n; i++)
            {
                Vec2F diagonal = (aabb.max - aabb.min) / f32(n);

                Vec2F lt = aabb.min + diagonal * f32(i);
                Vec2F rb = lt + diagonal;

                Color32 color = ui_element->shape.color;

                for (u32 filter_index = 0; filter_index < ui_element->filter_count; filter_index++)
                {
                    UiFilter *filter = ui_element->filters[filter_index];

                    switch (filter->type)
                    {
                        case UI_FILTER_BLUR:
                        {
                        }
                        break;

                        case UI_FILTER_SHADOW:
                        {
                            UiFilterShadow *shadow = &filter->shadow;
                            DrawRectangle(buffer, lt + make_vector2(shadow->distance), rb + make_vector2(shadow->distance), Color24::Black);
                        }
                        break;

                        case UI_FILTER_TINT:
                        {
                            UiFilterTint *tint = &filter->tint;
                            color.r = color.r * tint->multiply.r + tint->add.r;
                            color.g = color.g * tint->multiply.g + tint->add.g;
                            color.b = color.b * tint->multiply.b + tint->add.b;
                            color.a = color.a * tint->multiply.a + tint->add.a;
                        }
                        break;

                        default:
                            ASSERT_FAIL("Process all kinds of filters!");
                    }
                }

                if (ui_element == scene->hovered_element)
                {

                    if (ui_element == scene->clicked_element)
                    {
                        lt += make_vector2(5.0f);
                        rb += make_vector2(5.0f);

                        color = Color32{ 0.2f, 0.3f, 0.5f, 1.0f };
                    }
                    else
                    {
                        color = Color32{ 0.9f, 0.2f, 0.2f, 1.0f };
                    }
                }
                else
                {
                }

                DrawRectangle(buffer, lt, rb, color.rgb);
            }
        }
        break;

        case UI_ELEMENT_GROUP:
        {
            Matrix4 transform = Matrix4::Identity;
            for (u32 child_index = 0; child_index < ui_element->group.children_count; child_index++)
            {
                auto child = ui_element->group.children[child_index];

                // @todo: pass transform matrix
                ui_draw_element(scene, child, buffer);
            }
        }
        break;

        default:
            ASSERT_FAIL("You should process all UiElement types.");
    }
}


#if UI_EDITOR_ENABLED
INTERNAL
void ui_draw_editor(UiScene *scene, UiEditor *editor, OffscreenBuffer *buffer)
{
    UiElement *hovered = editor->hovered_element;
    if (hovered)
    {
        switch (hovered->type)
        {
            case UI_ELEMENT_SHAPE:
            {
                Rectangle2 aabb = get_bounding_box(hovered);
                Color24 color = { 215.0f / 255.0f, 215.0f / 255.0f, 215.0f / 255.0f };

                f32 width = 2;
                DrawRectangle(buffer, make_vector2(aabb.min.x - width, aabb.min.y - width), make_vector2(aabb.min.x, aabb.max.y + width), color);
                DrawRectangle(buffer, make_vector2(aabb.min.x, aabb.min.y - width), make_vector2(aabb.max.x + width, aabb.min.y), color);
                DrawRectangle(buffer, make_vector2(aabb.max.x, aabb.min.y), make_vector2(aabb.max.x + width, aabb.max.y), color);
                DrawRectangle(buffer, make_vector2(aabb.min.x, aabb.max.y), make_vector2(aabb.max.x + width, aabb.max.y + width), color);

            }
            break;

            case UI_ELEMENT_GROUP:
            {

            }
            break;

            default:
                ASSERT_FAIL("You should process all UiElement types.");
        }
    }

    UiElement *selection = editor->selection;
    if (selection)
    {
        switch (selection->type)
        {
            case UI_ELEMENT_SHAPE:
            {
                Rectangle2 aabb = get_bounding_box(selection);
                Color24 color = { 3.0f / 255.0f, 215.0f / 255.0f, 252.0f / 255.0f };

                f32 width = 2;
                DrawRectangle(buffer, make_vector2(aabb.min.x - width, aabb.min.y - width), make_vector2(aabb.min.x, aabb.max.y + width), color);
                DrawRectangle(buffer, make_vector2(aabb.min.x, aabb.min.y - width), make_vector2(aabb.max.x + width, aabb.min.y), color);
                DrawRectangle(buffer, make_vector2(aabb.max.x, aabb.min.y), make_vector2(aabb.max.x + width, aabb.max.y), color);
                DrawRectangle(buffer, make_vector2(aabb.min.x, aabb.max.y), make_vector2(aabb.max.x + width, aabb.max.y + width), color);

            }
            break;

            case UI_ELEMENT_GROUP:
            {

            }
            break;

            default:
                ASSERT_FAIL("You should process all UiElement types.");
        }
    }
}
#endif // UI_EDITOR_ENABLED


INTERNAL
void ui_draw_scene(UiScene *scene, OffscreenBuffer *buffer)
{
    ui_draw_element(scene, scene->root, buffer);
}


INTERNAL
void push_piece(VisiblePieceGroup *group, v3 offset_in_meters, v2 dim_in_meters, Bitmap *bitmap, Color32 color)
{
    // @note offset and dimensions are in world space (in meters, bottom-up coordinate space)
    ASSERT(group->count < ARRAY_COUNT(group->assets));

    VisiblePiece *asset = group->assets + (group->count++);
    memory::set(asset, 0, sizeof(VisiblePiece));

    asset->offset = make_vector2(offset_in_meters.x, -(offset_in_meters.y + offset_in_meters.z)) * group->pixels_per_meter;
    asset->dimensions = dim_in_meters * group->pixels_per_meter;
    asset->bitmap = bitmap;
    asset->color = color;
}


INTERNAL
void push_rectangle(VisiblePieceGroup *group, v3 offset_in_meters, v2 dim_in_meters, Color32 color)
{
    push_piece(group, offset_in_meters, dim_in_meters, NULL, color);
}


INTERNAL
void push_asset(VisiblePieceGroup *group, Bitmap *bitmap, v3 offset_in_meters, f32 alpha = 1.0f)
{
    push_piece(group, offset_in_meters, make_vector2(0, 0), bitmap, make_rgba(0, 0, 0, alpha));
}


INTERNAL
void draw_hitpoints(SimEntity *entity, VisiblePieceGroup *group)
{
    f32 health_width   = 0.1f; // meters
    f32 health_spacing = 1.5f * health_width; // pixels

    f32 offset_x = -0.5f * health_spacing * (entity->health_max - 1);
    f32 offset_y = 1.0f;

    for (i32 health_index = 0; health_index < entity->health_max; health_index++) {
        HealthPoint hp = entity->health[health_index];

        if (hp.fill > 0) {
            push_rectangle(group, make_vector3(offset_x, offset_y, 0), make_vector2(health_width), make_rgba(0.0f, 1.0f, 0.0f, 1.0f));
        } else {
            push_rectangle(group, make_vector3(offset_x, offset_y, 0), make_vector2(health_width), make_rgba(1.0f, 0.0f, 0.0f, 1.0f));
        }

        offset_x += health_spacing;
    }
}


struct EntityResult
{
    StoredEntity *entity;
    u32 index;
};


INTERNAL
EntityResult add_entity(GameState *game_state, WorldPosition position = null_position())
{
    ASSERT(game_state->entity_count < ARRAY_COUNT(game_state->entities));

    EntityResult result {};
    result.index = game_state->entity_count++;
    result.entity = get_stored_entity(game_state, result.index);

    ASSERT(result.entity);

    memory::set(result.entity, 0, sizeof(StoredEntity));
    result.entity->world_position = null_position();
    result.entity->sim.storage_index = result.index;
    result.entity->sim.distance_limit = INF;
    result.entity->sim.time_limit = INF;

    change_entity_location(game_state->world, result.index, result.entity, &position, &game_state->world_arena);

    return result;
}


INTERNAL
void init_hitpoints(StoredEntity *entity, u32 health_max)
{
    ASSERT(health_max < ARRAY_COUNT(entity->sim.health));

    entity->sim.health_max = health_max;
    entity->sim.health_fill_max = ENTITY_HEALTH_STARTING_FILL_MAX;
    for (i32 health_index = 0; health_index < entity->sim.health_max; health_index++)
    {
        entity->sim.health[health_index].fill = entity->sim.health_fill_max;
    }
}


INTERNAL
EntityResult add_sword(GameState *game_state)
{
    EntityResult result = add_entity(game_state);

    result.entity->sim.type = ENTITY_TYPE_SWORD;
    result.entity->sim.hitbox = make_vector3(0.4, 0.2, 0.2);
    set(&result.entity->sim, ENTITY_FLAG_NONSPATIAL);

    return result;
}


INTERNAL
EntityResult add_player(GameState *game_state)
{
    EntityResult result = add_entity(game_state, world_origin());

    result.entity->sim.type = ENTITY_TYPE_PLAYER;
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);

    // @todo: fix coordinates for hitbox
    result.entity->sim.hitbox = make_vector3(0.8, 0.2, 1.0); // In top-down coordinates, but in meters.

    EntityResult sword_ = add_sword(game_state);
    result.entity->sim.sword.index = sword_.index;

    init_hitpoints(result.entity, 3);

    return result;
}


INTERNAL
EntityResult add_familiar(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v3 p)
{
    WorldPosition position = world_position(game_state->world, chunk_x, chunk_y, chunk_z, p);

    EntityResult result = add_entity(game_state, position);
    result.entity->sim.type = ENTITY_TYPE_FAMILIAR;
    // @todo: fix coordinates for hitbox
    result.entity->sim.hitbox = make_vector3(0.8, 0.2, 0.2);
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);

    return result;
}


INTERNAL
EntityResult add_monster(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v3 p)
{
    WorldPosition position = world_position(game_state->world, chunk_x, chunk_y, chunk_z, p);

    EntityResult result = add_entity(game_state, position);
    result.entity->sim.type = ENTITY_TYPE_MONSTER;
    result.entity->world_position = position;
    result.entity->sim.hitbox = make_vector3(2.2, 2.2, 1.0);
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);

    init_hitpoints(result.entity, 7);

    return result;
}


INTERNAL
EntityResult add_wall(GameState *game_state, i32 chunk_x, i32 chunk_y, i32 chunk_z, v3 p)
{
    WorldPosition position = world_position(game_state->world, chunk_x, chunk_y, chunk_z, p);

    EntityResult result = add_entity(game_state, position);
    result.entity->sim.type = ENTITY_TYPE_WALL;
    result.entity->world_position = position;
    set(&result.entity->sim, ENTITY_FLAG_COLLIDABLE);
    result.entity->sim.hitbox = make_vector3(1.0, 0.4, 1.0);

    return result;
}


INTERNAL
b32 types_match(SimEntity **a, EntityType t, SimEntity **b, EntityType s)
{
    if (((*a)->type == s) && ((*b)->type == t))
    {
        SimEntity *tmp = *a;
        *a = *b;
        *b = tmp;
    }

    b32 result = (((*a)->type == t) && ((*b)->type == s));
    return result;
}


INTERNAL
void handle_collision(SimEntity *a, SimEntity *b)
{
    if (types_match(&a, ENTITY_TYPE_SWORD, &b, ENTITY_TYPE_MONSTER))
    {
        if (b->health_max > 0)
        {
            b->health_max -= 1;
        }
        make_entity_nonspatial(a);
        if (b->health_max == 0)
        {
            make_entity_nonspatial(b);
        }
    }
}


void move_entity(GameState *game_state, SimRegion *sim_region, SimEntity *entity, MoveSpec spec, f32 dt)
{
    ASSERT(!is(entity, ENTITY_FLAG_NONSPATIAL));

    // p = p0 + v0 * t + a * t^2 / 2

    v3 position = entity->position;
    v3 velocity = entity->velocity + spec.acceleration * dt;
    v3 destination = position + velocity * dt;

    /*
        @note:
    */
    // @todo: include this into common collision logic
    if (!spec.jump)
    {
        if (destination.z < 0)
        {
            /*
                (p.x, p.y, p.z) ----- (q.x, q.y, 0) ----- (d.x, d.y, d.z)
                q = p + v*t1
                A <- (0, 0, 0) -- any point on the plane Oz
                n <- (0, 0, 1) -- normal of the plane Oz

                dot((q - A), n) == 0
                dot(q, n) == 0
                dot(p + v*t1, n) == 0
                dot(p, n) + dot(v, n) * t1 == 0
                p.z + v.z * t1 == 0
                t1 == - p.z / v.z

                => q = p + v * (p)
            */

            f32 t1 = - position.z / velocity.z;
            dt -= t1;

            v3 q = position + velocity * t1;

            destination = q;
            velocity.z = 0;
        }
    }

    // @todo: restructure this code when you will implement more sofisticated collision code

    // ================= COLLISION DETECTION ====================== //

    f32 remaining_dt = dt;

    const int ASUKA_MAX_MOVE_TRIES = 5;
    for (i32 move_try = 0; move_try < ASUKA_MAX_MOVE_TRIES; move_try++)
    {
        f32 distance_to_move = length(destination - position);
        if (distance_to_move > entity->distance_limit)
        {
            // Reduce remaining dt so that destination is at the limit distance off of position
            remaining_dt *= entity->distance_limit / distance_to_move;
            destination = position + velocity * remaining_dt;

            f32 new_distance_to_move = length(destination - position);
            ASSERT(Asuka::is_equal(new_distance_to_move, entity->distance_limit));
            distance_to_move = entity->distance_limit;
        }

        v3 closest_destination = destination;
        v3 velocity_at_closest_destination = velocity;
        SimEntity *hit_entity = NULL;

        for (u32 test_index = 0; test_index < sim_region->entity_count; test_index++)
        {
            SimEntity *test_entity = get_sim_entity(sim_region, test_index);

            if ((test_entity->storage_index == entity->storage_index) ||
                is(test_entity, ENTITY_FLAG_NONSPATIAL))
            {
                continue;
            }

            f32 minkowski_test_width  = 0.5f * (test_entity->hitbox.x + entity->hitbox.x);
            f32 minkowski_test_height = 0.5f * (test_entity->hitbox.y + entity->hitbox.y);

            v2 vertices[4] =
            {
                v2{ test_entity->position.x - minkowski_test_width, test_entity->position.y + minkowski_test_height },
                v2{ test_entity->position.x + minkowski_test_width, test_entity->position.y + minkowski_test_height },
                v2{ test_entity->position.x + minkowski_test_width, test_entity->position.y - minkowski_test_height },
                v2{ test_entity->position.x - minkowski_test_width, test_entity->position.y - minkowski_test_height },
            };

            for (i32 vertex_idx = 0; vertex_idx < ARRAY_COUNT(vertices); vertex_idx++)
            {
                v2 w0 = vertices[vertex_idx];
                v2 w1 = vertices[(vertex_idx + 1) % ARRAY_COUNT(vertices)];

                v2 wall = w1 - w0;
                v2 normal = normalized(make_vector2(-wall.y, wall.x));

                if (dot(velocity.xy, normal) < 0)
                {
                    auto res = segment_segment_intersection(w0, w1, position.xy, destination.xy);

                    if (res.found == INTERSECTION_COLLINEAR)
                    {
                        if ((length2(destination - position) < length2(closest_destination - position))
                            && is(entity, ENTITY_FLAG_COLLIDABLE)
                            && is(test_entity, ENTITY_FLAG_COLLIDABLE))
                        {
                            closest_destination = destination;
                            velocity_at_closest_destination.xy = project(velocity.xy, wall);
                        }
                    }

                    if (res.found == INTERSECTION_FOUND)
                    {
                        // @todo: What if we collide with several entities during move tries?
                        hit_entity = test_entity;

                        // @note: Update only closest point.
                        if ((length2(res.intersection - position.xy) < length2(closest_destination.xy - position.xy))
                            && is(entity, ENTITY_FLAG_COLLIDABLE)
                            && is(test_entity, ENTITY_FLAG_COLLIDABLE))
                        {
                            // @todo: Make sliding better.
                            // @hack: Step out 3*eps from the wall to allow sliding along corners.
                            closest_destination.xy = res.intersection + 4 * EPSILON * normal;
                            velocity_at_closest_destination.xy = project(velocity.xy, wall); // wall * dot(velocity, wall) / wall.length_2();
                        }
                    }
                }
            }
        }

        // @note: this have to be calculated before we change current position.
        f32 move_distance = length(closest_destination - position);
        entity->distance_limit -= move_distance;
        if (entity->distance_limit < 0)
        {
            entity->distance_limit = 0;
        }

        if (move_distance > EPSILON)
        {
            // [m] / [m/s] => [s]
            if (is_zero(velocity))
            {
                remaining_dt = 0;
            }
            else
            {
                remaining_dt -= move_distance / length(velocity);
            }
        }

        position = closest_destination;
        velocity = velocity_at_closest_destination;
        destination = position + velocity * remaining_dt;

        if (hit_entity)
        {
            // @todo: Do something with "hit_entity" and "entity", like, register hit or something.
            handle_collision(entity, hit_entity);
        }

        // How much we have left to move?
        if (length(destination - position) < EPSILON2)
        {
            break;
        }
    }

    entity->position = position;
    entity->velocity = velocity;
}

} // namespace Game

// Random
#include <time.h>
#include <stdlib.h>

#if IN_CODE_TEXTURES
#include "../data/character_1.cpp"
#include "../data/character_2.cpp"
#include "../data/character_3.cpp"
#include "../data/character_4.cpp"
#include "../data/tree_60x100.cpp"
#include "../data/shadow.cpp"
#include "../data/monster_head.cpp"
#include "../data/monster_left_arm.cpp"
#include "../data/monster_right_arm.cpp"
#include "../data/sword.cpp"
#include "../data/familiar.cpp"
#endif // IN_CODE_TEXTURES

GAME_UPDATE_AND_RENDER(Game_UpdateAndRender)
{
    using namespace Game;
    using namespace Asuka;

    ASSERT(sizeof(GameState) <= Memory->PermanentStorageSize);

    f32 dt = Input->dt;

    GameState* game_state = (GameState*)Memory->PermanentStorage;

    i32 room_width_in_tiles = 16;
    i32 room_height_in_tiles = 9;

    // ===================== INITIALIZATION ===================== //

    if (!Memory->IsInitialized)
    {
        srand((unsigned int)time(NULL));

        // @note: reserve entity slot for the null entity
        game_state->entity_count  = 1;

#if IN_CODE_TEXTURES
#else
        game_state->test_wav_file = Asuka::load_wav_file("piano2.wav");
        game_state->test_current_sound_cursor = 0;
#endif

        // load_entire_file("../resources/train-images.idx3-ubyte");

// @todo: make it load in the exe, not hot loaded dll
#if IN_CODE_TEXTURES
        game_state->tree_texture        = get_tree_60x100_png();
        game_state->shadow_texture      = get_shadow_png();
        game_state->monster_head        = get_monster_head_png();
        game_state->monster_left_arm    = get_monster_left_arm_png();
        game_state->monster_right_arm   = get_monster_right_arm_png();
        game_state->sword_texture       = get_sword_png();
        game_state->familiar_texture    = get_familiar_png();

        game_state->player_textures[0]  = get_character_1_png();
        game_state->player_textures[1]  = get_character_2_png();
        game_state->player_textures[2]  = get_character_3_png();
        game_state->player_textures[3]  = get_character_4_png();
#else
        game_state->grass_texture       = Asuka::load_png_file("grass_texture.png");
        game_state->tree_texture        = Asuka::load_png_file("tree_60x100.png");
        game_state->heart_full_texture  = Asuka::load_png_file("heart_full.png");
        game_state->heart_empty_texture = Asuka::load_png_file("heart_empty.png");
        game_state->monster_head        = Asuka::load_png_file("monster_head.png");
        game_state->monster_left_arm    = Asuka::load_png_file("monster_left_arm.png");
        game_state->monster_right_arm   = Asuka::load_png_file("monster_right_arm.png");
        game_state->familiar_texture    = Asuka::load_png_file("familiar.png");
        game_state->shadow_texture      = Asuka::load_png_file("shadow.png");
        game_state->fireball_texture    = Asuka::load_png_file("fireball.png");
        game_state->sword_texture       = Asuka::load_png_file("sword.png");

        game_state->player_textures[0]  = Asuka::load_png_file("character_1.png");
        game_state->player_textures[1]  = Asuka::load_png_file("character_2.png");
        game_state->player_textures[2]  = Asuka::load_png_file("character_3.png");
        game_state->player_textures[3]  = Asuka::load_png_file("character_4.png");
#endif // IN_CODE_TEXTURES

        memory::arena_allocator *arena  = &game_state->world_arena;
        memory::arena_allocator *temp_arena = &game_state->temp_arena;
        memory::arena_allocator *ui_arena = &game_state->ui_arena;

        initialize(
            &game_state->experimental_pool,
            Memory->CustomHeapStorage,
            Memory->CustomHeapStorageSize,
            "experimental"
        );

        initialize(
            arena,
            (u8 *) Memory->PermanentStorage + sizeof(GameState),
            (Memory->PermanentStorageSize - sizeof(GameState)) / 2,
            "world"
        );

        initialize(
            ui_arena,
            (u8 *) Memory->PermanentStorage + sizeof(GameState)
                + (Memory->PermanentStorageSize - sizeof(GameState)) / 2,
            (Memory->PermanentStorageSize - sizeof(GameState)) / 2,
            "ui"
        );

        f32 tile_side_in_meters = 1.0f;
        i32 chunk_side_in_tiles = 5;
        f32 chunk_side_in_meters = chunk_side_in_tiles * tile_side_in_meters;

        World *world = ALLOCATE_STRUCT(arena, World);
        initialize_world(world, tile_side_in_meters, chunk_side_in_meters);
        game_state->world = world;

        // ===================== WORLD GENERATION ===================== //

        i32 screen_count = 6;

        i32 screen_x = 0;
        i32 screen_y = 0;
        i32 screen_z = 0;

        i32 room_width_in_meters  = truncate_to_i32(room_width_in_tiles  * world->tile_side_in_meters);
        i32 room_height_in_meters = truncate_to_i32(room_height_in_tiles * world->tile_side_in_meters);

        enum gen_direction {
            GEN_NONE,
            GEN_UP,
            GEN_RIGHT,
            GEN_FLOOR_UP,
            GEN_FLOOR_DOWN,
            // ---------
            GEN_MAX,
        };

        gen_direction previous_choice = GEN_NONE;

        for (i32 screen_idx = 0; screen_idx < screen_count; screen_idx++) {
            gen_direction choice = GEN_NONE;

            while(true) {
                choice = (gen_direction)(rand() % GEN_MAX);

                if (choice == GEN_FLOOR_UP || choice == GEN_FLOOR_DOWN) continue;
                // if (choice == GEN_FLOOR_UP && previous_choice == GEN_FLOOR_DOWN) continue;
                // if (choice == GEN_FLOOR_DOWN && previous_choice == GEN_FLOOR_UP) continue;

                if (choice != GEN_NONE) break;
            };

            if (screen_idx + 1 == screen_count) {
                choice = GEN_NONE;
            }

            i32 max_tile_x = room_width_in_tiles / 2;
            i32 min_tile_x = -room_width_in_tiles / 2;
            i32 max_tile_y = room_height_in_tiles / 2;
            i32 min_tile_y = -room_height_in_tiles / 2;

            for (i32 tile_y = min_tile_y; tile_y <= max_tile_y; tile_y++)
            {
                for (i32 tile_x = min_tile_x; tile_x <= max_tile_x; tile_x++)
                {
                    i32 x = screen_x * room_width_in_tiles  + tile_x;
                    i32 y = screen_y * room_height_in_tiles + tile_y;
                    i32 z = screen_z;

                    Tile tile_value = TILE_FREE;

                    // bottom wall
                    if ((tile_y == min_tile_y) && (tile_x != 0)) {
                        tile_value = TILE_WALL;
                    }

                    // upper wall
                    if ((tile_y == max_tile_y) && (tile_x != 0)) {
                        tile_value = TILE_WALL;
                    }

                    // left wall
                    if ((tile_x == min_tile_x) && (tile_y != 0)) {
                        tile_value = TILE_WALL;
                    }

                    // right wall
                    if ((tile_x == max_tile_x) && (tile_y != 0)) {
                        tile_value = TILE_WALL;
                    }

                    if ((choice == GEN_FLOOR_UP || previous_choice == GEN_FLOOR_DOWN) && (tile_x == 6 && tile_y == 6)) {
                        tile_value = TILE_DOOR_UP;
                    }

                    if ((choice == GEN_FLOOR_DOWN || previous_choice == GEN_FLOOR_UP) && (tile_x == 7 && tile_y == 6)) {
                        tile_value = TILE_DOOR_DOWN;
                    }

                    if ((screen_idx == screen_count - 1) && (tile_x == room_width_in_tiles - 2) && (tile_y == room_height_in_tiles - 2)) {
                        tile_value = TILE_WIN;
                    }

                    // SetTileValue(world, x, y, z, tile_value, arena);
                    i32 chunk_x = x / chunk_side_in_tiles;
                    i32 chunk_y = y / chunk_side_in_tiles;
                    i32 chunk_z = z;

                    f32 relative_x = (x % chunk_side_in_tiles) * tile_side_in_meters; // - 0.5f * chunk_side_in_meters + 0.5f * tile_side_in_meters;
                    f32 relative_y = (y % chunk_side_in_tiles) * tile_side_in_meters; // - 0.5f * chunk_side_in_meters + 0.5f * tile_side_in_meters;

                    if (tile_value == TILE_WALL) {
                        add_wall(game_state, chunk_x, chunk_y, chunk_z, v3{ relative_x, relative_y, 0 });
                    }
                }
            }

            switch (choice) {
                case GEN_UP:
                    screen_y += 1;
                    break;
                case GEN_RIGHT:
                    screen_x += 1;
                    break;
                case GEN_FLOOR_UP:
                    screen_z += 1;
                    break;
                case GEN_FLOOR_DOWN:
                    screen_z -= 1;
                    break;
                default:
                    break;
            }

            previous_choice = choice;
        }

        // @note: Add a monster BEFORE calling set camera position so it could be brought to the high entity set before player appearing on the screen.
        add_monster(game_state, 0, 0, 0, make_vector3(2, 1, 0));
        for (int i = 0; i < 1; i++) {
            f32 x = -5.0f + i * 1.0f;
            add_familiar(game_state, 0, 0, 0, make_vector3(x, 1, 0));
        }

        // ======================== UI =======================

        game_state->game_hud = ui_allocate_scene(ui_arena);

        game_state->game_hud->root = allocate_ui_group(ui_arena);

        auto hud_child_1 = allocate_ui_shape(ui_arena);
        hud_child_1->type = UI_ELEMENT_SHAPE;
        hud_child_1->scale = make_vector2(1, 1);
        hud_child_1->position = make_vector2(50, 50);
        hud_child_1->shape.size = make_vector2(50, 50);
        hud_child_1->shape.color = Color32::White;
        hud_child_1->shape.n = 1;

        hud_child_1->on_click = []()
        {
            osOutputDebugString("CLICKED!\n");
        };

        push_child(&game_state->game_hud->root->group, hud_child_1);


        auto hud_child_2 = allocate_ui_group(ui_arena);
        hud_child_2->scale = make_vector2(20, 20);
        push_child(&game_state->game_hud->root->group, hud_child_2);

        auto hud_child_2_1 = allocate_ui_shape(ui_arena);
        hud_child_2_1->type = UI_ELEMENT_SHAPE;
        hud_child_2_1->scale = make_vector2(1, 1);
        hud_child_2_1->position = make_vector2(200, 10);
        hud_child_2_1->shape.size = make_vector2(25, 100);
        hud_child_2_1->shape.color = make_color32(0.6, 0.3, 0.8, 1.0);
        push_child(&hud_child_2->group, hud_child_2_1);

        auto hud_child_2_2 = allocate_ui_shape(ui_arena);
        hud_child_2_2->type = UI_ELEMENT_SHAPE;
        hud_child_2_2->scale = make_vector2(1, 1);
        hud_child_2_2->position = make_vector2(225, 110);
        hud_child_2_2->shape.size = make_vector2(25, 100);
        hud_child_2_2->shape.color = make_color32(0.6, 0.3, 0.8, 1.0);
        push_child(&hud_child_2->group, hud_child_2_2);

        // push_child(game_state->game_hud->root, hud_child_2);


        // auto shadow_2 = allocate_struct(arena, UiFilter);
        // shadow_2->type = UI_FILTER_SHADOW;
        // shadow_2->shadow.angle = 45;
        // shadow_2->shadow.distance = 10;

        // push_filter(hud_child_2, shadow_2);

        // auto tint_2 = allocate_struct(arena, UiFilter);
        // tint_2->type = UI_FILTER_TINT;
        // tint_2->tint.multiply = V4(0, 1, 1, 1);
        // tint_2->tint.add = V4(0.2, 0, 0, 0);

        // push_filter(hud_child_2, tint_2);

#if UI_EDITOR_ENABLED
        game_state->ui_editor = ALLOCATE_STRUCT(ui_arena, UiEditor);
        game_state->ui_editor_enabled = false;
#endif // UI_EDITOR_ENABLED

        Memory->IsInitialized = true;
    }

    World *world = game_state->world;

    f32 pixels_per_meter = 60.f; // [pixels/m]
    f32 character_speed = 2.5f; // [m/s]
    f32 character_mass = 80.0f; // [kg]

    // Engine Input

    if (GetPressCount(Input->keyboard.F1))
    {
#if UI_EDITOR_ENABLED
        TOGGLE(game_state->ui_editor_enabled);
        STATIC int i = 0;
        osOutputDebugString("i = %d\n", i++);
#endif // UI_EDITOR_ENABLED
    }

    // Game Input

    for (InputIndex ControllerIndex { 0 }; ControllerIndex < ARRAY_COUNT(Input->ControllerInputs); ControllerIndex++)
    {
        ControllerInput* ControllerInput = GetControllerInput(Input, ControllerIndex);
        PlayerRequest *request = game_state->player_for_controller + ControllerIndex.index;

        if (request->entity_index == 0)
        {
            if (GetPressCount(ControllerInput->Start))
            {
                EntityResult added_player = add_player(game_state);

                ASSERT(request->entity_index == 0);

                request->entity_index = added_player.index;
                game_state->index_of_entity_for_camera_to_follow = added_player.index;
            }
        }
        else
        {
            // ALL CONTROLLER PROCESSING FOR THIS PLAYER GOES HERE //
            v3 sword_speed {};
            if (GetPressCount(ControllerInput->X))
            {
                sword_speed += make_vector3(-1, 0, 0);
            }
            else if (GetPressCount(ControllerInput->Y))
            {
                sword_speed += make_vector3(0, 1, 0);
            }
            else if (GetPressCount(ControllerInput->A))
            {
                sword_speed += make_vector3(0, -1, 0);
            }
            else if (GetPressCount(ControllerInput->B))
            {
                sword_speed += make_vector3(1, 0, 0);
            }

            f32 input_strength  = clamp(length(ControllerInput->LeftStickEnded), 0, 1);
            v2  input_direction = normalized(ControllerInput->LeftStickEnded);

            request->player_acceleration_strength = input_strength;
            request->player_acceleration_direction.xy = input_direction;
            request->sword_velocity = normalized(sword_speed);
            request->player_jump = GetPressCount(ControllerInput->Start) > 0;
        }
    }

    Rectangle3 sim_bounds = rect3::from_min_max(make_vector3(-10, -6, -5), make_vector3(10, 6, 5)); // in meters

    initialize(&game_state->temp_arena, (u8 *) Memory->TransientStorage, Memory->TransientStorageSize, "sim_region");

    WorldPosition sim_center = game_state->camera_position;
    sim_center.offset.z = 0;

    SimRegion *sim_region = begin_simulation(game_state, &game_state->temp_arena,
        sim_center, sim_bounds);

    // ===================== RENDERING ===================== //

    // Render pink background to see pixels I didn't drew.
    // DrawRectangle(Buffer, make_vector2(0, 0), make_vector2(Buffer->Width, Buffer->Height), rgb(1.f, 0.f, 1.f));
    DrawRectangle(Buffer, make_vector2(0, 0), make_vector2(Buffer->Width, Buffer->Height), make_rgb(0.5, 0.5, 0.5));

    // Background grass
    // DrawBitmap(Buffer, { 0, 0 }, { (f32)Buffer->Width, (f32)Buffer->Height }, &game_state->grass_texture);

#if 0
    // ===================== RENDERING ENTITIES ===================== //
    VisiblePieceGroup group {};
    group.pixels_per_meter = pixels_per_meter;

    for (u32 sim_entity_index = 0; sim_entity_index < sim_region->entity_count; sim_entity_index++)
    {
        SimEntity *entity = get_sim_entity(sim_region, sim_entity_index);
        group.count = 0;

        MoveSpec spec = move_spec();

        switch (entity->type)
        {
            case ENTITY_TYPE_PLAYER:
            {
                for (u32 ControllerIndex = 0; ControllerIndex < ARRAY_COUNT(game_state->player_for_controller); ControllerIndex++)
                {
                    PlayerRequest *request = game_state->player_for_controller + ControllerIndex;
                    if (request->entity_index == entity->storage_index)
                    {
                        f32 acceleration_coefficient = 100.0f; // [m/s^2]
                        v3 gravity = make_vector3(0, 0, -9.8); // [m/s^2]

                        // @note: accelerate the guy only if he's standing on the ground.
                        if (entity->position.z < EPSILON)
                        {
                            f32 friction_coefficient = 1.5f;

                            // [m/s^2] = [m/s] * [units] * [m/s^2]
                            // @todo: why units do not add up?
                            // @note: N = nu * g []
                            v2 friction_acceleration = -entity->velocity.xy * friction_coefficient * absolute(gravity.z);

                            spec.acceleration = acceleration_coefficient * request->player_acceleration_strength * request->player_acceleration_direction;
                            spec.acceleration += make_vector3(friction_acceleration, 0);
                        }

                        if (request->player_jump)
                        {
                            // @bug @fix: Sometimes after hitting the ground strange jittering happens
                            spec.acceleration.z += 200.0f;
                            spec.jump = true;
                        }

                        spec.acceleration += gravity;

                        if (!is_zero(request->player_acceleration_direction)) {
                            if (absolute(request->player_acceleration_direction.x) > absolute(request->player_acceleration_direction.y))
                            {
                                if (request->player_acceleration_direction.x > 0)
                                {
                                    entity->face_direction = FACE_DIRECTION_RIGHT;
                                }
                                else
                                {
                                    entity->face_direction = FACE_DIRECTION_LEFT;
                                }
                            }
                            else
                            {
                                if (request->player_acceleration_direction.y > 0)
                                {
                                    entity->face_direction = FACE_DIRECTION_UP;
                                }
                                else
                                {
                                    entity->face_direction = FACE_DIRECTION_DOWN;
                                }
                            }
                        }

                        if (!is_zero(request->sword_velocity))
                        {
                            SimEntity *sword = entity->sword.ptr;
                            if (sword)
                            {
                                make_entity_spatial(sword, entity->position, entity->velocity + request->sword_velocity * 4.0f);
                                sword->position.z += 0.5f;
                                // sword->distance_limit = 3.0f; // meters
                                sword->time_limit = 1.0f; // seconds
                            }
                        }
                    }
                }

                auto *shadow_texture = &game_state->shadow_texture;
                push_asset(&group, shadow_texture, make_vector3(-0.5f, 0.85f, 0), 1.0f / (1.0f + entity->position.z));

                auto *player_texture = &game_state->player_textures[entity->face_direction];
                push_asset(&group, player_texture, make_vector3(-0.4f, 1.0f, entity->position.z));

                draw_hitpoints(entity, &group);

            }
            break;

            case ENTITY_TYPE_FAMILIAR:
            {
                SimEntity *closest_entity = NULL;
                f32 closest_distance_squared = square(7.0f); // @note: maximum following distance

                for (u32 index = 0; index < sim_region->entity_count; index++) {
                    SimEntity *test_entity = get_sim_entity(sim_region, index);

                    if (test_entity->type == ENTITY_TYPE_PLAYER) {
                        f32 distance_squared = length2(test_entity->position - entity->position);
                        if (distance_squared < closest_distance_squared) {
                            closest_distance_squared = distance_squared;
                            closest_entity = test_entity;
                        }
                    }
                }

                if (closest_entity)
                {
                    if (closest_distance_squared > square(2.0f)) {
                        f32 speed = 5;
                        v3 direction = normalized(closest_entity->position - entity->position);
                        spec.acceleration = speed * direction; // + gravity;
                    }
                }

                v3 friction = -2.0f * entity->velocity;
                spec.acceleration += friction;

                entity->tBob += dt;
                if (entity->tBob > 2 * PI) {
                    entity->tBob -= 2 * PI;
                }

                f32 a = 2.0f;
                f32 t = a * sin(3.0f * entity->tBob);
                f32 h = 2.0f / (2.0f + a + t);

                auto *shadow = &game_state->shadow_texture;
                push_asset(&group, shadow, make_vector3(-0.5f, 0.85f, 0), h);

                auto *texture = &game_state->familiar_texture;
                push_asset(&group, texture, make_vector3(-0.5f, 0.8f, 0.2f / h));
            }
            break;

            case ENTITY_TYPE_MONSTER:
            {
#if 1
                auto *head = &game_state->monster_head;
                auto *left_arm  = &game_state->monster_left_arm;
                auto *right_arm = &game_state->monster_right_arm;

                push_asset(&group, head, make_vector3(-2.5f, 2.5f, 0));
                push_asset(&group, left_arm, make_vector3(-2.0f, 2.5f, 0));
                push_asset(&group, right_arm, make_vector3(-3.0f, 2.5f, 0));

                draw_hitpoints(entity, &group);
#endif
            }
            break;

            case ENTITY_TYPE_WALL:
            {
                auto *texture = &game_state->tree_texture;
                push_asset(&group, texture, make_vector3(-0.5f, 1.6f, 0));
            }
            break;

            case ENTITY_TYPE_SWORD:
            {
                // @note: swords fly linearly, with no acceleration
                spec.acceleration = make_vector3(0, 0, 0);

                if (entity->distance_limit < EPSILON)
                {
                    make_entity_nonspatial(entity);
                }

                auto *texture = &game_state->sword_texture;
                auto *shadow_texture = &game_state->shadow_texture;

                // @todo: If I have to take into account position.z in here, therefore I should
                push_asset(&group, shadow_texture, make_vector3(-0.5, 0.85, 0), 1.0f / (1.0f + entity->position.z));
                push_asset(&group, texture, make_vector3(-0.4f, 0.2f, entity->position.z));
            }
            break;

            default:
                INVALID_CODE_PATH();
        }

        if (entity->time_limit < 0.0f)
        {
            entity->time_limit = 0;
            make_entity_nonspatial(entity);
        }
        else
        {
            entity->time_limit -= dt;
        }

        if (!is(entity, ENTITY_FLAG_NONSPATIAL))
        {
            move_entity(game_state, sim_region, entity, spec, dt);
        }

        v2 entity_position_in_pixels =
            0.5f * make_vector2(Buffer->Width, Buffer->Height) +
            make_vector2(entity->position.x, -entity->position.y) * pixels_per_meter;

        f32 x = entity_position_in_pixels.x;
        f32 y = entity_position_in_pixels.y;

        // Hitbox rectangle
        // DrawRectangle(
        //     Buffer,
        //     entity_position_in_pixels - 0.5f * entity->hitbox * pixels_per_meter,
        //     entity_position_in_pixels + 0.5f * entity->hitbox * pixels_per_meter,
        //     Color24{ 1.f, 1.f, 0.f });
        Rectangle2 hitbox_in_pixels = Rectangle2::from_min_max(
            entity_position_in_pixels - 0.5f * entity->hitbox.xy * pixels_per_meter,
            entity_position_in_pixels + 0.5f * entity->hitbox.xy * pixels_per_meter);

        // @note: this draw hitboxes
        draw_empty_rectangle_in_meters(Buffer,
            Rectangle2::from_center_dim(entity->position.xy, entity->hitbox.xy),
            2, make_rgb(1, 1, 0), entity->position.xy, pixels_per_meter);

        for (u32 asset_index = 0; asset_index < group.count; asset_index++) {
            auto *asset = &group.assets[asset_index];
            ASSERT(asset);

            v2 center = make_vector2(x, y) + asset->offset;
            if (asset->bitmap) {
                DrawBitmap(Buffer, center.x, center.y, asset->bitmap, asset->color.a);
            } else {
                v2 half_dim = 0.5f * asset->dimensions;
                DrawRectangle(Buffer, center - half_dim, center + half_dim, asset->color.rgb);
            }
        }
    }

    end_simulation(game_state, sim_region);

    StoredEntity *followed_entity = get_stored_entity(game_state, game_state->index_of_entity_for_camera_to_follow);
    if (followed_entity)
    {
        game_state->camera_position = followed_entity->world_position;
    }

#if 1
    {
        // draw_empty_rectangle_in_meters(Buffer, sim_bounds, 2, rgb(1, 1, 0), pixels_per_meter);

        // @note: debug draw one meter square in the center fo the screen
        // Rectangle2 debug_rect_to_draw = Rectangle2::from_center_dim(make_vector2(0, 0), make_vector2(1, 1));
        // draw_empty_rectangle_in_meters(Buffer, debug_rect_to_draw, 2, rgb(0, 1, 0), make_vector2(0, 0), pixels_per_meter);
    }
#endif
#endif

    // ===================== RENDERING UI ===================== //

#if 0
#if UI_EDITOR_ENABLED
    if (game_state->ui_editor_enabled)
    {
        ui_update_editor(game_state->ui_editor, game_state->game_hud, Input);

        // @note: reset hovered element in the UI, so in editor it would not stuck in hovered state.
        game_state->game_hud->hovered_element = NULL;
    }
    else
    {
        ui_update_scene(game_state->game_hud, Input);
    }
    ui_draw_scene(game_state->game_hud, Buffer);
    if (game_state->ui_editor_enabled)
    {
        ui_draw_editor(game_state->game_hud, game_state->ui_editor, Buffer);
    }
#else // UI_EDITOR_ENABLED
    ui_update_scene(game_state->game_hud, Input);
    ui_draw_scene(game_state->game_hud, Buffer);
#endif // UI_EDITOR_ENABLED
#endif
    // ===================== RENDERING MEMORY LAYOUT ===================== //

#define ASUKA_DRAW_MEMORY_LAYOUT 1
#define DRAW_WORLD_ARENA         0
#define DRAW_EXPERIMENTAL_POOL   0
#define DRAW_MALLOCATOR_MEMORY   1

#if ASUKA_DEBUG && ASUKA_DRAW_MEMORY_LAYOUT
#if DRAW_WORLD_ARENA
    {
        uint64 strip_height = 20;

        auto *allocator_to_draw = &game_state->world_arena;

        uint64 how_many_bytes_i_want_to_see = Buffer->Width * Buffer->Height;
        int64 size_per_pixel = how_many_bytes_i_want_to_see / (Buffer->Width * Buffer->Height);

        // PERSIST int *allocations[1024];
        // PERSIST u32 allocations_count = 0;

        // f32 const chance = 0.05f;
        // f32 roll = uniform_real(0, 1);
        // if (roll < chance)
        // {
        //     u32 int_count = rand() % (20 - 5) + 5;
        //     int *p = ALLOCATE_BUFFER(allocator_to_draw, int, int_count);
        //     ASSERT(p);
        //     allocations[allocations_count++] = p;
        // }
        // else
        // {
        //     if (allocations_count > 0)
        //     {
        //         f32 const dealloc_chance = 0.03f;
        //         roll = uniform_real(0, 1);
        //         if (roll < dealloc_chance)
        //         {
        //             u32 index_to_deallocate = rand() % allocations_count;
        //             void *p = allocations[index_to_deallocate];
        //             ASSERT(p);
        //             DEALLOCATE_BUFFER(allocator_to_draw, p);
        //             allocations[index_to_deallocate] = allocations[--allocations_count];
        //         }
        //     }
        // }

        {
            void* start_p = allocator_to_draw->memory;

            int64 buffer_width_in_mapped_bytes = Buffer->Width * size_per_pixel;

            for (uint64 hash_entry_index = 0 ; hash_entry_index < ARRAY_COUNT(allocator_to_draw->hash_table); hash_entry_index++)
            {
                AllocationLogEntry *entry = allocator_to_draw->hash_table + hash_entry_index;
                if (entry->pointer == NULL) continue;

                // Let's say I made an allocation within this memory arena.
                // The pointer for this allocation is
                void *allocation_pointer = entry->pointer;
                usize allocation_size = entry->size; // bytes;

                int64 start = (int64)allocation_pointer - (int64)start_p;
                int64 rest = allocation_size; // in bytes

                while (rest > 0)
                {
                    int64 x = (start - (start / buffer_width_in_mapped_bytes) * buffer_width_in_mapped_bytes) / size_per_pixel;
                    int64 y = ((start / buffer_width_in_mapped_bytes) * strip_height) / size_per_pixel;

                    int64 w;
                    if (x * size_per_pixel + rest > buffer_width_in_mapped_bytes)
                    {
                        w = buffer_width_in_mapped_bytes / size_per_pixel;
                        rest -= (buffer_width_in_mapped_bytes - x * size_per_pixel);
                        start += (buffer_width_in_mapped_bytes - x * size_per_pixel);
                    }
                    else
                    {
                        w = rest / size_per_pixel;
                        rest -= rest;
                        start += rest;
                    }

                    v2 lt = make_vector2(x, y);
                    v2 wh = make_vector2(w, strip_height);

                    Color24 color = {};
                    color.g = (f32) hash_entry_index / ARRAY_COUNT(allocator_to_draw->hash_table);
                    color.b = (f32) hash_entry_index / ARRAY_COUNT(allocator_to_draw->hash_table);

                    DrawRectangle(Buffer, lt, lt+wh, color);
                }
            }
        }
    }
#endif // DRAW_WORLD_ARENA
#if DRAW_EXPERIMENTAL_POOL
    {
        u64 how_many_bytes_i_want_to_see = MEGABYTES(1);
        u32 strip_height         = 10; // px
        u32 size_per_pixel_width = 2;  // bytes

        auto *allocator_to_draw = &game_state->experimental_pool;

        // uint64 how_many_bytes_i_want_to_see = 2 * (Buffer->Width * Buffer->Height);
        // int64 size_per_pixel = how_many_bytes_i_want_to_see / (Buffer->Width * Buffer->Height);

        PERSIST int *allocations[1024];
        PERSIST u32 allocations_count = 0;

        f32 const chance = 0.05f;
        f32 roll = uniform_real(0, 1);
        if (roll < chance)
        {
            u32 int_count = rand() % (20 - 5) + 5;
            int *p = ALLOCATE_BUFFER(allocator_to_draw, int, int_count);
            ASSERT(p);
            allocations[allocations_count++] = p;
        }
        else
        {
            if (allocations_count > 0)
            {
                f32 const dealloc_chance = 0.03f;
                roll = uniform_real(0, 1);
                if (roll < dealloc_chance)
                {
                    u32 index_to_deallocate = rand() % allocations_count;
                    void *p = allocations[index_to_deallocate];
                    ASSERT(p);
                    DEALLOCATE_BUFFER(allocator_to_draw, p);
                    allocations[index_to_deallocate] = allocations[--allocations_count];
                }
            }
        }

        {
            void* start_p = allocator_to_draw->memory;
            Color24 color = Color24::Black;

            int64 buffer_width_in_mapped_bytes = Buffer->Width * size_per_pixel_width;

            for (uint64 hash_entry_index = 0 ; hash_entry_index < ARRAY_COUNT(allocator_to_draw->hash_table); hash_entry_index++)
            {
                AllocationLogEntry *entry = allocator_to_draw->hash_table + hash_entry_index;
                if (entry->pointer == NULL) continue;

                // Let's say I made an allocation within this memory arena.
                // The pointer for this allocation is
                void *allocation_pointer = entry->pointer;
                usize allocation_size = entry->size; // bytes;

                int64 start = (int64)allocation_pointer - (int64)start_p;
                int64 rest = allocation_size; // in bytes

                while (rest > 0)
                {
                    int64 x = (start - (start / buffer_width_in_mapped_bytes) * buffer_width_in_mapped_bytes) / size_per_pixel_width;
                    int64 y = ((start / buffer_width_in_mapped_bytes) * strip_height);

                    int64 w;
                    if (x * size_per_pixel_width + rest > buffer_width_in_mapped_bytes)
                    {
                        w = buffer_width_in_mapped_bytes / size_per_pixel_width;
                        rest -= (buffer_width_in_mapped_bytes - x * size_per_pixel_width);
                        start += (buffer_width_in_mapped_bytes - x * size_per_pixel_width);
                    }
                    else
                    {
                        w = rest / size_per_pixel_width;
                        rest -= rest;
                        start += rest;
                    }

                    v2 lt = make_vector2(x, y);
                    v2 wh = make_vector2(w, strip_height);

                    Color24 random_color = {};
                    random_color.g = (f32) hash_entry_index / ARRAY_COUNT(allocator_to_draw->hash_table);
                    random_color.b = (f32) hash_entry_index / ARRAY_COUNT(allocator_to_draw->hash_table);

                    DrawRectangle(Buffer, lt, lt+wh, random_color);
                }
            }
        }

        // Draw Buckets of pool allocator
        {
            void* start_p = allocator_to_draw->memory;
            int64 buffer_width_in_mapped_bytes = Buffer->Width * size_per_pixel_width;
            u32 line_width = 1;
            Color24 color = make_rgb( 0.0, 0.2, 0.2 );

            using chunk_t = typename GameState::experimental_pool_t::memory_chunk;

            chunk_t *first_chunk = (chunk_t *) allocator_to_draw->memory;

            for (usize chunk_index = 0; chunk_index < allocator_to_draw->size / sizeof(chunk_t); chunk_index++)
            {
                chunk_t *chunk = first_chunk + chunk_index;

                void *allocation_pointer = chunk;

                int64 start = (int64)allocation_pointer - (int64)start_p;
                int64 rest = sizeof(chunk_t);

                while (rest > 0)
                {
                    int64 x = (start - (start / buffer_width_in_mapped_bytes) * buffer_width_in_mapped_bytes) / size_per_pixel_width;
                    int64 y = ((start / buffer_width_in_mapped_bytes) * strip_height);

                    int64 w;
                    if (x * size_per_pixel_width + rest > buffer_width_in_mapped_bytes)
                    {
                        w = buffer_width_in_mapped_bytes / size_per_pixel_width;
                        rest -= (buffer_width_in_mapped_bytes - x * size_per_pixel_width);
                        start += (buffer_width_in_mapped_bytes - x * size_per_pixel_width);
                    }
                    else
                    {
                        w = rest / size_per_pixel_width;
                        rest -= rest;
                        start += rest;
                    }

                    v2 tl = make_vector2(x, y);
                    v2 wh = make_vector2(w, strip_height);
                    DrawRectangle(Buffer, tl, tl + make_vector2(wh.x, line_width), color);
                    DrawRectangle(Buffer, tl + make_vector2(wh.x - line_width, 0), tl + wh, color);
                }
            }
        }
    }
#endif // DRAW_EXPERIMENTAL_POOL
#if DRAW_MALLOCATOR_MEMORY
    {
        PERSIST b32 printed = 0;
        PERSIST i32 strip_user_offset    = 0;  // in strip count
        u32 strip_height         = 2;  // px
        u32 size_per_pixel_width = 20; // bytes

        if (GetPressCount(Input->keyboard.ArrowDown) > 0)
        {
            strip_user_offset += 1;
            printed = 0;
        }
        if (GetPressCount(Input->keyboard.ArrowUp) > 0)
        {
            strip_user_offset -= 1;
            printed = 0;
        }

        // if (!printed) osOutputDebugString("strip offset: %d\n", strip_user_offset);
        printed = 1;

        auto *allocator_to_draw = &game_state->experimental_mallocator;

        PERSIST int *allocations[2048];
        PERSIST u32 allocations_count = 0;

        int *last_allocation = 0;

        f32 const chance = 0.8f;
        f32 roll = uniform_real(0, 1);

        if ((roll < chance) && (allocations_count < ARRAY_COUNT(allocations)))
        {
            u32 int_count = rand() % 290 + 10;
            int *p = ALLOCATE_BUFFER(allocator_to_draw, int, int_count);
            ASSERT(p);
            allocations[allocations_count++] = p;
            last_allocation = p;
        }
        else
        {
            if (allocations_count > 0)
            {
                u32 index_to_deallocate = rand() % allocations_count;
                void *p = allocations[index_to_deallocate];
                ASSERT(p);
                DEALLOCATE_BUFFER(allocator_to_draw, p);
                allocations[index_to_deallocate] = allocations[allocations_count - 1];
                allocations[allocations_count - 1] = 0;
                allocations_count--;
            }
        }

        if (allocations_count > 0)
        {
            if (game_state->start_p == NULL) game_state->start_p = allocations[0];
            for (uint32 i = 0; i < allocations_count; i++)
            {
                if (allocations[i] < game_state->start_p) game_state->start_p = allocations[i];
            }

            usize size = GIGABYTES(1); // Actually we don't know what it is

            int64 buffer_width_in_mapped_bytes = Buffer->Width * size_per_pixel_width;

            for (uint64 hash_entry_index = 0 ; hash_entry_index < ARRAY_COUNT(allocator_to_draw->hash_table); hash_entry_index++)
            {
                AllocationLogEntry *entry = allocator_to_draw->hash_table + hash_entry_index;
                if (entry->pointer == NULL) continue;

                // Let's say I made an allocation within this memory arena.
                // The pointer for this allocation is
                void *allocation_pointer = entry->pointer;
                usize allocation_size = entry->size; // bytes;

                int64 start = (int64)allocation_pointer - (int64)game_state->start_p;
                int64 rest = allocation_size; // in bytes

                while (rest > 0)
                {
                    int64 x = (start - (start / buffer_width_in_mapped_bytes) * buffer_width_in_mapped_bytes) / size_per_pixel_width;
                    int64 y = ((start / buffer_width_in_mapped_bytes) * strip_height);

                    y -= strip_user_offset * strip_height;

                    int64 w;
                    if (x * size_per_pixel_width + rest > buffer_width_in_mapped_bytes)
                    {
                        w = buffer_width_in_mapped_bytes / size_per_pixel_width;
                        rest -= (buffer_width_in_mapped_bytes - x * size_per_pixel_width);
                        start += (buffer_width_in_mapped_bytes - x * size_per_pixel_width);
                    }
                    else
                    {
                        w = rest / size_per_pixel_width;
                        rest -= rest;
                        start += rest;
                    }

                    v2 lt = make_vector2(x, y);
                    v2 wh = make_vector2(w, strip_height);

                    Color24 color = {};
                    color.g = (f32) hash_entry_index / ARRAY_COUNT(allocator_to_draw->hash_table);
                    color.b = (f32) hash_entry_index / ARRAY_COUNT(allocator_to_draw->hash_table);

                    if (allocation_pointer == last_allocation)
                    {
                        color = Color24::Red;
                    }

                    DrawRectangle(Buffer, lt, lt+wh, color);
                }
            }
        }
    }
#endif
#endif // ASUKA_DEBUG && ASUKA_DRAW_MEMORY_LAYOUT

    // ===================== RENDERING SIGNALING BORDERS ================= //

#if ASUKA_PLAYBACK_LOOP
    Color24 BorderColor {};
    u32 BorderWidth = 10;
    b32 BorderVisible {};

    switch (Input->PlaybackLoopState) {
        case PLAYBACK_LOOP_IDLE: {
            BorderVisible = false;
            break;
        }
        case PLAYBACK_LOOP_RECORDING: {
            BorderVisible = true;
            BorderColor = make_rgb(1.0f, 244.0f / 255.0f, 43.0f / 255.0f);
            break;
        }
        case PLAYBACK_LOOP_PLAYBACK: {
            BorderVisible = true;
            BorderColor = make_rgb(29.0f / 255.0f, 166.0f / 255.0f, 8.0f / 255.0f);
            break;
        }
    }

    if (BorderVisible) {
        DrawBorder(Buffer, BorderWidth, BorderColor);
    }
#if UI_EDITOR_ENABLED
    else if (game_state->ui_editor_enabled)
    {
        DrawBorder(Buffer, 2, make_color24(0, 0, 0));
    }
#endif // UI_EDITOR_ENABLED

#endif // ASUKA_PLAYBACK_LOOP

#if 0
    [](auto Buffer)
    {
        for (u32 i = 0; i < 100000; i++)
        {
            f32 r = uniform_real(0, 1);
            f32 phi = uniform_real(0, 1) * 2 * PI;

            i32 PixelX = (int)(sqrt(r) * cos(phi) * (Buffer->Height / 2 - 1) + Buffer->Width / 2);
            i32 PixelY = (int)(sqrt(r) * sin(phi) * (Buffer->Height / 2 - 1) + Buffer->Height / 2);

            // if ((PixelX >= 0 && PixelX < Buffer->Width) &&
            //     (PixelY >= 0 && PixelY < Buffer->Height))
            {
                auto Pixel = (u8 *) Buffer->Memory + PixelY * Buffer->Pitch + PixelX * Buffer->BytesPerPixel;
                *(u32 *)(Pixel) = 0;
            }
            // else
            {
                // osOutputDebugString("(%d, %d)\n", PixelX, PixelY);
            }
        }
    }(Buffer);
#endif
}


namespace Game
{

INTERNAL
void Game_OutputSound_(SoundOutputBuffer *SoundBuffer, GameState* game_state) {
#if IN_CODE_TEXTURES
#else
    sound_sample_t* SampleOut = SoundBuffer->Samples;

    for (i32 SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; SampleIndex++) {
        u64 left_idx = (game_state->test_current_sound_cursor++) % game_state->test_wav_file.samples_count;
        u64 right_idx = (game_state->test_current_sound_cursor++) % game_state->test_wav_file.samples_count;

        sound_sample_t LeftSample =  game_state->test_wav_file.samples[left_idx];
        sound_sample_t RightSample = game_state->test_wav_file.samples[right_idx];

        // f32 volume = 0.05f;
        f32 volume = 0;

        *SampleOut++ = (sound_sample_t)(LeftSample  * volume);
        *SampleOut++ = (sound_sample_t)(RightSample * volume);
    }
#endif // IN_CODE_TEXTURES
}

}


// Game_OutputSound(ThreadContext *Thread, Game::Memory *Memory, Game::SoundOutputBuffer* SoundBuffer)
GAME_OUTPUT_SOUND(Game_OutputSound)
{
    Game::GameState* game_state = (Game::GameState*)Memory->PermanentStorage;
    Game_OutputSound_(SoundBuffer, game_state);
}
