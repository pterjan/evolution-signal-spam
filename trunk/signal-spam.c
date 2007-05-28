/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*-
 *
 *  Copyright 2007 Pascal Terjan <pterjan@linuxfr.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of version 2 of the GNU General Public
 *  License as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtktable.h>

#include <camel/camel-folder.h>
#include <camel/camel-data-wrapper.h>
#include <camel/camel-stream-mem.h>

#include <libedataserverui/e-passwords.h>

#include <e-util/e-config.h>

#include <gconf/gconf-client.h>

#include <mail/em-menu.h>
#include <mail/em-popup.h>
#include <mail/em-utils.h>

#define COMPONENT "signal-spam"
#define GCONFKEY_LOGIN "/apps/evolution/eplugin/signal_spam/login"

extern gboolean send_report(const char *message, const char *user, const char *password);
static GConfClient *gconf;

typedef struct {
	GPtrArray *uids;
	struct _CamelFolder *folder;
	char *user;
	char *password;
} ReportMessagesParams;


static gpointer _report_messages (ReportMessagesParams *data)
{
	int i;
	char *msg;
        CamelMimeMessage *message;
        CamelStream *stream;
	GByteArray *barray;

	g_return_val_if_fail(data != NULL, NULL);

        for (i = 0; i < (data->uids ? data->uids->len : 0); i++) {
                message = camel_folder_get_message (data->folder, g_ptr_array_index (data->uids, i), NULL);
		g_free(g_ptr_array_index (data->uids, i));
                if (!message) {
                        continue;
                }
		stream = camel_stream_mem_new();
		barray = g_byte_array_new();
		camel_stream_mem_set_byte_array(CAMEL_STREAM_MEM (stream), barray);
		camel_data_wrapper_write_to_stream(CAMEL_DATA_WRAPPER (message), stream);
		camel_stream_flush (stream);
                camel_object_unref (stream);
		msg = (char *) g_byte_array_free(barray, FALSE);
		send_report(msg, data->user, data->password);
	}
	g_ptr_array_free (data->uids, TRUE);
	free(data->user);
	free(data->password);
	g_free(data);
	return NULL;
}

static void report_messages (GPtrArray *uids, struct _CamelFolder *folder)
{
	ReportMessagesParams *params;
	gboolean remember;
	GError *error;

	params = (ReportMessagesParams *) g_malloc(sizeof(ReportMessagesParams));

	params->user = gconf_client_get_string(gconf, GCONFKEY_LOGIN, NULL);
	if (!params->user) {
		//FIXME Report to the user
		g_warning("signal-spam: Not yet setup.");
		g_free(params);
		return;
	}

	params->password = e_passwords_get_password(COMPONENT, params->user);
	if (!params->password) {
		params->password = e_passwords_ask_password("signal-spam",
							COMPONENT,
							params->user,
							"Please enter your password for signal-sapm.fr",
							E_PASSWORDS_REMEMBER_FOREVER|E_PASSWORDS_SECRET,
							&remember,
							NULL);
		if (!params->password) {
			//FIXME report to the user
			return;
		}
	}

	params->uids = uids;
	params->folder = folder;

	g_thread_create(_report_messages, params, FALSE, &error);
}

static void
copy_uids (char *uid, GPtrArray *uid_array)
{
	        g_ptr_array_add (uid_array, g_strdup (uid));
}

void signal_spam(EPlugin *ep, EMPopupTargetSelect *t)
{
        GPtrArray *uid_array = NULL;

        if (t->uids->len > 0) {
                uid_array = g_ptr_array_new ();
                g_ptr_array_foreach (t->uids, (GFunc)copy_uids, (gpointer) uid_array);
        } else {
                return;
        }

        report_messages (uid_array, t->folder);
}

void signal_spam_menu (EPlugin *ep, EMMenuTargetSelect *t)
{
        GPtrArray *uid_array = NULL;

        if (t->uids->len > 0) {
                uid_array = g_ptr_array_new ();
                g_ptr_array_foreach (t->uids, (GFunc)copy_uids, (gpointer) uid_array);
        } else {
                return;
        }

        report_messages (uid_array, t->folder);
}

static void
signal_spam_login_changed(GtkEntry *entry, void *dummy)
{
        char *login = gtk_entry_get_text(entry);
        gconf_client_set_string(gconf, GCONFKEY_LOGIN, login, NULL);
}

GtkWidget *signal_spam_login (EPlugin *epl, EConfigHookItemFactoryData *data);

GtkWidget *
signal_spam_login (EPlugin *epl, EConfigHookItemFactoryData *data)
{
	static GtkWidget *label;
	GtkWidget *entry, *parent;
	int row;
	static GtkWidget *hidden = NULL;
	char *login;

	if (!hidden)
		hidden = gtk_label_new ("");

	if (data->old)
		gtk_widget_destroy (label);

	parent = data->parent;

	row = ((GtkTable*)parent)->nrows;

	label = gtk_label_new("Login:");
	gtk_widget_show (label);
	gtk_misc_set_alignment (GTK_MISC (label), 0.0, 0.5);
	gtk_table_attach (GTK_TABLE (parent), label, 0, 1, row, row+1, GTK_FILL, 0, 0, 0);

	entry = gtk_entry_new ();
        g_signal_connect(entry, "changed", G_CALLBACK(signal_spam_login_changed), NULL);
	gtk_widget_show (entry);
	login = gconf_client_get_string(gconf, GCONFKEY_LOGIN, NULL);
	gtk_entry_set_text (GTK_ENTRY (entry), login);
	g_free(login);
	gtk_table_attach (GTK_TABLE (parent), entry, 1, 2, row, row+1, GTK_EXPAND | GTK_FILL, 0, 0, 0);
	gtk_label_set_mnemonic_widget (GTK_LABEL (label), entry);

	return entry;
}

int e_plugin_lib_enable(EPluginLib *ep, int enable);

int
e_plugin_lib_enable(EPluginLib *ep, int enable)
{
        if (enable) {
                gconf = gconf_client_get_default();
        } else {
                if (gconf) {
                        g_object_unref(gconf);
                        gconf = 0;
                }
        }

        return 0;
}
