/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/

#ifndef GTK4_GUI_H
#define GTK4_GUI_H


#include <gtk/gtk.h>


#if (defined __GNUC__ && (! defined (__sun)) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 3))) || defined _MSC_VER
# ifdef BUILDING_GTK4_GUI
#   define GTK4GUI_API __attribute__ ((visibility("default")))
# else
#   define GTK4GUI_API
# endif
#else
# define GTK4GUI_API
#endif

#include <gwenhywfar/gui.h>



GTK4GUI_API GWEN_GUI *Gtk4_Gui_new();

/*
  If callbacks for non-blocking dialogs will be implemented, this typedef
  needs to be moved to   gwenhywfar/src/gui/gui.h
 */
typedef void (*GWEN_GUI_GetFileName_Callback)(int result, char *filename);

#endif


