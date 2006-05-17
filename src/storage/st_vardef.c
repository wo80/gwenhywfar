/* This file is auto-generated from "st_vardef.xml" by the typemaker
   tool of Gwenhywfar. 
   Do not edit this file -- all changes will be lost! */
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "st_vardef_p.h"
#include <gwenhywfar/misc.h>
#include <gwenhywfar/db.h>
#include <gwenhywfar/debug.h>
#include <assert.h>
#include <stdlib.h>
#include <strings.h>

#include <gwenhywfar/gwenhywfarapi.h>


GWEN_LIST_FUNCTIONS(GWEN_STO_VARDEF, GWEN_StoVarDef)




GWEN_STO_VARDEF *GWEN_StoVarDef_new() {
  GWEN_STO_VARDEF *st;

  GWEN_NEW_OBJECT(GWEN_STO_VARDEF, st)
  st->_usage=1;
  GWEN_LIST_INIT(GWEN_STO_VARDEF, st)
  return st;
}


void GWEN_StoVarDef_free(GWEN_STO_VARDEF *st) {
  if (st) {
    assert(st->_usage);
    if (--(st->_usage)==0) {
  if (st->name)
    free(st->name);
  GWEN_LIST_FINI(GWEN_STO_VARDEF, st)
  GWEN_FREE_OBJECT(st);
    }
  }

}


GWEN_STO_VARDEF *GWEN_StoVarDef_dup(const GWEN_STO_VARDEF *d) {
  GWEN_STO_VARDEF *st;

  assert(d);
  st=GWEN_StoVarDef_new();
  if (d->name)
    st->name=strdup(d->name);
  st->minNum=d->minNum;
  st->maxNum=d->maxNum;
  return st;
}


int GWEN_StoVarDef_toDb(const GWEN_STO_VARDEF *st, GWEN_DB_NODE *db) {
  assert(st);
  assert(db);
  if (st->name)
    if (GWEN_DB_SetCharValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "name", st->name))
      return -1;
  if (GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "minNum", st->minNum))
    return -1;
  if (GWEN_DB_SetIntValue(db, GWEN_DB_FLAGS_OVERWRITE_VARS, "maxNum", st->maxNum))
    return -1;
  return 0;
}


int GWEN_StoVarDef_ReadDb(GWEN_STO_VARDEF *st, GWEN_DB_NODE *db) {
  assert(st);
  assert(db);
  GWEN_StoVarDef_SetName(st, GWEN_DB_GetCharValue(db, "name", 0, 0));
  GWEN_StoVarDef_SetMinNum(st, GWEN_DB_GetIntValue(db, "minNum", 0, 0));
  GWEN_StoVarDef_SetMaxNum(st, GWEN_DB_GetIntValue(db, "maxNum", 0, 0));
  return 0;
}


GWEN_STO_VARDEF *GWEN_StoVarDef_fromDb(GWEN_DB_NODE *db) {
  GWEN_STO_VARDEF *st;

  assert(db);
  st=GWEN_StoVarDef_new();
  GWEN_StoVarDef_ReadDb(st, db);
  st->_modified=0;
  return st;
}




const char *GWEN_StoVarDef_GetName(const GWEN_STO_VARDEF *st) {
  assert(st);
  return st->name;
}


void GWEN_StoVarDef_SetName(GWEN_STO_VARDEF *st, const char *d) {
  assert(st);
  if (st->name)
    free(st->name);
  if (d && *d)
    st->name=strdup(d);
  else
    st->name=0;
  st->_modified=1;
}




int GWEN_StoVarDef_GetMinNum(const GWEN_STO_VARDEF *st) {
  assert(st);
  return st->minNum;
}


void GWEN_StoVarDef_SetMinNum(GWEN_STO_VARDEF *st, int d) {
  assert(st);
  st->minNum=d;
  st->_modified=1;
}




int GWEN_StoVarDef_GetMaxNum(const GWEN_STO_VARDEF *st) {
  assert(st);
  return st->maxNum;
}


void GWEN_StoVarDef_SetMaxNum(GWEN_STO_VARDEF *st, int d) {
  assert(st);
  st->maxNum=d;
  st->_modified=1;
}




int GWEN_StoVarDef_IsModified(const GWEN_STO_VARDEF *st) {
  assert(st);
  return st->_modified;
}


void GWEN_StoVarDef_SetModified(GWEN_STO_VARDEF *st, int i) {
  assert(st);
  st->_modified=i;
}


void GWEN_StoVarDef_Attach(GWEN_STO_VARDEF *st) {
  assert(st);
  st->_usage++;
}
GWEN_STO_VARDEF_LIST *GWEN_StoVarDef_List_dup(const GWEN_STO_VARDEF_LIST *stl) {
  if (stl) {
    GWEN_STO_VARDEF_LIST *nl;
    GWEN_STO_VARDEF *e;

    nl=GWEN_StoVarDef_List_new();
    e=GWEN_StoVarDef_List_First(stl);
    while(e) {
      GWEN_STO_VARDEF *ne;

      ne=GWEN_StoVarDef_dup(e);
      assert(ne);
      GWEN_StoVarDef_List_Add(ne, nl);
      e=GWEN_StoVarDef_List_Next(e);
    } /* while (e) */
    return nl;
  }
  else
    return 0;
}




