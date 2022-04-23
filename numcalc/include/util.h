#pragma once

//template state on change value

//light dynamic array
template <typename data_T>
struct darray_t {
    data_T* buf;
    unsigned int count;
    unsigned int buf_len;
    unsigned int chunk_size;
};

template <typename data_T>
void add_space(darray_t<data_T> &a);

template <typename data_T> 
void darray_clear(darray_t<data_T> &a);

template <typename data_T> 
void darray_push(darray_t<data_T> &a, data_T t);

template <typename data_T> 
void darray_pop(darray_t<data_T> &a);

template <typename data_T> 
void darray_insert(darray_t<data_T> &a, data_T t, unsigned int i);

template <typename data_T> 
void darray_remove(darray_t<data_T> &a, int i);
