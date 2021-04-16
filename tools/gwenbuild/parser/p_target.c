/***************************************************************************
    begin       : Mon Feb 08 2021
    copyright   : (C) 2021 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gwenbuild/parser/p_target.h"
#include "gwenbuild/parser/parser.h"
#include "gwenbuild/builder_be.h"

#include <gwenhywfar/debug.h>



static GWB_TARGET *_readTarget(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseSourcesOrHeaders(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, int alwaysDist, int isSource);
static int _parseUsedTargets(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseIncludes(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseLibraries(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static int _parseDefines(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);

static int _parseBuildFiles(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static void _parseBuildInputFiles(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static void _parseBuildOutputFiles(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode);
static GWB_BUILD_SUBCMD *_parseBuildCommand(GWB_BUILD_CMD *bcmd,
                                            GWB_CONTEXT *currentContext,
                                            GWEN_XMLNODE *xmlNode,
                                            GWEN_DB_NODE *dbForCmd);
static void _addFilePathsToDb(GWB_CONTEXT *currentContext,
                              const GWB_FILE_LIST2 *fileList,
                              GWEN_DB_NODE *db,
                              const char *varName);






int GWB_ParseTarget(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  GWB_TARGET *target;
  GWB_CONTEXT *newContext;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  target=_readTarget(project, currentContext, xmlNode);
  if (target==NULL) {
    DBG_INFO(NULL, "No target created");
    return GWEN_ERROR_GENERIC;
  }

  newContext=GWB_Parser_CopyContextForTarget(currentContext);
  GWB_Context_SetCurrentTarget(newContext, target);
  GWB_Target_SetContext(target, newContext);

  rv=_parseChildNodes(project, newContext, xmlNode);
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    GWB_Context_free(newContext);
    GWB_Target_free(target);
    return rv;
  }

  GWB_Project_AddTarget(project, target);
  GWB_Context_Tree2_AddChild(currentContext, newContext);

  return 0;
}



GWB_TARGET *_readTarget(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  const char *s;
  GWBUILD_TARGETTYPE targetType;
  uint32_t flags=GWEN_DB_FLAGS_OVERWRITE_VARS;
  GWEN_DB_NODE *db;

  target=GWB_Target_new(project);

  s=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Target has no name");
    return NULL;
  }
  GWB_Target_SetName(target, s);
  GWB_Target_SetId(target, s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "id", NULL);
  if (s && *s)
    GWB_Target_SetId(target, s);

  s=GWEN_XMLNode_GetProperty(xmlNode, "type", NULL);
  if (!(s && *s)) {
    DBG_ERROR(NULL, "Target has no type");
    GWB_Target_free(target);
    return NULL;
  }
  targetType=GWBUILD_TargetType_fromString(s);
  if (targetType==GWBUILD_TargetType_Invalid) {
    DBG_ERROR(NULL, "Bad target type \"%s\"", s?s:"<empty>");
    GWB_Target_free(target);
    return NULL;
  }
  GWB_Target_SetTargetType(target, targetType);

  s=GWEN_XMLNode_GetProperty(xmlNode, "install", NULL);
  GWB_Target_SetInstallPath(target, s);

  GWB_Target_SetSoVersion(target,
                          GWEN_XMLNode_GetIntProperty(xmlNode, "so_current", 0),
                          GWEN_XMLNode_GetIntProperty(xmlNode, "so_age", 0),
                          GWEN_XMLNode_GetIntProperty(xmlNode, "so_revision", 0));

  db=GWB_Context_GetVars(currentContext);
  GWEN_DB_SetCharValue(db, flags, "target_name", GWB_Target_GetName(target));
  GWEN_DB_SetCharValueFromInt(db, flags, "target_so_current", GWB_Target_GetSoVersionCurrent(target));
  GWEN_DB_SetCharValueFromInt(db, flags, "target_so_age", GWB_Target_GetSoVersionAge(target));
  GWEN_DB_SetCharValueFromInt(db, flags, "target_so_revision", GWB_Target_GetSoVersionRevision(target));
  GWEN_DB_SetCharValueFromInt(db, flags, "target_so_effective",
                              GWB_Target_GetSoVersionCurrent(target)-GWB_Target_GetSoVersionAge(target));

  return target;
}



int _parseChildNodes(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_XMLNODE *n;

  n=GWEN_XMLNode_GetFirstTag(xmlNode);
  while (n) {
    const char *name;

    name=GWEN_XMLNode_GetData(n);
    if (name && *name) {
      int rv;

      DBG_DEBUG(NULL, "Handling element \"%s\"", name);

      if (strcasecmp(name, "subdirs")==0)
        rv=GWB_Parser_ParseSubdirs(project, currentContext, n, _parseChildNodes);
      else if (strcasecmp(name, "sources")==0)
        rv=_parseSourcesOrHeaders(project, currentContext, n, 1, 1);
      else if (strcasecmp(name, "headers")==0)
        rv=_parseSourcesOrHeaders(project, currentContext, n, 1, 0);
      else if (strcasecmp(name, "data")==0)
        rv=_parseSourcesOrHeaders(project, currentContext, n, 1, 0);
      else if (strcasecmp(name, "useTargets")==0)
        rv=_parseUsedTargets(currentContext, n);
      else if (strcasecmp(name, "includes")==0)
        rv=_parseIncludes(currentContext, n);
      else if (strcasecmp(name, "define")==0)
        rv=_parseDefines(currentContext, n);
      else if (strcasecmp(name, "libraries")==0)
        rv=_parseLibraries(currentContext, n);
      else if (strcasecmp(name, "target")==0)
        rv=GWB_ParseTarget(project, currentContext, n);
      else if (strcasecmp(name, "buildFiles")==0)
        rv=_parseBuildFiles(currentContext, n);
      else
        rv=GWB_Parser_ParseWellKnownElements(project, currentContext, n, _parseChildNodes);
      if (rv<0) {
        DBG_ERROR(GWEN_LOGDOMAIN, "Error in element \"%s\", aborting", name);
        return rv;
      }
    }

    n=GWEN_XMLNode_GetNextTag(n);
  }

  return 0;
}



int _parseSourcesOrHeaders(GWB_PROJECT *project, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode, int alwaysDist, int isSource)
{
  GWB_TARGET *target;
  uint32_t flags=0;
  int rv;
  const char *s;
  const char *installPath;
  const char *fileType;
  const char *currentFolder;
  const char *builder;
  GWEN_STRINGLIST *fileNameList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  currentFolder=GWB_Context_GetCurrentRelativeDir(currentContext);

  fileType=GWEN_XMLNode_GetProperty(xmlNode, "type", NULL);
  builder=GWEN_XMLNode_GetProperty(xmlNode, "builder", NULL);

  installPath=GWEN_XMLNode_GetProperty(xmlNode, "install", NULL);
  if (installPath && *installPath)
    flags|=GWB_FILE_FLAGS_INSTALL;

  s=GWEN_XMLNode_GetProperty(xmlNode, "generated", "FALSE");
  if (s && *s && (strcasecmp(s, "true")==0 || strcasecmp(s, "yes")==0))
    flags|=GWB_FILE_FLAGS_GENERATED;

  s=GWEN_XMLNode_GetProperty(xmlNode, "dist", alwaysDist?"TRUE":"FALSE");
  if (s && *s && (strcasecmp(s, "true")==0 || strcasecmp(s, "yes")==0))
    flags|=GWB_FILE_FLAGS_DIST;

  fileNameList=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (fileNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(fileNameList);
    while(se) {
      const char *sFileName;

      sFileName=GWEN_StringListEntry_Data(se);
      if (sFileName && *sFileName) {
        GWB_FILE *file;

        file=GWB_File_List2_GetOrCreateFile(GWB_Project_GetFileList(project), currentFolder, sFileName);
        GWB_File_AddFlags(file, flags);
        if (installPath)
          GWB_File_SetInstallPath(file, installPath);
        if (fileType)
          GWB_File_SetFileType(file, fileType);
        if (builder)
          GWB_File_SetBuilder(file, builder);
        if (isSource)
          GWB_Context_AddSourceFile(currentContext, file);
        //GWB_Target_AddSourceFile(target, file);
      }

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(fileNameList);
  }

  return 0;
}



int _parseUsedTargets(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  int rv;
  GWEN_STRINGLIST *targetNameList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  targetNameList=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (targetNameList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(targetNameList);
    while(se) {
      const char *sTargetName;

      sTargetName=GWEN_StringListEntry_Data(se);
      if (sTargetName && *sTargetName)
        GWB_Target_AddUsedTargetName(target, sTargetName);

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(targetNameList);
  }

  return 0;
}



int _parseIncludes(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  int rv;
  const char *builderType;
  GWEN_STRINGLIST *entryList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  builderType=GWEN_XMLNode_GetProperty(xmlNode, "type", "c");

  entryList=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (entryList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(entryList);
    while(se) {
      const char *sEntry;

      sEntry=GWEN_StringListEntry_Data(se);
      if (sEntry && *sEntry)
        GWB_Context_AddInclude(currentContext, builderType, sEntry);

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(entryList);
  }

  return 0;
}



int _parseDefines(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  int rv;
  const char *varName;
  const char *value;
  const char *quoted;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  quoted=GWEN_XMLNode_GetProperty(xmlNode, "quoted", "FALSE");
  varName=GWEN_XMLNode_GetProperty(xmlNode, "name", NULL);
  value=GWEN_XMLNode_GetProperty(xmlNode, "value", NULL);
  if (!(varName && *varName)) {
    DBG_ERROR(NULL, "Missing variable name in DEFINE");
    return GWEN_ERROR_GENERIC;
  }
  if (quoted && strcasecmp(quoted, "TRUE")==0) {
    GWEN_BUFFER *dbuf;

    dbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_Buffer_AppendString(dbuf, "\\\"");
    if (value && *value)
      GWEN_Buffer_AppendString(dbuf, value);
    GWEN_Buffer_AppendString(dbuf, "\\\"");
    GWB_Context_SetDefine(currentContext, varName, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }
  else {
    GWB_Context_SetDefine(currentContext, varName, value);
  }


  return 0;
}



int _parseLibraries(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  int rv;
  GWEN_STRINGLIST *entryList;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return rv;
  }

  entryList=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (entryList) {
    GWEN_STRINGLISTENTRY *se;

    se=GWEN_StringList_FirstEntry(entryList);
    while(se) {
      const char *sEntry;

      sEntry=GWEN_StringListEntry_Data(se);
      if (sEntry && *sEntry)
        GWB_Target_AddUsedLibraryName(target, sEntry);

      se=GWEN_StringListEntry_Next(se);
    }
    GWEN_StringList_free(entryList);
  }

  return 0;
}



int _parseBuildFiles(GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWB_TARGET *target;
  GWEN_XMLNODE *n;
  GWB_BUILD_CMD *bcmd;
  GWB_BUILD_SUBCMD *buildSubCmd=NULL;
  GWEN_DB_NODE *dbForCmd;
  const char *s;

  target=GWB_Context_GetCurrentTarget(currentContext);
  if (target==NULL) {
    DBG_ERROR(NULL, "No target in current context, SNH!");
    return GWEN_ERROR_INTERNAL;
  }

  bcmd=GWB_BuildCmd_new();
  GWB_BuildCmd_SetFolder(bcmd, GWB_Context_GetCurrentRelativeDir(currentContext));

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "input", NULL, NULL);
  if (n)
    _parseBuildInputFiles(bcmd, currentContext, n);

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "output", NULL, NULL);
  if (n)
    _parseBuildOutputFiles(bcmd, currentContext, n);

  dbForCmd=GWEN_DB_Group_new("dbForCmd");
  _addFilePathsToDb(currentContext, GWB_BuildCmd_GetInFileList2(bcmd), dbForCmd, "INPUT");
  _addFilePathsToDb(currentContext, GWB_BuildCmd_GetOutFileList2(bcmd), dbForCmd, "OUTPUT");

  n=GWEN_XMLNode_FindFirstTag(xmlNode, "cmd", NULL, NULL);
  if (n) {
    buildSubCmd=_parseBuildCommand(bcmd, currentContext, n, dbForCmd);
    if (buildSubCmd==NULL) {
      DBG_ERROR(NULL, "here");
      GWEN_DB_Group_free(dbForCmd);
      GWB_BuildCmd_free(bcmd);
      return GWEN_ERROR_GENERIC;
    }

    s=GWEN_XMLNode_GetProperty(n, "deleteOutFileFirst", "FALSE");
    if (s && strcasecmp(s, "TRUE")==0)
      GWB_BuildCmd_AddFlags(bcmd, GWB_BUILD_CMD_FLAGS_DEL_OUTFILES);
    GWB_BuildCmd_AddBuildCommand(bcmd, buildSubCmd);
  }

  /* pepare build message */
  n=GWEN_XMLNode_FindFirstTag(xmlNode, "buildMessage", NULL, NULL);
  if (n) {
    GWEN_BUFFER *dbuf;

    dbuf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(dbForCmd, n);
    if (dbuf) {
      GWB_BuildSubCmd_SetBuildMessage(buildSubCmd, GWEN_Buffer_GetStart(dbuf));
      GWEN_Buffer_free(dbuf);
    }
  }

  GWB_Target_AddExplicitBuild(target, bcmd);
  GWEN_DB_Group_free(dbForCmd);
  return 0;
}



void _parseBuildInputFiles(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_STRINGLIST *sl;
  GWB_TARGET *target;
  GWB_PROJECT *project;

  target=GWB_Context_GetCurrentTarget(currentContext);
  project=GWB_Target_GetProject(target);

  sl=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (sl) {
    GWBUILD_AddFilesFromStringList(GWB_Project_GetFileList(project),
                                   GWB_Context_GetCurrentRelativeDir(currentContext),
                                   sl,
                                   GWB_BuildCmd_GetInFileList2(bcmd),
                                   0,
                                   0);
    GWEN_StringList_free(sl);
  }
}



void _parseBuildOutputFiles(GWB_BUILD_CMD *bcmd, GWB_CONTEXT *currentContext, GWEN_XMLNODE *xmlNode)
{
  GWEN_STRINGLIST *sl;
  GWB_TARGET *target;
  GWB_PROJECT *project;

  target=GWB_Context_GetCurrentTarget(currentContext);
  project=GWB_Target_GetProject(target);

  sl=GWB_Parser_ReadXmlDataIntoStringList(GWB_Context_GetVars(currentContext), xmlNode);
  if (sl) {
    GWBUILD_AddFilesFromStringList(GWB_Project_GetFileList(project),
                                   GWB_Context_GetCurrentRelativeDir(currentContext),
                                   sl,
                                   GWB_BuildCmd_GetOutFileList2(bcmd),
                                   GWB_FILE_FLAGS_GENERATED,
                                   0);
    GWEN_StringList_free(sl);
  }
}



GWB_BUILD_SUBCMD *_parseBuildCommand(GWB_BUILD_CMD *bcmd,
                                     GWB_CONTEXT *currentContext,
                                     GWEN_XMLNODE *xmlNode,
                                     GWEN_DB_NODE *dbForCmd)
{
  GWEN_BUFFER *dbuf;
  const char *toolName;
  GWB_BUILD_SUBCMD *buildSubCmd;
  const char *s;
  int rv;

  rv=GWEN_XMLNode_ExpandProperties(xmlNode, GWB_Context_GetVars(currentContext));
  if (rv<0) {
    DBG_INFO(NULL, "here (%d)", rv);
    return NULL;
  }

  toolName=GWEN_XMLNode_GetProperty(xmlNode, "tool", NULL);
  if (!(toolName && *toolName)) {
    DBG_ERROR(NULL, "No tool in <cmd>");
    return NULL;
  }

  dbuf=GWB_Parser_ReadXmlDataIntoBufferAndExpand(dbForCmd, xmlNode);
  buildSubCmd=GWB_BuildSubCmd_new();
  GWB_BuildSubCmd_SetCommand(buildSubCmd, toolName);
  if (dbuf) {
    GWB_BuildSubCmd_SetArguments(buildSubCmd, GWEN_Buffer_GetStart(dbuf));
    GWEN_Buffer_free(dbuf);
  }

  s=GWEN_XMLNode_GetProperty(xmlNode, "checkDates", "TRUE");
  if (s && strcasecmp(s, "TRUE")==0)
    GWB_BuildCmd_AddFlags(bcmd, GWB_BUILD_CMD_FLAGS_CHECK_DATES);

  return buildSubCmd;
}



void _addFilePathsToDb(GWB_CONTEXT *currentContext,
                       const GWB_FILE_LIST2 *fileList,
                       GWEN_DB_NODE *db,
                       const char *varName)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(fileList);
  if (it) {
    const GWB_FILE *file;
    GWEN_BUFFER *fbuf;

    fbuf=GWEN_Buffer_new(0, 256, 0, 1);
    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      const char *folder;
      int useBuildDir;

      useBuildDir=GWB_File_GetFlags(file) & GWB_FILE_FLAGS_GENERATED;
      folder=GWB_File_GetFolder(file);
      GWB_Builder_AddRelativeFolderToBuffer(currentContext, folder, useBuildDir, fbuf);
      if (GWEN_Buffer_GetUsedBytes(fbuf))
        GWEN_Buffer_AppendString(fbuf, GWEN_DIR_SEPARATOR_S);
      GWEN_Buffer_AppendString(fbuf, GWB_File_GetName(file));
      GWEN_DB_SetCharValue(db, 0, varName, GWEN_Buffer_GetStart(fbuf));

      GWEN_Buffer_Reset(fbuf);
      file=GWB_File_List2Iterator_Next(it);
    }
    GWEN_Buffer_free(fbuf);

    GWB_File_List2Iterator_free(it);
  }

}




