#ifndef __HTTP_H_
#define __HTTP_H_

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

#include <ghttp.h>

/*
 * Called whenever a background request finishes,
 * either successfully or unsuccesfully.  Remember to
 * destroy the HTTP request object when done with it!
 */
typedef void (*HttpCallback) (ghttp_request *req,
                              ghttp_status   status,
                              gpointer       data);

/*
 * Set an HTTP request for background processing.
 * Upon error returns -1, else returns a bg job tag.
 */
extern guint http_process_bg (ghttp_request *req,
                              HttpCallback   cb,
                              gpointer       data);

/*
 * Clear a background HTTP request.  You are still responsible
 * for destroying the request that corresponds to this tag.
 */
extern void http_request_remove (int tag);

#endif /* __HTTP_H_ */

