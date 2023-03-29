/****************************************************************************
 * This file is part of the project Gwenhywfar.
 * Gwenhywfar (c) by 2023 Martin Preuss, all rights reserved.
 *
 * The license for this file can be found in the file COPYING which you
 * should have received along with this file.
 ****************************************************************************/

#ifndef GWEN_MSG_ENDPOINT_P_H
#define GWEN_MSG_ENDPOINT_P_H


#include "msgio/endpoint.h"



struct GWEN_MSG_ENDPOINT {
  GWEN_INHERIT_ELEMENT(GWEN_MSG_ENDPOINT)
  GWEN_LIST_ELEMENT(GWEN_MSG_ENDPOINT)

  int fd;
  char *name;
  int groupId;
  int acceptedGroupIds;

  GWEN_MSG_LIST *receivedMessageList;
  GWEN_MSG_LIST *sendMessageList;
  GWEN_MSG *currentlyReceivedMsg;

  GWEN_MSG_ENDPOINT_HANDLEREADABLE_FN handleReadableFn;
  GWEN_MSG_ENDPOINT_HANDLEWRITABLE_FN handleWritableFn;
  GWEN_MSG_ENDPOINT_GET_READFD_FN getReadFdFn;
  GWEN_MSG_ENDPOINT_GET_WRITEFD_FN getWriteFdFn;
  GWEN_MSG_ENDPOINT_RUN_FN runFn;
  GWEN_MSG_ENDPOINT_PROC_OUTMSG_FN processOutMsgFn;
  GWEN_MSG_ENDPOINT_ISMSGCOMPLETE_FN isMsgCompleteFn;
  GWEN_MSG_ENDPOINT_CREATECHILD_FN createChildFn;

  uint32_t flags;
  uint32_t defaultBufferSize;

  int sendingMessage;
};



#endif
