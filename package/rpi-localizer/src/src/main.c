/*
 * main.c
 *
 *  Created on: 31 Jan 2020
 *      Author: Hassaan
 */


/**
 * main
 */

#include <bcm_host.h>
#include <sysexits.h>

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/mmal_parameters_camera.h"

int main1(int argc, const char **argv)
{
  int exit_code = EX_OK;

  //MMAL_STATUS_T status = MMAL_SUCCESS;
  //MMAL_PORT_T *camera_video_port = NULL;

  bcm_host_init();

  return exit_code;
}
