/*
 * cvapp_fd_fm.h
 *
 *  Created on: Feb 15, 2023
 *      Author: bigcat-himax
 */

#ifndef APP_SCENARIO_APP_cmsis_stream_init_H_
#define APP_SCENARIO_APP_cmsis_stream_init_H_

#include "spi_protocol.h"

#include "cv_app_settings.h"

#ifdef __cplusplus
extern "C" {
#endif

int cmsis_stream_init(stream_env_t *env,bool security_enable, bool privilege_enable, uint32_t fd_model_addr, uint32_t fm_model_addr, uint32_t il_model_addr);


int cmsis_stream_deinit();

#ifdef __cplusplus
}
#endif

#endif /* APP_SCENARIO_APP_cmsis_stream_init_H_ */
