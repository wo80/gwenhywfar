/***************************************************************************
    begin       : Thu Jul 02 2009
    copyright   : (C) 2025 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "tm2c_tree2.h"
#include "tm2c_misc.h"

#include <gwenhywfar/debug.h>

#include <ctype.h>


#define GBAA GWEN_Buffer_AppendArgs
#define GBAS GWEN_Buffer_AppendString


static void _addGetByMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static int _addGetByMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addGetByMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);

static void _addSortByMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addSortByMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm);
static void _addSortByMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf);
static void _addCompareMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, int ascending);
static int _addCompareMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, int ascending);
static void _addCompareMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf, int ascending);




int TM2C_BuildTree2GetByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  int rv;

  _addGetByMemberProtoType(tb, ty, tm);
  rv=_addGetByMemberImplementation(tb, ty, tm);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



int TM2C_BuildTree2SortByMember(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  int rv;

  _addSortByMemberProtoType(tb, ty, tm);
  _addCompareMemberProtoType(tb, ty, tm, 1);         /* ascending */
  _addCompareMemberProtoType(tb, ty, tm, 0);         /* descending */

  _addSortByMemberImplementation(tb, ty, tm);
  rv=_addCompareMemberImplementation(tb, ty, tm, 1); /* ascending */
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }
  rv=_addCompareMemberImplementation(tb, ty, tm, 0); /* descending */
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  return 0;
}



void _addGetByMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s)
    GBAA(tbuf, "%s ", s);
  _addGetByMemberDeclaration(ty, tm, tbuf);
  GBAS(tbuf, ";\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _addGetByMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  _addGetByMemberDeclaration(ty, tm, tbuf);
  GBAS(tbuf, "{\n");

  GBAA(tbuf,   "  %s *p_struct;\n\n", sTypeId);
  GBAS(tbuf, "  assert(p_object);\n");
  GBAA(tbuf,   "  p_struct = %s_Tree2_GetFirstChild(p_object);\n", sTypePrefix);
  GBAS(tbuf, "  while(p_struct) {\n");
  GBAS(tbuf, "    int p_rv;\n");
  GBAS(tbuf, "\n");

  GBAS(tbuf, "    ");
  if (1) {
    GWEN_BUFFER *dstbuf;
    GWEN_BUFFER *srcbuf;
    int rv;

    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GBAS(srcbuf, "p_cmp");

    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GBAA(dstbuf, "p_struct->%s", sMemberName);

    rv=Typemaker2_Builder_Invoke_CompareFn(tb, ty, tm,
                                           GWEN_Buffer_GetStart(srcbuf),
                                           GWEN_Buffer_GetStart(dstbuf),
                                           tbuf);
    GWEN_Buffer_free(srcbuf);
    GWEN_Buffer_free(dstbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GBAS(tbuf, "\n");
  }


  GBAS(tbuf, "    if (p_rv == 0)\n");
  GBAS(tbuf, "      return p_struct;\n");
  GBAA(tbuf,   "    p_struct = %s_Tree2_GetBelow(p_struct);\n", sTypePrefix);
  GBAS(tbuf, "  }\n");

  GBAS(tbuf, "  return NULL;\n");
  GBAS(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addGetByMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;
  const char *sMemberTypeId;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);
  sMemberTypeId=Typemaker2_Type_GetIdentifier(mty);

  GBAA(tbuf,
                         "%s *%s_Tree2_GetBy%c%s(const %s *p_object, ",
                         sTypeId,
                         sTypePrefix,
                         toupper(*sMemberName),
                         sMemberName+1,
                         sTypeId);
  if (Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Pointer ||
      Typemaker2_Type_GetType(mty)==TypeMaker2_Type_Array)
    GBAA(tbuf, " const %s *", sMemberTypeId);
  else
    GBAA(tbuf, "%s ", sMemberTypeId);
  GBAS(tbuf, "p_cmp)");
}




/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * sort
 * ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */


void _addSortByMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  GWEN_BUFFER *tbuf;
  const char *s;
  TYPEMAKER2_TYPEMANAGER *tym;
  TYPEMAKER2_TYPE *mty;

  tym=Typemaker2_Builder_GetTypeManager(tb);
  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  s=Typemaker2_TypeManager_GetApiDeclaration(tym);
  if (s)
    GBAA(tbuf, "%s ", s);
  _addSortByMemberDeclaration(ty, tm, tbuf);
  GBAS(tbuf, ";\n");

  Typemaker2_Builder_AddPublicDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void _addSortByMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPE *mty;
  const char *sTypePrefix;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  _addSortByMemberDeclaration(ty, tm, tbuf);
  GBAS(tbuf, "{\n");

  GBAA(tbuf, "  %s_Tree2_SortChildren(p_tree2, (GWEN_TREE2_COMPARE_CB)(p_ascending?%s_Tree2_Compare_%c%s_asc:%s_Tree2_Compare_%c%s_desc));\n",
       sTypePrefix,
       sTypePrefix, toupper(*sMemberName), sMemberName+1,
       sTypePrefix, toupper(*sMemberName), sMemberName+1);
  GBAS(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



void _addSortByMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf)
{
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);

  GBAA(tbuf,
       "void %s_Tree2_SortChildrenBy%c%s(%s *p_tree2, int p_ascending)",
       sTypePrefix,
       toupper(*sMemberName),
       sMemberName+1,
       sTypeId);
}



void _addCompareMemberProtoType(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, int ascending)
{
  GWEN_BUFFER *tbuf;

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  GBAS(tbuf, "static ");
  _addCompareMemberDeclaration(ty, tm, tbuf, ascending);
  GBAS(tbuf, ";\n");

  Typemaker2_Builder_AddPrivateDeclaration(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);
}



int _addCompareMemberImplementation(TYPEMAKER2_BUILDER *tb, TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, int ascending)
{
  GWEN_BUFFER *tbuf;
  TYPEMAKER2_TYPE *mty;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sMemberName=Typemaker2_Member_GetName(tm);

  tbuf=GWEN_Buffer_new(0, 256, 0, 1);

  _addCompareMemberDeclaration(ty, tm, tbuf, ascending);
  GBAS(tbuf, "{\n");
  GBAS(tbuf, "  int p_rv;\n");
  GBAS(tbuf, "\n");

  if (1) {
    GWEN_BUFFER *dstbuf;
    GWEN_BUFFER *srcbuf;
    int rv;

    srcbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GBAA(srcbuf, "p_a->%s", sMemberName);

    dstbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GBAA(dstbuf, "p_b->%s", sMemberName);

    rv=Typemaker2_Builder_Invoke_CompareFn(tb, ty, tm,
                                           GWEN_Buffer_GetStart(srcbuf),
                                           GWEN_Buffer_GetStart(dstbuf),
                                           tbuf);
    GWEN_Buffer_free(srcbuf);
    GWEN_Buffer_free(dstbuf);
    if (rv<0) {
      DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
      GWEN_Buffer_free(tbuf);
      return rv;
    }
    GBAS(tbuf, "\n");
  }

  if (ascending)
    GBAS(tbuf, "  return p_rv;\n");
  else
    GBAS(tbuf, "  return -p_rv;\n");
  GBAS(tbuf, "}\n");

  Typemaker2_Builder_AddCode(tb, GWEN_Buffer_GetStart(tbuf));
  GWEN_Buffer_free(tbuf);

  return 0;
}



void _addCompareMemberDeclaration(TYPEMAKER2_TYPE *ty, TYPEMAKER2_MEMBER *tm, GWEN_BUFFER *tbuf, int ascending)
{
  TYPEMAKER2_TYPE *mty;
  const char *sTypeId;
  const char *sTypePrefix;
  const char *sMemberName;

  mty=Typemaker2_Member_GetTypePtr(tm);
  assert(mty);

  sTypeId=Typemaker2_Type_GetIdentifier(ty);
  sTypePrefix=Typemaker2_Type_GetPrefix(ty);
  sMemberName=Typemaker2_Member_GetName(tm);

  GBAA(tbuf,
       "int GWENHYWFAR_CB %s_Tree2_Compare_%c%s_%s(const %s *p_a, const %s *p_b)",
       sTypePrefix,
       toupper(*sMemberName),
       sMemberName+1,
       ascending?"asc":"desc",
       sTypeId,
       sTypeId);
}



