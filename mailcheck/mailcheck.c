/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* GNOME panel mail check module.
 * (C) 1997, 1998, 1999, 2000 The Free Software Foundation
 * (C) 2001 Eazel, Inc.
 *
 * Authors: Miguel de Icaza
 *          Jacob Berkman
 *          Jaka Mocnik
 *          Lennart Poettering
 *          George Lebl
 *
 */

#include <config.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <libgnome/libgnome.h>
#include <libgnomeui/libgnomeui.h>
#include <gconf/gconf-client.h>
#include <panel-applet.h>
#include <panel-applet-gconf.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <libgnomeui/gnome-window-icon.h>
#include <stdlib.h>
#include <ctype.h>

#include "popcheck.h"
#include "remote-helper.h"
#include "mailcheck.h"

#include "egg-screen-help.h"
#include "egg-screen-exec.h"

#define ENCODE 1  
#define DECODE 0

#define NEVER_SENSITIVE "never_sensitive"

static const char *to_b64 =
 "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* encode 72 characters per line */
#define CHARS_PER_LINE 72 

typedef enum {
	MAILBOX_LOCAL,
	MAILBOX_LOCALDIR,
	MAILBOX_POP3,
	MAILBOX_IMAP
} MailboxType;

typedef struct
{
        int     size;
        char    *data;
} memchunk;

typedef struct _MailCheck MailCheck;
struct _MailCheck {
	char *mail_file;

	/* Does the user have any mail at all? */
	gboolean anymail;

	/* whether new mail has arrived */
	gboolean newmail;

	/* number of unread/total mails */
	int unreadmail;
	int totalmail;

	/* whether to automatically check for mails */
	gboolean auto_update;

	/* interval to check for mails in milliseconds */
	guint update_freq;

	/* Pref is reset_on_clicked. Show animation is intended to restart animation for newmail */
	gboolean reset_on_clicked;
	gboolean show_animation;

	/* execute a command when the applet is clicked (launch email prog) */
	char *clicked_cmd;
	gboolean clicked_enabled;

	/* execute a command when new mail arrives (play a sound etc.) */
	char *newmail_cmd;
	gboolean newmail_enabled;

	/* execute a command before checking email (fetchmail etc.) */
	char *pre_check_cmd;
	gboolean pre_check_enabled;	

	PanelApplet *applet;
	/* This is the event box for catching events */
	GtkWidget *ebox;

	/* This holds either the drawing area or the label */
	GtkWidget *bin;

	/* The widget that holds the label with the mail information */
	GtkWidget *label;

	/* Points to whatever we have inside the bin */
	GtkWidget *containee;

	/* The drawing area */
	GtkWidget *da;
	GdkPixmap *email_pixmap;
	GdkBitmap *email_mask;

	/* handle for the timeout */
	int mail_timeout;

	/* how do we report the mail status */
	enum {
		REPORT_MAIL_USE_TEXT,
		REPORT_MAIL_USE_BITMAP,
		REPORT_MAIL_USE_ANIMATION
	} report_mail_mode;

	/* current frame on the animation */
	int nframe;

	/* number of frames on the pixmap */
	int frames;

	/* handle for the animation timeout handler */
	int animation_tag;

	/* for the selection routine */
	char *selected_pixmap_name;

	/* the about box */
	GtkWidget *about;

	/* The property window */
	GtkWidget *property_window;
	GtkWidget *min_spin, *sec_spin;
	GtkWidget *pre_check_cmd_entry, *pre_check_cmd_check;
	GtkWidget *newmail_cmd_entry, *newmail_cmd_check;
	GtkWidget *clicked_cmd_entry, *clicked_cmd_check;

	GtkWidget *password_dialog;

	gboolean anim_changed;

	char *mailcheck_text_only;

	char *animation_file;
        
	GtkWidget *mailfile_entry, *mailfile_label, *mailfile_fentry;
	GtkWidget *remote_server_entry, *remote_username_entry, *remote_password_entry, *remote_folder_entry;
	GtkWidget *remote_server_label, *remote_username_label, *remote_password_label, *remote_folder_label;
	GtkWidget *remote_option_menu;
	GtkWidget *remote_password_checkbox;
	GtkWidget *play_sound_check;
        
	char *remote_server, *remote_username, *remote_password, *real_password, *remote_folder;
	char *remote_encrypted_password;

	MailboxType mailbox_type; /* local = 0; maildir = 1; pop3 = 2; imap = 3 */
        MailboxType mailbox_type_temp;

	gboolean play_sound;

	gboolean save_remote_password;

	int type; /*mailcheck = 0; mailbox = 1 */
	
	off_t oldsize;
	
	int size;

	gulong applet_realized_signal;

	/* see remote-helper.h */
	gpointer remote_handle;
};

static char*           b64enc (const memchunk *chunk);
static memchunk*       b64dec (const char *string);
static memchunk*       memchunkAlloc (int size);
static char*           util_base64 (char *data, int flag);
static void            remote_password_save_toggled (GtkToggleButton *button, 
				gpointer data);
static void            remote_password_changed (GtkEntry *entry, gpointer data);
static gboolean        focus_out_cb (GtkWidget *entry, GdkEventFocus *event, gpointer data);

static int mail_check_timeout (gpointer data);
static void after_mail_check (MailCheck *mc);

static void applet_load_prefs(MailCheck *mc);

static void set_atk_name_description (GtkWidget *widget, const gchar *name,
					const gchar *description);
static void set_atk_relation (GtkWidget *label, GtkWidget *entry, AtkRelationType);
static void got_remote_answer (int mails, gpointer data);
static void error_handler (int mails, gpointer data);
static void null_remote_handle (gpointer data);

#define WANT_BITMAPS(x) (x == REPORT_MAIL_USE_ANIMATION || x == REPORT_MAIL_USE_BITMAP)
#define HIG_IDENTATION  "    "

static memchunk* 
memchunkAlloc(int size)
{
	memchunk *tmp = (memchunk*) calloc(1, sizeof(memchunk));

	if (tmp) {
		tmp->size = size;
		tmp->data = (char *) malloc(size);

		if (tmp->data == (char *) 0) {
			free(tmp);
			tmp = 0;
		}
	}

	return tmp;
}

/*
 * b64enc:
 * @chunk: A structure which contains the data to be encrypted.
 * 
 * This routine encrypts the data using base64 encryption technique
 *
 * Returns the encrypted data in a character array.
 */


static char* 
b64enc(const memchunk *chunk)
{
	int div = chunk->size / 3;
	int rem = chunk->size % 3;
	int chars = div*4 + rem + 1;
	int newlines = (chars + CHARS_PER_LINE - 1) / CHARS_PER_LINE;

	const char *data = chunk->data;
	char *string = (char *) malloc(chars + newlines + 1);

	if (string) {
		register char* buf = string;

		chars = 0;

		/*@+charindex@*/
		while (div > 0) {
			buf[0] = to_b64[ (data[0] >> 2) & 0x3f];
			buf[1] = to_b64[((data[0] << 4) & 0x30) +
				 ((data[1] >> 4) & 0xf)];
			buf[2] = to_b64[((data[1] << 2) & 0x3c) +
				 ((data[2] >> 6) & 0x3)];
			buf[3] = to_b64[  data[2] & 0x3f];
			data += 3;
			buf += 4;
			div--;
			chars += 4;
			if (chars == CHARS_PER_LINE) {
				chars = 0;
				*(buf++) = '\n';
			}
		}

		switch (rem) {
			case 2:
				buf[0] = to_b64[ (data[0] >> 2) & 0x3f];
				buf[1] = to_b64[((data[0] << 4) & 0x30) +
					 ((data[1] >> 4) & 0xf)];
				buf[2] = to_b64[ (data[1] << 2) & 0x3c];
				buf[3] = '=';
				buf += 4;
				chars += 4;
				break;
			case 1:
				buf[0] = to_b64[ (data[0] >> 2) & 0x3f];
				buf[1] = to_b64[ (data[0] << 4) & 0x30];
				buf[2] = '=';
				buf[3] = '=';
				buf += 4;
				chars += 4;
				break;
		}
		/*@=charindex@*/

	 /*      *(buf++) = '\n'; This would result in a buffer overrun */
		*buf = '\0';
	 }

	return string;
}

/*
 * b64dec:
 * @string: A character array which contains the data to be decoded.
 *
 * This routine decrypts the data using base64 decryption technique
 *
 * Returns the decrypted data.
 */

static memchunk* 
b64dec(const char *string)
{

/* return a decoded memchunk, or a null pointer in case of failure */

	memchunk *rc = 0;

	if (string) {
	register int length = strlen(string);

	/* do a format verification first */
		if (length > 0) {
			register int count = 0, rem = 0;
			register const char* tmp = string;

			while (length > 0) {
				register int skip = strspn(tmp, to_b64);
				count += skip;
				length -= skip;
				tmp += skip;
				if (length > 0) {
					register int i, vrfy =
						 strcspn(tmp, to_b64);

					for (i = 0; i < vrfy; i++) {
						if (isspace(tmp[i]))
							continue;

						if (tmp[i] == '=') {
							/* we should check if
							 * we're close to the
							 * end of the string */
							rem = count % 4;

							/* rem must be either
							 * 2 or 3, otherwise
							 * no '=' should be
							 * here */
							if (rem < 2)
								return 0;

							/* end-of-message
							 * recognized */
							break;
						} else {
							/* Transmission error */

							return 0;
						}
					}

					length -= vrfy;
					tmp += vrfy;
				}
			}

			rc = memchunkAlloc((count / 4) * 3 + (rem ? (rem - 1) : 0));

			if (rc) {
				if (count > 0) {
					register int i, qw = 0, tw = 0;
					register char * data = rc->data;

					length = strlen(tmp = string);

					for (i = 0; i < length; i++) {
						register char ch = string[i];
						register char bits;

						if (isspace(ch))
							continue;

						bits = 0;
						if ((ch >= 'A') && (ch <= 'Z')) {
							bits = (char)
							       (ch - 'A');
						}
						else if ((ch >= 'a') &&
							 (ch <= 'z')) {
							bits = (char)
							       (ch - 'a' + 26);
						}
						else if ((ch >= '0') &&
							 (ch <= '9')) {
							bits = (char)
							       (ch - '0' + 52);
						}
						else if (ch == '=')
							break;

						switch (qw++) {
							case 0:
								data[tw+0] =
									 (bits << 2) & 0xfc;
								break;
							case 1:
								data[tw+0] |=
									 (bits >> 4) & 0x03;
								data[tw+1] =
									 (bits << 4) & 0xf0;
								break;
							case 2:
								data[tw+1] |=
									 (bits >> 2) & 0x0f;
								data[tw+2] =
									 (bits << 6) & 0xc0;
								break;
							case 3:
								data[tw+2] |=
									 bits & 0x3f;
								break;
						}

						if (qw == 4) {
							qw = 0;
							tw += 3;
						}
					}
				}
			}
		}
	}

	return rc;
}


/*
 * util_base64:
 * @data: A character array which contains the data.
 * @flag: Indicates the action to be performed ENCODE/DECODE
 *
 * This is the interface called by other routines for encoding or decoding using
 * base64 encryption/decryption technique. The action is performed based on the flag.
 *
 * Returns either the encrypted data or the decrypted data based on the flag.
 */

static char* 
util_base64(char* data, int flag)
{
	char *enc_data;
	memchunk chunk;
	memchunk *ret_data;
	char *dec_data;

	switch (flag) {
		case ENCODE :

			chunk.data = data;
			chunk.size = strlen(data);
			enc_data = b64enc(&chunk);

			return enc_data;

		case DECODE :

			ret_data = (memchunk *) b64dec((char *)data);
			if(ret_data) {
				dec_data = g_strndup(ret_data->data,
						     ret_data->size);
				free (ret_data);
			}
			else
				dec_data = NULL;

			return dec_data;

		default :
			return NULL;

	}
}

static gboolean
key_writable (PanelApplet *applet, const char *key)
{
	gboolean writable;
	char *fullkey;
	static GConfClient *client = NULL;
	if (client == NULL)
		client = gconf_client_get_default ();

	fullkey = panel_applet_gconf_get_full_key (applet, key);

	writable = gconf_client_key_is_writable (client, fullkey, NULL);

	g_free (fullkey);

	return writable;
}

/* set sensitive and setup NEVER_SENSITIVE appropriately */
static void
hard_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	gtk_widget_set_sensitive (w, sensitivity);
	g_object_set_data (G_OBJECT (w), NEVER_SENSITIVE,
			   GINT_TO_POINTER ( ! sensitivity));
}


/* set sensitive, but always insensitive if NEVER_SENSITIVE is set */
static void
soft_set_sensitive (GtkWidget *w, gboolean sensitivity)
{
	if (g_object_get_data (G_OBJECT (w), NEVER_SENSITIVE))
		gtk_widget_set_sensitive (w, FALSE);
	else
		gtk_widget_set_sensitive (w, sensitivity);
}

/* stolen from gsearchtool */
static GtkWidget*
hig_dialog_new (GtkWindow      *parent,
		GtkDialogFlags flags,
		GtkMessageType type,
		GtkButtonsType buttons,
		const gchar    *header,
		const gchar    *message)
{
	GtkWidget *dialog;
	GtkWidget *dialog_vbox;
	GtkWidget *dialog_action_area;
	GtkWidget *hbox;
	GtkWidget *vbox;
	GtkWidget *label;
	GtkWidget *button;
	GtkWidget *image = NULL;
	gchar     *title;

	dialog = gtk_dialog_new ();
	
	gtk_dialog_set_has_separator (GTK_DIALOG (dialog), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_title (GTK_WINDOW (dialog), "");
  
	dialog_vbox = GTK_DIALOG (dialog)->vbox;
	gtk_box_set_spacing (GTK_BOX (dialog_vbox), 14);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (dialog_vbox), hbox, FALSE, FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (hbox), 5);
	gtk_widget_show (hbox);

	if (type == GTK_MESSAGE_ERROR) {
		image = gtk_image_new_from_stock ("gtk-dialog-error", GTK_ICON_SIZE_DIALOG);
	} else if (type == GTK_MESSAGE_QUESTION) {
		image = gtk_image_new_from_stock ("gtk-dialog-question", GTK_ICON_SIZE_DIALOG);
	} else {
		g_assert_not_reached ();
	}
	gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
	gtk_widget_show (image);

	vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
	gtk_widget_show (vbox);
	
	title = g_strconcat ("<b>", header, "</b>", NULL);
	label = gtk_label_new (title);  
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_show (label);
	g_free (title);
	
	label = gtk_label_new (message);
	gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_widget_show (label);
	
	dialog_action_area = GTK_DIALOG (dialog)->action_area;
	gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area), GTK_BUTTONBOX_END);

	switch (buttons) 
  	{		
		case GTK_BUTTONS_OK_CANCEL:
	
			button = gtk_button_new_from_stock ("gtk-cancel");
  			gtk_widget_show (button);
  			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_CANCEL);
  			GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);

		  	button = gtk_button_new_from_stock ("gtk-ok");
  			gtk_widget_show (button);
  			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);
  			GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
			break;
		
		case GTK_BUTTONS_OK:
		
			button = gtk_button_new_from_stock ("gtk-ok");
			gtk_dialog_add_action_widget (GTK_DIALOG (dialog), button, GTK_RESPONSE_OK);
			GTK_WIDGET_SET_FLAGS (button, GTK_CAN_DEFAULT);
			gtk_widget_show (button);
			break;
		
		default:
			g_warning ("Unhandled GtkButtonsType");
			break;
  	}

	if (parent != NULL) {
		gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (parent));
	}
	if (flags & GTK_DIALOG_MODAL) {
		gtk_window_set_modal (GTK_WINDOW (dialog), TRUE);
	}
	if (flags & GTK_DIALOG_DESTROY_WITH_PARENT) {
		gtk_window_set_destroy_with_parent (GTK_WINDOW (dialog), TRUE);
	}
	
  	return dialog;
}

static void
set_tooltip (GtkWidget  *applet,
	     const char *tip)
{
	GtkTooltips *tooltips;

	tooltips = g_object_get_data (G_OBJECT (applet), "tooltips");
	if (!tooltips) {
		tooltips = gtk_tooltips_new ();
		g_object_ref (tooltips);
		gtk_object_sink (GTK_OBJECT (tooltips));
		g_object_set_data_full (
			G_OBJECT (applet), "tooltips", tooltips,
			(GDestroyNotify) g_object_unref);
	}

	gtk_tooltips_set_tip (tooltips, applet, tip, NULL);
	set_atk_name_description (applet, tip, "");
}

static void
mailcheck_execute_shell (MailCheck  *mailcheck,
			 const char *command)
{
	GError *error = NULL;

	gdk_spawn_command_line_on_screen (gtk_widget_get_screen (GTK_WIDGET (mailcheck->applet)),
					  command, &error); 

	if (error) {
		GtkWidget *dialog;
		char *msg;

		msg = g_strdup_printf (_("There was an error executing %s: %s"),
				       command,
				       error->message);
		dialog = hig_dialog_new (NULL /* parent */,
					 0 /* flags */,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 _("Error checking mail"),
					 msg);
		g_free (msg);

		g_signal_connect (dialog, "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (mailcheck->applet)));

		gtk_widget_show (dialog);

		g_error_free (error);
	}
}

void
command_execute_shell (gpointer data,
                       const char *command)
{
        MailCheck *mc = (MailCheck *) data;
        mailcheck_execute_shell (mc, command);
}

static G_CONST_RETURN char *
mail_animation_filename (MailCheck *mc)
{
	if (!mc->animation_file) {
		mc->animation_file =
			gnome_program_locate_file (
				NULL, GNOME_FILE_DOMAIN_PIXMAP,
				"mailcheck/email.png", TRUE, NULL);

		return mc->animation_file;

	} else if (mc->animation_file [0]) {
		if (g_file_test (mc->animation_file, G_FILE_TEST_EXISTS))
			return mc->animation_file;

		g_free (mc->animation_file);
		mc->animation_file = NULL;

		return NULL;
	} else
		/* we are using text only, since the filename was "" */
		return NULL;
}

static int
calc_dir_contents (char *dir)
{
       DIR *dr;
       struct dirent *de;
       int size=0;

       dr = opendir(dir);
       if (dr == NULL)
               return 0;
       while((de = readdir(dr))) {
               if (strlen(de->d_name) < 1 || de->d_name[0] == '.')
                       continue;
               size ++;
       }
       closedir(dr);
       return size;
}

static void
check_remote_mailbox (MailCheck *mc)
{
	if (!mc->real_password || !mc->remote_username || !mc->remote_server)
		return;

	if (mc->mailbox_type == MAILBOX_POP3)
		mc->remote_handle = helper_pop3_check (got_remote_answer,
						       error_handler,
						       mc,
						       null_remote_handle,
						       NULL,
						       mc->remote_server,
						       mc->remote_username,
						       mc->real_password);
	else if (mc->mailbox_type == MAILBOX_IMAP)
		mc->remote_handle = helper_imap_check (got_remote_answer,
						       error_handler,
						       mc,
						       null_remote_handle,
						       NULL,
						       mc->remote_server,
						       mc->remote_username,
						       mc->real_password,
						       mc->remote_folder);
}

static void
password_response_cb (GtkWidget  *dialog,
		      int         response_id,
		      MailCheck  *mc)
{

	switch (response_id) {
		GtkWidget *entry;
		GtkWidget *save_toggle_button;

	case GTK_RESPONSE_OK:
		entry = g_object_get_data (G_OBJECT (dialog), "password_entry");
		mc->real_password = g_strdup (gtk_entry_get_text (GTK_ENTRY (entry)));
		save_toggle_button = g_object_get_data (G_OBJECT (dialog), "save_password");
		if (gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (save_toggle_button)))
			 remote_password_save_toggled (GTK_TOGGLE_BUTTON (save_toggle_button), mc);
		check_remote_mailbox (mc);
		break;
	}

	gtk_widget_destroy (dialog);
	mc->password_dialog = NULL;
}
static void
get_remote_password (MailCheck *mc)
{
	GtkWidget *dialog;
	GtkWidget *hbox;
	GtkWidget *label;
	GtkWidget *entry;
	GtkWidget *save_password_checkbox;

	if (mc->password_dialog) {
		gtk_window_set_screen (GTK_WINDOW (mc->password_dialog),
				       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
		gtk_window_present (GTK_WINDOW (mc->password_dialog));
		return;
	}

	mc->password_dialog = dialog =
		gtk_dialog_new_with_buttons (
			_("Inbox Monitor"), NULL, 0,
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);

	gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

	label = gtk_label_new (_("You didn't set a password in the preferences for the Inbox Monitor,\nso you have to enter it each time it starts up."));
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), label, FALSE, FALSE, GNOME_PAD_BIG);
	gtk_widget_show (label);

	hbox = gtk_hbox_new (FALSE, 1);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox),
			    hbox, FALSE, FALSE, GNOME_PAD_SMALL);

	label = gtk_label_new_with_mnemonic (_("Please enter your mailserver's _password:"));
	gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);

	entry = gtk_entry_new ();

	set_atk_name_description (entry, _("Password Entry box"), "");
	set_atk_relation (entry, label, ATK_RELATION_LABELLED_BY);	
	gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);
	gtk_entry_set_activates_default (GTK_ENTRY (entry), TRUE);
	gtk_box_pack_start (GTK_BOX (hbox), entry, FALSE, FALSE, 0);
	gtk_widget_show_all (hbox);
	gtk_widget_grab_focus (GTK_WIDGET (entry));

	hbox = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (dialog)->vbox), hbox, 
			    FALSE, FALSE, GNOME_PAD_SMALL);

	gtk_widget_show (hbox);  
	
	save_password_checkbox = gtk_check_button_new_with_mnemonic (
						_("_Save password to disk"));
	if ( ! key_writable (mc->applet, "remote_encrypted_password")) {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (save_password_checkbox), 
					      FALSE);
		hard_set_sensitive (save_password_checkbox, FALSE);
	} else {
		gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (save_password_checkbox), 
					      mc->save_remote_password);
	}

	if ( ! key_writable (mc->applet, "save_password")) {
		hard_set_sensitive (save_password_checkbox, FALSE);
	}

	gtk_widget_show (save_password_checkbox);
	gtk_box_pack_start (GTK_BOX (hbox), save_password_checkbox, FALSE, FALSE, 0);
	
	gtk_window_set_screen (GTK_WINDOW (dialog),
			       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));

	g_signal_connect (dialog, "response",
                          G_CALLBACK (password_response_cb), mc);

	g_object_set_data (G_OBJECT (dialog), "save_password", save_password_checkbox);
	g_object_set_data (G_OBJECT (dialog), "password_entry", entry);
	gtk_widget_show (GTK_WIDGET (dialog));
}

static void
got_remote_answer (int mails, gpointer data)
{
	MailCheck *mc = data;
	int old_unreadmail;

	mc->remote_handle = NULL;
	
	old_unreadmail = mc->unreadmail;
	mc->unreadmail = (signed int) (((unsigned int) mails) >> 16);
	if(mc->unreadmail > old_unreadmail) /* lt */
		mc->newmail = 1;
	else
		mc->newmail = 0;
	mc->totalmail = (signed int) (((unsigned int) mails) & 0x0000FFFFL);
	mc->anymail = mc->totalmail ? 1 : 0;

	after_mail_check (mc);
}

static void
error_handler (int error, gpointer data)
{
	MailCheck *mc = data;
	gchar *details;
	
	switch (error) {
	case NETWORK_ERROR:
		details = _("Could not connect to the Internet.");
		break;
	case INVALID_USER:
	case INVALID_PASS:
		details = _("The username or password is incorrect.");
		break;	
	case INVALID_SERVER:
		details = _("The server name is incorrect.");
		break;
	case NO_SERVER_INFO:
	default:
		details = _("Error connecting to mail server.");
		break;
	}
	/* FIXME: Show some sort of error images for non text mode */
	if (mc->report_mail_mode == REPORT_MAIL_USE_TEXT) {
		gtk_label_set_text (GTK_LABEL (mc->label), _("Error"));
	}
		
	set_tooltip (GTK_WIDGET (mc->applet), details);
	if (mc->animation_tag != 0){
		gtk_timeout_remove (mc->animation_tag);
		mc->animation_tag = 0;
	}
}

static void
applet_realized_cb (GtkWidget *widget, gpointer data)
{
	MailCheck *mc = data;
	mail_check_timeout (mc);
	g_signal_handler_disconnect (G_OBJECT(widget), mc->applet_realized_signal);
}

static void
null_remote_handle (gpointer data)
{
	MailCheck *mc = data;

	mc->remote_handle = NULL;
}

/*
 * Get file modification time, based upon the code
 * of Byron C. Darrah for coolmail and reused on fvwm95
 */
static void
check_mail_file_status (MailCheck *mc)
{
	off_t newsize;
	struct stat s;
	int status;
	
	if ((mc->mailbox_type == MAILBOX_POP3) || 
	    (mc->mailbox_type == MAILBOX_IMAP)) {
		if (mc->remote_handle != NULL)
			/* check in progress */
			return;

		if (mc->remote_password != NULL &&
		    mc->remote_password[0] != '\0') {
			g_free (mc->real_password);
			mc->real_password = g_strdup (mc->remote_password);

		} else if (!mc->real_password)
			get_remote_password (mc);

		check_remote_mailbox (mc);
	}
	else if (mc->mailbox_type == MAILBOX_LOCAL) {
		status = stat (mc->mail_file, &s);
		if (status < 0) {
			mc->oldsize = 0;
			mc->anymail = mc->newmail = mc->unreadmail = 0;
			after_mail_check (mc);
			return;
		}
		
		newsize = s.st_size;
		mc->anymail = newsize > 0;
		mc->unreadmail = (s.st_mtime >= s.st_atime && newsize > 0);
		
		if (newsize != mc->oldsize && mc->unreadmail)
			mc->newmail = 1;
		else
			mc->newmail = 0;
		
		mc->oldsize = newsize;

		after_mail_check (mc);
	}
	else if (mc->mailbox_type == MAILBOX_LOCALDIR) {
		int newmail, oldmail;
		char tmp[1024];
		g_snprintf(tmp, sizeof (tmp), "%s/new", mc->mail_file);
		newmail = calc_dir_contents(tmp);
		g_snprintf(tmp, sizeof (tmp), "%s/cur", mc->mail_file);
		oldmail = calc_dir_contents(tmp);
		mc->newmail = newmail > mc->oldsize;
		mc->unreadmail = newmail;
		mc->oldsize = newmail;
		mc->anymail = newmail || oldmail;
		mc->totalmail = newmail + oldmail;

		after_mail_check (mc);
	}
}

static gboolean
mailcheck_load_animation (MailCheck *mc, const char *fname)
{
	int width, height;
	int pbwidth, pbheight;
	GdkPixbuf *pb;

	if (mc->email_pixmap)
		g_object_unref (mc->email_pixmap);

	if (mc->email_mask)
		g_object_unref (mc->email_mask);

	mc->email_pixmap = NULL;
	mc->email_mask = NULL;

	pb = gdk_pixbuf_new_from_file (fname, NULL);
	if (!pb)
		return FALSE;

	pbwidth = gdk_pixbuf_get_width (pb);
	pbheight = gdk_pixbuf_get_height (pb);

	if(pbheight != mc->size) {
		GdkPixbuf *pbt;
		height = mc->size;
		width = pbwidth*((double)height/pbheight);

		pbt = gdk_pixbuf_scale_simple(pb, width, height,
					      GDK_INTERP_NEAREST);
		g_object_unref (pb);
		pb = pbt;
	} else {
		width = pbwidth;
		height = pbheight;
	}

	/* yeah, they have to be square, in case you were wondering :-) */
	mc->frames = width / height;
	if (mc->frames < 3)
		return FALSE;
	else if (mc->frames == 3)
		mc->report_mail_mode = REPORT_MAIL_USE_BITMAP;
	else
		mc->report_mail_mode = REPORT_MAIL_USE_ANIMATION;
	mc->nframe = 0;

	mc->email_pixmap = gdk_pixmap_new (gdk_screen_get_root_window (
					   gtk_widget_get_screen (GTK_WIDGET (mc->applet))),
					   width, height, -1);

	gdk_draw_pixbuf (mc->email_pixmap, mc->da->style->black_gc, pb,
			 0, 0, 0, 0, width, height,
			 GDK_RGB_DITHER_NORMAL, 0, 0);
	
	g_object_unref (pb);
	
	return TRUE;
}

static int
next_frame (gpointer data)
{
	MailCheck *mc = data;

	mc->nframe = (mc->nframe + 1) % mc->frames;
	if (mc->nframe == 0)
		mc->nframe = 1;

	gtk_widget_queue_draw (mc->da);

	return TRUE;
}

static void
after_mail_check (MailCheck *mc)
{
	static const char *supinfo[] = {"mailcheck", "new-mail", NULL};
	char *text;
	char *plural1, *plural2;

	if (mc->anymail){
		if(mc->mailbox_type == MAILBOX_LOCAL) {
			if(mc->newmail)
				text = g_strdup(_("You have new mail."));
			else
				text = g_strdup(_("You have mail."));
		}
		else {
			if (mc->unreadmail) {
				plural1 = g_strdup_printf(ngettext ("%d unread", "%d unread",  mc->unreadmail), mc->unreadmail);
				plural2 = g_strdup_printf(ngettext ("%d message", "%d messages", mc->totalmail), mc->totalmail);

				/* translators: this is of the form "%d unread/%d messages" */
				text = g_strdup_printf(_("%s/%s"), plural1, plural2);

				g_free (plural1);
				g_free (plural2);
			}
			else
				text = g_strdup_printf(ngettext ("%d message",
								 "%d messages",
								 mc->totalmail),
						       mc->totalmail);
		} 
	}
	else
		text = g_strdup_printf(_("No mail."));

	if (mc->newmail) {
		mc->show_animation = TRUE;
		
		if(mc->play_sound)
			gnome_triggers_vdo("You've got new mail!", "program", supinfo);

		if (mc->newmail_enabled &&
		    mc->newmail_cmd && 
		    (strlen(mc->newmail_cmd) > 0))
			mailcheck_execute_shell (mc, mc->newmail_cmd);
	}

	switch (mc->report_mail_mode) {
	case REPORT_MAIL_USE_ANIMATION:
		if (mc->anymail){
			if (mc->unreadmail){
				if (mc->animation_tag == 0 && mc->show_animation){
					mc->animation_tag = gtk_timeout_add (150, next_frame, mc);
					mc->nframe = 1;
				}
			} else {
				if (mc->animation_tag != 0){
					gtk_timeout_remove (mc->animation_tag);
					mc->animation_tag = 0;
				}
				mc->nframe = 1;
			}
		} else {
			if (mc->animation_tag != 0){
				gtk_timeout_remove (mc->animation_tag);
				mc->animation_tag = 0;
			}
			mc->nframe = 0;
		}

		gtk_widget_queue_draw (mc->da);

		break;
	case REPORT_MAIL_USE_BITMAP:
		if (mc->anymail){
			if (mc->newmail)
				mc->nframe = 2;
			else
				mc->nframe = 1;
		} else
			mc->nframe = 0;

		gtk_widget_queue_draw (mc->da);

		break;
	case REPORT_MAIL_USE_TEXT:
		gtk_label_set_text (GTK_LABEL (mc->label), text);
		break;
	}

	set_tooltip (GTK_WIDGET (mc->applet), text);
	g_free (text);
}

static gboolean
mail_check_timeout (gpointer data)
{
	MailCheck *mc = data;

	if (mc->pre_check_enabled &&
	    mc->pre_check_cmd && 
	    (strlen(mc->pre_check_cmd) > 0)){
		/*
		 * if we have to execute a command before checking for mail, we
		 * remove the mail-check timeout and re-add it after the command
		 * returns, just in case the execution takes too long.
		 */
		
		if(mc->mail_timeout != 0) {
			gtk_timeout_remove (mc->mail_timeout);
			mc->mail_timeout = 0;
		}

		mailcheck_execute_shell (mc, mc->pre_check_cmd);

		if(mc->auto_update)
			mc->mail_timeout = gtk_timeout_add(mc->update_freq, mail_check_timeout, mc);
	}

	check_mail_file_status (mc);
	
	if (mc->auto_update)      
		return TRUE;
	else
		/* This handler should just run once */
		return FALSE;
}

/*
 * this gets called when we have to redraw the nice icon
 */
static gint
icon_expose (GtkWidget *widget, GdkEventExpose *event, gpointer data)
{
	MailCheck *mc = data;
	int        h = mc->size;

	gdk_draw_drawable (
		mc->da->window, mc->da->style->black_gc,
		mc->email_pixmap, mc->nframe * h,
		0, 0, 0, h, h);

	return TRUE;
}

static gint
exec_clicked_cmd (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
	MailCheck *mc = data;
	gboolean retval = FALSE;

	if (event->button == 1 && event->type == GDK_BUTTON_PRESS) {

		if (mc->clicked_enabled && mc->clicked_cmd && (strlen(mc->clicked_cmd) > 0))
			mailcheck_execute_shell (mc, mc->clicked_cmd);

		if (mc->reset_on_clicked) {
	
			mc->show_animation = FALSE;
			if (mc->animation_tag != 0){
				gtk_timeout_remove (mc->animation_tag);
				mc->animation_tag = 0;
			}
		}

		retval = TRUE;
	}	
	return(retval);
}

static gboolean
key_press_cb (GtkWidget *widget, GdkEventKey *event, MailCheck *mc)
{
	switch (event->keyval) {	
	case GDK_u:
		if (event->state == GDK_CONTROL_MASK) {
			mail_check_timeout(mc);
			return TRUE;
		}
		break;
	case GDK_KP_Enter:
	case GDK_ISO_Enter:
	case GDK_3270_Enter:
	case GDK_Return:
	case GDK_space:
	case GDK_KP_Space:
		if (mc->clicked_enabled && mc->clicked_cmd && (strlen(mc->clicked_cmd) > 0))
			mailcheck_execute_shell (mc, mc->clicked_cmd);

		if (mc->reset_on_clicked) {
	
			mc->show_animation = FALSE;
			if (mc->animation_tag != 0){
				gtk_timeout_remove (mc->animation_tag);
				mc->animation_tag = 0;
			}
		}
		return TRUE;
		break;
	default:
		break;
	}

	return FALSE;
}

static void
load_new_pixmap (MailCheck *mc)
{
	gtk_widget_hide (mc->containee);
	gtk_container_remove (GTK_CONTAINER (mc->bin), mc->containee);
	
	if (mc->selected_pixmap_name == mc->mailcheck_text_only) {
		mc->report_mail_mode = REPORT_MAIL_USE_TEXT;
		mc->containee = mc->label;
		g_free(mc->animation_file);
		mc->animation_file = NULL;
	} else {
		char *fname;
		char *full;

		fname = g_build_filename ("mailcheck", mc->selected_pixmap_name, NULL);
		full = gnome_program_locate_file (
				NULL, GNOME_FILE_DOMAIN_PIXMAP,
				fname, TRUE, NULL);
		free (fname);
		
		if(full != NULL &&
		   mailcheck_load_animation (mc, full)) {
			mc->containee = mc->da;
			g_free(mc->animation_file);
			mc->animation_file = full;
		} else {
			g_free (full);
			mc->report_mail_mode = REPORT_MAIL_USE_TEXT;
			mc->containee = mc->label;
			g_free(mc->animation_file);
			mc->animation_file = NULL;
		}
	}

	mail_check_timeout (mc);

	gtk_container_add (GTK_CONTAINER (mc->bin), mc->containee);
	gtk_widget_show (mc->containee);
}

static void
animation_selected (GtkMenuItem *item, gpointer data)
{
	MailCheck *mc = g_object_get_data(G_OBJECT(item), "MailCheck");
	mc->selected_pixmap_name = data;
	
	load_new_pixmap (mc);
	panel_applet_gconf_set_string(mc->applet, "animation_file", 
				      mc->animation_file ? mc->animation_file : "", NULL);
}

static void
mailcheck_new_entry (MailCheck *mc, GtkWidget *menu, GtkWidget *item, char *s)
{
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	g_object_set_data (G_OBJECT (item), "MailCheck", mc);
	
	g_signal_connect_data (item, "activate", G_CALLBACK (animation_selected),
			       g_strdup (s), (GClosureNotify) g_free, 0);
}

static GtkWidget *
mailcheck_get_animation_menu (MailCheck *mc)
{
	GtkWidget *omenu, *menu, *item;
	struct     dirent *e;
	char      *dname;
	DIR       *dir;
	char      *basename = NULL;
	int        i = 0, select_item = 0;

	dname = gnome_program_locate_file (
			NULL, GNOME_FILE_DOMAIN_PIXMAP,
			"mailcheck", FALSE, NULL);

	mc->selected_pixmap_name = mc->mailcheck_text_only;
	omenu = gtk_option_menu_new ();
	menu = gtk_menu_new ();

	item = gtk_menu_item_new_with_label (mc->mailcheck_text_only);
	gtk_widget_show (item);
	mailcheck_new_entry (mc, menu, item, mc->mailcheck_text_only);

	if (mc->animation_file != NULL)
		basename = g_path_get_basename (mc->animation_file);
	else
		basename = NULL;

	i = 1;
	dir = opendir (dname);
	if (dir){
		while ((e = readdir (dir)) != NULL){
			char *s;
			
			if (! (strstr (e->d_name, ".xpm") ||
			       strstr (e->d_name, ".png") ||
			       strstr (e->d_name, ".gif") ||
			       strstr (e->d_name, ".jpg")))
				continue;

			s = g_strdup (e->d_name);
			/* FIXME the string s will be freed in a second so 
			** this should be a strdup */
			if (!mc->selected_pixmap_name)
				mc->selected_pixmap_name = s;
			if (basename && strcmp (basename, e->d_name) == 0)
				select_item = i;
			item = gtk_menu_item_new_with_label (s);
			
			i++;
			gtk_widget_show (item);
			
			mailcheck_new_entry (mc,menu, item, s);

			g_free (s);
		}
		closedir (dir);
	}
	gtk_option_menu_set_menu (GTK_OPTION_MENU (omenu), menu);
	gtk_option_menu_set_history (GTK_OPTION_MENU (omenu), select_item);
	gtk_widget_show (omenu);

	g_free (dname);
	g_free (basename);

	return omenu;
}

static void
make_check_widgets_sensitive(MailCheck *mc)
{
	soft_set_sensitive (GTK_WIDGET (mc->min_spin), mc->auto_update);
	soft_set_sensitive (GTK_WIDGET (mc->sec_spin), mc->auto_update);
}

static void
make_remote_widgets_sensitive(MailCheck *mc)
{
	gboolean b = mc->mailbox_type != MAILBOX_LOCAL &&
	             mc->mailbox_type != MAILBOX_LOCALDIR;
        gboolean f = mc->mailbox_type == MAILBOX_IMAP;
	gboolean p = mc->mailbox_type == MAILBOX_POP3;
	
	soft_set_sensitive (mc->newmail_cmd_check, !p);
	if (p)
		soft_set_sensitive (mc->newmail_cmd_entry, !p);
	else
		soft_set_sensitive (mc->newmail_cmd_entry, mc->newmail_enabled);
	soft_set_sensitive (mc->play_sound_check, !p);

	soft_set_sensitive (mc->mailfile_fentry, !b);
	soft_set_sensitive (mc->mailfile_label, !b);
	
	soft_set_sensitive (mc->remote_server_entry, b);
	soft_set_sensitive (mc->remote_password_entry, b);
	soft_set_sensitive (mc->remote_password_checkbox, b);
	soft_set_sensitive (mc->remote_username_entry, b);
        soft_set_sensitive (mc->remote_folder_entry, f);
	soft_set_sensitive (mc->remote_server_label, b);
	soft_set_sensitive (mc->remote_password_label, b);
	soft_set_sensitive (mc->remote_username_label, b);
        soft_set_sensitive (mc->remote_folder_label, f);
}

static void
mail_file_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->mail_file)
		g_free (mc->mail_file);
		
	mc->mail_file = text;
	panel_applet_gconf_set_string(mc->applet, "mail_file", mc->mail_file, NULL);
}

static void
remote_server_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->remote_server)
		g_free (mc->remote_server);
		
	mc->remote_server = text;
	panel_applet_gconf_set_string(mc->applet, "remote_server", 
				      mc->remote_server, NULL);
}

static void
remote_username_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->remote_username)
		g_free (mc->remote_username);
		
	mc->remote_username = text;
	panel_applet_gconf_set_string(mc->applet, "remote_username", 
				      mc->remote_username, NULL);
}

static void
remote_password_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;

	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->remote_password)
		g_free (mc->remote_password); 
	mc->remote_password = text; 

	if (mc->remote_encrypted_password)
		g_free (mc->remote_encrypted_password);
	mc->remote_encrypted_password = util_base64 (text, ENCODE);  

	if (key_writable (mc->applet, "remote_encrypted_password")) {
		if (mc->save_remote_password)
			panel_applet_gconf_set_string (mc->applet, "remote_encrypted_password", 
						       mc->remote_encrypted_password, NULL);
		else
			panel_applet_gconf_set_string (mc->applet, "remote_encrypted_password",
						       "", NULL);
	}
}

static gboolean
focus_out_cb (GtkWidget *entry, GdkEventFocus *event, gpointer data)
{
	remote_password_changed (GTK_ENTRY (entry), data);

	return FALSE;

} 

static void
remote_folder_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->remote_folder)
		g_free (mc->remote_folder);
		
	mc->remote_folder = text;
	panel_applet_gconf_set_string(mc->applet, "remote_folder", 
				      mc->remote_folder, NULL);
}

static void 
set_mailbox_selection (GtkWidget *widget, gpointer data)
{
	MailCheck *mc = g_object_get_data(G_OBJECT(widget), "MailCheck");
	mc->mailbox_type = GPOINTER_TO_INT(data);
        panel_applet_gconf_set_int(mc->applet, "mailbox_type", 
        			  (gint)mc->mailbox_type, NULL);
        make_remote_widgets_sensitive(mc);
        
        if ((mc->mailbox_type != MAILBOX_POP3) &&
	    (mc->mailbox_type != MAILBOX_IMAP) &&
	    (mc->remote_handle != NULL)) {
		helper_whack_handle (mc->remote_handle);
		mc->remote_handle = NULL;
	}
	gtk_label_set_text (GTK_LABEL (mc->label), _("Status not updated"));
	set_tooltip (GTK_WIDGET (mc->applet), _("Status not updated"));
}

static void
pre_check_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;
	
	mc->pre_check_enabled = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(mc->applet, "exec_enabled", 
				    mc->pre_check_enabled, NULL);
	soft_set_sensitive (mc->pre_check_cmd_entry, mc->pre_check_enabled);

}

static void
remote_password_save_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;

	mc->save_remote_password = gtk_toggle_button_get_active (button);
	if (key_writable (mc->applet, "save_password")) {
		panel_applet_gconf_set_bool (mc->applet, "save_password",
					     mc->save_remote_password, NULL);
	}

	if (key_writable (mc->applet, "remote_encrypted_password")) {
		if (mc->save_remote_password)
			panel_applet_gconf_set_string (mc->applet, "remote_encrypted_password",
						       mc->remote_encrypted_password, NULL);
		else
			panel_applet_gconf_set_string (mc->applet, "remote_encrypted_password",
						       "", NULL);
	}
}

static void
pre_check_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->pre_check_cmd)
		g_free (mc->pre_check_cmd);	
	mc->pre_check_cmd = g_strdup (text);
	panel_applet_gconf_set_string(mc->applet, "exec_command", 
				      mc->pre_check_cmd, NULL);
	g_free (text);
	
}

static void
newmail_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;
	
	mc->newmail_enabled = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(mc->applet, "newmail_enabled", 
				    mc->newmail_enabled, NULL);
	soft_set_sensitive (mc->newmail_cmd_entry, mc->newmail_enabled);
				    
}

static void
newmail_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->newmail_cmd)
		g_free (mc->newmail_cmd);	
	mc->newmail_cmd = g_strdup (text);
	panel_applet_gconf_set_string(mc->applet, "newmail_command", 
				      mc->newmail_cmd, NULL);
	g_free (text);
	
}

static void
clicked_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;
	
	mc->clicked_enabled = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(mc->applet, "clicked_enabled", 
				    mc->clicked_enabled, NULL);
	soft_set_sensitive (mc->clicked_cmd_entry, mc->clicked_enabled);
				    
}

static void
clicked_changed (GtkEntry *entry, gpointer data)
{
	MailCheck *mc = data;
	gchar *text;
	
	text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
	if (!text)
		return;
		
	if (mc->clicked_cmd)
		g_free (mc->clicked_cmd);	
	mc->clicked_cmd = g_strdup (text);
	panel_applet_gconf_set_string(mc->applet, "clicked_command", mc->clicked_cmd, NULL);
	g_free (text);
	
}

static void
reset_on_clicked_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;
	
	mc->reset_on_clicked = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(mc->applet, "reset_on_clicked", 
				    mc->reset_on_clicked, NULL);
				    
}

static void
auto_update_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;
	
	mc->auto_update = gtk_toggle_button_get_active (button);

	if(mc->mail_timeout != 0) {
		gtk_timeout_remove(mc->mail_timeout);
		mc->mail_timeout = 0;
	}
	if(mc->auto_update)
		mc->mail_timeout = gtk_timeout_add(mc->update_freq, mail_check_timeout, mc);

	make_check_widgets_sensitive(mc);
	panel_applet_gconf_set_bool(mc->applet, "auto_update", mc->auto_update, NULL);

	/*
	 * check the mail right now, so we don't have to wait
	 * for the first timeout
	 */
	mail_check_timeout (mc);
}

static void
update_spin_changed (GtkSpinButton *spin, gpointer data)
{
	MailCheck *mc = data;
	
	mc->update_freq = 1000 * (guint)(gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (mc->sec_spin)) + 60 * gtk_spin_button_get_value_as_int (GTK_SPIN_BUTTON (mc->min_spin)));
	
	if (mc->update_freq == 0) {
		gtk_spin_button_set_value(GTK_SPIN_BUTTON (mc->sec_spin), 1.0);
		gtk_spin_button_set_value(GTK_SPIN_BUTTON (mc->min_spin), 0.0);
		mc->update_freq = 1000;
	}
	if(mc->mail_timeout != 0)
		gtk_timeout_remove (mc->mail_timeout);
	mc->mail_timeout = gtk_timeout_add (mc->update_freq, mail_check_timeout, mc);
	panel_applet_gconf_set_int(mc->applet, "update_frequency", mc->update_freq, NULL);
}

static void
sound_toggled (GtkToggleButton *button, gpointer data)
{
	MailCheck *mc = data;
	
	mc->play_sound = gtk_toggle_button_get_active (button);
	panel_applet_gconf_set_bool(mc->applet, "play_sound", mc->play_sound, NULL);
}

static GtkWidget *
mailbox_properties_page(MailCheck *mc)
{
	GtkWidget *indent, *categories_vbox, *category_vbox, *control_vbox, *control_hbox;
	GtkWidget *vbox, *hbox, *l, *l2, *item, *label, *entry;
	GtkSizeGroup *size_group;
	gchar *title;

	mc->type = 1;

	size_group = gtk_size_group_new (GTK_SIZE_GROUP_HORIZONTAL);

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
	gtk_widget_show (vbox);

	categories_vbox = gtk_vbox_new (FALSE, 18);
	gtk_box_pack_start (GTK_BOX (vbox), categories_vbox, TRUE, TRUE, 0);
	gtk_widget_show (categories_vbox);

	category_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
	gtk_widget_show (category_vbox);        

	title = g_strconcat ("<span weight=\"bold\">", _("Inbox Settings"), "</span>", NULL);
	label = gtk_label_new (title);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (category_vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (title);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (category_vbox), hbox, FALSE, FALSE, 0);
	gtk_widget_show (hbox);

	indent = gtk_label_new (HIG_IDENTATION);
	gtk_label_set_justify (GTK_LABEL (indent), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), indent, FALSE, FALSE, 0);
	gtk_widget_show (indent);

	control_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), control_vbox, TRUE, TRUE, 0);
	gtk_widget_show (control_vbox);

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);

	label = gtk_label_new_with_mnemonic(_("Mailbox _resides on:"));
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0f, 0.5f);
	gtk_size_group_add_widget (size_group, label);
	gtk_widget_show(label);
	gtk_box_pack_start (GTK_BOX (control_hbox), label, FALSE, FALSE, 0);

	mc->remote_option_menu = l = gtk_option_menu_new();
	set_atk_relation (mc->remote_option_menu, label, ATK_RELATION_LABELLED_BY);
        
	l2 = gtk_menu_new();
	item = gtk_menu_item_new_with_label(_("Local mailspool")); 
	gtk_widget_show(item);
	g_object_set_data(G_OBJECT(item), "MailCheck", mc);
	g_signal_connect (G_OBJECT(item), "activate", 
			    G_CALLBACK(set_mailbox_selection), 
			    GINT_TO_POINTER(MAILBOX_LOCAL));
	gtk_menu_shell_append (GTK_MENU_SHELL (l2), item);

	item = gtk_menu_item_new_with_label(_("Local maildir")); 
	gtk_widget_show(item);
	g_object_set_data(G_OBJECT(item), "MailCheck", mc);
	g_signal_connect (G_OBJECT(item), "activate", 
			    G_CALLBACK(set_mailbox_selection), 
			    GINT_TO_POINTER(MAILBOX_LOCALDIR));
	gtk_menu_shell_append (GTK_MENU_SHELL (l2), item);

	item = gtk_menu_item_new_with_label(_("Remote POP3-server")); 
	gtk_widget_show(item);
	g_object_set_data(G_OBJECT(item), "MailCheck", mc);
	g_signal_connect (G_OBJECT(item), "activate", 
			    G_CALLBACK(set_mailbox_selection), 
			    GINT_TO_POINTER(MAILBOX_POP3));
        
	gtk_menu_shell_append (GTK_MENU_SHELL (l2), item);
	item = gtk_menu_item_new_with_label(_("Remote IMAP-server")); 
	gtk_widget_show(item);
	g_object_set_data(G_OBJECT(item), "MailCheck", mc);
	g_signal_connect (G_OBJECT(item), "activate", 
			    G_CALLBACK(set_mailbox_selection), 
			    GINT_TO_POINTER(MAILBOX_IMAP));
	gtk_menu_shell_append (GTK_MENU_SHELL (l2), item);
	
	gtk_widget_show(l2);

	gtk_option_menu_set_menu(GTK_OPTION_MENU(l), l2);
	gtk_option_menu_set_history(GTK_OPTION_MENU(l), mc->mailbox_type_temp = mc->mailbox_type);
	gtk_widget_show(l);
  
	gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);

	if ( ! key_writable (mc->applet, "mailbox_type")) {
		hard_set_sensitive (l, FALSE);
		hard_set_sensitive (label, FALSE);
	}

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);

	mc->mailfile_label = l = gtk_label_new_with_mnemonic(_("Mail _spool file:"));
	gtk_label_set_justify (GTK_LABEL (l), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (l), 0.0f, 0.5f);
	gtk_size_group_add_widget (size_group, l);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);

	mc->mailfile_fentry = l = gnome_file_entry_new ("spool_file", _("Browse"));
	entry = gnome_file_entry_gtk_entry (GNOME_FILE_ENTRY (mc->mailfile_fentry));
	set_atk_relation (entry, mc->mailfile_label, ATK_RELATION_LABELLED_BY);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);

	mc->mailfile_entry = l = gnome_file_entry_gtk_entry(GNOME_FILE_ENTRY (l));
	gtk_entry_set_text(GTK_ENTRY(l), mc->mail_file);
	g_signal_connect(G_OBJECT(l), "changed",
			   G_CALLBACK(mail_file_changed), mc);

	if ( ! key_writable (mc->applet, "mail_file")) {
		hard_set_sensitive (mc->mailfile_label, FALSE);
		hard_set_sensitive (mc->mailfile_fentry, FALSE);
	}

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);
  
	mc->remote_server_label = l = gtk_label_new_with_mnemonic(_("Mail s_erver:"));
	gtk_label_set_justify (GTK_LABEL (l), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (l), 0.0f, 0.5f);
	gtk_size_group_add_widget (size_group, l);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);
  
	mc->remote_server_entry = l = gtk_entry_new();

	set_atk_name_description (mc->remote_server_entry, _("Mail Server Entry box"), "");
	set_atk_relation (mc->remote_server_entry, mc->remote_server_label, ATK_RELATION_LABELLED_BY);
	if (mc->remote_server)
		gtk_entry_set_text(GTK_ENTRY(l), mc->remote_server);
  	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);      
	
	g_signal_connect(G_OBJECT(l), "changed",
			   G_CALLBACK(remote_server_changed), mc);

	if ( ! key_writable (mc->applet, "remote_server")) {
		hard_set_sensitive (mc->remote_server_entry, FALSE);
		hard_set_sensitive (mc->remote_server_label, FALSE);
	}
	
	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);
  
	mc->remote_username_label = l = gtk_label_new_with_mnemonic(_("_Username:"));
	gtk_label_set_justify (GTK_LABEL (l), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (l), 0.0f, 0.5f);
	gtk_size_group_add_widget (size_group, l);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);
	
	mc->remote_username_entry = l = gtk_entry_new();
	if (mc->remote_username)
		gtk_entry_set_text(GTK_ENTRY(l), mc->remote_username);

	set_atk_name_description (mc->remote_username_entry, _("Username Entry box"), "");
	set_atk_relation (mc->remote_username_entry, mc->remote_username_label, ATK_RELATION_LABELLED_BY);
  
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);      
  
	g_signal_connect(G_OBJECT(l), "changed",
			   G_CALLBACK(remote_username_changed), mc);

	if ( ! key_writable (mc->applet, "remote_username")) {
		hard_set_sensitive (mc->remote_username_entry, FALSE);
		hard_set_sensitive (mc->remote_username_label, FALSE);
	}

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);

	mc->remote_password_label = l = gtk_label_new_with_mnemonic(_("_Password:"));
	gtk_label_set_justify (GTK_LABEL (l), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (l), 0.0f, 0.5f);
	gtk_size_group_add_widget (size_group, l);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);
	
	mc->remote_password_entry = l = gtk_entry_new();
	if (mc->remote_password)
		gtk_entry_set_text(GTK_ENTRY(l), mc->remote_password);

	set_atk_name_description (mc->remote_password_entry, _("Password Entry box"), "");
	set_atk_relation (mc->remote_password_entry, mc->remote_password_label, ATK_RELATION_LABELLED_BY);
	gtk_entry_set_visibility(GTK_ENTRY (l), FALSE);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);      
	
	g_signal_connect(G_OBJECT(l), "focus_out_event",
			 G_CALLBACK(focus_out_cb), mc);

	g_signal_connect(G_OBJECT(l), "activate",
			 G_CALLBACK(remote_password_changed), mc);  

	if ( ! key_writable (mc->applet, "remote_encrypted_password")) {
		hard_set_sensitive (mc->remote_password_entry, FALSE);
		hard_set_sensitive (mc->remote_password_label, FALSE);
	}

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);

	l = gtk_label_new ("");
	gtk_size_group_add_widget (size_group, l);
	gtk_widget_show (l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);

	mc->remote_password_checkbox = l = gtk_check_button_new_with_mnemonic (
					_("_Save password to disk"));
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(l), mc->save_remote_password);
	gtk_widget_show (l);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);
	
	g_signal_connect (G_OBJECT (l), "toggled", 
			  G_CALLBACK(remote_password_save_toggled), mc);

	if ( ! key_writable (mc->applet, "save_password"))
		hard_set_sensitive (mc->remote_password_checkbox, FALSE);

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show (control_hbox);  

        mc->remote_folder_label = l = gtk_label_new_with_mnemonic(_("_Folder:"));
        gtk_label_set_justify (GTK_LABEL(l), GTK_JUSTIFY_LEFT);
        gtk_misc_set_alignment (GTK_MISC(l), 0.0f, 0.5f);
        gtk_size_group_add_widget (size_group, l);
        gtk_widget_show(l);
        gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);
 
        mc->remote_folder_entry = l = gtk_entry_new();
        if (mc->remote_folder)
                gtk_entry_set_text(GTK_ENTRY(l), mc->remote_folder);
  
	    set_atk_name_description (mc->remote_folder_entry, _("Folder Entry box"), "");
        set_atk_relation (mc->remote_folder_entry, mc->remote_folder_label, ATK_RELATION_LABELLED_BY);
        gtk_widget_show(l);
        gtk_box_pack_start (GTK_BOX (control_hbox), l, TRUE, TRUE, 0);
  
        g_signal_connect(G_OBJECT(l), "changed",
                           G_CALLBACK(remote_folder_changed), mc);

	if ( ! key_writable (mc->applet, "remote_folder")) {
		hard_set_sensitive (mc->remote_folder_entry, FALSE);
		hard_set_sensitive (mc->remote_folder_label, FALSE);
	}
  
	make_remote_widgets_sensitive(mc);
	
	return vbox;
}

static GtkWidget *
mailcheck_properties_page (MailCheck *mc)
{
	GtkWidget *vbox, *hbox, *hbox2, *l, *table, *check_box, *animation_option_menu;
	GtkWidget *label, *indent, *categories_vbox, *category_vbox, *control_vbox, *control_hbox;
	GtkObject *freq_a;
	gchar *title;
	gboolean writable;
	GConfClient *client;
	gboolean inhibit_command_line;

	client = gconf_client_get_default ();
	inhibit_command_line = gconf_client_get_bool (client, "/desktop/gnome/lockdown/inhibit_command_line", NULL);

	mc->type = 0;

	vbox = gtk_vbox_new (FALSE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (vbox), 12);
	gtk_widget_show (vbox);

	categories_vbox = gtk_vbox_new (FALSE, 18);
	gtk_box_pack_start (GTK_BOX (vbox), categories_vbox, TRUE, TRUE, 0);
	gtk_widget_show (categories_vbox);

	category_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
	gtk_widget_show (category_vbox);

	title = g_strconcat ("<span weight=\"bold\">", _("General"), "</span>", NULL);
	label = gtk_label_new (title);
	gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
	gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
	gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
	gtk_box_pack_start (GTK_BOX (category_vbox), label, FALSE, FALSE, 0);
	gtk_widget_show (label);
	g_free (title);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_box_pack_start (GTK_BOX (category_vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox); 

	indent = gtk_label_new (HIG_IDENTATION);
	gtk_label_set_justify (GTK_LABEL (indent), GTK_JUSTIFY_LEFT);
	gtk_box_pack_start (GTK_BOX (hbox), indent, FALSE, FALSE, 0);
	gtk_widget_show (indent);

	control_vbox = gtk_vbox_new (FALSE, 6);
	gtk_box_pack_start (GTK_BOX (hbox), control_vbox, TRUE, TRUE, 0);
	gtk_widget_show (control_vbox);

	control_hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start(GTK_BOX(control_vbox), control_hbox, TRUE, TRUE, 0);
	gtk_widget_show(control_hbox);

	check_box = l = gtk_check_button_new_with_mnemonic (_("Check for mail _every:"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l), mc->auto_update);
	g_signal_connect(G_OBJECT(l), "toggled",
			 G_CALLBACK(auto_update_toggled), mc);
	gtk_box_pack_start (GTK_BOX (control_hbox), l, FALSE, FALSE, 0);
	gtk_widget_show(l);
	if ( ! key_writable (mc->applet, "auto_update"))
		hard_set_sensitive (l, FALSE);

        hbox2 = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start(GTK_BOX(control_hbox), hbox2, FALSE, FALSE, 0);
	gtk_widget_show(hbox2);


	writable = key_writable (mc->applet, "update_frequency");
	
	freq_a = gtk_adjustment_new((float)((mc->update_freq/1000)/60), 0, 1440, 1, 5, 5);
	mc->min_spin = gtk_spin_button_new( GTK_ADJUSTMENT (freq_a), 1, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (mc->min_spin), TRUE);
	g_signal_connect (G_OBJECT (mc->min_spin), "value_changed",
			  G_CALLBACK (update_spin_changed), mc);			  
	gtk_box_pack_start (GTK_BOX (hbox2), mc->min_spin,  FALSE, FALSE, 0);
	set_atk_name_description (mc->min_spin, _("minutes"), _("Choose time interval in minutes to check mail"));
	set_atk_relation (mc->min_spin, check_box, ATK_RELATION_CONTROLLED_BY);
	gtk_widget_show(mc->min_spin);
	if ( ! writable)
		hard_set_sensitive (mc->min_spin, FALSE);

	l = gtk_label_new (_("minutes"));
	set_atk_relation (mc->min_spin, l, ATK_RELATION_LABELLED_BY);
	gtk_widget_show(l);
	gtk_box_pack_start (GTK_BOX (hbox2), l, FALSE, FALSE, 0);
	if ( ! writable)
		hard_set_sensitive (l, FALSE);

        hbox2 = gtk_hbox_new (FALSE, 6);
	gtk_box_pack_start(GTK_BOX(control_hbox), hbox2, FALSE, FALSE, 0);
	gtk_widget_show(hbox2);
	
	freq_a = gtk_adjustment_new((float)((mc->update_freq/1000)%60), 0, 59, 1, 5, 5);
	mc->sec_spin = gtk_spin_button_new (GTK_ADJUSTMENT (freq_a), 1, 0);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (mc->sec_spin), TRUE);
	g_signal_connect (G_OBJECT (mc->sec_spin), "value_changed",
			  G_CALLBACK (update_spin_changed), mc);
	gtk_box_pack_start (GTK_BOX (hbox2), mc->sec_spin,  FALSE, FALSE, 0);
	set_atk_name_description (mc->sec_spin, _("seconds"), _("Choose time interval in seconds to check mail"));
	set_atk_relation (mc->sec_spin, check_box, ATK_RELATION_CONTROLLED_BY);
	gtk_widget_show(mc->sec_spin);
	if ( ! writable)
		hard_set_sensitive (mc->sec_spin, FALSE);

	l = gtk_label_new (_("seconds"));
	set_atk_relation (mc->sec_spin, l,  ATK_RELATION_LABELLED_BY);
	gtk_widget_show(l);
	gtk_misc_set_alignment (GTK_MISC (l), 0.0f, 0.5f);
	gtk_box_pack_start (GTK_BOX (hbox2), l, TRUE, TRUE, 0);
	if ( ! writable)
		hard_set_sensitive (l, FALSE);

	set_atk_relation (check_box, mc->min_spin, ATK_RELATION_CONTROLLER_FOR);
	set_atk_relation (check_box, mc->sec_spin, ATK_RELATION_CONTROLLER_FOR);

	mc->play_sound_check = gtk_check_button_new_with_mnemonic(_("Play a _sound when new mail arrives"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mc->play_sound_check), mc->play_sound);
	g_signal_connect(G_OBJECT(mc->play_sound_check), "toggled",
			   G_CALLBACK(sound_toggled), mc);
	gtk_widget_show(mc->play_sound_check);
	gtk_box_pack_start(GTK_BOX (control_vbox), mc->play_sound_check, TRUE, TRUE, 0);
	if ( ! key_writable (mc->applet, "play_sound"))
		hard_set_sensitive (mc->play_sound_check, FALSE);

	/*l = gtk_check_button_new_with_mnemonic (_("Set the number of unread mails to _zero when clicked"));*/
	l = gtk_check_button_new_with_mnemonic (_("Sto_p animation when clicked"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l), mc->reset_on_clicked);
	g_signal_connect(G_OBJECT(l), "toggled",
			   G_CALLBACK(reset_on_clicked_toggled), mc);
	gtk_widget_show(l);
	gtk_box_pack_start(GTK_BOX (control_vbox), l, TRUE, TRUE, 0);
	if ( ! key_writable (mc->applet, "reset_on_clicked"))
		hard_set_sensitive (l, FALSE);

	hbox = gtk_hbox_new (FALSE, 12);
	gtk_box_pack_start (GTK_BOX (control_vbox), hbox, TRUE, TRUE, 0);
	gtk_widget_show (hbox);

	l = gtk_label_new_with_mnemonic (_("Select a_nimation:"));
	gtk_misc_set_alignment (GTK_MISC (l), 0.0, 0.5);
	gtk_widget_show (l);
	gtk_box_pack_start (GTK_BOX (hbox), l, FALSE, FALSE, 0);
	animation_option_menu = mailcheck_get_animation_menu (mc);
	gtk_box_pack_start (GTK_BOX (hbox), animation_option_menu, FALSE, FALSE, 0);
	set_atk_relation (animation_option_menu, l, ATK_RELATION_LABELLED_BY);
	make_check_widgets_sensitive(mc);
	if ( ! key_writable (mc->applet, "animation_file")) {
		hard_set_sensitive (l, FALSE);
		hard_set_sensitive (animation_option_menu, FALSE);
	}

	if ( ! inhibit_command_line) {
		category_vbox = gtk_vbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (categories_vbox), category_vbox, TRUE, TRUE, 0);
		gtk_widget_show (category_vbox);

		title = g_strconcat ("<span weight=\"bold\">", _("Commands"), "</span>", NULL);
		label = gtk_label_new (title);
		gtk_label_set_use_markup (GTK_LABEL (label), TRUE);
		gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
		gtk_misc_set_alignment (GTK_MISC (label), 0, 0.5);
		gtk_box_pack_start (GTK_BOX (category_vbox), label, FALSE, FALSE, 0);
		gtk_widget_show (label);
		g_free (title);

		hbox = gtk_hbox_new (FALSE, 0);
		gtk_box_pack_start (GTK_BOX (category_vbox), hbox, TRUE, TRUE, 0);
		gtk_widget_show (hbox);

		indent = gtk_label_new (HIG_IDENTATION);
		gtk_label_set_justify (GTK_LABEL (indent), GTK_JUSTIFY_LEFT);
		gtk_box_pack_start (GTK_BOX (hbox), indent, FALSE, FALSE, 0);
		gtk_widget_show (indent);

		control_vbox = gtk_vbox_new (FALSE, 6);
		gtk_box_pack_start (GTK_BOX (hbox), control_vbox, TRUE, TRUE, 0);
		gtk_widget_show (control_vbox);

		table = gtk_table_new (3, 2, FALSE);
		gtk_table_set_col_spacings (GTK_TABLE (table), 12);
		gtk_table_set_row_spacings (GTK_TABLE (table), 6);
		gtk_container_set_border_width (GTK_CONTAINER (table), 0);
		gtk_widget_show(table);
		gtk_container_add (GTK_CONTAINER (control_vbox), table);

		l = gtk_check_button_new_with_mnemonic(_("Before each _update:"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l), mc->pre_check_enabled);
		g_signal_connect(G_OBJECT(l), "toggled",
				 G_CALLBACK(pre_check_toggled), mc);
		gtk_widget_show(l);
		mc->pre_check_cmd_check = l;
		if ( ! key_writable (mc->applet, "exec_enabled"))
			hard_set_sensitive (l, FALSE);

		gtk_table_attach (GTK_TABLE (table), mc->pre_check_cmd_check, 
				  0, 1, 0, 1, GTK_FILL, 0, 0, 0);


		mc->pre_check_cmd_entry = gtk_entry_new();
		if(mc->pre_check_cmd)
			gtk_entry_set_text(GTK_ENTRY(mc->pre_check_cmd_entry), 
					   mc->pre_check_cmd);
		set_atk_name_description (mc->pre_check_cmd_entry, _("Command to execute before each update"), "");
		set_atk_relation (mc->pre_check_cmd_entry, mc->pre_check_cmd_check, ATK_RELATION_CONTROLLED_BY);
		set_atk_relation (mc->pre_check_cmd_check, mc->pre_check_cmd_entry, ATK_RELATION_CONTROLLER_FOR);
		soft_set_sensitive (mc->pre_check_cmd_entry, mc->pre_check_enabled);
		g_signal_connect(G_OBJECT(mc->pre_check_cmd_entry), "changed",
				 G_CALLBACK(pre_check_changed), mc);
		gtk_widget_show(mc->pre_check_cmd_entry);
		gtk_table_attach_defaults (GTK_TABLE (table), mc->pre_check_cmd_entry,
					   1, 2, 0, 1);
		if ( ! key_writable (mc->applet, "exec_command"))
			hard_set_sensitive (mc->pre_check_cmd_entry, FALSE);

		l = gtk_check_button_new_with_mnemonic (_("When new mail _arrives:"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l), mc->newmail_enabled);
		g_signal_connect(G_OBJECT(l), "toggled",
				 G_CALLBACK(newmail_toggled), mc);
		gtk_widget_show(l);
		gtk_table_attach (GTK_TABLE (table), l, 0, 1, 1, 2, GTK_FILL, 0, 0, 0);
		mc->newmail_cmd_check = l;
		if ( ! key_writable (mc->applet, "newmail_enabled"))
			hard_set_sensitive (l, FALSE);

		mc->newmail_cmd_entry = gtk_entry_new();
		if (mc->newmail_cmd) {
			gtk_entry_set_text(GTK_ENTRY(mc->newmail_cmd_entry),
					   mc->newmail_cmd);
		}
		set_atk_name_description (mc->newmail_cmd_entry, _("Command to execute when new mail arrives"), "");
		set_atk_relation (mc->newmail_cmd_entry, mc->newmail_cmd_check, ATK_RELATION_CONTROLLED_BY);
		set_atk_relation (mc->newmail_cmd_check, mc->newmail_cmd_entry, ATK_RELATION_CONTROLLER_FOR);
		soft_set_sensitive (mc->newmail_cmd_entry, mc->newmail_enabled);
		g_signal_connect(G_OBJECT (mc->newmail_cmd_entry), "changed",
				 G_CALLBACK(newmail_changed), mc);
		gtk_widget_show(mc->newmail_cmd_entry);
		gtk_table_attach_defaults (GTK_TABLE (table), mc->newmail_cmd_entry,
					   1, 2, 1, 2);
		if ( ! key_writable (mc->applet, "newmail_command"))
			hard_set_sensitive (mc->newmail_cmd_entry, FALSE);

		l = gtk_check_button_new_with_mnemonic (_("When clicke_d:"));
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l), mc->clicked_enabled);
		g_signal_connect(G_OBJECT(l), "toggled",
				 G_CALLBACK(clicked_toggled), mc);
		gtk_widget_show(l);
		gtk_table_attach (GTK_TABLE (table), l, 0, 1, 2, 3, GTK_FILL, 0, 0, 0);
		mc->clicked_cmd_check = l;
		if ( ! key_writable (mc->applet, "clicked_enabled"))
			hard_set_sensitive (l, FALSE);

		mc->clicked_cmd_entry = gtk_entry_new();
		if(mc->clicked_cmd) {
			gtk_entry_set_text(GTK_ENTRY(mc->clicked_cmd_entry), 
					   mc->clicked_cmd);
		}
		set_atk_name_description (mc->clicked_cmd_entry, _("Command to execute when clicked"), "");
		set_atk_relation (mc->clicked_cmd_entry, mc->clicked_cmd_check, ATK_RELATION_CONTROLLED_BY);
		set_atk_relation (mc->clicked_cmd_check, mc->clicked_cmd_entry, ATK_RELATION_CONTROLLER_FOR);
		soft_set_sensitive (mc->clicked_cmd_entry, mc->clicked_enabled);
		g_signal_connect(G_OBJECT(mc->clicked_cmd_entry), "changed",
				 G_CALLBACK(clicked_changed), mc);
		gtk_widget_show(mc->clicked_cmd_entry);
		gtk_table_attach_defaults (GTK_TABLE (table), mc->clicked_cmd_entry,
					   1, 2, 2, 3);
		if ( ! key_writable (mc->applet, "clicked_command"))
			hard_set_sensitive (mc->clicked_cmd_entry, FALSE);
	}

	return vbox;
}

static void
phelp_cb (GtkDialog *w, gint tab, MailCheck *mc)
{
	GError *error = NULL;
	static GnomeProgram *applet_program = NULL;

	if (!applet_program) {
		int argc = 1;
		char *argv[2] = { "mailcheck" };
		applet_program = gnome_program_init ("mailcheck", VERSION,
						      LIBGNOME_MODULE, argc, argv,
     						      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	}

	egg_help_display_desktop_on_screen (
			applet_program, "mailcheck", "mailcheck", "mailcheck-prefs",
			gtk_widget_get_screen (GTK_WIDGET (mc->applet)),
			&error);
	if (error) {
		GtkWidget *dialog;
		char *msg;

		msg = g_strdup_printf (_("There was an error displaying help: %s"),
				       error->message);
		dialog = hig_dialog_new (GTK_WINDOW (w),
					 GTK_DIALOG_DESTROY_WITH_PARENT,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 _("Error displaying help"),
					 msg);
		g_free (msg);

		g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
		gtk_widget_show (dialog);
		g_error_free (error);
	}
}	

static void
response_cb (GtkDialog *dialog, gint id, MailCheck *mc)
{
	if (id == GTK_RESPONSE_HELP) {
		phelp_cb (dialog, id, mc);
		return;	
	}

	gtk_widget_destroy (GTK_WIDGET (dialog));
	mc->property_window = NULL;
}


static void
mailcheck_properties (BonoboUIComponent *uic, MailCheck *mc, const gchar *verbname)
{
	GtkWidget *p;
	GtkWidget *notebook;

	if (mc->property_window) {
		gtk_window_set_screen (GTK_WINDOW (mc->property_window),
				       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
		gtk_window_present (GTK_WINDOW (mc->property_window));
		return;
	}
	
	mc->property_window = gtk_dialog_new_with_buttons (_("Inbox Monitor Preferences"), 
							   NULL,
						           GTK_DIALOG_DESTROY_WITH_PARENT,
						           GTK_STOCK_CLOSE, 
						           GTK_RESPONSE_CLOSE,
						           GTK_STOCK_HELP, 
						           GTK_RESPONSE_HELP,
						           NULL);
	gtk_window_set_resizable (GTK_WINDOW (mc->property_window), FALSE);
	gtk_window_set_screen (GTK_WINDOW (mc->property_window), 
			       gtk_widget_get_screen (GTK_WIDGET (mc->property_window)));
	gtk_dialog_set_default_response (GTK_DIALOG (mc->property_window), GTK_RESPONSE_CLOSE);
	gtk_dialog_set_has_separator (GTK_DIALOG (mc->property_window), FALSE);
	gtk_container_set_border_width (GTK_CONTAINER (mc->property_window), 5);
	gtk_box_set_spacing (GTK_BOX (GTK_DIALOG (mc->property_window)->vbox), 2);
	gnome_window_icon_set_from_file (GTK_WINDOW (mc->property_window),
					 GNOME_ICONDIR"/gnome-mailcheck.png");
	gtk_window_set_screen (GTK_WINDOW (mc->property_window),
			       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
	
	notebook = gtk_notebook_new ();
	gtk_container_set_border_width (GTK_CONTAINER (notebook), 5);
	gtk_widget_show (notebook);
	gtk_box_pack_start (GTK_BOX (GTK_DIALOG (mc->property_window)->vbox), notebook,
			    TRUE, TRUE, 0);
	p = mailcheck_properties_page (mc);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), p,
				  gtk_label_new_with_mnemonic (_("Behavior")));
				  
	p = mailbox_properties_page (mc);
	gtk_notebook_append_page (GTK_NOTEBOOK (notebook), p,
				  gtk_label_new_with_mnemonic (_("Inbox Settings")));
	
	g_signal_connect (G_OBJECT (mc->property_window), "response",
			  G_CALLBACK (response_cb), mc);
	gtk_widget_show (GTK_DIALOG (mc->property_window)->vbox);
	gtk_widget_show (mc->property_window);
}

static void
check_callback (BonoboUIComponent *uic, gpointer data, const gchar *verbname)
{
	MailCheck *mc = data;

	mail_check_timeout(mc);
}

static void
applet_load_prefs(MailCheck *mc)
{
	panel_applet_gconf_set_string(mc->applet, "remote_password", "", NULL);

	mc->animation_file = panel_applet_gconf_get_string(mc->applet, "animation_file", NULL);
	
	mc->auto_update = panel_applet_gconf_get_bool(mc->applet, "auto_update", NULL);
	mc->reset_on_clicked = panel_applet_gconf_get_bool (mc->applet, "reset_on_clicked", NULL);
	mc->update_freq = panel_applet_gconf_get_int(mc->applet, "update_frequency", NULL);
	mc->pre_check_cmd = panel_applet_gconf_get_string(mc->applet, "exec_command", NULL);
	mc->pre_check_enabled = panel_applet_gconf_get_bool(mc->applet, "exec_enabled", NULL);
	mc->newmail_cmd = panel_applet_gconf_get_string(mc->applet, "newmail_command", NULL);
	mc->newmail_enabled = panel_applet_gconf_get_bool(mc->applet, "newmail_enabled", NULL);
	mc->clicked_cmd = panel_applet_gconf_get_string(mc->applet, "clicked_command", NULL);
	mc->clicked_enabled = panel_applet_gconf_get_bool(mc->applet, "clicked_enabled", NULL);
	mc->remote_server = panel_applet_gconf_get_string(mc->applet, "remote_server", NULL);
	mc->remote_username = panel_applet_gconf_get_string(mc->applet, "remote_username", NULL);
	if(!mc->remote_username) {
		g_free(mc->remote_username);
		mc->remote_username = g_strdup(g_getenv("USER"));
	}
	mc->remote_encrypted_password = panel_applet_gconf_get_string (mc->applet, 
					"remote_encrypted_password", NULL); 
	mc->remote_password = util_base64 (mc->remote_encrypted_password, DECODE);
	mc->save_remote_password = panel_applet_gconf_get_bool (mc->applet, "save_password", NULL);
	mc->remote_folder = panel_applet_gconf_get_string(mc->applet, "remote_folder", NULL);
	mc->mailbox_type = panel_applet_gconf_get_int(mc->applet, "mailbox_type", NULL);
	mc->mail_file = panel_applet_gconf_get_string (mc->applet, "mail_file", NULL);
	mc->play_sound = panel_applet_gconf_get_bool(mc->applet, "play_sound", NULL);
}

static void
mailcheck_about(BonoboUIComponent *uic, MailCheck *mc, const gchar *verbname)
{
	GdkPixbuf *pixbuf = NULL;
	gchar *file;

	static const gchar     *authors [] =
	{
		"Miguel de Icaza <miguel@kernel.org>",
		"Jacob Berkman <jberkman@andrew.cmu.edu>",
		"Jaka Mocnik <jaka.mocnik@kiss.uni-lj.si>",
		"Lennart Poettering <poettering@gmx.net>",
		NULL
	};
	const char *documenters [] = {
		"Eric Baudais <baudais@okstate.edu>",
		"Telsa Gwynne <telsa@linuxchix.org>",
                "Sun GNOME Documentation Team <gdocteam@sun.com>",
	        NULL
	};
	const char *translator_credits = _("translator_credits");

	if (mc->about) {
		gtk_window_set_screen (GTK_WINDOW (mc->about),
				       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));

		gtk_window_present (GTK_WINDOW (mc->about));
		return;
	}
	
	file = gnome_program_locate_file (NULL, GNOME_FILE_DOMAIN_PIXMAP, "gnome-mailcheck.png", TRUE, NULL);
	pixbuf = gdk_pixbuf_new_from_file (file, NULL);
	g_free (file);
	
	mc->about = gnome_about_new (_("Inbox Monitor"), VERSION,
				     "Copyright \xc2\xa9 1998-2002 Free Software Foundation, Inc.",
				     _("Inbox Monitor notifies you when new mail arrives in your mailbox"),
				     authors,
				     documenters,
   				     strcmp (translator_credits, "translator_credits") != 0 ? translator_credits : NULL,
				     pixbuf);

	if (pixbuf)
		g_object_unref (pixbuf);

	gtk_window_set_wmclass (GTK_WINDOW (mc->about), "mailcheck", "Mailcheck");

	gtk_window_set_screen (GTK_WINDOW (mc->about),
			       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));

	gnome_window_icon_set_from_file (GTK_WINDOW (mc->about),
					 GNOME_ICONDIR"/gnome-mailcheck.png");

	g_signal_connect( G_OBJECT(mc->about), "destroy",
			    G_CALLBACK(gtk_widget_destroyed), &mc->about );

	gtk_widget_show(mc->about);
}

static void
applet_size_allocate (PanelApplet *w, GtkAllocation *allocation, MailCheck *mc)
{
	const char *fname;

	switch (panel_applet_get_orient (w)) {
	case PANEL_APPLET_ORIENT_DOWN:
	case PANEL_APPLET_ORIENT_UP:
		if( mc->size == allocation->height )
			return;

		mc->size = allocation->height;
		gtk_widget_set_size_request (mc->label, -1, mc->size);
		break;

	case PANEL_APPLET_ORIENT_LEFT:
	case PANEL_APPLET_ORIENT_RIGHT:
		if( mc->size == allocation->width )
			return;

		mc->size = allocation->width;
		gtk_widget_set_size_request (mc->label, mc->size, -1);
		break;
	}

	fname = mail_animation_filename (mc);

	gtk_widget_set_size_request (GTK_WIDGET(mc->da), mc->size, mc->size);

	if (!fname)
		return;

	mailcheck_load_animation (mc, fname);
}

static void
applet_change_orient(PanelApplet * w, PanelAppletOrient orient, gpointer data)
{
	MailCheck *mc = data;
	
	switch (orient) {
     	case PANEL_APPLET_ORIENT_DOWN:
        case PANEL_APPLET_ORIENT_UP:
        	gtk_widget_set_size_request (mc->label, -1, mc->size);
        	break;
     	case PANEL_APPLET_ORIENT_LEFT:
     	case PANEL_APPLET_ORIENT_RIGHT:
     		gtk_widget_set_size_request (mc->label, mc->size, -1);
		break;
	}

}

static void
applet_change_background(PanelApplet *a,
			 PanelAppletBackgroundType type,
			 GdkColor *color, GdkPixmap *pixmap,
			 MailCheck *mc)
{
	GtkRcStyle *rc_style = gtk_rc_style_new ();

	switch (type) {
		case PANEL_PIXMAP_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (mc->ebox), rc_style);
			gtk_widget_modify_style (GTK_WIDGET (mc->applet), rc_style);
			break;

		case PANEL_COLOR_BACKGROUND:
			gtk_widget_modify_bg (GTK_WIDGET (mc->ebox), GTK_STATE_NORMAL, color);
			gtk_widget_modify_bg (GTK_WIDGET (mc->applet), GTK_STATE_NORMAL, color);
			break;

		case PANEL_NO_BACKGROUND:
			gtk_widget_modify_style (GTK_WIDGET (mc->ebox), rc_style);
			gtk_widget_modify_style (GTK_WIDGET (mc->applet), rc_style);
			break;

		default:
			gtk_widget_modify_style (GTK_WIDGET (mc->ebox), rc_style);
			gtk_widget_modify_style (GTK_WIDGET (mc->applet), rc_style);
			break;
	}

	gtk_rc_style_unref (rc_style);
}

static void
help_callback (BonoboUIComponent *uic, MailCheck *mc, const gchar *verbname)
{
	GError *error = NULL;
	static GnomeProgram *applet_program = NULL;

	if (!applet_program) {
		int argc = 1;
		char *argv[2] = { "mailcheck" };
		applet_program = gnome_program_init ("mailcheck", VERSION,
						      LIBGNOME_MODULE, argc, argv,
						      GNOME_PROGRAM_STANDARD_PROPERTIES, NULL);
	}

	egg_help_display_desktop_on_screen (
		applet_program, "mailcheck", "mailcheck",NULL,
		gtk_widget_get_screen (GTK_WIDGET (mc->applet)),
		&error);
	if (error) {
		GtkWidget *dialog;
		char *msg;

		msg = g_strdup_printf (_("There was an error displaying help: %s"),
				       error->message);
		dialog = hig_dialog_new (NULL /* parent */,
					 0 /* flags */,
					 GTK_MESSAGE_ERROR,
					 GTK_BUTTONS_OK,
					 _("Error displaying help"),
					 msg);
		g_free (msg);

		g_signal_connect (G_OBJECT (dialog), "response",
				  G_CALLBACK (gtk_widget_destroy),
				  NULL);

		gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
		gtk_window_set_screen (GTK_WINDOW (dialog),
				       gtk_widget_get_screen (GTK_WIDGET (mc->applet)));
		gtk_widget_show (dialog);
		g_error_free (error);
	}
}

static void
mailcheck_destroy (GtkWidget *widget, gpointer data)
{
	MailCheck *mc = data;

	mc->bin = NULL;

	if (mc->property_window != NULL)
		gtk_widget_destroy (mc->property_window);
	if (mc->about != NULL)
		gtk_widget_destroy (mc->about);
	if (mc->password_dialog != NULL)
		gtk_widget_destroy (mc->password_dialog);

	gtk_widget_unref (mc->da);

	g_free (mc->pre_check_cmd);
	g_free (mc->newmail_cmd);
	g_free (mc->clicked_cmd);

	g_free (mc->remote_server);
	g_free (mc->remote_username);
	g_free (mc->remote_password);
	g_free (mc->remote_encrypted_password);
	g_free (mc->remote_folder);
	g_free (mc->real_password);

	g_free (mc->animation_file);
	g_free (mc->mail_file);

	if (mc->email_pixmap)
		g_object_unref (mc->email_pixmap);

	if (mc->email_mask)
		g_object_unref (mc->email_mask);

	if (mc->mail_timeout != 0)
		gtk_timeout_remove (mc->mail_timeout);

	if (mc->animation_tag != 0)
		gtk_timeout_remove (mc->animation_tag);

	if (mc->remote_handle != NULL)
		helper_whack_handle (mc->remote_handle);

	/* just for sanity */
	memset(mc, 0, sizeof(MailCheck));

	g_free(mc);
}

static GtkWidget *
create_mail_widgets (MailCheck *mc)
{
	const char *fname;
	GtkWidget *alignment;

	fname = mail_animation_filename (mc);

	mc->ebox = gtk_event_box_new();
        gtk_widget_set_events(mc->ebox, 
                              gtk_widget_get_events(mc->ebox) |
                              GDK_BUTTON_PRESS_MASK);
	gtk_widget_show (mc->ebox);
	
	alignment = gtk_alignment_new (0.5, 0.5, 0.0, 0.0);
	gtk_container_add (GTK_CONTAINER (mc->ebox), alignment);
	gtk_widget_show (alignment);
	
	/*
	 * This is so that the properties dialog is destroyed if the
	 * applet is removed from the panel while the dialog is
	 * active.
	 */
	g_signal_connect (G_OBJECT (mc->ebox), "destroy",
			    (GtkSignalFunc) mailcheck_destroy,
			    mc);

	mc->bin = gtk_hbox_new (0, 0);
	gtk_container_add(GTK_CONTAINER(alignment), mc->bin);

	gtk_widget_show (mc->bin);
	
	if (mc->auto_update)
		mc->mail_timeout = gtk_timeout_add (mc->update_freq, mail_check_timeout, mc);
	else
		mc->mail_timeout = 0;

	/* The drawing area */
	mc->da = gtk_drawing_area_new ();
	gtk_widget_ref (mc->da);

	gtk_widget_set_size_request (mc->da, mc->size, mc->size);

	g_signal_connect (G_OBJECT(mc->da), "expose_event", (GtkSignalFunc)icon_expose, mc);
	gtk_widget_show (mc->da);

	/* The label */
	mc->label = gtk_label_new (NULL);
	gtk_label_set_justify (GTK_LABEL (mc->label), GTK_JUSTIFY_CENTER);
	gtk_label_set_line_wrap (GTK_LABEL (mc->label), TRUE);
	gtk_widget_show (mc->label);
	gtk_widget_ref (mc->label);
	
	if (fname != NULL &&
	    WANT_BITMAPS (mc->report_mail_mode) &&
	    mailcheck_load_animation (mc, fname)) {
		mc->containee = mc->da;
	} else {
		mc->report_mail_mode = REPORT_MAIL_USE_TEXT;
		mc->containee = mc->label;
	}

	gtk_container_add (GTK_CONTAINER (mc->bin), mc->containee);
	switch (panel_applet_get_orient (PANEL_APPLET (mc->applet))) {
     	case PANEL_APPLET_ORIENT_DOWN:
        case PANEL_APPLET_ORIENT_UP:
        	gtk_widget_set_size_request (mc->label, -1, mc->size);
        	break;
     	case PANEL_APPLET_ORIENT_LEFT:
     	case PANEL_APPLET_ORIENT_RIGHT:
     		gtk_widget_set_size_request (mc->label, mc->size, -1);
		break;
	}

	return mc->ebox;
}

static void
set_atk_name_description (GtkWidget *widget, const gchar *name,
    const gchar *description)
{	
	AtkObject *aobj;
	
	aobj = gtk_widget_get_accessible (widget);
	/* Check if gail is loaded */
	if (GTK_IS_ACCESSIBLE (aobj) == FALSE)
		return; 
	atk_object_set_name (aobj, name);
	atk_object_set_description (aobj, description);
}

static void
set_atk_relation (GtkWidget *widget1, GtkWidget *widget2, AtkRelationType relation_type)
{
	AtkObject *atk_widget1;
	AtkObject *atk_widget2;
	AtkRelationSet *relation_set;
	AtkRelation *relation;
	AtkObject *targets[1];

	atk_widget1 = gtk_widget_get_accessible (widget1);
	atk_widget2 = gtk_widget_get_accessible (widget2);

	/* Set the label-for relation only if label-by is being set */
	if (relation_type == ATK_RELATION_LABELLED_BY) {
		gtk_label_set_mnemonic_widget (GTK_LABEL (widget2), widget1); 
		return;
	}

	/* Check if gail is loaded */
	if (GTK_IS_ACCESSIBLE (atk_widget1) == FALSE)
		return;

	/* Set the labelled-by relation */
	relation_set = atk_object_ref_relation_set (atk_widget1);
	targets[0] = atk_widget2;
	relation = atk_relation_new (targets, 1, relation_type);
	atk_relation_set_add (relation_set, relation);
	g_object_unref (G_OBJECT (relation));
}

static const BonoboUIVerb mailcheck_menu_verbs [] = {
	BONOBO_UI_UNSAFE_VERB ("Preferences", mailcheck_properties),
	BONOBO_UI_UNSAFE_VERB ("Help",        help_callback),
	BONOBO_UI_UNSAFE_VERB ("About",       mailcheck_about),
	BONOBO_UI_UNSAFE_VERB ("Check",       check_callback),
        BONOBO_UI_VERB_END
};

static gboolean
mailcheck_applet_fill (PanelApplet *applet)
{
	GtkWidget *mailcheck;
	MailCheck *mc;

	mc = g_new0(MailCheck, 1);
	mc ->applet = applet;
	mc->animation_file = NULL;
	mc->property_window = NULL;
	mc->anim_changed = FALSE;
	mc->anymail = mc->unreadmail = mc->newmail = FALSE;
	mc->mail_timeout = 0;
	mc->animation_tag = 0;
	mc->password_dialog = NULL;
	mc->oldsize = 0;
	mc->show_animation = TRUE;

	/*initial state*/
	mc->report_mail_mode = REPORT_MAIL_USE_ANIMATION;
	
	mc->mail_file = NULL;	

	panel_applet_add_preferences (applet, "/schemas/apps/mailcheck_applet/prefs", NULL);
	panel_applet_set_flags (applet, PANEL_APPLET_EXPAND_MINOR);
	applet_load_prefs(mc);
	if (mc->mail_file == NULL || strlen(mc->mail_file)==0) {
		const char *mail_file = g_getenv ("MAIL");
		if (mail_file == NULL) {
			const char *user = g_getenv ("USER");
			if (user != NULL)
				mc->mail_file = g_strdup_printf ("/var/spool/mail/%s",
							 user);
		} else
			mc->mail_file = g_strdup (mail_file);
	}

	mc->mailcheck_text_only = _("Text only");

	mc->size = panel_applet_get_size (applet);

	g_signal_connect(G_OBJECT(applet), "size_allocate",
			 G_CALLBACK(applet_size_allocate),
			 mc);
			 
	g_signal_connect(G_OBJECT(applet), "change_orient",
			 G_CALLBACK(applet_change_orient),
			 mc);

	g_signal_connect(G_OBJECT(applet), "change_background",
			 G_CALLBACK(applet_change_background),
			 mc);

	mailcheck = create_mail_widgets (mc);
	gtk_widget_show(mailcheck);

	gtk_container_add (GTK_CONTAINER (applet), mailcheck);

	g_signal_connect(G_OBJECT(mc->ebox), "button_press_event",
			 G_CALLBACK(exec_clicked_cmd),
			 mc);

	g_signal_connect(G_OBJECT(applet), "key_press_event",
			 G_CALLBACK(key_press_cb),
			 mc);

	panel_applet_setup_menu_from_file (applet,
					   NULL,
					   "GNOME_MailCheckApplet.xml",
					   NULL, 
			        	   mailcheck_menu_verbs,
					   mc);

	if (panel_applet_get_locked_down (applet)) {
		BonoboUIComponent *popup_component;

		popup_component = panel_applet_get_popup_component (applet);

		bonobo_ui_component_set_prop (popup_component,
					      "/commands/Preferences",
					      "hidden", "1",
					      NULL);
	}
	
	gtk_label_set_text (GTK_LABEL (mc->label), _("Status not updated"));
	set_tooltip (GTK_WIDGET (mc->applet), _("Status not updated"));
	set_atk_name_description (GTK_WIDGET (mc->applet), _("Mail check"), 
			_("Mail check notifies you when new mail arrives in your mailbox"));
	gtk_widget_show_all (GTK_WIDGET (applet));

	/*
	 * check the mail if the applet is  realized. Checking the mail 
	 * right now (in case the applet is not realized), will give us 
	 * wrong screen value. 
	 */

	if (GTK_WIDGET_REALIZED (GTK_WIDGET (applet)))
		mail_check_timeout (mc);
	else
		mc->applet_realized_signal =
			g_signal_connect (G_OBJECT(applet), "realize",
					  G_CALLBACK(applet_realized_cb), mc);

	return(TRUE);
}

static gboolean
mailcheck_factory (PanelApplet *applet,
		   const char  *iid,
		   gpointer     data)
{
	gboolean retval = FALSE;

	if (!strcmp (iid, "OAFIID:GNOME_MailcheckApplet"))
		retval = mailcheck_applet_fill (applet);

	return retval;
}

PANEL_APPLET_BONOBO_FACTORY ("OAFIID:GNOME_MailcheckApplet_Factory",
			     PANEL_TYPE_APPLET,
                             "InboxMonitor",
                             "0",
                             mailcheck_factory,
                             NULL)
