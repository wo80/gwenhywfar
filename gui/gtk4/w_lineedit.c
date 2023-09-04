/***************************************************************************
    begin       : Sun May 16 2010
    copyright   : (C) 2010 by Martin Preuss
    email       : martin@libchipcard.de

 ***************************************************************************
 *          Please see toplevel file COPYING for license details           *
 ***************************************************************************/





static GWENHYWFAR_CB
int Gtk4Gui_WLineEdit_SetIntProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     GWEN_UNUSED int index,
                                     int value,
                                     GWEN_UNUSED int doSignal)
{
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    gtk_widget_set_sensitive(GTK_WIDGET(g), (value==0)?FALSE:TRUE);
    return 0;

  case GWEN_DialogProperty_Focus:
    gtk_widget_grab_focus(GTK_WIDGET(g));
    return 0;

  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
    /* just ignore these for now */
    return 0;

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}




static GWENHYWFAR_CB
int Gtk4Gui_WLineEdit_GetIntProperty(GWEN_WIDGET *w,
                                     GWEN_DIALOG_PROPERTY prop,
                                     GWEN_UNUSED int index,
                                     int defaultValue)
{
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Enabled:
    return (gtk_widget_get_sensitive(GTK_WIDGET(g))==TRUE)?1:0;

  case GWEN_DialogProperty_Focus:
    return (gtk_widget_has_focus(GTK_WIDGET(g))==TRUE)?1:0;
    return 0;

  case GWEN_DialogProperty_Width:
  case GWEN_DialogProperty_Height:
    /* just ignore these for now */
    return 0;

  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}



static GWENHYWFAR_CB
int Gtk4Gui_WLineEdit_SetCharProperty(GWEN_WIDGET *w,
                                      GWEN_DIALOG_PROPERTY prop,
                                      GWEN_UNUSED int index,
                                      const char *value,
                                      GWEN_UNUSED int doSignal)
{
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Value:
    if (value && *value)
      gtk_editable_set_text(GTK_EDITABLE(g), value);
    return 0;
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return GWEN_ERROR_INVALID;
}



static GWENHYWFAR_CB
const char *Gtk4Gui_WLineEdit_GetCharProperty(GWEN_WIDGET *w,
                                              GWEN_DIALOG_PROPERTY prop,
                                              GWEN_UNUSED int index,
                                              const char *defaultValue)
{
  GtkWidget *g;

  g=GTK_WIDGET(GWEN_Widget_GetImplData(w, GTK4_DIALOG_WIDGET_REAL));
  assert(g);

  switch (prop) {
  case GWEN_DialogProperty_Value:
    return gtk_editable_get_text(GTK_EDITABLE(g));
  default:
    break;
  }

  DBG_WARN(GWEN_LOGDOMAIN,
           "Function is not appropriate for this type of widget (%s)",
           GWEN_Widget_Type_toString(GWEN_Widget_GetType(w)));
  return defaultValue;
}


static void Gtk4Gui_WLineEdit_Text_Changed_handler(GWEN_UNUSED GtkEditable* self,
                                                   gpointer data)
{
  GWEN_WIDGET *w;
  int rv;

  w=data;
  assert(w);
  rv=GWEN_Dialog_EmitSignal(GWEN_Widget_GetDialog(w),
                            GWEN_DialogEvent_TypeValueChanged,
                            GWEN_Widget_GetName(w));
  if (rv==GWEN_DialogEvent_ResultAccept)
    Gtk4Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 1);
  else if (rv==GWEN_DialogEvent_ResultReject)
    Gtk4Gui_Dialog_Leave(GWEN_Widget_GetTopDialog(w), 0);
}

int Gtk4Gui_WLineEdit_Setup(GWEN_WIDGET *w)
{
  GtkWidget *g;
  const char *s;
  uint32_t flags;
  GWEN_WIDGET *wParent;
  gboolean text_is_visible;

  flags=GWEN_Widget_GetFlags(w);
  text_is_visible = (flags & GWEN_WIDGET_FLAGS_PASSWORD) == 0;
  wParent=GWEN_Widget_Tree_GetParent(w);
  s=GWEN_Widget_GetText(w, 0);

  /* create widget */
  g=gtk_entry_new();
  if (s && *s)
    gtk_editable_set_text(GTK_EDITABLE(g), s);
  gtk_entry_set_visibility(GTK_ENTRY(g), text_is_visible);

#if GTK4_GUI_DIALOG_DEFAULT_BOX_PADDING
 gtk_widget_set_margin_start(g, GTK4_GUI_DIALOG_DEFAULT_BOX_PADDING);
 gtk_widget_set_margin_end(g, GTK4_GUI_DIALOG_DEFAULT_BOX_PADDING);
 gtk_widget_set_margin_top(g, GTK4_GUI_DIALOG_DEFAULT_BOX_PADDING);
 gtk_widget_set_margin_bottom(g, GTK4_GUI_DIALOG_DEFAULT_BOX_PADDING);
#endif

  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_REAL, (void *) g);
  GWEN_Widget_SetImplData(w, GTK4_DIALOG_WIDGET_CONTENT, (void *) g);

  GWEN_Widget_SetSetIntPropertyFn(w, Gtk4Gui_WLineEdit_SetIntProperty);
  GWEN_Widget_SetGetIntPropertyFn(w, Gtk4Gui_WLineEdit_GetIntProperty);
  GWEN_Widget_SetSetCharPropertyFn(w, Gtk4Gui_WLineEdit_SetCharProperty);
  GWEN_Widget_SetGetCharPropertyFn(w, Gtk4Gui_WLineEdit_GetCharProperty);

  g_signal_connect(GTK_EDITABLE(g),
                   "changed",
                   G_CALLBACK(Gtk4Gui_WLineEdit_Text_Changed_handler),
                   w);
  if (wParent)
    GWEN_Widget_AddChildGuiWidget(wParent, w);

  return 0;
}


