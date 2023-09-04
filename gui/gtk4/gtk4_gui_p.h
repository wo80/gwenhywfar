/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GTK4_GUI_P_H
#define GTK4_GUI_P_H


#include "gtk4_gui.h"

#include <gwenhywfar/gui_be.h>

/* Datatype to enable non-blocking dialog callbacks for GTK4 */
typedef struct {
  GWEN_GUI_GetFileName_Callback callback;
  GWEN_BUFFER *buffer;
} GTK4_GUI_GetFileName_Callback_Wrapper;


typedef struct GTK4_GUI GTK4_GUI;
struct GTK4_GUI {

  GWEN_GUI_EXEC_DIALOG_FN execDialogFn;

  GWEN_GUI_OPEN_DIALOG_FN openDialogFn;
  GWEN_GUI_CLOSE_DIALOG_FN closeDialogFn;
  GWEN_GUI_RUN_DIALOG_FN runDialogFn;
  GWEN_GUI_GET_FILENAME_FN getFileNameDialogFn;
};

static GWENHYWFAR_CB
void Gtk4_Gui_FreeData(void *bp, void *p);


static GWENHYWFAR_CB
int GTK4_Gui_ExecDialog(GWEN_GUI *gui,
                        GWEN_DIALOG *dlg,
                        uint32_t guiid);


static GWENHYWFAR_CB
int GTK4_Gui_OpenDialog(GWEN_GUI *gui,
                        GWEN_DIALOG *dlg,
                        uint32_t guiid);

static GWENHYWFAR_CB
int GTK4_Gui_CloseDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg);

static GWENHYWFAR_CB
int GTK4_Gui_RunDialog(GWEN_GUI *gui, GWEN_DIALOG *dlg, int timeout);

static GWENHYWFAR_CB
int GTK4_Gui_GetFileName(GWEN_GUI *gui,
                         const char *caption,
                         GWEN_GUI_FILENAME_TYPE fnt,
                         uint32_t flags,
                         const char *patterns,
                         GWEN_BUFFER *pathBuffer,
                         uint32_t guiid);

/*
  This implementation should replace the above. That should allow staying compatible
  with previous versions. The calling code (open/save keyfile in HBCI/EBICS backend)
  has to make sure that both blocking and non-blocking options are supported.
*/

static GWENHYWFAR_CB
int GTK4_Gui_GetFileName_NonBlocking(GWEN_GUI *gui,
                         const char *caption,
                         GWEN_GUI_FILENAME_TYPE fnt,
                         uint32_t flags,
                         const char *patterns,
                         GWEN_BUFFER *pathBuffer,
                         uint32_t guiid,
                         GWEN_GUI_GetFileName_Callback callback);

#endif


