/* ************************************************************************
 * Copyright 2016-2019 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#pragma once
#include "handle.h"
#include "rocblas.h"
#include "rocblas_swap.hpp"

template <typename T>
__global__ void rocblas_swap_kernel_batched(rocblas_int n,
                                            T*          x[],
                                            ptrdiff_t   shiftx,
                                            rocblas_int incx,
                                            T*          y[],
                                            ptrdiff_t   shifty,
                                            rocblas_int incy)
{
    ssize_t tid = blockIdx.x * blockDim.x + threadIdx.x; // only dim1

    if(tid < n)
    {
        T* xb = x[blockIdx.y] + shiftx;
        T* yb = y[blockIdx.y] + shifty;

        rocblas_swap_vals(xb + tid * incx, yb + tid * incy);
    }
}

template <rocblas_int NB, typename T>
rocblas_status rocblas_swap_batched_template(rocblas_handle handle,
                                             rocblas_int    n,
                                             T*             x[],
                                             rocblas_int    offsetx,
                                             rocblas_int    incx,
                                             T*             y[],
                                             rocblas_int    offsety,
                                             rocblas_int    incy,
                                             rocblas_int    batch_count)
{
    // Quick return if possible.
    if(n <= 0 || !batch_count)
        return rocblas_status_success;

    // in case of negative inc shift pointer to end of data for negative indexing tid*inc
    ptrdiff_t   shiftx = incx < 0 ? offsetx - ptrdiff_t(incx) * (n - 1) : offsetx;
    ptrdiff_t   shifty = incy < 0 ? offsety - ptrdiff_t(incy) * (n - 1) : offsety;
    rocblas_int blocks = (n - 1) / NB + 1;

    hipLaunchKernelGGL(rocblas_swap_kernel_batched,
                       dim3(blocks, batch_count),
                       dim3(NB),
                       0,
                       handle->rocblas_stream,
                       n,
                       x,
                       shiftx,
                       incx,
                       y,
                       shifty,
                       incy);

    return rocblas_status_success;
}
