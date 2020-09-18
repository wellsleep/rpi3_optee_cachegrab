/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#define STR_TRACE_USER_TA "ECC_DEMO"


#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "ta.h"
#include <tee_ta_api.h>
#include <string.h>
#include <trace.h>

//static uint32_t keysize;
static uint32_t algorithm;
static uint32_t mode;
static uint32_t maxKeySize;

static uint8_t ecc_x[32] = {
		0xa8, 0xb3, 0xb2, 0x84, 0xaf, 0x8e, 0xb5, 0x0b,
		0xa8, 0xb3, 0xb2, 0x84, 0xaf, 0x8e, 0xb5, 0x0b
};
static uint8_t ecc_y[32] = {
		0x53, 0x33, 0x9c, 0xfd, 0xb7, 0x9f, 0xc8, 0x46,
		0x53, 0x33, 0x9c, 0xfd, 0xb7, 0x9f, 0xc8, 0x46
};
static uint8_t ecc_k[32] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01
};

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */

/* object and operation handle */
static TEE_ObjectHandle eccHandle;
static TEE_OperationHandle eccOperation;

TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{

	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param  params[4],
		void **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	if (eccOperation)
		TEE_FreeOperation(eccOperation);
	DMSG("Goodbye!\n");
}


/*=-ECC algorithm-=*/
static TEE_Result ecc_algorithm(uint32_t param_types,
	TEE_Param params[4])
{

	TEE_Result res;
	uint32_t objectType;
	uint32_t maxObjectSize;
	TEE_Attribute eccAttr[4];

//	char *p = params[0].memref.buffer;
//	char *q = params[1].memref.buffer;

	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* define algorithm and mode apply RSA operation */
	//if(params[2].value.a == 1)
		//setRSAParams(TEE_ALG_RSA_NOPAD,TEE_MODE_ENCRYPT,1024,2048);
		//setRSAParams(TEE_ALG_RSA_NOPAD,TEE_MODE_DECRYPT,1024,2048);

	/* allocate operation */
	algorithm = TEE_ALG_ECDSA_P256; //to define hash, follow GP_core_API@p171
	mode = TEE_MODE_SIGN;
	maxKeySize = 256;

	res = TEE_AllocateOperation(&eccOperation,algorithm,mode,maxKeySize);
	if (res != TEE_SUCCESS)
		DMSG("allocate operation failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("allocate operation succeed with code 0x%x", res);


	/* allocate public transient object */
	objectType = TEE_TYPE_ECDSA_KEYPAIR;
	maxObjectSize = 256;
	res = TEE_AllocateTransientObject(objectType,
					maxObjectSize,
					&eccHandle);
	if (res != TEE_SUCCESS)
		DMSG("allocate transient object failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("allocate pub transient object succeed with code 0x%x", res);

	/* import RSA keys and components */
	TEE_InitValueAttribute(&eccAttr[0],
			     TEE_ATTR_ECC_CURVE,
			     TEE_ECC_CURVE_NIST_P256,
			     sizeof(size_t));
	TEE_InitRefAttribute(&eccAttr[1],
			     TEE_ATTR_ECC_PUBLIC_VALUE_X,
				 ecc_x,
			     sizeof(ecc_x));
	TEE_InitRefAttribute(&eccAttr[2],
				 TEE_ATTR_ECC_PUBLIC_VALUE_Y,
				 ecc_y,
			     sizeof(ecc_y));
	TEE_InitRefAttribute(&eccAttr[3],
			     TEE_ATTR_ECC_PRIVATE_VALUE,
				 ecc_k,
			     sizeof(ecc_k));


	/* populate transient object */
	res = TEE_PopulateTransientObject(eccHandle,eccAttr,(uint32_t)sizeof(eccAttr)/sizeof(TEE_Attribute));
	if (res != TEE_SUCCESS)
		DMSG("populate failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("populate succeed with code 0x%x", res);


	/* set operation key */
	res = TEE_SetOperationKey(eccOperation,eccHandle);
	if (res != TEE_SUCCESS)
		DMSG("set operation key with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("set operation key succeed with code 0x%x", res);

	/* free object */
	TEE_FreeTransientObject(eccHandle);

//	// test purpose only
//
//	for(int j=0; j<64; j++) {
//		*q = (*p+1);
//		q++;
//		p++;
//	}

	/* Asymmetric encrypt */
	res = TEE_AsymmetricSignDigest(eccOperation,
				    eccAttr,
				    (uint32_t)sizeof(eccAttr)/sizeof(TEE_Attribute),
				    params[0].memref.buffer,
				    params[0].memref.size,
				    params[1].memref.buffer,
				    &params[1].memref.size);
//	// test purpose only
//
//	for(int j=0; j<63; j++) {
//		*q = (*p+1);
//		q++;
//		p++;
//	}
//	*q = 0x02;
//	res = TEE_SUCCESS;

	if (res != TEE_SUCCESS)
		DMSG("AsymmetricEncrypt failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("AsymmetricEncrypt succeed with code 0x%x", res);

	/* free RSA operatipon */
	TEE_FreeOperation(eccOperation);

	return TEE_SUCCESS;
}


/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
// add new function
	case TA_SIGN_DEMO_CMD_ECC:
		return ecc_algorithm(param_types, params);

	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
