#pragma once


template <typename T>
struct Index {
    u32 index = 0;

    Index() : index(0) {}
    Index(u32 value) : index(value) {}
};


template <typename T> Index<T> operator ++ (Index<T> &index, int) {
    Index<T> result = index;
    index.index++;

    return result;
}

template <typename T> Index<T> &operator += (Index<T> &index, u32 value) {
    index.index += value;
    return index;
}

template <typename T> T * operator + (T *array, Index<T> index) {
    T *result = array + index.index;

    return result;
}

template <typename T> bool operator == (Index<T> index1, Index<T> index2) {
    bool result = (index1.index == index2.index);

    return result;
}

template <typename T> bool operator != (Index<T> index1, Index<T> index2) {
    bool result = !(index1 == index2);

    return result;
}

template <typename T> bool operator == (Index<T> index, u32 value) {
    bool result = index.index == value;

    return result;
}

template <typename T> bool operator != (Index<T> index, u32 value) {
    bool result = !(index == value);

    return result;
}

template <typename T> bool operator < (Index<T> index, u32 value) {
    bool result = index.index < value;

    return result;
}

template <typename T> bool operator > (Index<T> index, u32 value) {
    bool result = index.index > value;

    return result;
}
