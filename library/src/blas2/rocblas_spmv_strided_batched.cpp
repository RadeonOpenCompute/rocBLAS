/* ************************************************************************
 * Copyright 2016-2020 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "logging.hpp"
#include "rocblas_spmv.hpp"
#include "utility.hpp"

namespace
{
    template <typename>
    constexpr char rocblas_spmv_strided_batched_name[] = "unknown";
    template <>
    constexpr char rocblas_spmv_strided_batched_name<float>[] = "rocblas_sspmv_strided_batched";
    template <>
    constexpr char rocblas_spmv_strided_batched_name<double>[] = "rocblas_dspmv_strided_batched";

    template <typename T, typename U, typename V, typename W>
    rocblas_status rocblas_spmv_strided_batched_impl(rocblas_handle handle,
                                                     rocblas_fill   uplo,
                                                     rocblas_int    n,
                                                     const V*       alpha,
                                                     const U*       A,
                                                     rocblas_stride strideA,
                                                     const U*       x,
                                                     rocblas_int    incx,
                                                     rocblas_stride stridex,
                                                     const V*       beta,
                                                     W*             y,
                                                     rocblas_int    incy,
                                                     rocblas_stride stridey,
                                                     rocblas_int    batch_count)
    {
        if(!handle)
            return rocblas_status_invalid_handle;
        RETURN_ZERO_DEVICE_MEMORY_SIZE_IF_QUERIED(handle);

        auto layer_mode = handle->layer_mode;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto uplo_letter = rocblas_fill_letter(uplo);

            if(layer_mode & rocblas_layer_mode_log_trace)
                log_trace(handle,
                          rocblas_spmv_strided_batched_name<T>,
                          uplo,
                          n,
                          LOG_TRACE_SCALAR_VALUE(handle, alpha),
                          A,
                          strideA,
                          x,
                          incx,
                          stridex,
                          LOG_TRACE_SCALAR_VALUE(handle, beta),
                          y,
                          incy,
                          stridey,
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_bench)
                log_bench(handle,
                          "./rocblas-bench -f spmv_strided_batched -r",
                          rocblas_precision_string<T>,
                          "--uplo",
                          uplo_letter,
                          "-n",
                          n,
                          LOG_BENCH_SCALAR_VALUE(handle, alpha),
                          "--stride_a",
                          strideA,
                          "--incx",
                          incx,
                          "--stride_x",
                          stridex,
                          LOG_BENCH_SCALAR_VALUE(handle, beta),
                          "--incy",
                          incy,
                          "--stride_y",
                          stridey,
                          "--batch_count",
                          batch_count);

            if(layer_mode & rocblas_layer_mode_log_profile)
                log_profile(handle,
                            rocblas_spmv_strided_batched_name<T>,
                            "uplo",
                            uplo_letter,
                            "N",
                            n,
                            "stride_a",
                            strideA,
                            "incx",
                            incx,
                            "stride_x",
                            stridex,
                            "incy",
                            incy,
                            "stride_y",
                            stridey,
                            "batch_count",
                            batch_count);
        }

        rocblas_status arg_status = rocblas_spmv_arg_check<T>(handle,
                                                              uplo,
                                                              n,
                                                              alpha,
                                                              0,
                                                              A,
                                                              0,
                                                              strideA,
                                                              x,
                                                              0,
                                                              incx,
                                                              stridex,
                                                              beta,
                                                              0,
                                                              y,
                                                              0,
                                                              incy,
                                                              stridey,
                                                              batch_count);
        if(arg_status != rocblas_status_continue)
            return arg_status;

        return rocblas_spmv_template<T>(handle,
                                        uplo,
                                        n,
                                        alpha,
                                        0,
                                        A,
                                        0,
                                        strideA,
                                        x,
                                        0,
                                        incx,
                                        stridex,
                                        beta,
                                        0,
                                        y,
                                        0,
                                        incy,
                                        stridey,
                                        batch_count);
    }

} // namespace

/*
* ===========================================================================
*    C wrapper
* ===========================================================================
*/

extern "C" {

#ifdef IMPL
#error IMPL ALREADY DEFINED
#endif

#define IMPL(routine_name_, T_)                                    \
    rocblas_status routine_name_(rocblas_handle  handle,           \
                                 rocblas_fill    uplo,             \
                                 rocblas_int     n,                \
                                 const T_* const alpha,            \
                                 const T_*       A,                \
                                 rocblas_stride  strideA,          \
                                 const T_*       x,                \
                                 rocblas_int     incx,             \
                                 rocblas_stride  stridex,          \
                                 const T_*       beta,             \
                                 T_*             y,                \
                                 rocblas_int     incy,             \
                                 rocblas_stride  stridey,          \
                                 rocblas_int     batch_count)      \
    try                                                            \
    {                                                              \
        return rocblas_spmv_strided_batched_impl<T_>(handle,       \
                                                     uplo,         \
                                                     n,            \
                                                     alpha,        \
                                                     A,            \
                                                     strideA,      \
                                                     x,            \
                                                     incx,         \
                                                     stridex,      \
                                                     beta,         \
                                                     y,            \
                                                     incy,         \
                                                     stridey,      \
                                                     batch_count); \
    }                                                              \
    catch(...)                                                     \
    {                                                              \
        return exception_to_rocblas_status();                      \
    }

IMPL(rocblas_sspmv_strided_batched, float);
IMPL(rocblas_dspmv_strided_batched, double);

#undef IMPL

} // extern "C"
