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

#include <pthread.h>
#include <signal.h>

#include <gnome.h>

#include "http.h"


/*
 * Request structure
 */

struct _HttpBgRequest {
    struct _HttpBgRequest *next;  /* Must be first field */
    ghttp_request         *req;
    HttpCallback          cb;
    gpointer              cb_data;
};

typedef struct _HttpBgRequest HttpBgRequest;


/*
 * Request queue
 */

static HttpBgRequest *http_req_queue = NULL;

static void http_req_enqueue (HttpBgRequest *req)
{
    HttpBgRequest **p = &http_req_queue;

    while (*p != NULL)
        p = (HttpBgRequest **)*p;
    req->next = NULL;
    *p = req;
}

static HttpBgRequest *http_req_dequeue (void)
{
    HttpBgRequest *req = http_req_queue;
    if (http_req_queue != NULL)
        http_req_queue = http_req_queue->next;
    return req;
}

/*
 * HTTP task thread
 */

static pthread_mutex_t http_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  http_cond = PTHREAD_COND_INITIALIZER;

static void *http_task (void *arg)
{
    HttpBgRequest *req;
    ghttp_status status;


    while (1) {
        pthread_mutex_lock(&http_mutex);
        pthread_cond_wait(&http_cond, &http_mutex);
        while ((req = http_req_dequeue()) != NULL) {
            fprintf(stderr, "Processing HTTP request...\n");
            ghttp_prepare(req->req);
            ghttp_set_sync(req->req, ghttp_sync);
            status = ghttp_process(req->req);

            pthread_mutex_unlock(&http_mutex);
            gdk_threads_enter();
            (*req->cb)(req->req, status, req->cb_data);
            gdk_threads_leave();
            pthread_mutex_lock(&http_mutex);

            g_free(req);
        }
        pthread_mutex_unlock(&http_mutex);
    }

    g_assert_not_reached();
}


/*
 * Exported functions
 */

guint http_process_bg (ghttp_request *req, HttpCallback cb, gpointer data)
{
    HttpBgRequest *bg_req;

    /*
     * FIX !!
     * Program will deadlock if gdk_threads_leave() is not
     * called here _without_ a matching gdk_threads_enter().
     * This is because this function's caller may either
     * hold or not hold the GTK mutex.
     * Ommiting gtk_threads_leave() will cause a deadlock
     * (because of contention for the GTK mutex and http_mutex)
     * if caller holds GTK mutex.  Adding gtk_threads_enter()
     * will deadlock if caller does not hold GTK mutex (because
     * the call will be normally repeated later, in the main
     * GTK loop).
     */
    gdk_threads_leave();

    pthread_mutex_lock(&http_mutex);
    bg_req = g_new(HttpBgRequest, 1);
    bg_req->req = req;
    bg_req->cb = cb;
    bg_req->cb_data = data;

    /* Insert request into queue and signal HTTP task */
    http_req_enqueue(bg_req);
    pthread_cond_signal(&http_cond);
    pthread_mutex_unlock(&http_mutex);

    return 0;
}

static pthread_t http_thread;

void http_init (void)
{
    /* Create HTTP task thread */
    pthread_create(&http_thread, NULL, http_task, NULL);
}

void http_done (void)
{
    pthread_kill(http_thread, SIGINT);
}

