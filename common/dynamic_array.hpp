#pragma once


template <typename T>
struct dynamic_array
{
    using value_t = T; // For arrays
    using char_t  = T; // For strings
    using allocator_t = memory::mallocator;

    value_t *data;
    usize size;
    usize capacity;
    allocator_t *allocator; // &memory:global_mallocator_instance;

    constexpr value_t* get_data() { return data; }
    constexpr value_t const* get_data() const { return data; }
    constexpr usize get_size() const { return size; }
    constexpr bool is_empty() const { return (size == 0); }

    T& operator [] (usize index)
    {
        if (index + 1 < capacity)
        {
            ensure_capacity(index + 5);
        }

        if (size < index + 1)
        {
            size = index + 1;
        }

        return data[index];
    }

    T const& operator [] (usize index) const
    {
        ASSERT_MSG(index < size, "Attempt to access array memory out of bounds.");
        return data[index];
    }

    T& push (T const& t)
    {
        ASSERT(size <= capacity); // This is just an invariant.
        if (size == capacity)
        {
            ensure_capacity((capacity + 1) * 2);
        }

        return data[size++] = t;
    }

    void reserve(usize new_capacity)
    {
        if (new_capacity > capacity)
        {
            ensure_capacity(new_capacity);
        }
    }

    void ensure_capacity(usize new_capacity)
    {
        value_t *new_buffer = ALLOCATE_BUFFER(allocator, value_t, new_capacity);

        for (int32 i = 0; i < size; i++)
        {
            new_buffer[i] = data[i];
        }

        if (data)
        {
            DEALLOCATE_BUFFER(allocator, data);
        }

        data = new_buffer;
        capacity = new_capacity;
    }

    template <typename T, bool IsConst>
    struct iterator_
    {
    private:
        using ptr_t = typename type::sfinae_if<IsConst, T const*, T *>::type;
        using ref_t = typename type::sfinae_if<IsConst, T const&, T &>::type;

        ptr_t data;
        usize index;

    public:
        iterator_(ptr_t data_, usize index_) : data(data_), index(index_) {}
        iterator_& operator ++ () { index += 1; return *this; }
        iterator_  operator ++ (int) { iterator_ result = *this; index += 1; return result; }
        bool operator == (iterator_ other) const { return (data == other.data) && (index == other.index); }
        bool operator != (iterator_ other) const { return !(*this == other); }
        ref_t operator * () const { return data[index]; }
    };

    using iterator = iterator_<T, false>;
    using const_iterator = iterator_<T, true>;

    const_iterator cbegin() { return const_iterator(data, 0); }
    const_iterator cend() { return const_iterator(data, size); }
    const_iterator begin() const { return const_iterator(data, 0); }
    const_iterator end() const { return const_iterator(data, size); }
    iterator begin() { return iterator(data, 0); }
    iterator end() { return iterator(data, size); }
};


template <typename T>
void copy_dynamic_array(dynamic_array<T> *dest, dynamic_array<T> source)
{
    for (usize i = 0; i < dest->size; )

    if (dest.size < source.size)
    {
        return;
    }

    for (usize i = 0; i < source.size; i++)
    {
        dest[i] = source[i];
    }
}


template <typename T>
dynamic_array<T> make_dynamic_array()
{
    dynamic_array<T> result = {};
    result.allocator = &memory::global_mallocator_instance;

    return result;
}

template <typename T>
dynamic_array<T> make_dynamic_array(usize size)
{
    dynamic_array<T> result = {};
    result.allocator = alloc;

    result.data = ALLOCATE_BUFFER_(alloc, T, size);
    result.size = 0;
    result.capacity = size;

    return result;
}

// template <typename T, typename Allocator>
// void deallocate_array(dynamic_array<T, Allocator>& arr)
// {
//     memory::deallocate_buffer(arr->allocator, arr->data);
//     arr->data = NULL;
//     arr->size = 0;
//     arr->capacity = 0;
// }


// template <typename Allocator = memory::mallocator>
// void deallocate_string(dynamic_string<Allocator>& str)
// {
//     deallocate_array<char, Allocator>(str);
// }


// template <typename T, typename Allocator>
// dynamic_array<T, Allocator> make_copy(Allocator *allocator, dynamic_array<T, Allocator> source)
// {
//     dynamic_array<T, Allocator> result = make_dynamic_array<T, Allocator>(allocator, source.size);
//     copy_dynamic_array<T, Allocator>(source, result);
//     return result;
// }
