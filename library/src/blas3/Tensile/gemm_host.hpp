/* ************************************************************************
 * Copyright 2016-2019 Advanced Micro Devices, Inc.
 * ************************************************************************ */

#pragma once
#ifndef _GEMM_HOST_HPP_
#define _GEMM_HOST_HPP_
#include "rocblas.h"
#include "gemm_device.hpp"
#include "handle.h"
#include "Tensile.h"
#include "rocblas-types.h"
#include "utility.h"



/*******************************************************************************
 * Infer Batch Strides
 ******************************************************************************/
inline void infer_batch_strides(rocblas_operation trans_a,
                                rocblas_operation trans_b,
                                rocblas_int       m,
                                rocblas_int       n,
                                rocblas_int       k,
                                rocblas_int       ld_a,
                                rocblas_int*      stride_a,
                                rocblas_int       ld_b,
                                rocblas_int*      stride_b,
                                rocblas_int       ld_c,
                                rocblas_int*      stride_c)
{

    rocblas_int num_cols_c = n;
    rocblas_int num_rows_c = m;
    rocblas_int num_cols_a = (trans_a == rocblas_operation_none ? k : m);
    rocblas_int num_rows_a = (trans_a == rocblas_operation_none ? m : k);
    rocblas_int num_cols_b = (trans_b == rocblas_operation_none ? n : k);
    rocblas_int num_rows_b = (trans_b == rocblas_operation_none ? k : n);

    *stride_a = ld_a * num_cols_a;
    *stride_b = ld_b * num_cols_b;
    *stride_c = ld_c * num_cols_c;

} // infer batched strides

/*******************************************************************************
 * Validate Arguments
 ******************************************************************************/
inline rocblas_status validateArgs(rocblas_handle    handle,
                                   rocblas_operation trans_a,
                                   rocblas_operation trans_b,
                                   rocblas_int       m,
                                   rocblas_int       n,
                                   rocblas_int       k,
                                   const void*       alpha,
                                   const void*       a,
                                   rocblas_int       ld_a,
                                   rocblas_int       stride_a,
                                   const void*       b,
                                   rocblas_int       ld_b,
                                   rocblas_int       stride_b,
                                   const void*       beta,
                                   void*             c,
                                   rocblas_int       ld_c,
                                   rocblas_int       stride_c,
                                   rocblas_int       batch_count)
{
    // quick return 0 is valid in BLAS
    if(!m || !n || !k || !batch_count)
        return rocblas_status_success;

    // sizes must not be negative
    if(m < 0 || n < 0 || k < 0 || batch_count < 0)
        return rocblas_status_invalid_size;

    // handle must be valid
    if(!handle)
        return rocblas_status_invalid_handle;

    // pointers must be valid
    if(!c || !a || !b || !alpha || !beta)
        return rocblas_status_invalid_pointer;

    rocblas_int num_cols_c = n;
    rocblas_int num_rows_c = m;
    rocblas_int num_cols_a = trans_a == rocblas_operation_none ? k : m;
    rocblas_int num_rows_a = trans_a == rocblas_operation_none ? m : k;
    rocblas_int num_cols_b = trans_b == rocblas_operation_none ? n : k;
    rocblas_int num_rows_b = trans_b == rocblas_operation_none ? k : n;

    // leading dimensions must be valid
    if(num_rows_a > ld_a || num_rows_b > ld_b || num_rows_c > ld_c)
        return rocblas_status_invalid_size;

    return rocblas_status_success;
} // validate parameters

template <typename T>
inline rocblas_status validateArgs(rocblas_handle    handle,
                                   rocblas_operation trans_a,
                                   rocblas_operation trans_b,
                                   rocblas_int       m,
                                   rocblas_int       n,
                                   rocblas_int       k,
                                   const T*          alpha,
                                   const T* const    a[],
                                   rocblas_int       ld_a,
                                   const T* const    b[],
                                   rocblas_int       ld_b,
                                   const T*          beta,
                                   T* const          c[],
                                   rocblas_int       ld_c,
                                   rocblas_int       batch_count)
{
    // quick return 0 is valid in BLAS
    if(!m || !n || !k || !batch_count)
        return rocblas_status_success;

     // sizes must not be negative
    if(m < 0 || n < 0 || k < 0 || batch_count < 0)
        return rocblas_status_invalid_size;

     // handle must be valid
    if(!handle)
        return rocblas_status_invalid_handle;

     // pointers must be valid
    if(!c || !a || !b || !alpha || !beta)
        return rocblas_status_invalid_pointer;

     // for(int i = 0; i < batch_count; i++)
    //     if(!a[i] || !b[i] || !c[i])
    //         return rocblas_status_invalid_pointer;

     rocblas_int num_cols_c = n;
    rocblas_int num_rows_c = m;
    rocblas_int num_cols_a = trans_a == rocblas_operation_none ? k : m;
    rocblas_int num_rows_a = trans_a == rocblas_operation_none ? m : k;
    rocblas_int num_cols_b = trans_b == rocblas_operation_none ? n : k;
    rocblas_int num_rows_b = trans_b == rocblas_operation_none ? k : n;

     // leading dimensions must be valid
    if(num_rows_a > ld_a || num_rows_b > ld_b || num_rows_c > ld_c)
        return rocblas_status_invalid_size;

     return rocblas_status_success;
} // validate parameters


/*******************************************************************************
 * Tensile Solution Name (debug only)
 ******************************************************************************/
template <typename T>
const char* tensileGetSolutionName(rocblas_operation trans_a,
                                   rocblas_operation trans_b,
                                   rocblas_int       strideC1,
                                   rocblas_int       strideC2,
                                   rocblas_int       strideA1,
                                   rocblas_int       strideA2,
                                   rocblas_int       strideB1,
                                   rocblas_int       strideB2,
                                   rocblas_int       sizeI,
                                   rocblas_int       sizeJ,
                                   rocblas_int       sizeK,
                                   rocblas_int       sizeL)
{
    return "";
};

// This macro condenses all the identical arguments to the various
// tensileGetSolutionName function calls for consistency / brevity
#define TENSILE_ARG_NAMES                                                                         \
    strideC1, strideC2, strideC1, strideC2, strideA1, strideA2, strideB1, strideB2, sizeI, sizeJ, \
        sizeK, sizeL

template <>
const char* tensileGetSolutionName<rocblas_half>(rocblas_operation trans_a,
                                                 rocblas_operation trans_b,
                                                 rocblas_int       strideC1,
                                                 rocblas_int       strideC2,
                                                 rocblas_int       strideA1,
                                                 rocblas_int       strideA2,
                                                 rocblas_int       strideB1,
                                                 rocblas_int       strideB2,
                                                 rocblas_int       sizeI,
                                                 rocblas_int       sizeJ,
                                                 rocblas_int       sizeK,
                                                 rocblas_int       sizeL)
{
    switch(GetTransposeMode(trans_a, trans_b))
    {
    case NN:
        return tensileGetSolutionName_Cijk_Ailk_Bljk_HB(TENSILE_ARG_NAMES);
    case NT:
    case NC:
        return tensileGetSolutionName_Cijk_Ailk_Bjlk_HB(TENSILE_ARG_NAMES);
    case TN:
    case CN:
        return tensileGetSolutionName_Cijk_Alik_Bljk_HB(TENSILE_ARG_NAMES);
    case TT:
    case TC:
    case CT:
    case CC:
        return tensileGetSolutionName_Cijk_Alik_Bjlk_HB(TENSILE_ARG_NAMES);
    }
}

template <>
const char* tensileGetSolutionName<float>(rocblas_operation trans_a,
                                          rocblas_operation trans_b,
                                          rocblas_int       strideC1,
                                          rocblas_int       strideC2,
                                          rocblas_int       strideA1,
                                          rocblas_int       strideA2,
                                          rocblas_int       strideB1,
                                          rocblas_int       strideB2,
                                          rocblas_int       sizeI,
                                          rocblas_int       sizeJ,
                                          rocblas_int       sizeK,
                                          rocblas_int       sizeL)
{
    switch(GetTransposeMode(trans_a, trans_b))
    {
    case NN:
        return tensileGetSolutionName_Cijk_Ailk_Bljk_SB(TENSILE_ARG_NAMES);
    case NT:
    case NC:
        return tensileGetSolutionName_Cijk_Ailk_Bjlk_SB(TENSILE_ARG_NAMES);
    case TN:
    case CN:
        return tensileGetSolutionName_Cijk_Alik_Bljk_SB(TENSILE_ARG_NAMES);
    case TT:
    case TC:
    case CT:
    case CC:
        return tensileGetSolutionName_Cijk_Alik_Bjlk_SB(TENSILE_ARG_NAMES);
    }
}

template <>
const char* tensileGetSolutionName<double>(rocblas_operation trans_a,
                                           rocblas_operation trans_b,
                                           rocblas_int       strideC1,
                                           rocblas_int       strideC2,
                                           rocblas_int       strideA1,
                                           rocblas_int       strideA2,
                                           rocblas_int       strideB1,
                                           rocblas_int       strideB2,
                                           rocblas_int       sizeI,
                                           rocblas_int       sizeJ,
                                           rocblas_int       sizeK,
                                           rocblas_int       sizeL)
{
    switch(GetTransposeMode(trans_a, trans_b))
    {
    case NN:
        return tensileGetSolutionName_Cijk_Ailk_Bljk_DB(TENSILE_ARG_NAMES);
    case NT:
    case NC:
        return tensileGetSolutionName_Cijk_Ailk_Bjlk_DB(TENSILE_ARG_NAMES);
    case TN:
    case CN:
        return tensileGetSolutionName_Cijk_Alik_Bljk_DB(TENSILE_ARG_NAMES);
    case TT:
    case TC:
    case CT:
    case CC:
        return tensileGetSolutionName_Cijk_Alik_Bjlk_DB(TENSILE_ARG_NAMES);
    }
}

template <>
const char* tensileGetSolutionName<rocblas_float_complex>(rocblas_operation trans_a,
                                                          rocblas_operation trans_b,
                                                          rocblas_int       strideC1,
                                                          rocblas_int       strideC2,
                                                          rocblas_int       strideA1,
                                                          rocblas_int       strideA2,
                                                          rocblas_int       strideB1,
                                                          rocblas_int       strideB2,
                                                          rocblas_int       sizeI,
                                                          rocblas_int       sizeJ,
                                                          rocblas_int       sizeK,
                                                          rocblas_int       sizeL)
{
    switch(GetTransposeMode(trans_a, trans_b))
    {
    case NN:
        return tensileGetSolutionName_Cijk_Ailk_Bljk_CB(TENSILE_ARG_NAMES);
    case NT:
        return tensileGetSolutionName_Cijk_Ailk_Bjlk_CB(TENSILE_ARG_NAMES);
    case TN:
        return tensileGetSolutionName_Cijk_Alik_Bljk_CB(TENSILE_ARG_NAMES);
    case TT:
        return tensileGetSolutionName_Cijk_Alik_Bjlk_CB(TENSILE_ARG_NAMES);
    case NC:
        return tensileGetSolutionName_Cijk_Ailk_BjlkC_CB(TENSILE_ARG_NAMES);
    case CN:
        return tensileGetSolutionName_Cijk_AlikC_Bljk_CB(TENSILE_ARG_NAMES);
    case TC:
        return tensileGetSolutionName_Cijk_Alik_BjlkC_CB(TENSILE_ARG_NAMES);
    case CT:
        return tensileGetSolutionName_Cijk_AlikC_Bjlk_CB(TENSILE_ARG_NAMES);
    case CC:
        return tensileGetSolutionName_Cijk_AlikC_BjlkC_CB(TENSILE_ARG_NAMES);
    }
}

template <>
const char* tensileGetSolutionName<rocblas_double_complex>(rocblas_operation trans_a,
                                                           rocblas_operation trans_b,
                                                           rocblas_int       strideC1,
                                                           rocblas_int       strideC2,
                                                           rocblas_int       strideA1,
                                                           rocblas_int       strideA2,
                                                           rocblas_int       strideB1,
                                                           rocblas_int       strideB2,
                                                           rocblas_int       sizeI,
                                                           rocblas_int       sizeJ,
                                                           rocblas_int       sizeK,
                                                           rocblas_int       sizeL)
{
    switch(GetTransposeMode(trans_a, trans_b))
    {
    case NN:
        return tensileGetSolutionName_Cijk_Ailk_Bljk_ZB(TENSILE_ARG_NAMES);
    case NT:
        return tensileGetSolutionName_Cijk_Ailk_Bjlk_ZB(TENSILE_ARG_NAMES);
    case TN:
        return tensileGetSolutionName_Cijk_Alik_Bljk_ZB(TENSILE_ARG_NAMES);
    case TT:
        return tensileGetSolutionName_Cijk_Alik_Bjlk_ZB(TENSILE_ARG_NAMES);
    case NC:
        return tensileGetSolutionName_Cijk_Ailk_BjlkC_ZB(TENSILE_ARG_NAMES);
    case CN:
        return tensileGetSolutionName_Cijk_AlikC_Bljk_ZB(TENSILE_ARG_NAMES);
    case TC:
        return tensileGetSolutionName_Cijk_Alik_BjlkC_ZB(TENSILE_ARG_NAMES);
    case CT:
        return tensileGetSolutionName_Cijk_AlikC_Bjlk_ZB(TENSILE_ARG_NAMES);
    case CC:
        return tensileGetSolutionName_Cijk_AlikC_BjlkC_ZB(TENSILE_ARG_NAMES);
    }
}

#undef TENSILE_ARG_NAMES

/* ============================================================================================ */

/*
 * ===========================================================================
 *    template interface
 *    template specialization
 *    call GEMM C interfaces (see gemm.cpp in the same dir)
 * ===========================================================================
 */


/* ============================================================================================ */

template <typename T>
inline rocblas_status rocblas_gemm_strided_batched_template(rocblas_handle      handle,
                                                            rocblas_operation   trans_a,
                                                            rocblas_operation   trans_b,
                                                            rocblas_int         m,
                                                            rocblas_int         n,
                                                            rocblas_int         k,
                                                            const T*            alpha,
                                                            const T*            A,
                                                            rocblas_int         ld_a,
                                                            rocblas_int         stride_a,
                                                            const T*            B,
                                                            rocblas_int         ld_b,
                                                            rocblas_int         stride_b,
                                                            const T*            beta,
                                                            T*                  C,
                                                            rocblas_int         ld_c,
                                                            rocblas_int         stride_c,
                                                            rocblas_int         b_c)
{
    unsigned int strideC1 = unsigned(ld_c);
    unsigned int strideC2 = unsigned(stride_c);
    unsigned int strideA1 = unsigned(ld_a);
    unsigned int strideA2 = unsigned(stride_a);
    unsigned int strideB1 = unsigned(ld_b);
    unsigned int strideB2 = unsigned(stride_b);
    unsigned int sizeI    = unsigned(m);
    unsigned int sizeJ    = unsigned(n);
    unsigned int sizeK    = unsigned(b_c);
    unsigned int sizeL    = unsigned(k);

    hipError_t status = call_tensile<T>(alpha, beta, A, B, C,
                                        trans_a, trans_b,
                                        strideC1, strideC2,
                                        strideA1, strideA2,
                                        strideB1, strideB2,
                                        sizeI, sizeJ, sizeK, sizeL,
                                        handle);

    return get_rocblas_status_for_hip_status(status);
}

/* ============================================================================================ */

template <typename T>
inline rocblas_status rocblas_gemm_batched_template(rocblas_handle            handle,
                                                    rocblas_operation         trans_a,
                                                    rocblas_operation         trans_b,
                                                    rocblas_int               m,
                                                    rocblas_int               n,
                                                    rocblas_int               k,
                                                    const T*                  alpha,
                                                    const T* const            A[],
                                                    rocblas_int               offsetA,
                                                    rocblas_int               ld_a,
                                                    const T* const            B[],
                                                    rocblas_int               offsetB,
                                                    rocblas_int               ld_b,
                                                    const T*                  beta,
                                                    T* const                  C[],
                                                    rocblas_int               offsetC,
                                                    rocblas_int               ld_c,
                                                    rocblas_int               b_c)
{
    rocblas_int stride_a;
    rocblas_int stride_b;
    rocblas_int stride_c;

    infer_batch_strides(trans_a, trans_b, m, n, k, ld_a,
                        &stride_a, ld_b, &stride_b, ld_c, &stride_c);

    unsigned int strideC1 = static_cast<unsigned int>(ld_c);
    unsigned int strideC2 = static_cast<unsigned int>(stride_c);
    unsigned int strideA1 = static_cast<unsigned int>(ld_a);
    unsigned int strideA2 = static_cast<unsigned int>(stride_a);
    unsigned int strideB1 = static_cast<unsigned int>(ld_b);
    unsigned int strideB2 = static_cast<unsigned int>(stride_b);
    unsigned int sizeI    = static_cast<unsigned int>(m);
    unsigned int sizeJ    = static_cast<unsigned int>(n);
    unsigned int sizeK    = 1; // b_c
    unsigned int sizeL    = static_cast<unsigned int>(k);

    size_t sizecopy = b_c * sizeof(T*);
    // Host arrays of device pointers.
    T* hostA[b_c];
    T* hostB[b_c];
    T* hostC[b_c];

    hipError_t errA = hipMemcpy(hostA, A, sizecopy, hipMemcpyDeviceToHost);
    hipError_t errB = hipMemcpy(hostB, B, sizecopy, hipMemcpyDeviceToHost);
    hipError_t errC = hipMemcpy(hostC, C, sizecopy, hipMemcpyDeviceToHost);

    if(get_rocblas_status_for_hip_status(errA) != rocblas_status_success)
        return get_rocblas_status_for_hip_status(errA);
    else if(get_rocblas_status_for_hip_status(errB) != rocblas_status_success)
        return get_rocblas_status_for_hip_status(errB);
    else if(get_rocblas_status_for_hip_status(errC) != rocblas_status_success)
        return get_rocblas_status_for_hip_status(errC);

    hipError_t status;
    for(int i = 0; i < b_c; i++)
    {
        // We cannot do this with a device array, so array of pointers
        // must be on host for now
        status = call_tensile<T>(alpha, beta, hostA[i] + offsetA, hostB[i] + offsetB, hostC[i] + offsetC,
                                trans_a, trans_b,
                                strideC1, strideC2,
                                strideA1, strideA2,
                                strideB1, strideB2,
                                sizeI, sizeJ, sizeK, sizeL,
                                handle);
                

        if(get_rocblas_status_for_hip_status(status) != rocblas_status_success)
            break;
    }

    // clang-format on
    return get_rocblas_status_for_hip_status(status);
}

/* ============================================================================================ */
template <typename T>
inline void rocblas_gemm_strided_batched_kernel_name_template(rocblas_operation trans_a,
                                                              rocblas_operation trans_b,
                                                              rocblas_int       m,
                                                              rocblas_int       n,
                                                              rocblas_int       k,
                                                              rocblas_int       ld_a,
                                                              rocblas_int       stride_a,
                                                              rocblas_int       ld_b,
                                                              rocblas_int       stride_b,
                                                              rocblas_int       ld_c,
                                                              rocblas_int       stride_c,
                                                              rocblas_int       b_c)
{
    unsigned int strideC1 = unsigned(ld_c);
    unsigned int strideC2 = unsigned(stride_c);
    unsigned int strideA1 = unsigned(ld_a);
    unsigned int strideA2 = unsigned(stride_a);
    unsigned int strideB1 = unsigned(ld_b);
    unsigned int strideB2 = unsigned(stride_b);
    unsigned int sizeI    = unsigned(m);
    unsigned int sizeJ    = unsigned(n);
    unsigned int sizeK    = unsigned(b_c);
    unsigned int sizeL    = unsigned(k);

    std::cout << "gemm kernel Name: ";


    const char* solution_name = tensileGetSolutionName<T>(trans_a, trans_b,
                                                          strideC1, strideC2,
                                                          strideA1, strideA2,
                                                          strideB1, strideB2,
                                                          sizeI, sizeJ, sizeK, sizeL);

    std::cout << solution_name << std::endl;
}

/* ============================================================================================ */
template <typename T>
inline void rocblas_gemm_batched_kernel_name_template(rocblas_operation trans_a,
                                                      rocblas_operation trans_b,
                                                      rocblas_int       m,
                                                      rocblas_int       n,
                                                      rocblas_int       k,
                                                      rocblas_int       ld_a,
                                                      rocblas_int       ld_b,
                                                      rocblas_int       ld_c,
                                                      rocblas_int       b_c)
{
    std::cout << "gemm kernel Name: ";
    std::cout << "batched kernels have not yet been implemented" << std::endl;
}




#endif // _GEMM_HOST_HPP_
