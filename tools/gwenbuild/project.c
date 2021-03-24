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


#include "gwenbuild/project_p.h"

#include <gwenhywfar/memory.h>




GWB_PROJECT *GWB_Project_new(GWENBUILD *gwbuild, GWB_CONTEXT *ctx)
{
  GWB_PROJECT *project;

  GWEN_NEW_OBJECT(GWB_PROJECT, project);

  project->gwbuild=gwbuild;
  project->contextTree=ctx;

  project->fileList=GWB_File_List2_new();
  project->targetList=GWB_Target_List2_new();
  project->builderList=GWB_Builder_List2_new();

  return project;
}



void GWB_Project_free(GWB_PROJECT *project)
{
  if (project) {
    GWB_File_List2_free(project->fileList);
    GWB_Context_free(project->contextTree);
    GWB_Target_List2_free(project->targetList);
    GWB_Builder_List2_free(project->builderList);
    GWB_KeyValuePair_List_free(project->defineList);

    GWEN_FREE_OBJECT(project);
  }
}



GWENBUILD *GWB_Project_GetGwbuild(const GWB_PROJECT *project)
{
  return project->gwbuild;
}



const char *GWB_Project_GetProjectName(const GWB_PROJECT *project)
{
  return project->projectName;
}



void GWB_Project_SetProjectName(GWB_PROJECT *project, const char *s)
{
  if (project->projectName)
    free(project->projectName);
  if (s)
    project->projectName=strdup(s);
  else
    project->projectName=NULL;
}



void GWB_Project_SetVersion(GWB_PROJECT *project, int vMajor, int vMinor, int vPatchlevel, int vBuild)
{
  project->versionMajor=vMajor;
  project->versionMinor=vMinor;
  project->versionPatchlevel=vPatchlevel;
  project->versionBuild=vBuild;
}



int GWB_Project_GetVersionMajor(const GWB_PROJECT *project)
{
  return project->versionMajor;
}



int GWB_Project_GetVersionMinor(const GWB_PROJECT *project)
{
  return project->versionMinor;
}



int GWB_Project_GetVersionPatchlevel(const GWB_PROJECT *project)
{
  return project->versionPatchlevel;
}



int GWB_Project_GetVersionBuild(const GWB_PROJECT *project)
{
  return project->versionBuild;
}



void GWB_Project_SetSoVersion(GWB_PROJECT *project, int vCurrent, int vAge, int vRevision)
{
  project->soVersionCurrent=vCurrent;
  project->soVersionAge=vAge;
  project->soVersionRevision=vRevision;
}



int GWB_Project_GetSoVersionCurrent(const GWB_PROJECT *project)
{
  return project->soVersionCurrent;
}



int GWB_Project_GetSoVersionAge(const GWB_PROJECT *project)
{
  return project->soVersionAge;
}



int GWB_Project_GetSoVersionRevision(const GWB_PROJECT *project)
{
  return project->soVersionRevision;
}





GWB_FILE *GWB_Project_GetFileByPathAndName(const GWB_PROJECT *project, const char *folder, const char *fname)
{
  GWB_FILE_LIST2_ITERATOR *it;

  it=GWB_File_List2_First(project->fileList);
  if (it) {
    GWB_FILE *file;

    file=GWB_File_List2Iterator_Data(it);
    while(file) {
      const char *currentName;

      currentName=GWB_File_GetName(file);
      if (currentName && *currentName && strcasecmp(currentName, fname)==0) {
        const char *currentFolder;

        currentFolder=GWB_File_GetFolder(file);
        if (currentFolder && *currentFolder && strcasecmp(currentFolder, folder)==0) {
          GWB_File_List2Iterator_free(it);
          return file;
        }
      }
      file=GWB_File_List2Iterator_Next(it);
    }
    GWB_File_List2Iterator_free(it);
  }

  return NULL;
}



void GWB_Project_AddFile(GWB_PROJECT *project, GWB_FILE *file)
{
  GWB_File_List2_PushBack(project->fileList, file);
}



GWB_CONTEXT *GWB_Project_GetRootContext(const GWB_PROJECT *project)
{
  return project->contextTree;
}




GWB_TARGET_LIST2 *GWB_Project_GetTargetList(const GWB_PROJECT *project)
{
  return project->targetList;
}



void GWB_Project_AddTarget(GWB_PROJECT *project, GWB_TARGET *target)
{
  GWB_Target_List2_PushBack(project->targetList, target);
}



GWB_BUILDER_LIST2 *GWB_Project_GetBuilderList(const GWB_PROJECT *project)
{
  return project->builderList;
}



void GWB_Project_AddBuilder(GWB_PROJECT *project, GWB_BUILDER *builder)
{
  GWB_Builder_List2_PushBack(project->builderList, builder);
}



GWB_KEYVALUEPAIR_LIST *GWB_Project_GetDefineList(const GWB_PROJECT *project)
{
  return project->defineList;
}



void GWB_Project_SetDefine(GWB_PROJECT *project, const char *name, const char *value)
{
  if (name && *name) {
    GWB_KEYVALUEPAIR *kvp;

    if (project->defineList==NULL)
      project->defineList=GWB_KeyValuePair_List_new();

    kvp=GWB_KeyValuePair_List_GetFirstByKey(project->defineList, name);
    if (kvp)
      GWB_KeyValuePair_SetValue(kvp, value);
    else
    GWB_KeyValuePair_List_Add(GWB_KeyValuePair_new(name, value), project->defineList);
  }
}



void GWB_Project_ClearDefineList(GWB_PROJECT *project)
{
if (project->defineList==NULL)
    project->defineList=GWB_KeyValuePair_List_new();
  else
    GWB_KeyValuePair_List_Clear(project->defineList);
}



void GWB_Project_Dump(const GWB_PROJECT *project, int indent)
{
  int i;

  for(i=0; i<indent; i++)
    fprintf(stderr, " ");
  fprintf(stderr, "Project:\n");

  GWBUILD_Debug_PrintValue(   "projectName......", project->projectName, indent+2);
  GWBUILD_Debug_PrintIntValue("versionMajor.....", project->versionMajor, indent+2);
  GWBUILD_Debug_PrintIntValue("versionMinor.....", project->versionMinor, indent+2);
  GWBUILD_Debug_PrintIntValue("versionPatchlevel", project->versionPatchlevel, indent+2);
  GWBUILD_Debug_PrintIntValue("versionBuild.....", project->versionBuild, indent+2);
  GWBUILD_Debug_PrintIntValue("soVersionCurrent.", project->soVersionCurrent, indent+2);
  GWBUILD_Debug_PrintIntValue("soVersionAge.....", project->soVersionAge, indent+2);
  GWBUILD_Debug_PrintIntValue("soVersionRevision", project->soVersionRevision, indent+2);

  GWB_Context_Tree2_Dump(project->contextTree, indent+2);

  GWBUILD_Debug_PrintFileList2("fileList", project->fileList, indent+2);
  GWBUILD_Debug_PrintTargetList2("targetList", project->targetList, indent+2);
  GWBUILD_Debug_PrintKvpList("defineList", project->defineList, indent+2);
}




