/* ************************************************************************
 * Copyright 2016-2019 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "handle.h"
#include "rocblas.h"
#include "utility.h"

template <typename T, typename U, typename V>
__global__ void scal_kernel(rocblas_int n,
                            V           alpha_device_host,
                            U           xa,
                            rocblas_int offsetx,
                            rocblas_int incx,
                            rocblas_int stridex)
{
    #ifdef scal_batched
        T* x = xa[hipBlockIdx_y] + offsetx;
    #else
        T* x = xa + hipBlockIdx_y * stridex;
    #endif

    auto      alpha = load_scalar(alpha_device_host);
    ptrdiff_t tid   = hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x;

    // bound
    if(tid < n)
        x[tid * incx] *= alpha;
}

template <rocblas_int NB, typename T, typename U, typename V>
rocblas_status rocblas_scal_template(rocblas_handle  handle,
                                     rocblas_int     n,
                                     const V*        alpha,
                                     U               x,
                                     rocblas_int     batched_offsetx,
                                     rocblas_int     incx,
                                     rocblas_int     stridex,
                                     rocblas_int     batch_count)
{
    // Quick return if possible. Not Argument error
    if(n <= 0 || incx <= 0 || batch_count <= 0)
        return rocblas_status_success;

    dim3        blocks((n - 1) / NB + 1, batch_count);
    dim3        threads(NB);
    hipStream_t rocblas_stream = handle->rocblas_stream;

    if(rocblas_pointer_mode_device == handle->pointer_mode)
        hipLaunchKernelGGL(
            scal_kernel<T>, blocks, threads, 0, rocblas_stream, n, alpha, x, batched_offsetx, incx, stridex);
    else // alpha is on host
        hipLaunchKernelGGL(
            scal_kernel<T>, blocks, threads, 0, rocblas_stream, n, *alpha, x, batched_offsetx, incx, stridex);

    return rocblas_status_success;
}
