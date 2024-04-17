/*

Generated with CMSIS-Stream python scripts.
The generated code is not covered by CMSIS-Stream license.

The support classes and code are covered by CMSIS-Stream license.

*/

#ifndef _CV_SCHEDULER_H_ 
#define _CV_SCHEDULER_H_

#ifdef   __cplusplus
extern "C"
{
#endif



extern int init_buffer_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result);
extern void free_buffer_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result);

extern int init_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result);
extern void free_cv_scheduler(stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result);
extern uint32_t cv_scheduler(int *error,stream_env_t *env,
                              struct_algoResult *alg_result,
                              struct_fm_algoResult_with_fps *alg_fm_result);

#ifdef   __cplusplus
}
#endif

#endif

