/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Thu Nov 06 2003
 copyright   : (C) 2003 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston,                 *
 *   MA  02111-1307  USA                                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef GWENHYWFAR_CRYPTSSL_BF_P_H
#define GWENHYWFAR_CRYPTSSL_BF_P_H


#define GWEN_CRYPTSSL_BF_KEY_SIZE_BYTES 16
#define GWEN_CRYPT_BF_NAME "BF"

#include <gwenhywfar/crypt.h>
#include <openssl/des.h>
#include <openssl/objects.h>


GWEN_ERRORCODE GWEN_CryptKeyBF_Register();


GWEN_CRYPTKEY *GWEN_CryptKeyBF_new();
GWEN_CRYPTKEY *GWEN_CryptKeyBF_dup(const GWEN_CRYPTKEY *key);
void GWEN_CryptKeyBF_FreeKeyData(GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyBF_Encrypt(const GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyBF_Decrypt(const GWEN_CRYPTKEY *key,
                                       GWEN_BUFFER *src,
                                       GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyBF_Sign(const GWEN_CRYPTKEY *key,
                                    GWEN_BUFFER *src,
                                    GWEN_BUFFER *dst);
GWEN_ERRORCODE GWEN_CryptKeyBF_Verify(const GWEN_CRYPTKEY *key,
                                      GWEN_BUFFER *src,
                                      GWEN_BUFFER *signature);
unsigned int GWEN_CryptKeyBF_GetChunkSize(const GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyBF_FromDb(GWEN_CRYPTKEY *key,
                                      GWEN_DB_NODE *db);
GWEN_ERRORCODE GWEN_CryptKeyBF_ToDb(const GWEN_CRYPTKEY *key,
                                    GWEN_DB_NODE *db,
                                    int pub);
GWEN_ERRORCODE GWEN_CryptKeyBF_Generate(GWEN_CRYPTKEY *key,
                                        unsigned keylength);
GWEN_ERRORCODE GWEN_CryptKeyBF_Open(GWEN_CRYPTKEY *key);
GWEN_ERRORCODE GWEN_CryptKeyBF_Close(GWEN_CRYPTKEY *key);


GWEN_ERRORCODE GWEN_CryptKeyBF_Crypt(const GWEN_CRYPTKEY *key,
                                     GWEN_BUFFER *src,
                                     GWEN_BUFFER *dst,
                                     int cryptMode);






#endif

