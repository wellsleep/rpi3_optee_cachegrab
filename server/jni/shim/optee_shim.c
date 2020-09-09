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

typedef struct {
	uint32_t timeLow;
	uint16_t timeMid;
	uint16_t timeHiandVer;
	uint8_t  clockSq[8];
} TEEC_UUID;

int TEEC_InitializeContext(const char *name, void *ctx);
int TEEC_FinalizeContext(void *ctx);
int TEEC_OpenSession (void *ctx, void *session, const TEEC_UUID *dest, uint32_t cnMethod, const void *cnData, void * op, uint32_t *ret);
int TEEC_CloseSession (void *session);
int TEEC_InvokeCommand(void *session, uint32_t cmdID, void *op, uint32_t *ret);

bool load_funcs (void);
void cg_log (const char* fmt, ...);
void cg_buf (const char* name, uint32_t buf, uint32_t len);

int (*TEEC_InitializeContext_orig) (const char *name, void *);
int (*TEEC_FinalizeContext_orig) (void *ctx);
int (*TEEC_OpenSession_orig) (void*, void*, const TEEC_UUID*, uint32_t, const void*, void*, uint32_t*);
int (*TEEC_CloseSession_orig) (void*);
int (*TEEC_InvokeCommand_orig) (void*, uint32_t, void*, uint32_t*);

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
  void *lib = dlopen("/lib/libteec.so.1.0.origin", RTLD_LAZY);
  if (!lib)
    return false;
  
  cg_log("Successfully opened library.");

  TEEC_InitializeContext_orig =			\
    (int (*) (const char *, void *)) \
    dlsym(lib, "TEEC_InitializeContext");
  if (!TEEC_InitializeContext_orig)
    return false;
  cg_log("Successfully hooked TEEC_InitializeContext");

  TEEC_FinalizeContext_orig =			\
    (int (*) (void*)) \
    dlsym(lib, "TEEC_FinalizeContext");
  if (!TEEC_FinalizeContext_orig)
    return false;
  cg_log("Successfully hooked TEEC_FinalizeContext");

  TEEC_OpenSession_orig =			\
    (int (*) (void*, void*, const TEEC_UUID*, uint32_t, const void*, void*, uint32_t*)) \
    dlsym(lib, "TEEC_OpenSession");
  if (!TEEC_OpenSession_orig)
    return false;
  cg_log("Successfully hooked TEEC_OpenSession");

  TEEC_CloseSession_orig =			\
    (int (*) (void *))				\
    dlsym(lib, "TEEC_CloseSession");
  if (!TEEC_CloseSession_orig)
    return false;
  cg_log("Successfully hooked TEEC_CloseSession");

  TEEC_InvokeCommand_orig =					\
    (int (*) (void*, uint32_t, void*, uint32_t*))		\
    dlsym(lib, "TEEC_InvokeCommand");
  if (!TEEC_InvokeCommand_orig)
    return false;
  cg_log("Successfully hooked TEEC_InvokeCommand");
  
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

int TEEC_InitializeContext(const char *name, void *ctx) {
  cg_log("Initialze Ctx");
  int res = TEEC_InitializeContext_orig(name, ctx);
  cg_log("InitializeContext return %x", res);

  return res;
}
int TEEC_FinalizeContext(void *ctx) {

  cg_log("Finalize Ctx");
  int res = TEEC_FinalizeContext_orig(ctx);
  cg_log("FinalizeContext return %x", res);

  return res;
}

int TEEC_OpenSession (void *ctx, void *session, const TEEC_UUID *dest, uint32_t cnMethod, const void *cnData, void * op, uint32_t *ret) {
  cg_log("Starting UUID %lx", (*dest).timeLow);
  int res = TEEC_OpenSession_orig(ctx, session, dest, cnMethod, cnData, op, ret);
  cg_log("TEEC_OpenSession returned session %p", session);
  if (valid &&
      session != NULL &&
      target_handle == NULL) {
    target_handle = session;
  }
  return res;
}

int TEEC_CloseSession (void *session) {
  cg_log("Shutting down handle %p", session);
  if (valid &&
      target_handle != NULL &&
      target_handle == session) {
    target_handle = NULL;
  }
  return TEEC_CloseSession_orig(session);
}

int TEEC_InvokeCommand(void *session, uint32_t cmdID, void *op, uint32_t *ret) {
  bool intercepting = false;
  int res;
  cg_log("Sending command to handle %p", session);
  cg_buf("CBUF", cmdID, sizeof(uint32_t));
  if (valid &&
      target_handle != NULL &&
      sizeof(cmdID) >= target_clen &&
      0 == memcmp(&cmdID, target_cbuf, target_clen)) {
    cg_log("Intercepting command");
    intercepting = true;
  }
  if (intercepting) {
    start_intercept();
  }
  res = TEEC_InvokeCommand_orig(session, cmdID, op, ret);
  if (intercepting) {
    stop_intercept();
  }
  return res;
}

