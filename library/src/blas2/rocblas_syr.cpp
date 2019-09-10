/* ************************************************************************
 * Copyright 2016-2019 Advanced Micro Devices, Inc.
 * ************************************************************************ */
#include "rocblas_syr.hpp"


/*
 * ===========================================================================
 *    C wrapper
 * ===========================================================================
 */

extern "C" {

rocblas_status rocblas_ssyr(rocblas_handle handle,
                            rocblas_fill   uplo,
                            rocblas_int    n,
                            const float*   alpha,
                            const float*   x,
                            rocblas_int    incx,
                            float*         A,
                            rocblas_int    lda)
{
    return rocblas_syr(handle, uplo, n, alpha, x, incx, A, lda);
}

rocblas_status rocblas_dsyr(rocblas_handle handle,
                            rocblas_fill   uplo,
                            rocblas_int    n,
                            const double*  alpha,
                            const double*  x,
                            rocblas_int    incx,
                            double*        A,
                            rocblas_int    lda)
{
    return rocblas_syr(handle, uplo, n, alpha, x, incx, A, lda);
}

} // extern "C"
