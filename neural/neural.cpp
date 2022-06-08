#include <defines.hpp>
#include <math/float.hpp>
#include <allocator.hpp>

#include <os/file.hpp>
#include <math/integer.hpp>

#include <windows.h>

// sprintf for osOutputDebugString
#include <stdio.h>

// #include <stdlib.h>
// #include <math.h>
// #include <time.h>


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

    u32 sample_size;  // size of one sample
    u32 sample_count; // number of samples in the batch
};


DataBatch allocate_data_batch(Asuka::memory::arena_allocator *arena, u32 sample_size, u32 sample_count)
{
    DataBatch result = {};
    result.samples = allocate_array_(arena, f32, sample_size * sample_count);
    result.sample_size  = sample_size;
    result.sample_count = sample_count;

    return result;
}


void print_data_batch(DataBatch data)
{
    osOutputDebugString("DataBatch:\n");
    for (u32 batch_index = 0; batch_index < data.sample_count; batch_index++)
    {
        osOutputDebugString("[");
        for (u32 i = 0; i < data.sample_size; i++)
        {
            osOutputDebugString("%6.3f ", data.samples[batch_index * data.sample_size + i]);
        }
        osOutputDebugString("]\n");
    }
}

// Lost the meaning! Where is the result?!
void softmax(DataBatch data)
{
    for (u32 batch_index = 0; batch_index < data.sample_count; batch_index++)
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


struct Layer
{
    f32 *weights;
    f32 *biases;

    u32 inputs_count;
    u32 neuron_count;

    activation_function *activate;
};


Layer allocate_layer(Asuka::memory::arena_allocator *arena, u32 inputs_count, u32 neuron_count, activation_function *f)
{
    Layer layer = {};
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

void forward(Layer layer, DataBatch input, DataBatch output)
{
    ASSERT(layer.inputs_count == input.sample_size);
    ASSERT(layer.neuron_count == output.sample_size);

    for (u32 batch_index = 0; batch_index < input.sample_count; batch_index++)
    {
        f32 *samples = input.samples + batch_index * input.sample_size;
        for (u32 neuron_index = 0; neuron_index < layer.neuron_count; neuron_index++)
        {
            f32 *weights = layer.weights + neuron_index * layer.inputs_count;
            f32 bias = layer.biases[neuron_index];

            f32 out_value = dot(samples, weights, input.sample_size) + bias;
            output.samples[batch_index * output.sample_size + neuron_index] = layer.activate(out_value);
        }
    }
}


struct Network
{
    Layer *layers;
    u32 layer_count;

    DataBatch *batches;
    u32 batch_count;
};


//
// Allocates neural network with all data batches necessary for forward pass.
// @note: layer_count does not include "input-layer", only deep layers and output-layer.
//     Layer1 DataBatch1 Layer2 DataBatch2 ... LayerN DataBatchN
// Inputs data batch should be allocated by the user for now.
//
// Example:
//     Network network = allocate_network(arena, relu_activation, 2, inputs.sample_count, 2, 2);
//     network.forward(inputs);
//
Network allocate_network(Asuka::memory::arena_allocator *arena, activation_function *activation, u32 sample_count, u32 layer_count, ...)
{
    Network network = {};
    network.layers = allocate_array(arena, Layer, layer_count);
    network.layer_count = layer_count;
    network.batches = allocate_array(arena, DataBatch, layer_count);
    network.batch_count = layer_count;

    va_list args;
    va_start(args, layer_count);

    u32 sample_size  = va_arg(args, u32);
    u32 neuron_count = va_arg(args, u32);

    Layer *layer = network.layers;
    DataBatch *batch = network.batches;
    for (u32 layer_index = 0; layer_index < layer_count; layer_index++)
    {
        *layer = allocate_layer(arena, sample_size, neuron_count, activation);
        (*batch).samples = allocate_array_(arena, f32, neuron_count * sample_count);
        (*batch).sample_size  = layer->neuron_count;
        (*batch).sample_count = sample_count;

        sample_size = neuron_count;
        neuron_count = va_arg(args, u32);

        layer += 1;
        batch += 1;
    }

    va_end(args);

    return network;
}


DataBatch forward(Network network, DataBatch input)
{
    u32 output_batch_index = 0;
    for (u32 layer_index = 0;
         layer_index < network.layer_count;
         layer_index += 1)
    {
        forward(network.layers[layer_index], input, network.batches[output_batch_index]);
        output_batch_index += 1;
    }

    return network.batches[network.batch_count - 1];
}


struct InputData
{
    f32 *samples;
    u32 *labels;

    u32 sample_count;
    u32 sample_size;

    u32 n_classes;
};


InputData generate_spiral_data(Asuka::memory::arena_allocator *arena, u32 point_count, u32 n_classes)
{
    u32 sample_size = 2; // 2 floats per sample

    InputData data;
    data.samples = allocate_array(arena, f32, sample_size * point_count * n_classes);
    data.labels  = allocate_array(arena, u32, point_count * n_classes);
    data.sample_count = point_count * n_classes;
    data.sample_size  = sample_size;
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


f32 calculate_categorical_crossentropy_loss(InputData input, DataBatch data)
{
    ASSERT(input.sample_count == data.sample_count);

    f32 sum = 0.0f;
    for (u32 batch_index = 0; batch_index < data.sample_count; batch_index++)
    {
        u32 index_of_correct_answer = input.labels[batch_index];
        f32 confidence = data.samples[batch_index * data.sample_size + index_of_correct_answer];
        sum += -logf(confidence);
    }

    // mean(-ln(correct_confidence))
    f32 loss = sum / data.sample_count;
    return loss;
}


f32 calculate_accuracy(InputData input, DataBatch batch)
{
    f32 sum = 0;

    for (u32 batch_index = 0; batch_index < batch.sample_count; batch_index++)
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

    return sum / batch.sample_count;
}


Asuka::byte_array load_mnist_db(char const *images_filename)
{
    Asuka::byte_array contents = Asuka::os::load_entire_file(images_filename);
    if (contents.data == NULL)
    {
        return contents;
    }

    Asuka::memory::byte *bytes = contents.data;

    auto magic_number = change_endiannes(*((UInt32 *)bytes));
    bytes += sizeof(UInt32);

    if (magic_number != 0x00000803) {
        // @error: free memory, report error, exit function
        return contents;
    }

    auto number_of_images = change_endiannes(*((Int32 *)bytes));
    bytes += sizeof(Int32);
    auto number_of_rows = change_endiannes(*((Int32 *) bytes));
    bytes += sizeof(Int32);
    auto number_of_columns = change_endiannes(*((Int32 *) bytes));
    bytes += sizeof(Int32);

    for (Int32 image_index = 0; image_index < number_of_images; image_index++)
    {
        for (Int32 row = 0; row < number_of_rows; row++)
        {
            for (Int32 column = 0; column < number_of_columns; column++)
            {
                auto pixel = *bytes;
                osOutputDebugString("%s", (pixel > (255 / 2)) ? "%" : " ");
                bytes += 1;
            }
            osOutputDebugString("\n");
        }

        osOutputDebugString("\n");

        if (image_index > 5) break;
    }

    return contents;
}


int main()
{
    memory::os_allocator os_allocator;
    initialize(&os_allocator, NULL, 0);

    // load_mnist_db("train-images.idx3-ubyte");

    // srand((u32)time(0));

    usize memory_size = MEGABYTES(1);
    void *memory = VirtualAlloc((LPVOID)TERABYTES(2), memory_size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    memory::arena arena_;
    memory::arena *arena = &arena_;
    memory::initialize(arena, memory, memory_size);

    // InputData input = generate_spiral_data(arena, 100, 3);
    f32 input_samples[] = {
        0, 0,
        0, 1,
        1, 0,
        1, 1,
    };
    u32 desired_outputs[] = {
        1,
        0,
        0,
        1,
    };

    // The network architecture:
    u32 sample_size   = 2; // How many inputs the first layer has
    u32 sample_count  = 4; // How many samples are in the data batch
    u32 neuron1_count = 2;
    u32 neuron2_count = 2; // Number of neurons in the output layer

    InputData input {};
    input.samples = input_samples;
    input.labels = desired_outputs;
    input.sample_count = sample_count; // division to account for the 2 floats per sample
    input.sample_size  = sample_size;
    input.n_classes = 2; // possible outputs (labels) 0 and 1


    // Input data batch
    DataBatch data_1 = {};
    data_1.samples = input.samples;
    data_1.sample_size = input.sample_size;
    data_1.sample_count = input.sample_count;


    // Layer 1
    Layer layer1 = allocate_layer(arena, input.sample_size, neuron1_count, relu_activation);
    DataBatch data_1_2 = allocate_data_batch(arena, neuron1_count, sample_count);

    // Layer 2
    Layer layer2 = allocate_layer(arena, neuron1_count, neuron2_count, linear_activation);
    DataBatch output = allocate_data_batch(arena, neuron2_count, sample_count);

    // Forward pass
    forward(layer1, data_1, data_1_2);
    forward(layer2, data_1_2, output);
    softmax(output);

    print_data_batch(output);
    f32 loss = calculate_categorical_crossentropy_loss(input, output);

    osOutputDebugString("Loss = %5.2f\n", loss);

    f32 accuracy = calculate_accuracy(input, output);

    osOutputDebugString("Accuracy = %5.1f%%\n", accuracy * 100);

    return 0;
}
