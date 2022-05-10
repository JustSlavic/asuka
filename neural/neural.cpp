#include <defines.hpp>
#include <math/float.hpp>
#include <memory_arena.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


f32 uniform_real(f32 from, f32 to)
{
    ASSERT(to > from);

    f32 r = (f32)rand()/(f32)(RAND_MAX/to);; // Uniform [0, 1]
    return r + from;
}


f32 linear_activation(f32 x)
{
    return x;
}

f32 relu_activation(f32 x)
{
    if (x < 0) { x = 0; }
    return x;
}


typedef f32 (activation_function)(f32);


struct DataBatch
{
    f32 *samples;

    u32 sample_size;
    u32 batch_count;
};


void print_data_batch(DataBatch data)
{
    printf("DataBatch:\n");
    for (u32 batch_index = 0; batch_index < data.batch_count; batch_index++)
    {
        printf("[");
        for (u32 i = 0; i < data.sample_size; i++)
        {
            printf("%6.3f ", data.samples[batch_index * data.sample_size + i]);
        }
        printf("]\n");
    }
}


void softmax(DataBatch data)
{
    for (u32 batch_index = 0; batch_index < data.batch_count; batch_index++)
    {
        f32 *sample = data.samples + batch_index * data.sample_size;

        f32 max_ = 0.0f;
        for (u32 i = 0; i < data.sample_size; i++)
        {
            if (sample[i] > max_) max_ = sample[i];
        }

        f32 sum = 0.0f;
        for (u32 i = 0; i < data.sample_size; i++)
        {
            sample[i] = expf(sample[i] - max_);
            sum += sample[i];
        }
        for (u32 i = 0; i < data.sample_size; i++)
        {
            sample[i] /= sum;
        }
    }
}


struct LayerDense
{
    f32 *weights;
    f32 *biases;

    u32 inputs_count;
    u32 neuron_count;

    activation_function *activate;
};


LayerDense allocate_layer(memory::arena_allocator *arena, u32 inputs_count, u32 neuron_count, activation_function *f)
{
    LayerDense layer = {};
    layer.weights = allocate_array_(arena, f32, inputs_count * neuron_count);

    for (u32 i = 0; i < inputs_count * neuron_count; i++)
    {
        layer.weights[i] = uniform_real(-0.1f, 0.1f); // Uniform between -0.1 and 0.1
    }

    layer.biases  = allocate_array(arena, f32, neuron_count);
    layer.inputs_count = inputs_count;
    layer.neuron_count = neuron_count;
    layer.activate = f;
    return layer;
}

f32 dot(f32 *a, f32 *b, u32 size)
{
    f32 result = 0;
    for (u32 i = 0; i < size; i++)
    {
        result += a[i] * b[i];
    }

    return result;
}

void forward(LayerDense layer, DataBatch input, DataBatch output)
{
    ASSERT(layer.inputs_count == input.sample_size);
    ASSERT(layer.neuron_count == output.sample_size);

    for (u32 batch_index = 0; batch_index < input.batch_count; batch_index++)
    {
        for (u32 neuron_index = 0; neuron_index < layer.neuron_count; neuron_index++)
        {
            f32 out_value = dot(input.samples + batch_index * input.sample_size, layer.weights + neuron_index * layer.inputs_count, input.sample_size) + layer.biases[neuron_index];
            output.samples[batch_index * output.sample_size + neuron_index] = layer.activate(out_value);
        }
    }
}


struct Network
{
    LayerDense *layers;
    u32 layer_count;
};


Network allocate_network(memory::arena_allocator *arena, u32 layer_count, ...)
{
    Network network = {};
    network.layers = allocate_array(arena, LayerDense, layer_count);
    network.layer_count = layer_count;

    va_list args;
    va_start(args, layer_count);

    u32 inputs = va_arg(args, u32);
    u32 neurons = inputs;
    printf("Input layer: %d inputs\n", inputs);

    for (u32 layer_index = 1; layer_index < layer_count; layer_index++)
    {
        LayerDense *layer = network.layers + layer_index;
        inputs = neurons;
        neurons = va_arg(args, u32);
        printf("Layer %d: %d inputs => %d neurons;\n", layer_index, inputs, neurons);
    }

    va_end(args);

    return network;
}


struct SpiralData
{
    f32 *samples;
    u32 *labels;

    u32 sample_count;
    u32 sample_size;
    u32 label_count;

    u32 n_classes;
};


SpiralData generate_spiral_data(memory::arena_allocator *arena, u32 point_count, u32 n_classes)
{
    u32 sample_size = 2; // 2 floats per sample

    SpiralData data;
    data.samples = allocate_array(arena, f32, sample_size * point_count * n_classes);
    data.labels  = allocate_array(arena, u32, point_count * n_classes);
    data.sample_count = point_count * n_classes;
    data.sample_size  = sample_size;
    data.label_count  = point_count;
    data.n_classes    = n_classes;

    f32 *point = data.samples;
    u32 *label = data.labels;
    for (u32 class_index = 0; class_index < n_classes; class_index++)
    {
        for (u32 point_index = 0; point_index < point_count; point_index++)
        {
            f32 r = (f32) point_index / (f32) point_count;
            f32 t = (f32) class_index * 4.0f + (4.0f * r);

            *point++ = r * cosf(t * 2.5f) + uniform_real(-0.15f, 0.15f);
            *point++ = r * sinf(t * 2.5f) + uniform_real(-0.15f, 0.15f);
            *label++ = class_index;
        }
    }

    return data;
}


f32 calculate_categorical_crossentropy_loss(SpiralData input, DataBatch data)
{
    ASSERT(input.sample_count == data.batch_count);

    f32 sum = 0.0f;
    for (u32 batch_index = 0; batch_index < data.batch_count; batch_index++)
    {
        u32 index_of_correct_answer = input.labels[batch_index];
        f32 confidence = data.samples[batch_index * data.sample_size + index_of_correct_answer];
        sum += -logf(confidence);
    }

    // mean(-ln(correct_confidence))
    f32 loss = sum / data.batch_count;
    return loss;
}


f32 calculate_accuracy(SpiralData input, DataBatch batch)
{
    f32 sum = 0;

    for (u32 batch_index = 0; batch_index < batch.batch_count; batch_index++)
    {
        u32 max_index = 0;
        f32 max_value = -INF;
        f32 *sample = batch.samples + batch_index * batch.sample_size;

        for (u32 i = 0; i < batch.sample_size; i++)
        {
            if (sample[i] > max_value)
            {
                max_value = sample[i];
                max_index = i;
            }
        }

        if (max_index == input.labels[batch_index])
        {
            sum += 1;
        }
    }

    return sum / batch.batch_count;
}


int main()
{
    srand((u32)time(0));

    u32 memory_size = MEGABYTES(3);
    void *memory = malloc(memory_size);
    memory::arena_allocator arena_;
    memory::arena_allocator *arena = &arena_;
    memory::initialize_arena(arena, memory, memory_size);

    // allocate_network(arena, 3, 40, 50, 3);

    SpiralData input = generate_spiral_data(arena, 100, 3);

    u32 neuron1_count = 3;
    u32 neuron2_count = input.n_classes;

    // Input data batch
    DataBatch data_1 = {};
    data_1.samples = input.samples;
    data_1.sample_size = input.sample_size;
    data_1.batch_count = input.sample_count;

    // Layer 1
    LayerDense layer1 = allocate_layer(arena, input.sample_size, neuron1_count, relu_activation);

    // Intermediate data batch
    DataBatch data_1_2 = {};
    data_1_2.samples = allocate_array_(arena, f32, neuron1_count * input.sample_count);
    data_1_2.sample_size = layer1.neuron_count;
    data_1_2.batch_count = input.sample_count;

    // Layer 2
    LayerDense layer2 = allocate_layer(arena, neuron1_count, neuron2_count, linear_activation);

    // Output data batch
    DataBatch data_2 = {};
    data_2.samples = allocate_array_(arena, f32, layer2.neuron_count * input.sample_count);
    data_2.sample_size = layer2.neuron_count;
    data_2.batch_count = input.sample_count;

    // Forward pass
    forward(layer1, data_1, data_1_2);
    forward(layer2, data_1_2, data_2);
    softmax(data_2);

    print_data_batch(data_2);
    f32 loss = calculate_categorical_crossentropy_loss(input, data_2);

    printf("Loss = %5.2f\n", loss);

    f32 accuracy = calculate_accuracy(input, data_2);

    printf("Accuracy = %5.1f%%\n", accuracy * 100);

    return 0;
}
