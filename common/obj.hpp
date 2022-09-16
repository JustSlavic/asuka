#pragma once

#include <math/vector3.hpp>
#include <array.hpp>


struct loaded_obj
{
    array<vector3> vertices;
    array<vector3> normals;
    array<vector2> uvs;
    array<vector3i> faces;
};


struct Mesh
{
    array<vector3> vertices;
    array<vector2> texture_uvs;
    array<uint32>  indices;
};


[[nodiscard]]
Mesh load_wavefront_obj(byte_array contents, memory::mallocator *allocator)
{
    auto vertices = allocate_array<vector3>(allocator, 50);
    auto normals = allocate_array<vector3>(allocator, 50);
    auto uvs = allocate_array<vector2>(allocator, 50);

    auto result_vertices = allocate_array<vector3>(allocator, 10);
    auto result_uvs = allocate_array<vector2>(allocator, 10);
    auto result_indices = allocate_array<uint32>(allocator, 20);

    uint32 index = 0;

    while (index < contents.size)
    {
        auto byte = contents[index];
        if (byte == '#')
        {
            // Skip comment
            do
            {
                byte = contents[index++];
            }
            while (byte != '\n');
        }
        else if (byte == 'v')
        {
            index++; // eat 'v'
            byte = contents[index++]; // eat next symbol

            if (byte == 'n')
            {
                int n;
                float x, y, z;
                sscanf((char *) contents.get_data() + index, " %f %f %f%n", &x, &y, &z, &n);
                index += n;

                normals.push(make_vector3(x, y, z));
            }
            else if (byte == 't')
            {
                int n;
                float u, v;
                sscanf((char *) contents.get_data() + index, " %f %f%n", &u, &v, &n);
                index += n;

                uvs.push(make_vector2(u, v));
            }
            else if (byte == ' ')
            {
                int n;
                float x, y, z;
                sscanf((char *) contents.get_data() + index, " %f %f %f%n", &x, &y, &z, &n);
                index += n;

                vertices.push(make_vector3(x, y, z));
            }
            else
            {
                // Skip unrecognized line
                do
                {
                    byte = contents[index++];
                }
                while (byte != '\n');
            }
        }
        else if (byte == 'f')
        {
            index++;

            int n;
            int v1, v2, v3, v4;
            int t1, t2, t3, t4;
            int n1, n2, n3, n4;

            sscanf((char *) contents.data + index, " %d/%d/%d %d/%d/%d %d/%d/%d%n",
                &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3, &n);

            index += n;

            result_vertices.push(vertices[v1 - 1]);
            result_vertices.push(vertices[v2 - 1]);
            result_vertices.push(vertices[v3 - 1]);

            result_uvs.push(uvs[t1 - 1]);
            result_uvs.push(uvs[t2 - 1]);
            result_uvs.push(uvs[t3 - 1]);

            result_indices.push((uint32) result_vertices.size - 3);
            result_indices.push((uint32) result_vertices.size - 2);
            result_indices.push((uint32) result_vertices.size - 1);

            byte = contents[index];
            if (byte == ' ')
            {
                sscanf((char *) contents.data + index, " %d/%d/%d%n",
                    &v4, &t4, &n4, &n);
                index += n;

                result_vertices.push(vertices[v4 - 1]);
                result_uvs.push(uvs[t4 - 1]);

                result_indices.push((uint32) result_vertices.size - 2);
                result_indices.push((uint32) result_vertices.size - 1);
                result_indices.push((uint32) result_vertices.size - 4);
            }
        }
        else
        {
            // Skip unrecognized line
            do
            {
                byte = contents[index++];
            }
            while (byte != '\n');
        }
    }

    deallocate_array(vertices);
    deallocate_array(normals);
    deallocate_array(uvs);

    Mesh result = {};
    result.vertices = result_vertices;
    result.texture_uvs = result_uvs;
    result.indices = result_indices;

    return result;
}
