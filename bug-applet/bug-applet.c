/* This applet, written by Elliot Lee, is an example of HOW not to structure your
programs in order to avoid bugs :) */

#include <applet-widget.h>
#include <parser.h>

typedef struct {
  GtkWidget *num_label, *brief_label;
  GtkWidget *desc_text;
} BugDisplay;

static BugDisplay bugd;

static GtkWidget *make_popup(void);

static const char *
node_get_attribute(xmlNodePtr node, const char *attr_name)
{
  xmlAttrPtr curattr;

  for(curattr = node->properties; curattr; curattr = curattr->next) {
    if(!strcasecmp(attr_name, curattr->name))
      return curattr->val->content; /* XXX bad hardcoded assertion */
  }

  return NULL;
}

static void
item_selected(GtkWidget *w)
{
  char *desc;
  gint position;

  gtk_label_set(GTK_LABEL(bugd.num_label),
		gtk_object_get_data(GTK_OBJECT(w), "id"));
  gtk_label_set(GTK_LABEL(bugd.brief_label),
		gtk_object_get_data(GTK_OBJECT(w), "title"));
  gtk_editable_delete_text(GTK_EDITABLE(bugd.desc_text), 0, -1);
  position = 0;
  desc = gtk_object_get_data(GTK_OBJECT(w), "desc");
  gtk_editable_insert_text(GTK_EDITABLE(bugd.desc_text), desc, strlen(desc), &position);
}

static void make_popup_menu(xmlNodePtr node, GtkWidget *item_parent, GtkWidget *item)
{
  xmlNodePtr cur;

  if(!strcasecmp(node->name, "document")) {

    for(cur = node->childs; cur; cur = cur->next)
      make_popup_menu(cur, item_parent, item);

  } else if(!strcasecmp(node->name, "bug")) {
    GtkWidget *new_item, *submenu;
    GList *t;

    new_item = gtk_menu_item_new();
    gtk_signal_connect(GTK_OBJECT(new_item), "select", GTK_SIGNAL_FUNC(item_selected), NULL);
    gtk_container_add(GTK_CONTAINER(item_parent), new_item);

    gtk_widget_show(new_item);
    submenu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_item), submenu);

    gtk_object_set_data(GTK_OBJECT(new_item), "id", (gpointer)node_get_attribute(node, "id"));

    for(cur = node->childs; cur; cur = cur->next)
      make_popup_menu(cur, submenu, new_item);

    t = gtk_container_children(GTK_CONTAINER(submenu));
    if(!t)
      gtk_menu_item_remove_submenu(GTK_MENU_ITEM(new_item));
    else
      g_list_free(t);
  } else if(!strcasecmp(node->name, "title")) {
    GtkWidget *wtmp;

    gtk_object_set_data(GTK_OBJECT(item), "title", node->childs->content);
    wtmp = gtk_label_new(node->childs->content);
    gtk_widget_show(wtmp);
    gtk_container_add(GTK_CONTAINER(item), wtmp);
  } else if(!strcasecmp(node->name, "desc")) {
    
    gtk_object_set_data(GTK_OBJECT(item), "desc", node->childs->content);
  }
}

static void
toggle_visibility(GtkWidget *w, GtkWidget *win)
{
  if(GTK_WIDGET_VISIBLE(win))
    gtk_widget_hide(win);
  else {
    gtk_label_set(GTK_LABEL(bugd.num_label), "");
    gtk_label_set(GTK_LABEL(bugd.brief_label), "");
    gtk_editable_delete_text(GTK_EDITABLE(bugd.desc_text), 0, -1);
    gtk_widget_show(win);
  }
  return;
  w = NULL;
}

static gboolean do_hide(GtkWidget *w, GdkEvent *ev, GtkWidget *win)
{
  toggle_visibility(w, win);

  return TRUE;
  ev = NULL;
}

int main(int argc, char *argv[])
{
  GtkWidget *applet, *btn, *popup;

  applet_widget_init("bug-applet", "0.1", argc, argv, NULL, 0, NULL);

  applet = applet_widget_new("bug-applet");
  btn = gnome_stock_button(GNOME_STOCK_PIXMAP_EXEC);
  popup = make_popup();
  gtk_signal_connect(GTK_OBJECT(btn), "clicked", GTK_SIGNAL_FUNC(toggle_visibility), popup);

  applet_widget_add(APPLET_WIDGET(applet), btn);
  gtk_widget_show_all(applet);

  applet_widget_gtk_main();

  return 0;
}

static GtkWidget *
make_popup(void)
{
  GtkWidget *retval, *vbox, *mb, *hbox, *wtmp, *wtmp2;
  xmlDocPtr doc;
  char *file;

  retval = gtk_window_new(GTK_WINDOW_DIALOG);

  gtk_signal_connect(GTK_OBJECT(retval), "delete_event", GTK_SIGNAL_FUNC(do_hide), retval);

  vbox = gtk_vbox_new(FALSE, GNOME_PAD);

  hbox = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
  wtmp = gtk_label_new("Bugtype number:");
  gtk_container_add(GTK_CONTAINER(hbox), wtmp);
  bugd.num_label = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(hbox), bugd.num_label);
  gtk_container_add(GTK_CONTAINER(vbox), hbox);

  hbox = gtk_hbox_new(TRUE, GNOME_PAD_SMALL);
  wtmp = gtk_label_new("Brief:");
  gtk_container_add(GTK_CONTAINER(hbox), wtmp);
  bugd.brief_label = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(hbox), bugd.brief_label);
  gtk_container_add(GTK_CONTAINER(vbox), hbox);

  bugd.desc_text = gtk_text_new(NULL, NULL);
  gtk_text_set_editable(GTK_TEXT(bugd.desc_text), FALSE);
  gtk_container_add(GTK_CONTAINER(vbox), bugd.desc_text);

  file = gnome_datadir_file("bug-applet/bugtypes.xml");
  g_print ("%s\n", file);
  if (!g_file_exists (file))
	  g_warning ("can't find file");
  /*file = "bugtypes.xml";*/

  doc = xmlParseFile(file);
  g_free (file);
  if (!doc) {
    gtk_container_add(GTK_CONTAINER(vbox),
		      gtk_label_new("Error loading bug-applet/bugtypes.xml"));
    goto out;
  }

  mb = gtk_menu_bar_new();
  gtk_container_add(GTK_CONTAINER(vbox), mb);

  wtmp = gtk_menu_item_new_with_label("Classifications");
  gtk_widget_show(wtmp);
  gtk_container_add(GTK_CONTAINER(mb), wtmp);
  wtmp2 = gtk_menu_new();
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(wtmp), wtmp2);

  make_popup_menu(doc->root, wtmp2, NULL);

 out:
  gtk_widget_show_all(vbox);

  gtk_container_add(GTK_CONTAINER(retval), vbox);

  return retval;
}
