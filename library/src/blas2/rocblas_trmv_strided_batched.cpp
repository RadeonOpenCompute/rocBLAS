/* ************************************************************************
 * Copyright 2016-2019 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "rocblas_trmv_strided_batched.hpp"
#include "handle.h"
#include "logging.h"
#include "rocblas.h"
#include "utility.h"
#include <algorithm>
#include <cstdio>
#include <tuple>

namespace
{

    template <typename>
    constexpr char rocblas_trmv_strided_batched_name[] = "unknown";
    template <>
    constexpr char rocblas_trmv_strided_batched_name<float>[] = "rocblas_strmv_strided_batched";
    template <>
    constexpr char rocblas_trmv_strided_batched_name<double>[] = "rocblas_dtrmv_strided_batched";
    template <>
    constexpr char rocblas_trmv_strided_batched_name<rocblas_float_complex>[]
        = "rocblas_ctrmv_strided_batched";
    template <>
    constexpr char rocblas_trmv_strided_batched_name<rocblas_double_complex>[]
        = "rocblas_ztrmv_strided_batched";

    template <typename T>
    rocblas_status rocblas_trmv_strided_batched_impl(rocblas_handle    handle,
                                                     rocblas_fill      uplo,
                                                     rocblas_operation transa,
                                                     rocblas_diagonal  diag,
                                                     rocblas_int       m,
                                                     const T*          a,
                                                     rocblas_int       lda,
                                                     rocblas_stride    stridea,
                                                     T*                x,
                                                     rocblas_int       incx,
                                                     rocblas_stride    stridex,
                                                     rocblas_int       batch_count)
    {
        if(!handle)
        {
            return rocblas_status_invalid_handle;
        }

        auto layer_mode = handle->layer_mode;
        if(layer_mode
           & (rocblas_layer_mode_log_trace | rocblas_layer_mode_log_bench
              | rocblas_layer_mode_log_profile))
        {
            auto uplo_letter   = rocblas_fill_letter(uplo);
            auto transa_letter = rocblas_transpose_letter(transa);
            auto diag_letter   = rocblas_diag_letter(diag);
            if(layer_mode & rocblas_layer_mode_log_trace)
            {
                log_trace(handle,
                          rocblas_trmv_strided_batched_name<T>,
                          uplo,
                          transa,
                          diag,
                          m,
                          a,
                          lda,
                          x,
                          incx,

                          "--stride_A",
                          stridea,
                          "--incx",
                          incx,
                          "--stride_x",
                          stridex,

                          batch_count);
            }

            if(layer_mode & rocblas_layer_mode_log_bench)
            {
                log_bench(handle,
                          "./rocblas-bench",
                          "-f",
                          "trmv_strided_batched",
                          "-r",
                          rocblas_precision_string<T>,
                          "--uplo",
                          uplo_letter,
                          "--transposeA",
                          transa_letter,
                          "--diag",
                          diag_letter,
                          "-m",
                          m,
                          "--lda",
                          lda,
                          "--stride_A",
                          stridea,
                          "--incx",
                          incx,
                          "--stride_x",
                          stridex,
                          "--batch_count",
                          batch_count);
            }

            if(layer_mode & rocblas_layer_mode_log_profile)
            {
                log_profile(handle,
                            rocblas_trmv_strided_batched_name<T>,
                            "uplo",
                            uplo_letter,
                            "transA",
                            transa_letter,
                            "diag",
                            diag_letter,
                            "M",
                            m,
                            "lda",
                            lda,
                            "stride_A",
                            stridea,
                            "incx",
                            incx,
                            "stride_x",
                            stridex,
                            "batch_count",
                            batch_count);
            }
        }

        if(uplo != rocblas_fill_lower && uplo != rocblas_fill_upper)
        {
            return rocblas_status_not_implemented;
        }

        if(m < 0 || lda < m || lda < 1 || !incx || batch_count < 0)
        {
            return rocblas_status_invalid_size;
        }

        if(!m || !batch_count)
        {
            return handle->is_device_memory_size_query() ? rocblas_status_size_unchanged
                                                         : rocblas_status_success;
        }

        size_t dev_bytes = m * batch_count * sizeof(T);
        if(handle->is_device_memory_size_query())
        {
            return handle->set_optimal_device_memory_size(dev_bytes);
        }

        if(!a || !x)
        {
            return rocblas_status_invalid_pointer;
        }

        T* w = (T*)handle->device_malloc(dev_bytes);
        if(!w)
        {
            return rocblas_status_memory_error;
        }

        rocblas_stride stridew = m;
        return rocblas_trmv_strided_batched_template<T>(handle,
                                                        uplo,
                                                        transa,
                                                        diag,
                                                        m,
                                                        a,
                                                        lda,
                                                        stridea,
                                                        x,
                                                        incx,
                                                        stridex,
                                                        w,
                                                        stridew,
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

#define IMPL(routine_name_, T_)                                                             \
    rocblas_status routine_name_(rocblas_handle    handle,                                  \
                                 rocblas_fill      uplo,                                    \
                                 rocblas_operation transA,                                  \
                                 rocblas_diagonal  diag,                                    \
                                 rocblas_int       m,                                       \
                                 const T_*         A,                                       \
                                 rocblas_int       lda,                                     \
                                 rocblas_stride    stridea,                                 \
                                 T_*               x,                                       \
                                 rocblas_int       incx,                                    \
                                 rocblas_stride    stridex,                                 \
                                 rocblas_int       batch_count)                             \
    {                                                                                       \
        return rocblas_trmv_strided_batched_impl(                                           \
            handle, uplo, transA, diag, m, A, lda, stridea, x, incx, stridex, batch_count); \
    }

IMPL(rocblas_strmv_strided_batched, float);
IMPL(rocblas_dtrmv_strided_batched, double);
IMPL(rocblas_ctrmv_strided_batched, rocblas_float_complex);
IMPL(rocblas_ztrmv_strided_batched, rocblas_double_complex);

#undef IMPL

} // extern "C"
