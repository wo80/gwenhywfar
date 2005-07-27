/***************************************************************************
 $RCSfile$
 -------------------
 cvs         : $Id$
 begin       : Tue May 03 2005
 copyright   : (C) 2005 by Martin Preuss
 email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define GCT_IS_EXPERIMENTAL 1


#include <gwenhywfar/debug.h>

#include "globals.h"
#include "cryptmanager_l.h"

#ifdef OS_WIN32
# define DIRSEP "\\"
#else
# define DIRSEP "/"
#endif


int main(int argc, char **argv) {
  GWEN_DB_NODE *db;
  const char *cmd;
  int rv;
  GWEN_PLUGIN_MANAGER *cm;
  const GWEN_ARGS args[]={
  {
    GWEN_ARGS_FLAGS_HAS_ARGUMENT, /* flags */
    GWEN_ArgsTypeChar,            /* type */
    "cfgfile",                    /* name */
    0,                            /* minnum */
    1,                            /* maxnum */
    "C",                          /* short option */
    "cfgfile",                    /* long option */
    "Specify the configuration file",     /* short description */
    "Specify the configuration file"      /* long description */
  },
  {
    GWEN_ARGS_FLAGS_HELP | GWEN_ARGS_FLAGS_LAST, /* flags */
    GWEN_ArgsTypeInt,             /* type */
    "help",                       /* name */
    0,                            /* minnum */
    0,                            /* maxnum */
    "h",                          /* short option */
    "help",                       /* long option */
    "Show this help screen",      /* short description */
    "Show this help screen"       /* long description */
  }
  };

#ifdef HAVE_I18N
  setlocale(LC_ALL,"");
  if (bindtextdomain(PACKAGE,  LOCALEDIR)==0)
    fprintf(stderr, "Error binding locale\n");
#endif

  GWEN_Logger_Open("gct-tool", "gct-tool", 0,
                   GWEN_LoggerTypeConsole,
                   GWEN_LoggerFacilityUser);

#ifdef DEBUG_GCT_TOOL
  GWEN_Logger_SetLevel("gct-tool", GWEN_LoggerLevelInfo);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevelInfo);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelInfo);
#else
  GWEN_Logger_SetLevel("gct-tool", GWEN_LoggerLevelNotice);
  GWEN_Logger_SetLevel(GWEN_LOGDOMAIN, GWEN_LoggerLevelNotice);
  GWEN_Logger_SetLevel(0, GWEN_LoggerLevelNotice);
#endif

#ifdef GCT_IS_EXPERIMENTAL
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "=================== WARNING ===================\n");
  fprintf(stderr, "This tool is still EXPERIMENTAL !!!\n");
  fprintf(stderr, "Please DON'T USE it with your daily key files !\n");
  fprintf(stderr, "===============================================\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "\n");
#endif

  db=GWEN_DB_Group_new("arguments");
  rv=GWEN_Args_Check(argc, argv, 1,
		     GWEN_ARGS_MODE_ALLOW_FREEPARAM |
		     GWEN_ARGS_MODE_STOP_AT_FREEPARAM,
		     args,
		     db);
  if (rv==GWEN_ARGS_RESULT_ERROR) {
    fprintf(stderr, "ERROR: Could not parse arguments main\n");
    return -1;
  }
  else if (rv==GWEN_ARGS_RESULT_HELP) {
    GWEN_BUFFER *ubuf;

    ubuf=GWEN_Buffer_new(0, 1024, 0, 1);
    GWEN_Buffer_AppendString(ubuf,
                             I18N("Usage: "));
    GWEN_Buffer_AppendString(ubuf, argv[0]);
    GWEN_Buffer_AppendString(ubuf,
                             I18N(" [GLOBAL OPTIONS] COMMAND "
                                  "[LOCAL OPTIONS]\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nGlobal Options:\n"));
    if (GWEN_Args_Usage(args, ubuf, GWEN_ArgsOutTypeTXT)) {
      fprintf(stderr, "ERROR: Could not create help string\n");
      return 1;
    }
    GWEN_Buffer_AppendString(ubuf,
                             I18N("\nCommands:\n\n"));
    GWEN_Buffer_AppendString(ubuf,
                             I18N("  sign:\n"
                                  "    This command signs the given files"
                                  "\n\n"));

    fprintf(stderr, "%s\n", GWEN_Buffer_GetStart(ubuf));
    GWEN_Buffer_free(ubuf);
    return 0;
  }
  if (rv) {
    argc-=rv-1;
    argv+=rv-1;
  }

  cm=CON_CryptManager_new();
  if (cm) {
    GWEN_BUFFER *ctbuf;

    /* add path from gwen since all crypt token plugins are installed there */
    ctbuf=GWEN_Buffer_new(0, 256, 0, 1);
    GWEN_GetPluginPath(ctbuf);
    GWEN_Buffer_AppendString(ctbuf, DIRSEP);
    GWEN_Buffer_AppendString(ctbuf, "crypttoken");
    GWEN_PluginManager_AddPath(cm,
			       GWEN_Buffer_GetStart(ctbuf));
    GWEN_Buffer_free(ctbuf);

    if (GWEN_PluginManager_Register(cm)) {
      DBG_ERROR(GWEN_LOGDOMAIN,
		"Could not register crypttoken plugin manager");
      return 2;
    }
  }
  else {
    DBG_ERROR(GWEN_LOGDOMAIN, "Could not create crypt token plugin manager");
    return 2;
  }


  cmd=GWEN_DB_GetCharValue(db, "params", 0, 0);
  if (!cmd) {
    fprintf(stderr, "ERROR: Command needed.\n");
    return 1;
  }

  if (strcasecmp(cmd, "sign")==0) {
    rv=signFiles(db, argc, argv);
  }
  else if (strcasecmp(cmd, "create")==0) {
    rv=createToken(db, argc, argv);
  }
  else if (strcasecmp(cmd, "showctx")==0) {
    rv=showCtx(db, argc, argv);
  }
  else if (strcasecmp(cmd, "readkey")==0) {
    rv=readKey(db, argc, argv);
  }
  else if (strcasecmp(cmd, "showuser")==0) {
    rv=showUser(db, argc, argv);
  }
  else if (strcasecmp(cmd, "changepin")==0) {
    rv=changePin(db, argc, argv);
  }
  else {
    fprintf(stderr, "ERROR: Unknown command \"%s\".\n", cmd);
    rv=1;
  }

  if (GWEN_PluginManager_Unregister(cm)) {
    DBG_ERROR(GWEN_LOGDOMAIN,
	      "Could not unregister crypttoken plugin manager");
  }
  GWEN_PluginManager_free(cm);

  return rv;
}


