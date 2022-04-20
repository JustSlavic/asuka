#define INVALID_CHUNK_POSITION INT32_MAX


void initialize_world(World *world, f32 tile_side_in_meters, f32 chunk_side_in_meters)
{
    memory::set(world, 0, sizeof(World));

    world->tile_side_in_meters = tile_side_in_meters;
    world->chunk_side_in_meters = chunk_side_in_meters;
}

WorldPosition world_origin()
{
    WorldPosition result {};
    return result;
}

WorldPosition null_position()
{
    WorldPosition result {};
    result.chunk_x = INVALID_CHUNK_POSITION;

    return result;
}

WorldPosition world_position(World *world, i32 chunk_x, i32 chunk_y, i32 chunk_z, v2 offset)
{
    WorldPosition result;
    result.chunk_x = chunk_x;
    result.chunk_y = chunk_y;
    result.chunk_z = chunk_z;
    result.offset  = offset;

    result = canonicalize_position(world, result);

    return result;
}

b32 is_valid(WorldPosition p)
{
    b32 result = (p.chunk_x != INVALID_CHUNK_POSITION) && (p.chunk_y != INVALID_CHUNK_POSITION) && (p.chunk_z != INVALID_CHUNK_POSITION);
    return result;
}

INTERNAL_FUNCTION INLINE
Chunk* get_chunk(World* world, i32 chunk_x, i32 chunk_y, i32 chunk_z, memory::arena_allocator *arena = NULL)
{
    // @todo: MAKE BETTER HASH FUNCTION!!!
    hash_t hash = chunk_x * 3 + chunk_y * 13 + chunk_z * 53;
    u32 index = hash & (ARRAY_COUNT(world->chunks_hash_table) - 1);
    ASSERT(index < ARRAY_COUNT(world->chunks_hash_table));

    Chunk *chunk = world->chunks_hash_table + index;
    ASSERT(chunk);

    while (chunk) {
        // If initialized and coordinates match up, break from the loop.
        if ((chunk->entities == NULL) ||
            (chunk->chunk_x == chunk_x &&
             chunk->chunk_y == chunk_y &&
             chunk->chunk_z == chunk_z))
        {
            break;
        }

        if (arena && chunk->next_in_hashtable == NULL)
        {
            osOutputDebugString("Allocate Chunk at (%d, %d, %d)\n", chunk_x, chunk_y, chunk_z);

            chunk->next_in_hashtable = push_struct(arena, Chunk);
        }

        chunk = chunk->next_in_hashtable;
    }

    if (arena && chunk && (chunk->entities == NULL)) {
        chunk->chunk_x = chunk_x;
        chunk->chunk_y = chunk_y;
        chunk->chunk_z = chunk_z;

        chunk->entities = push_struct(arena, EntityBlock);

        chunk->next_in_hashtable = 0;
    }

    ASSERT(chunk);
    return chunk;
}


INLINE INTERNAL_FUNCTION
Chunk *get_chunk(World *world, WorldPosition position, memory::arena_allocator *arena = NULL) {
    Chunk *result = get_chunk(world, position.chunk_x, position.chunk_y, position.chunk_z, arena);
    return result;
}


v2 position_difference(World *world, WorldPosition p1, WorldPosition p2) {
    v2 result {};

    if (p1.chunk_z != p2.chunk_z) {
        return result;
    }

    result.x =
        (p1.chunk_x * world->chunk_side_in_meters + p1.offset.x) -
        (p2.chunk_x * world->chunk_side_in_meters + p2.offset.x);
    result.y =
        (p1.chunk_y * world->chunk_side_in_meters + p1.offset.y) -
        (p2.chunk_y * world->chunk_side_in_meters + p2.offset.y);

    return result;
}


b32 in_same_chunk(WorldPosition p1, WorldPosition p2)
{
    b32 result = ((p1.chunk_x == p2.chunk_x) &&
                  (p1.chunk_y == p2.chunk_y) &&
                  (p1.chunk_z == p2.chunk_z));

    return result;
}


INTERNAL_FUNCTION
EntityBlock *add_entity_block_to_chunk(World *world, Chunk *chunk, memory::arena_allocator *arena) {
    EntityBlock *block = chunk->entities;

    if (world->next_free_block) {
        osOutputDebugString("Reused free entity block from world.\n");

        EntityBlock *next_free_block = world->next_free_block;
        world->next_free_block = world->next_free_block->next_block;

        chunk->entities = next_free_block;
    } else {
        osOutputDebugString("Allocated entity block from memory arena.\n");

        chunk->entities = push_struct(arena, EntityBlock);
    }

    chunk->entities->next_block = block;
    return chunk->entities;
}


INTERNAL_FUNCTION
void push_entity_into_chunk(World *world, Chunk *chunk, u32 low_index, memory::arena_allocator *arena) {
    EntityBlock *block = chunk->entities;

    if ((block == NULL) ||
        (block->entity_count == ARRAY_COUNT(block->entities)))
    {
        block = add_entity_block_to_chunk(world, chunk, arena);
    }

    block->entities[block->entity_count++] = low_index;
    osOutputDebugString("Pushed Entity #%d to (%d %d %d)\n", low_index, chunk->chunk_x, chunk->chunk_y, chunk->chunk_z);
}


INTERNAL_FUNCTION
void remove_entity_from_chunk(World *world, Chunk *chunk, u32 storage_index) {
    EntityBlock *first_block = chunk->entities;

    for (EntityBlock *block = chunk->entities; block; block = block->next_block) {
        for (u32 idx = 0; idx < block->entity_count; idx++) {
            if (block->entities[idx] == storage_index) {
                // Found entity to remove!
                osOutputDebugString("Removed Entity #%d\n", storage_index);

                block->entities[idx] = first_block->entities[first_block->entity_count - 1];
                first_block->entity_count -= 1;

                if (first_block->entity_count == 0) {
                    // Free block.
                    chunk->entities = first_block->next_block;

                    first_block->next_block = world->next_free_block;
                    world->next_free_block = first_block;
                    memory::set(first_block, 0, sizeof(EntityBlock));
                }

                return;
            }
        }
    }
}


void sanity_check(World *world, u32 storage_index, u32 expected)
{
    u32 count = 0;

    u32 first_hash_entry_index = 0;
    Chunk *first_chunk = 0;
    EntityBlock *first_block = 0;
    u32 first_idx = 0;

    for (u32 hash_entry_index = 0; hash_entry_index < ARRAY_COUNT(world->chunks_hash_table); hash_entry_index++)
    {
        for (Chunk *chunk = world->chunks_hash_table + hash_entry_index; chunk; chunk = chunk->next_in_hashtable)
        {
            for (EntityBlock *block = chunk->entities; block; block = block->next_block)
            {
                for (u32 idx = 0; idx < block->entity_count; idx++)
                {
                    u32 entity_index = block->entities[idx];
                    if (entity_index == storage_index)
                    {
                        if (count == 0) {
                            first_hash_entry_index = hash_entry_index;
                            first_chunk = chunk;
                            first_block = block;
                            first_idx = idx;
                        }

                        if (count > 0)
                        {
                            osOutputDebugString("Found duplicate\n");
                        }

                        count += 1;
                    }
                }
            }
        }
    }

    ASSERT(count == expected);
}


//
// @note: if StoredEntity pointer is null, remove entity with this storage index from sparse storage.
//
void change_entity_location_internal(World *world, u32 index, StoredEntity *entity, WorldPosition *new_position, memory::arena_allocator *arena)
{
    if ((new_position && in_same_chunk(entity->world_position, *new_position)) ||
        (!new_position && !is_valid(entity->world_position)))
    {
        // Leave entity where it is.
    }
    else
    {
        if (is_valid(entity->world_position))
        {
            Chunk *old_chunk = get_chunk(world, entity->world_position, arena);
            if (entity->sim.type == ENTITY_TYPE_WALL)
            {
                ASSERT_FAIL(Debug break this);
            }

            remove_entity_from_chunk(world, old_chunk, index);

#if ASUKA_DEBUG
            sanity_check(world, index, 0);
#endif // ASUKA_DEBUG
        }

        if (new_position && is_valid(*new_position))
        {
            Chunk *chunk = get_chunk(world, *new_position, arena);
            push_entity_into_chunk(world, chunk, index, arena);

#if ASUKA_DEBUG
            sanity_check(world, index, (u32) ((new_position != 0) && (is_valid(*new_position))));
#endif // ASUKA_DEBUG
        } else {
            push_entity_into_chunk(world, &world->void_chunk, index, arena);
        }
    }
}


void change_entity_location(World *world, u32 storage_index, StoredEntity *entity, WorldPosition *new_position, memory::arena_allocator *arena)
{
    if (entity)
    {
        change_entity_location_internal(world, storage_index, entity, new_position, arena);
        if (new_position && is_valid(*new_position))
        {
            entity->world_position = *new_position;
            unset(&entity->sim, ENTITY_FLAG_NONSPATIAL);
        }
        else
        {
            entity->world_position = null_position();
            set(&entity->sim, ENTITY_FLAG_NONSPATIAL);
        }
    }
}


b32 is_canonical(World *world, v2 p)
{
    b32 result = (p.x >= -0.5f * world->chunk_side_in_meters) &&
                 (p.x <=  0.5f * world->chunk_side_in_meters) &&
                 (p.y >= -0.5f * world->chunk_side_in_meters) &&
                 (p.y <=  0.5f * world->chunk_side_in_meters);

    return result;
}


b32 is_canonical(World *world, WorldPosition p) {
    b32 result = is_canonical(world, p.offset);

    return result;
}


WorldPosition canonicalize_position(World *world, WorldPosition p) {
    WorldPosition result = p;

    v2i chunk_offset = round_to_v2i(p.offset / world->chunk_side_in_meters);

    result.chunk_x += chunk_offset.x;
    result.chunk_y += chunk_offset.y;
    result.offset -= upcast_to_v2(chunk_offset) * world->chunk_side_in_meters;

    ASSERT(is_canonical(world, result.offset));
    return result;
}


WorldPosition map_into_world_space(World *world, WorldPosition base_position, v2 offset)
{
    WorldPosition result = base_position;
    result.offset += offset;

    result = canonicalize_position(world, result);
    return result;
}
