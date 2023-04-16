/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_IPC_H
#define GWEN_MSG_ENDPOINT_IPC_H


#include <gwenhywfar/endpoint.h>



#ifdef __cplusplus
extern "C" {
#endif


GWENHYWFAR_API GWEN_MSG_ENDPOINT *GWEN_IpcEndpoint_new(const char *name, int groupId);

GWENHYWFAR_API void GWEN_IpcEndpoint_Extend(GWEN_MSG_ENDPOINT *ep);



#ifdef __cplusplus
}
#endif


#endif


