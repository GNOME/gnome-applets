/* $Id$ */

/*
 *  Papadimitriou Spiros <spapadim+@cs.cmu.edu>
 *
 *  This code released under the GNU GPL.
 *  Read the file COPYING for more information.
 *
 *  Background HTTP processing
 *
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <gnome.h>

#include "http.h"

struct _HttpBgRequest {
    ghttp_request *req;
    HttpCallback   cb;
    gpointer       cb_data;
};

typedef struct _HttpBgRequest HttpBgRequest;


static gint http_check_idle_cb (gpointer data)
{
    HttpBgRequest *bg_req = (HttpBgRequest *)data;
    ghttp_status status = ghttp_process(bg_req->req);
    if (status != ghttp_not_done) {
        (*bg_req->cb)(bg_req->req, status, bg_req->cb_data);
        return 0;  /* We're done with this idle callback */
    }
    return 1;  /* Call us next time around, still work to do */
}

static void http_check_destroy_cb (gpointer data)
{
    HttpBgRequest *bg_req = (HttpBgRequest *)data;
    g_free(bg_req);
}


guint http_process_bg (ghttp_request *req, HttpCallback cb, gpointer data)
{
    HttpBgRequest *bg_req = g_new(HttpBgRequest, 1);
    bg_req->req = req;
    bg_req->cb = cb;
    bg_req->cb_data = data;
    ghttp_prepare(req);
    ghttp_set_sync(req, ghttp_async);
    return gtk_idle_add_full(GTK_PRIORITY_DEFAULT, http_check_idle_cb,
                             NULL, bg_req, http_check_destroy_cb);
}

void http_request_remove (int tag)
{
    gtk_idle_remove(tag);
}

