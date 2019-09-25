#pragma once

#include "handle.h"
#include "rocblas.h"
#include "utility.h"
#include <type_traits>
#include <utility>
#include "logging.h"

//
// Define the representation of a mutable strided batched set of arrays.
//
template <typename T>
using strided_batched_arrays = T*;

//
// Define the representation of a non-mutable strided batched set of arrays.
//
template <typename T>
using const_strided_batched_arrays = const T*;

//
// Define the representation of a mutable batched set of arrays.
//
template <typename T>
using batched_arrays = T**;

//
// Define the representation of a non-mutable batched set of arrays.
//
template <typename T>
using const_batched_arrays = const T* const *;

template <typename U>
struct batched_traits
{
};

template <typename T>
struct batched_traits< const_batched_arrays<T> >
{
  using data_type = T;
  using ptr_type = const data_type *;
};

template <typename T>
struct batched_traits< const_strided_batched_arrays<T> >
{
  using data_type = T;
  using ptr_type = const data_type *;
};

template <typename T>
struct batched_traits< batched_arrays<T> >
{
  using data_type = T;
  using ptr_type = data_type *;
};

template <typename T>
struct batched_traits< strided_batched_arrays<T> >
{
  using data_type = T;
  using ptr_type = data_type *;
};

template <typename U>
using batched_data_t = typename batched_traits< U >::data_type;

template <typename U>
using batched_ptr_t = typename batched_traits< U >::ptr_type;


template <typename T>
struct is_batched_arrays
{
  enum { value = false };
};

template <typename T>
struct is_batched_arrays< batched_arrays<T> >
{
  enum { value = true };
};

template <typename T>
struct is_batched_arrays< const_batched_arrays<T> >
{
  enum { value = true };
};


template <typename T>
struct is_strided_batched_arrays
{
  enum { value = false };
};

template <typename T>
struct is_strided_batched_arrays< strided_batched_arrays<T> >
{
  enum { value = true };
};

template <typename T>
struct is_strided_batched_arrays< const_strided_batched_arrays<T> >
{
  enum { value = true };
};


template <typename U>
__forceinline__ __device__ __host__  typename batched_traits< U >::ptr_type load_batched_ptr(U x,
											     rocblas_int batch_index,
											     rocblas_int 	offset,
											     rocblas_stride stride);
template <typename T>
__forceinline__ __device__ __host__  typename batched_traits< const_batched_arrays<T> >::ptr_type load_batched_ptr(const_batched_arrays<T> x,
														   rocblas_int batch_index,
														   rocblas_int 	offset,
														   rocblas_stride stride)
{
  return x[batch_index] + offset;
};

template <typename T>
__forceinline__ __device__ __host__ typename batched_traits< batched_arrays<T> >::ptr_type load_batched_ptr(batched_arrays<T> x,
													    rocblas_int batch_index,
													    rocblas_int offset,
													    rocblas_stride stride)
{
  return x[batch_index] + offset;
};

template <typename T>
__forceinline__ __device__ __host__ typename batched_traits< const_strided_batched_arrays<T> >::ptr_type load_batched_ptr(const_strided_batched_arrays<T> x,
															  rocblas_int batch_index,
															  rocblas_int offset,
															  rocblas_stride stride)
{
  return (x + rocblas_stride(batch_index) * stride) + offset;
};


template <typename T>
__forceinline__ __device__ __host__ typename batched_traits< strided_batched_arrays<T> >::ptr_type load_batched_ptr(strided_batched_arrays<T> x,
														    rocblas_int batch_index,
														    rocblas_int offset,
														    rocblas_stride stride)
{
  return (x + rocblas_stride(batch_index) * stride) + offset;
};


template <typename U>
typename batched_traits<U>::ptr_type load_batched_ptr(U x,
						      rocblas_int    batch_index,
						      rocblas_stride stride)
{
  return load_batched_ptr(x,batch_index,0,stride);
};