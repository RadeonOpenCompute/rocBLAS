/* ************************************************************************
 * Copyright 2016-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "handle.h"
#include "logging.h"
#include "rocblas.h"
#include "utility.h"

template <rocblas_int DIM_X,
          rocblas_int DIM_Y,
          bool        CONJ,
          typename T,
          typename U,
          typename V,
          typename W>
__global__ void ger_kernel(rocblas_int    m,
                           rocblas_int    n,
                           W              alpha_device_host,
                           rocblas_stride stride_alpha,
                           const U __restrict__ xa,
                           ptrdiff_t   shiftx,
                           rocblas_int incx,
                           rocblas_int stridex,
                           const U __restrict__ ya,
                           ptrdiff_t   shifty,
                           rocblas_int incy,
                           rocblas_int stridey,
                           V           Aa,
                           ptrdiff_t   shifta,
                           rocblas_int lda,
                           rocblas_int strideA)
{
    __shared__ T xdata[DIM_X];
    __shared__ T ydata[DIM_Y];

    const T* __restrict__ x = load_ptr_batch(xa, hipBlockIdx_z, shiftx, stridex);
    const T* __restrict__ y = load_ptr_batch(ya, hipBlockIdx_z, shifty, stridey);
    auto alpha              = load_scalar(alpha_device_host, hipBlockIdx_z, stride_alpha);
    T*   A                  = load_ptr_batch(Aa, hipBlockIdx_z, shifta, strideA);

    ptrdiff_t tx = hipBlockIdx_x * hipBlockDim_x + hipThreadIdx_x;
    ptrdiff_t ty = hipBlockIdx_y * hipBlockDim_y + hipThreadIdx_y;

    if(hipThreadIdx_y == 0 && tx < m)
    {
        xdata[hipThreadIdx_x] = x[tx * incx];
    }

    if(hipThreadIdx_x == 0 && ty < n)
    {
        ydata[hipThreadIdx_y] = y[ty * incy];
    }

    __syncthreads();

    if(tx < m && ty < n)
    {
        A[tx + lda * ty] += alpha * xdata[hipThreadIdx_x]
                            * (CONJ ? conj(ydata[hipThreadIdx_y]) : ydata[hipThreadIdx_y]);
    }
}

template <bool CONJ, typename T, typename U, typename V, typename W>
inline rocblas_status rocblas_ger_arg_check(rocblas_int    m,
                                            rocblas_int    n,
                                            const W*       alpha,
                                            rocblas_stride stride_alpha,
                                            const U*       x,
                                            rocblas_int    offsetx,
                                            rocblas_int    incx,
                                            rocblas_int    stridex,
                                            const U*       y,
                                            rocblas_int    offsety,
                                            rocblas_int    incy,
                                            rocblas_int    stridey,
                                            V*             A,
                                            rocblas_int    offsetA,
                                            rocblas_int    lda,
                                            rocblas_int    strideA,
                                            rocblas_int    batch_count)
{
    if(m < 0 || n < 0 || !incx || !incy || lda < m || lda < 1 || batch_count < 0)
        return rocblas_status_invalid_size;

    if(!m || !n || !batch_count)
        return rocblas_status_success;

    if(!alpha || !x || !y || !A)
        return rocblas_status_invalid_pointer;

    return rocblas_status_continue;
}

template <bool CONJ, typename T, typename U, typename V, typename W>
rocblas_status rocblas_ger_template(rocblas_handle handle,
                                    rocblas_int    m,
                                    rocblas_int    n,
                                    const W*       alpha,
                                    rocblas_stride stride_alpha,
                                    const U*       x,
                                    rocblas_int    offsetx,
                                    rocblas_int    incx,
                                    rocblas_int    stridex,
                                    const U*       y,
                                    rocblas_int    offsety,
                                    rocblas_int    incy,
                                    rocblas_int    stridey,
                                    V*             A,
                                    rocblas_int    offsetA,
                                    rocblas_int    lda,
                                    rocblas_int    strideA,
                                    rocblas_int    batch_count)
{
    // Quick return if possible. Not Argument error
    if(!m || !n || !batch_count)
        return rocblas_status_success;

    hipStream_t rocblas_stream = handle->rocblas_stream;

    // in case of negative inc shift pointer to end of data for negative indexing tid*inc
    auto shiftx = incx < 0 ? offsetx - ptrdiff_t(incx) * (m - 1) : offsetx;
    auto shifty = incy < 0 ? offsety - ptrdiff_t(incy) * (n - 1) : offsety;

    static constexpr int DIM_X   = 64;
    static constexpr int DIM_Y   = 16;
    rocblas_int          blocksX = (m - 1) / DIM_X + 1;
    rocblas_int          blocksY = (n - 1) / DIM_Y + 1;

    dim3 grid(blocksX, blocksY, batch_count);
    dim3 threads(DIM_X, DIM_Y);

    if(handle->pointer_mode == rocblas_pointer_mode_device)
        hipLaunchKernelGGL((ger_kernel<DIM_X, DIM_Y, CONJ, T>),
                           grid,
                           threads,
                           0,
                           rocblas_stream,
                           m,
                           n,
                           alpha,
                           stride_alpha,
                           x,
                           shiftx,
                           incx,
                           stridex,
                           y,
                           shifty,
                           incy,
                           stridey,
                           A,
                           offsetA,
                           lda,
                           strideA);
    else
        hipLaunchKernelGGL((ger_kernel<DIM_X, DIM_Y, CONJ, T>),
                           grid,
                           threads,
                           0,
                           rocblas_stream,
                           m,
                           n,
                           *alpha,
                           stride_alpha,
                           x,
                           shiftx,
                           incx,
                           stridex,
                           y,
                           shifty,
                           incy,
                           stridey,
                           A,
                           offsetA,
                           lda,
                           strideA);
    return rocblas_status_success;
}
