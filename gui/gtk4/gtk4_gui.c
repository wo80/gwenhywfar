/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif


#include "gtk4_gui_p.h"
#include "gtk4_gui_dialog_l.h"

#include <assert.h>

#include <gwenhywfar/inherit.h>
#include <gwenhywfar/debug.h>
#include <gwenhywfar/gui_be.h>
#include <gwenhywfar/i18n.h>

#include <gwenhywfar/text.h>
#include <gwenhywfar/mdigest.h>
#include <gwenhywfar/debug.h>


#define I18N(msg) GWEN_I18N_Translate(PACKAGE, msg)



GWEN_INHERIT(GWEN_GUI, GTK4_GUI)



/* Global variable to hold the dialog callback. */
static GTK4_GUI_GetFileName_Callback_Wrapper GTK4_GetFileName_Handler;



GWEN_GUI *Gtk4_Gui_new()
{
  GWEN_GUI *gui;
  GTK4_GUI *xgui;

  gui=GWEN_Gui_new();
  GWEN_NEW_OBJECT(GTK4_GUI, xgui);
  GWEN_INHERIT_SETDATA(GWEN_GUI, GTK4_GUI, gui, xgui, Gtk4_Gui_FreeData);

  GWEN_Gui_AddFlags(gui, GWEN_GUI_FLAGS_DIALOGSUPPORTED);
  GWEN_Gui_UseDialogs(gui);
  xgui->execDialogFn=GWEN_Gui_SetExecDialogFn(gui, GTK4_Gui_ExecDialog);
  xgui->openDialogFn=GWEN_Gui_SetOpenDialogFn(gui, GTK4_Gui_OpenDialog);
  xgui->closeDialogFn=GWEN_Gui_SetCloseDialogFn(gui, GTK4_Gui_CloseDialog);
  xgui->runDialogFn=GWEN_Gui_SetRunDialogFn(gui, GTK4_Gui_RunDialog);
  xgui->getFileNameDialogFn=GWEN_Gui_SetGetFileNameFn(gui, GTK4_Gui_GetFileName);
  xgui->getFileNameNonBlockingDialogFn=GWEN_Gui_SetGetFileNameNonBlockingFn(gui, GTK4_Gui_GetFileName_NonBlocking);

  return gui;
}



GWENHYWFAR_CB void Gtk4_Gui_FreeData(GWEN_UNUSED void *bp, void *p)
{
  GTK4_GUI *xgui;

  xgui=(GTK4_GUI *) p;

  GWEN_FREE_OBJECT(xgui);
}



GWENHYWFAR_CB int GTK4_Gui_ExecDialog(GWEN_GUI *gui,
                                      GWEN_DIALOG *dlg,
                                      uint32_t guiid)
{
  int rv;

  assert(dlg);
  rv=GTK4_Gui_OpenDialog(gui, dlg, guiid);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  rv=GTK4_Gui_RunDialog(gui, dlg, 1);
  GTK4_Gui_CloseDialog(gui, dlg);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }

  return rv;
}



GWENHYWFAR_CB int GTK4_Gui_OpenDialog(GWEN_UNUSED GWEN_GUI *gui,
                                      GWEN_DIALOG *dlg,
                                      GWEN_UNUSED uint32_t guiid)
{
  int rv;
  GtkWidget *g;

  assert(dlg);
  Gtk4Gui_Dialog_Extend(dlg);
  rv=Gtk4Gui_Dialog_Setup(dlg, NULL);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    Gtk4Gui_Dialog_Unextend(dlg);
    return rv;
  }

  g=Gtk4Gui_Dialog_GetMainWidget(dlg);
  if (g==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No main widget");
    Gtk4Gui_Dialog_Unextend(dlg);
    return GWEN_ERROR_INVALID;
  }

  rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeInit, "");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error initializing dialog: %d", rv);
    Gtk4Gui_Dialog_Unextend(dlg);
    return rv;
  }

  /* show dialog */
  gtk_widget_show(g);

  return 0;
}



GWENHYWFAR_CB int GTK4_Gui_CloseDialog(GWEN_UNUSED GWEN_GUI *gui, GWEN_DIALOG *dlg)
{
  GtkWidget *g;
  int rv;

  assert(dlg);
  g=Gtk4Gui_Dialog_GetMainWidget(dlg);
  if (g==NULL) {
    DBG_ERROR(GWEN_LOGDOMAIN, "No main widget");
    Gtk4Gui_Dialog_Unextend(dlg);
    return GWEN_ERROR_INVALID;
  }

  /* hide dialog */
  gtk_widget_hide(g);

  /* send fini signal to dialog */
  rv=GWEN_Dialog_EmitSignalToAll(dlg, GWEN_DialogEvent_TypeFini, "");
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "Error deinitializing dialog: %d", rv);
    Gtk4Gui_Dialog_Unextend(dlg);
    return rv;
  }

  Gtk4Gui_Dialog_Unextend(dlg);
  return 0;
}



GWENHYWFAR_CB int GTK4_Gui_RunDialog(GWEN_UNUSED GWEN_GUI *gui, GWEN_DIALOG *dlg, int untilEnd)
{
  int rv;

  assert(dlg);
  rv=GTK4_Gui_Dialog_Run(dlg, untilEnd);
  if (rv<0) {
    DBG_INFO(GWEN_LOGDOMAIN, "here (%d)", rv);
    return rv;
  }
  return rv;
}

/*
   Opening file dialogs blocking the UI thread is no longer possible in GTK4, see
   https://docs.gtk.org/gtk4/migrating-3to4.html#stop-using-blocking-dialog-functions
*/

GWENHYWFAR_CB int GTK4_Gui_GetFileName(GWEN_UNUSED GWEN_GUI *gui,
                                       const char *caption,
                                       GWEN_GUI_FILENAME_TYPE fnt,
                                       GWEN_UNUSED uint32_t flags,
                                       GWEN_UNUSED const char *patterns,
                                       GWEN_BUFFER *pathBuffer,
                                       GWEN_UNUSED uint32_t guiid)
{
  return GWEN_INFO_USE_CALLBACK;
}


/* Proposal for non-blocking GUI using a simple callback. */

static void
GTK4_on_file_chooser_response(GtkDialog *dialog,
                              int        response)
{
  char *fileName = NULL;
  GWEN_BUFFER *pathBuffer;

  if (response == GTK_RESPONSE_ACCEPT && GTK4_GetFileName_Handler.callback)
  {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER(dialog);

    g_autoptr(GFile) file = gtk_file_chooser_get_file(chooser);

    fileName = g_file_get_path(file);
    pathBuffer = GTK4_GetFileName_Handler.buffer;

    if (pathBuffer)
    {
      GWEN_Buffer_Reset(pathBuffer);
      GWEN_Buffer_AppendString(pathBuffer, fileName);
    }

    gtk_window_destroy (GTK_WINDOW (dialog));

    GTK4_GetFileName_Handler.callback(response, pathBuffer, GTK4_GetFileName_Handler.dlg);

    g_free(fileName);
  } else {
    /* TODO: should the callback be fired with GWEN_ERROR_USER_ABORTED? */

    gtk_window_destroy (GTK_WINDOW (dialog));
  }
}

GWENHYWFAR_CB int GTK4_Gui_GetFileName_NonBlocking(GWEN_UNUSED GWEN_GUI *gui,
                                       const char *caption,
                                       GWEN_GUI_FILENAME_TYPE fnt,
                                       GWEN_UNUSED uint32_t flags,
                                       GWEN_UNUSED const char *patterns,
                                       GWEN_BUFFER *pathBuffer,
                                       GWEN_DIALOG *dlg,
                                       GWEN_GUI_GetFileName_Callback callback)
{
  GtkWidget *dialog;
  GtkFileChooserAction action;

  switch (fnt) {
  case GWEN_Gui_FileNameType_OpenFileName: {
    action = GTK_FILE_CHOOSER_ACTION_OPEN;
    if (!(caption && *caption))
      caption=I18N("Open File");
    break;
  }

  case GWEN_Gui_FileNameType_SaveFileName: {
    action = GTK_FILE_CHOOSER_ACTION_SAVE;
    if (!(caption && *caption))
      caption=I18N("Save File");
    break;
  }

  case GWEN_Gui_FileNameType_OpenDirectory: {
    action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
    if (!(caption && *caption))
      caption=I18N("Select Folder");
    break;
  }
  default:
    /* Returning GWEN_ERROR_USER_ABORTED here is consistent with the GTK3 implementation. */
    return GWEN_ERROR_USER_ABORTED;
  }

  GtkWidget *parent = Gtk4Gui_Dialog_GetMainWidget(dlg);

  dialog=gtk_file_chooser_dialog_new(caption,
                                     GTK_WINDOW(parent),
                                     action,
                                     "_Cancel", GTK_RESPONSE_CANCEL,
                                     "_Open", GTK_RESPONSE_ACCEPT,
                                     NULL);

  if (action == GTK_FILE_CHOOSER_ACTION_OPEN || action == GTK_FILE_CHOOSER_ACTION_SAVE) {
    char *folder=NULL;
    char *fileName=NULL;

    if (GWEN_Buffer_GetUsedBytes(pathBuffer)) {
      folder=strdup(GWEN_Buffer_GetStart(pathBuffer));
      fileName=strrchr(folder, GWEN_DIR_SEPARATOR);
      if (fileName) {
        *fileName=0;
        fileName++;
        if (*fileName==0)
          fileName=NULL;
      }
    }

    if (folder && *folder) {
      GFile *tmp_file = g_file_new_for_path (folder);
      gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), tmp_file, NULL);
      g_object_unref (tmp_file);
    }
    if (fileName && *fileName) {
      gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), fileName);
    }
    free(folder);
  }

  /* Use global variable to register callback and pass buffer and parent dialog. */
  GTK4_GetFileName_Handler.callback = callback;
  GTK4_GetFileName_Handler.buffer = pathBuffer;
  GTK4_GetFileName_Handler.dlg = dlg;

  gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

  gtk_widget_show (dialog);
  g_signal_connect (dialog, "response",
                  G_CALLBACK (GTK4_on_file_chooser_response),
                  NULL);

  return 0;
}





