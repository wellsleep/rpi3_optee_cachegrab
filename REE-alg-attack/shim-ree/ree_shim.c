/**
 * This file is part of the Cachegrab TEE library shim.
 *
 * Copyright (C) 2017 NCC Group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cachegrab.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 Version 1.0
 Keegan Ryan, NCC Group
*/

#include <dlfcn.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <driver.h>

#include "utils.h"

#define ENV_NAME "CACHEGRAB_NAME"
#define ENV_CMDBUF "CACHEGRAB_COMMAND_BUF"
#define ENV_DEBUG "CACHEGRAB_DEBUG"

#define TAG "CACHEGRAB_SHIM: "

int my_func (void);
int REE_run (int trig);

bool load_funcs (void);
void cg_log (const char* fmt, ...);
void cg_buf (const char* name, uint32_t buf, uint32_t len);

static int fd;
static char* target_name;
static uint8_t* target_cbuf;
static size_t target_clen;
static void* target_handle;
static int valid = 0;
static int cg_debug = 0;
static int old_policy = 0;
static struct sched_param old_param;
static int old_sched_valid = 0;

// Get GCC to call this function when library is loaded
__attribute__((constructor))
void init (void) {
  // Initialize shim variables
  int funcs_valid = 0;
  int fd_valid = 0;
  int name_valid = 0;
  int cbuf_valid = 0;
  
  // Initialize debug
  char* dbg = getenv(ENV_DEBUG);
  if (dbg && (0 == strcmp(dbg, "y") || 0 == strcmp(dbg, "Y"))) {
    cg_debug = 1;
  }

  // Initialize functions
  if (load_funcs())
    funcs_valid = 1;

  // Initialize driver file
  fd = open(DEVFILE, O_RDWR);
  if (fd >= 0) {
    fd_valid = 1;
  }

  // Initialize target name
  target_name = getenv(ENV_NAME);
  if (target_name != NULL) {
    name_valid = 1;
  }

  // Initialize command buffer
  char* cbuf = getenv(ENV_CMDBUF);
  if (cbuf != NULL) {
    size_t len = strlen(cbuf);
    target_cbuf = (uint8_t*)malloc(len);
    if (target_cbuf) {
      size_t rv = hex_to_bytes(target_cbuf, cbuf, len);
      target_clen = rv;
      cbuf_valid = 1;
    }
  }
  
  if (fd_valid && funcs_valid && name_valid && cbuf_valid)
    valid = 1;

  if (valid && cg_debug)
    cg_log("Successfully initialized");

  printf("<== hook init done ==>\n\n");
}

bool load_funcs () {
  cg_log("Load (empty) funcs finish!");
  
  return true;
}

void cg_log (const char* fmt, ...) {
  if (!cg_debug)
    return;

  fprintf(stderr, TAG);

  va_list args;
  va_start(args,fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
 
  fprintf(stderr, "\n");
}

void cg_buf (const char* name, uint32_t buf, uint32_t len) {
  if (!cg_debug)
    return;
  
  cg_log("%s:", name);
  fprintf(stderr, "%X\n", buf);
}

void start_intercept() {
  struct sched_param param;

  pthread_t this_thread = pthread_self();
  int ret = pthread_getschedparam(this_thread, &old_policy, &old_param);
  if (ret == 0) {
    old_sched_valid = 1;
    param.sched_priority = sched_get_priority_max(SCHED_FIFO);
    pthread_setschedparam(this_thread, SCHED_FIFO, &param);
  }

  ioctl(fd, CG_SCOPE_ACTIVATE, NULL);
}

void stop_intercept() {
  ioctl(fd, CG_SCOPE_DEACTIVATE, NULL);

  if (!old_sched_valid)
    return;
  pthread_t this_thread = pthread_self();
  int res = pthread_setschedparam(this_thread, old_policy, &old_param);
  if (res < 0)
    return;
  old_sched_valid = 0;
}

int REE_run(int trig) {
  bool intercepting = false;
  int res;
  cg_log("Start to run custom algos");
  cg_buf("CBUF", trig, sizeof(int));
  if (valid &&
      sizeof(trig) >= target_clen) {
    cg_log("Intercepting command");
    intercepting = true;
  }
  cg_log("valid? %d", valid);
  if (intercepting) {
    start_intercept();
  }
  res = my_func();
  if (intercepting) {
    stop_intercept();
  }
  return res;
}

int my_func() {
  int i = 0, sum = 0, j = 0;
  for(i = 0; i < 20000; i++) {
	  for(j = 0; j<20000; j++)
	  sum += i;
  }
  cg_log("sum is %d", sum);
  return 0;
}

