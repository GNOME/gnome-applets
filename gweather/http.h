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
 * Initialize asynchronous HTTP transfer functions.
 */
extern void http_init (void);

/*
 * Set an HTTP request for background processing.
 * Returns 0 upon success, -1 otherwise.
 */
extern guint http_process_bg (ghttp_request *req,
                              HttpCallback   cb,
                              gpointer       data);

#endif /* __HTTP_H_ */

