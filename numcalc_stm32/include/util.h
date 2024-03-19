#pragma once

//template state on change value

// //light dynamic array
// template <typename data_T>
// struct darray_t;
// template <typename data_T> 
// struct darray_t {
//     data_T* buf;
//     unsigned int count;
//     unsigned int buf_len;
//     unsigned int chunk_size;
// } ;

// template <typename data_T>
// void add_space(darray_t<data_T> &a){
//     const int bss = a.chunk_size;
//     if(a.buf_len == 0){
//         a.buf = (data_T*)malloc(sizeof(data_T)*bss);
//         a.buf_len = bss;
//         a.count = 0;
//     }
//     else if(a.count == a.buf_len){
//         //printf("increse buf size %d -> %d\n", a.buf_len, a.buf_len+bss);
//         a.buf_len += bss;
//         a.buf = (data_T*)realloc(a.buf, sizeof(data_T) * (a.buf_len));
//     }
//     //printf("add_space\n");
// }

// template <typename data_T> 
// void darray_clear(darray_t<data_T> &a){
//     if(a.buf) free(a.buf);
//     a.buf_len = 0;
//     a.count = 0;
// }

// template <typename data_T> 
// void darray_push(darray_t<data_T> &a, data_T t){
//     add_space(a);

//     a.buf[a.count] = t;
//     a.count++;
//     //printf("pushed\n");
//     return;
// }
// template <typename data_T> 
// void darray_pop(darray_t<data_T> &a){
//     if(a.count == 0) return;
//     a.count--;
// }

// template <typename data_T> 
// void darray_insert(darray_t<data_T> &a, data_T t, unsigned int i){
//     if(i >= a.count) return darray_push(a,t);
    
//     if(a.count+1 > a.buf_len) add_space(a);
    
//     for(int j=a.count; j>i; j--){
//         auto aa = a.buf[j-1];
//         a.buf[j] = aa;
//     }
//     a.buf[i] = t;
//     a.count++;
    
//     //printf("inserted\n");
//     return;
// }
// template <typename data_T> 
// void darray_remove(darray_t<data_T> &a, int i){
//     if(a.count == 0) return;
//     for(int j=i; j<(a.count); j++){
//         data_T aa = a.buf[j+1];
//         a.buf[j] = aa;
//     }
//     a.count--;
// }



template <typename T> 
struct sarray_t{
    T* buf;
    unsigned int count;
    unsigned int lim;
};

template <typename T> 
void sarray_clear(sarray_t<T> &a, T set_to){
    a.count = 0;
    for(int i=0; i<a.lim; i++)
        a.buf[i] = set_to;
}

template <typename T> 
void sarray_push(sarray_t<T> &a, T t){
    if(a.count >= a.lim) return;

    a.buf[a.count] = t;
    a.count++;
    return;
}

template <typename T> 
void sarray_pop(sarray_t<T> &a){
    if(a.count == 0) return;
    a.count--;
}

template <typename T> 
void sarray_insert(sarray_t<T> &a, T t, unsigned int i){
    if(i >= a.count) return sarray_push(a,t);

    for(int j=a.count; j>i; j--){
        auto aa = a.buf[j-1];
        a.buf[j] = aa;
    }
    a.buf[i] = t;
    a.count++;
    
    //printf("inserted\n");
    return;
}
template <typename T> 
void sarray_remove(sarray_t<T> &a, int i){
    if(a.count == 0) return;
    for(int j=i; j<(a.count); j++){
        auto aa = a.buf[j+1];
        a.buf[j] = aa;
    }
    a.count--;
}

template <typename T> 
T& sarray_peek(sarray_t<T> &a){
    return a.buf[a.count-1];
}