/**
  ******************************************************************************
  * @file    NeaiDPU.h
  * @author  SRA - MCD
  * @brief   Digital processing Unit specialized for the NanoEdgeAI library  *
  * This DPU process the data using neai library generated by NanoEdgeAI studio.  *
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
  
#ifndef DPU_INC_NEAIDPU_H_
#define DPU_INC_NEAIDPU_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "ADPU.h"
#include "ADPU_vtbl.h"
#include "NanoEdgeAI.h"
#include "NanoEdgeAI_ncc.h"


/**
 * Specifies the execution mode for the DPU. The execution mode tells the DPU what to do when it is running
 * and new signals provided by the data source is ready to be processed by the AI library.
 */
typedef enum _ENeaiMode {
  E_NEAI_MODE_NONE = 0,  //!< no execution mode is selected.
  E_NEAI_ANOMALY_LEARN,
  E_NEAI_ANOMALY_DETECT,
  E_NEAI_ONE_CLASS,
  E_NEAI_CLASSIFICATION,
  E_NEAI_EXTRAPOLATION
} ENeaiMode_t;

typedef struct
{
  enum neai_state (*anomalyLearn)(float *);
  enum neai_state (*anomalyDetect)(float *,uint8_t *);
  enum neai_state (*oneClass)(float *, uint8_t *);
  enum neai_state (*classification)(float *,float *,uint16_t *);
  enum neai_state (*extrapolation)(float *, float *);
} NEAI_ProcFunction_t;

typedef struct
{
  enum neai_state (*anomalyInit)(void);
  enum neai_state (*oneClassInit)(const float *);
  enum neai_state (*classificationInit)(const float *);
  enum neai_state (*extrapolationInit)(const float *);
} NEAI_ProcInitFunction_t;



/**
 * Create  type name for _NeaiDPU_t.
 */
typedef struct _NeaiDPU_t NeaiDPU_t;

/**
 * NeaiDPU_t internal state.
 * It declares only the virtual table used to implement the inheritance.
 */
struct _NeaiDPU_t {
  /**
   * Base class object.
   */
  ADPU super;

  /**
   * Specifies if the in and out stream of teh DPU have been initialized.
   * To initialize the streams the application must use
   */
  boolean_t stream_ready;

  /**
   * Specifies the sensitivity of the model in detection mode. It can be tuned at any time without having to go through a new learning phase.
   */
  float sensitivity;

  /**
   * Specifies the output data from neai library. We use float to make all scalar DPU out format uniform.
   */
  float neai_out;

  /**
   * Specifies NanoEdgeAI processing function to use in order to process the signals.
   */
  ENeaiMode_t             proc_mode;
  NEAI_ProcInitFunction_t proc_init;
  NEAI_ProcFunction_t     proc;
};


/* Public API declaration */
/**************************/

/**
 * Allocate an instance of NeaiDPU_t in the heap.
 *
 * @return a pointer to the generic object ::IDPU if success,
 * or NULL if out of memory error occurs.
 */
IDPU *NeaiDPUAlloc(void);

/**
 * Allocate an instance of NeaiDPU_t in a memory block specified by the application.
 * The size of the memory block must be greater or equal to sizeof(NeaiDPU_t).
 * This allocator allows the application to avoid the dynamic allocation.
 *
 * \code
 * NeaiDPU_t dpu;
 * NeaiDPUStaticAlloc(&dpu);
 * \endcode
 *
 * @param p_mem_block [IN] specify a memory block allocated by the application.
 *        The size of the memory block must be greater or equal to sizeof(NeaiDPU_t).
 * @return a pointer to the generic object ::IDPU if success,
 * or NULL if out of memory error occurs.
 */
IDPU *NeaiDPUStaticAlloc(void *p_mem_block);


/**
 * Compute the size in byte of the buffer that the application need to allocate and pass
 * to the DPU when it is attached to a sensor or to another DPU.
 *
 * \anchor fig400 \image html 400_api_DPUSetInputParams.png "Fig.400 - API - NeaiDPUSetInputParam() " width=630px
 *
 * @param _this [IN] specifies a pointer to the object.
 * @param signal_size [IN] specifies the number of input elements that the DPU must collect before starting the processing phase.
 * @param axes [IN] specifies the number of axes (or dimension) of each element.
 * @param cb_items [IN] specifies the number of items for the ::CircularBuffer used by the DPU.
 *                      An item of the ::CirculaBuffer is used to store `size` elements from the input source.
 * @return The size in byte of the buffer need by the DPU to acquire and process input data if success, zero otherwise.
 */
uint32_t NeaiDPUSetStreamsParam(NeaiDPU_t *_this, uint16_t signal_size, uint8_t axes, uint8_t cb_items);

/**
 * Set the processing mode for the DPU. It specifies to the DPU if a new signal is used
 * to learn and improve the model, or to detect anomalies.
 *
 * @param _this [IN] specifies a pointer to the object.
 * @param mode [IN] specifies the processing mode. Valid value are:
 *  - E_NEAI_LEARNING
 *  - E_NEAI_DETECTION
 * @return SYS_NO_ERROR_CODE if success, an error code otherwise.
 */
sys_error_code_t NeaiDPUSetProcessingMode(NeaiDPU_t *_this, ENeaiMode_t mode);

/**
 * Initializes the DPU for the processing mode selected by NeaiDPUSetProcessingMode
 *
 * @param _this [IN] specifies a pointer to the object.
 * @return SYS_NO_ERROR_CODE if success, an error code otherwise.
 */
sys_error_code_t NeaiDPUProcessingInitialize(NeaiDPU_t *_this);

/**
 * This function sets the sensitivity of the model in detection mode.
 * It can be tuned at any time without having to go through a new learning phase.
 *
 * @param _this [IN] specifies a pointer to the object.
 * @param sensitivity [IN] specifies the sensitivity of the model.
 * The default sensitivity value is 1. A sensitivity value between 0 and 1 (excluded)
 * decreases the sensitivity of the model, while a value in between 1 and 100 increases it.
 * @return SYS_NO_ERROR_CODE if success, an error code otherwise.
 */
sys_error_code_t NeaiDPUSetSensitivity(NeaiDPU_t *_this, float sensitivity);

/**
 * Get the actual processing mode for the DPU.
 *
 * @param _this [IN] specifies a pointer to the object.
 * @return the actual processing mode of the DPU.
 */
inline ENeaiMode_t NeaiDPUGetProcessingMode(NeaiDPU_t *_this);

/**
 * Get the result of the last processed signal. The value depend on the mode.
 *
 * @param _this  [IN] specifies a pointer to the object.
 * @return the result of the last processed signal.
 */
inline float NeaiDPUGetProcessResult(NeaiDPU_t *_this);

/**
 * Partial reset of the DPU internal state: all input and output buffers are re-initialized to prepare
 * the DPU to process a new stream of data.
 *
 * @param _this [IN] specifies a pointer to the object.
 * @return SYS_NO_ERROR_CODE if success, an error code otherwise.
 */
sys_error_code_t NeaiDPUPrepareToProcessData(NeaiDPU_t *_this);


/* Inline functions definition */
/*******************************/

SYS_DEFINE_INLINE
ENeaiMode_t NeaiDPUGetProcessingMode(NeaiDPU_t *_this)
{
  assert_param(_this != NULL);
  return _this->proc_mode;
}

SYS_DEFINE_INLINE
float NeaiDPUGetProcessResult(NeaiDPU_t *_this)
{
  assert_param(_this != NULL);
  return _this->neai_out;
}

#ifdef __cplusplus
}
#endif

#endif /* DPU_INC_NEAIDPU_H_ */
