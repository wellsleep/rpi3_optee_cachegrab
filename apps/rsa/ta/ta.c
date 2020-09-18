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

#define STR_TRACE_USER_TA "RSA_DEMO"


#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>
#include "ta.h"
#include <tee_ta_api.h>
#include <string.h>
#include <trace.h>

static uint32_t keysize;
static uint32_t algorithm;
static uint32_t mode;
static uint32_t maxKeySize;


/* an example of RSA keys and componentes */
 static  uint8_t ac_rsaes_pkcs1_v1_5_example1_modulus[] = {
	0xa8, 0xb3, 0xb2, 0x84, 0xaf, 0x8e, 0xb5, 0x0b, 0x38, 0x70, 0x34, 0xa8,
	0x60, 0xf1, 0x46, 0xc4, 0x91, 0x9f, 0x31, 0x87, 0x63, 0xcd, 0x6c, 0x55,
	0x98, 0xc8, 0xae, 0x48, 0x11, 0xa1, 0xe0, 0xab, 0xc4, 0xc7, 0xe0, 0xb0,
	0x82, 0xd6, 0x93, 0xa5, 0xe7, 0xfc, 0xed, 0x67, 0x5c, 0xf4, 0x66, 0x85,
	0x12, 0x77, 0x2c, 0x0c, 0xbc, 0x64, 0xa7, 0x42, 0xc6, 0xc6, 0x30, 0xf5,
	0x33, 0xc8, 0xcc, 0x72, 0xf6, 0x2a, 0xe8, 0x33, 0xc4, 0x0b, 0xf2, 0x58,
	0x42, 0xe9, 0x84, 0xbb, 0x78, 0xbd, 0xbf, 0x97, 0xc0, 0x10, 0x7d, 0x55,
	0xbd, 0xb6, 0x62, 0xf5, 0xc4, 0xe0, 0xfa, 0xb9, 0x84, 0x5c, 0xb5, 0x14,
	0x8e, 0xf7, 0x39, 0x2d, 0xd3, 0xaa, 0xff, 0x93, 0xae, 0x1e, 0x6b, 0x66,
	0x7b, 0xb3, 0xd4, 0x24, 0x76, 0x16, 0xd4, 0xf5, 0xba, 0x10, 0xd4, 0xcf,
	0xd2, 0x26, 0xde, 0x88, 0xd3, 0x9f, 0x16, 0xfb
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_pub_exp[] = {
	0x01, 0x00, 0x01
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_priv_exp[] = {
	0x53, 0x33, 0x9c, 0xfd, 0xb7, 0x9f, 0xc8, 0x46, 0x6a, 0x65, 0x5c, 0x73,
	0x16, 0xac, 0xa8, 0x5c, 0x55, 0xfd, 0x8f, 0x6d, 0xd8, 0x98, 0xfd, 0xaf,
	0x11, 0x95, 0x17, 0xef, 0x4f, 0x52, 0xe8, 0xfd, 0x8e, 0x25, 0x8d, 0xf9,
	0x3f, 0xee, 0x18, 0x0f, 0xa0, 0xe4, 0xab, 0x29, 0x69, 0x3c, 0xd8, 0x3b,
	0x15, 0x2a, 0x55, 0x3d, 0x4a, 0xc4, 0xd1, 0x81, 0x2b, 0x8b, 0x9f, 0xa5,
	0xaf, 0x0e, 0x7f, 0x55, 0xfe, 0x73, 0x04, 0xdf, 0x41, 0x57, 0x09, 0x26,
	0xf3, 0x31, 0x1f, 0x15, 0xc4, 0xd6, 0x5a, 0x73, 0x2c, 0x48, 0x31, 0x16,
	0xee, 0x3d, 0x3d, 0x2d, 0x0a, 0xf3, 0x54, 0x9a, 0xd9, 0xbf, 0x7c, 0xbf,
	0xb7, 0x8a, 0xd8, 0x84, 0xf8, 0x4d, 0x5b, 0xeb, 0x04, 0x72, 0x4d, 0xc7,
	0x36, 0x9b, 0x31, 0xde, 0xf3, 0x7d, 0x0c, 0xf5, 0x39, 0xe9, 0xcf, 0xcd,
	0xd3, 0xde, 0x65, 0x37, 0x29, 0xea, 0xd5, 0xd1
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_prime1[] = {
	0xd3, 0x27, 0x37, 0xe7, 0x26, 0x7f, 0xfe, 0x13, 0x41, 0xb2, 0xd5, 0xc0,
	0xd1, 0x50, 0xa8, 0x1b, 0x58, 0x6f, 0xb3, 0x13, 0x2b, 0xed, 0x2f, 0x8d,
	0x52, 0x62, 0x86, 0x4a, 0x9c, 0xb9, 0xf3, 0x0a, 0xf3, 0x8b, 0xe4, 0x48,
	0x59, 0x8d, 0x41, 0x3a, 0x17, 0x2e, 0xfb, 0x80, 0x2c, 0x21, 0xac, 0xf1,
	0xc1, 0x1c, 0x52, 0x0c, 0x2f, 0x26, 0xa4, 0x71, 0xdc, 0xad, 0x21, 0x2e,
	0xac, 0x7c, 0xa3, 0x9d
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_prime2[] = {
	0xcc, 0x88, 0x53, 0xd1, 0xd5, 0x4d, 0xa6, 0x30, 0xfa, 0xc0, 0x04, 0xf4,
	0x71, 0xf2, 0x81, 0xc7, 0xb8, 0x98, 0x2d, 0x82, 0x24, 0xa4, 0x90, 0xed,
	0xbe, 0xb3, 0x3d, 0x3e, 0x3d, 0x5c, 0xc9, 0x3c, 0x47, 0x65, 0x70, 0x3d,
	0x1d, 0xd7, 0x91, 0x64, 0x2f, 0x1f, 0x11, 0x6a, 0x0d, 0xd8, 0x52, 0xbe,
	0x24, 0x19, 0xb2, 0xaf, 0x72, 0xbf, 0xe9, 0xa0, 0x30, 0xe8, 0x60, 0xb0,
	0x28, 0x8b, 0x5d, 0x77
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_exp1[] = {
	0x0e, 0x12, 0xbf, 0x17, 0x18, 0xe9, 0xce, 0xf5, 0x59, 0x9b, 0xa1, 0xc3,
	//0xf1, 0xed, 0xbf, 0x17, 0x18, 0xe9, 0xce, 0xf5, 0x59, 0x9b, 0xa1, 0xc3,
	0x88, 0x2f, 0xe8, 0x04, 0x6a, 0x90, 0x87, 0x4e, 0xef, 0xce, 0x8f, 0x2c,
	0xcc, 0x20, 0xe4, 0xf2, 0x74, 0x1f, 0xb0, 0xa3, 0x3a, 0x38, 0x48, 0xae,
	0xc9, 0xc9, 0x30, 0x5f, 0xbe, 0xcb, 0xd2, 0xd7, 0x68, 0x19, 0x96, 0x7d,
	0x46, 0x71, 0xac, 0xc6, 0x43, 0x1e, 0x40, 0x37, 0x96, 0x8d, 0xb3, 0x78,
	0x78, 0xe6, 0x95, 0xc1
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_exp2[] = {
	0x95, 0x29, 0x7b, 0x0f, 0x95, 0xa2, 0xfa, 0x67, 0xd0, 0x07, 0x07, 0xd6,
	0x09, 0xdf, 0xd4, 0xfc, 0x05, 0xc8, 0x9d, 0xaf, 0xc2, 0xef, 0x6d, 0x6e,
	0xa5, 0x5b, 0xec, 0x77, 0x1e, 0xa3, 0x33, 0x73, 0x4d, 0x92, 0x51, 0xe7,
	0x90, 0x82, 0xec, 0xda, 0x86, 0x6e, 0xfe, 0xf1, 0x3c, 0x45, 0x9e, 0x1a,
	0x63, 0x13, 0x86, 0xb7, 0xe3, 0x54, 0xc8, 0x99, 0xf5, 0xf1, 0x12, 0xca,
	0x85, 0xd7, 0x15, 0x83
};
static  uint8_t ac_rsaes_pkcs1_v1_5_example1_coeff[] = {
	0x4f, 0x45, 0x6c, 0x50, 0x24, 0x93, 0xbd, 0xc0, 0xed, 0x2a, 0xb7, 0x56,
	0xa3, 0xa6, 0xed, 0x4d, 0x67, 0x35, 0x2a, 0x69, 0x7d, 0x42, 0x16, 0xe9,
	0x32, 0x12, 0xb1, 0x27, 0xa6, 0x3d, 0x54, 0x11, 0xce, 0x6f, 0xa9, 0x8d,
	0x5d, 0xbe, 0xfd, 0x73, 0x26, 0x3e, 0x37, 0x28, 0x14, 0x27, 0x43, 0x81,
	0x81, 0x66, 0xed, 0x7d, 0xd6, 0x36, 0x87, 0xdd, 0x2a, 0x8c, 0xa1, 0xd2,
	0xf4, 0xfb, 0xd8, 0xe1
};

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */

/* object and operation handle */
static TEE_ObjectHandle rsaHandle;
static TEE_OperationHandle rsaOperation;

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
	if (rsaOperation)
		TEE_FreeOperation(rsaOperation);
	DMSG("Goodbye!\n");
}

/* =-set RSA operation parameters-= */
static int setRSAParams(uint32_t selectedAlgorithm, uint32_t operationMode, uint32_t selectedKeySize,uint32_t selectedMaxKeysize)
{
	switch	(selectedAlgorithm)
	{
	case TEE_ALG_RSA_NOPAD:
		algorithm = TEE_ALG_RSA_NOPAD;
		break;
	case TEE_ALG_RSAES_PKCS1_V1_5:
		algorithm = TEE_ALG_RSAES_PKCS1_V1_5;
		break;
	case TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA1:
		algorithm = TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA1;
		break;
	case TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA224:
		algorithm = TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA224;
		break;
	case TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA256:
		algorithm = TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA256;
		break;
	case TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA384:
		algorithm = TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA384;
		break;
	case TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA512:
		algorithm = TEE_ALG_RSAES_PKCS1_OAEP_MGF1_SHA512;
		break;
	default:
		algorithm = TEE_ALG_RSA_NOPAD;
		break;
	}
	switch(operationMode)
	{
	case TEE_MODE_ENCRYPT:
		mode = TEE_MODE_ENCRYPT;
		break;
	case TEE_MODE_DECRYPT:
		mode = TEE_MODE_DECRYPT;
		break;
	default:
		mode = TEE_MODE_ENCRYPT;
		break;
	}
	switch(selectedKeySize)
	{
	case 1024:
		keysize = 1024;
		break;
	case 2048:
		keysize = 2048;
		break;
	default :
		keysize = 1024;
		break;

	}
	switch (selectedMaxKeysize)
	{
	case 2048:
		maxKeySize = 2048;
		break;
	case 4096:
		maxKeySize = 4096;
		break;
	default:
   		maxKeySize = 2048;
		break;
	}
	return 0;
}

/*=-RSA algorithm-=*/
static TEE_Result rsa_algorithm(uint32_t param_types,
	TEE_Param params[4])
{

	TEE_Result res;
	uint32_t objectType;
	uint32_t maxObjectSize;
	TEE_Attribute rsaAttr[8];
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* define algorithm and mode apply RSA operation */
	if(params[2].value.a == 1)
		//setRSAParams(TEE_ALG_RSA_NOPAD,TEE_MODE_ENCRYPT,1024,2048);
		setRSAParams(TEE_ALG_RSA_NOPAD,TEE_MODE_DECRYPT,1024,2048);

	/* allcate operation */
	res = TEE_AllocateOperation(&rsaOperation,algorithm,mode,maxKeySize);
	if (res != TEE_SUCCESS)
		DMSG("allocate operation failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("allocate operation succeed with code 0x%x", res);

	/* allocate public transient object */
	objectType = TEE_TYPE_RSA_KEYPAIR;
	maxObjectSize = 2048;
	res = TEE_AllocateTransientObject(objectType,
					maxObjectSize,
					&rsaHandle);
	if (res != TEE_SUCCESS)
		DMSG("allocate transient object failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("allocate pub transient object succeed with code 0x%x", res);

	/* import RSA keys and components */
	TEE_InitRefAttribute(&rsaAttr[0],
			     TEE_ATTR_RSA_MODULUS,
			     ac_rsaes_pkcs1_v1_5_example1_modulus,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_modulus));
	TEE_InitRefAttribute(&rsaAttr[1],
			     TEE_ATTR_RSA_PUBLIC_EXPONENT,
			     ac_rsaes_pkcs1_v1_5_example1_pub_exp,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_pub_exp));
	TEE_InitRefAttribute(&rsaAttr[2],
			     TEE_ATTR_RSA_PRIVATE_EXPONENT,
			     ac_rsaes_pkcs1_v1_5_example1_priv_exp,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_priv_exp));
	TEE_InitRefAttribute(&rsaAttr[3],
			     TEE_ATTR_RSA_PRIME1,
			     ac_rsaes_pkcs1_v1_5_example1_prime1,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_prime1));
	TEE_InitRefAttribute(&rsaAttr[4],
			     TEE_ATTR_RSA_PRIME2,
			     ac_rsaes_pkcs1_v1_5_example1_prime2,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_prime2));
	TEE_InitRefAttribute(&rsaAttr[5],
			     TEE_ATTR_RSA_EXPONENT1,
			     ac_rsaes_pkcs1_v1_5_example1_exp1,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_exp1));
	TEE_InitRefAttribute(&rsaAttr[6],
			     TEE_ATTR_RSA_EXPONENT2,
			     ac_rsaes_pkcs1_v1_5_example1_exp2,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_exp2));
	TEE_InitRefAttribute(&rsaAttr[7],
			     TEE_ATTR_RSA_COEFFICIENT,
			     ac_rsaes_pkcs1_v1_5_example1_coeff,
			     sizeof(ac_rsaes_pkcs1_v1_5_example1_coeff));

	/* populate transient object */
	res = TEE_PopulateTransientObject(rsaHandle,rsaAttr,(uint32_t)sizeof(rsaAttr)/sizeof(TEE_Attribute));
	if (res != TEE_SUCCESS)
		DMSG("populate failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("populate succeed with code 0x%x", res);


	/* set operation key */
	res = TEE_SetOperationKey(rsaOperation,rsaHandle);
	if (res != TEE_SUCCESS)
		DMSG("set operation key with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("set operation key succeed with code 0x%x", res);

	/* free object */
	TEE_FreeTransientObject(rsaHandle);

	/* Asymmetric encrypt */
	res = TEE_AsymmetricDecrypt(rsaOperation,
				    rsaAttr,
				    (uint32_t)sizeof(rsaAttr)/sizeof(TEE_Attribute),
				    params[0].memref.buffer,
				    params[0].memref.size,
				    params[1].memref.buffer,
				    &params[1].memref.size);

	if (res != TEE_SUCCESS)
		DMSG("AsymmetricEncrypt failed with code 0x%x", res);
	if (res == TEE_SUCCESS)
		DMSG("AsymmetricEncrypt succeed with code 0x%x", res);

	/* free RSA operatipon */
	TEE_FreeOperation(rsaOperation);

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
	case TA_DECRYPT_DEMO_CMD_RSA:
		return rsa_algorithm(param_types, params);

	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
