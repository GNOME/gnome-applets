#ifndef __WINMGR_H
#define __WINMGR_H

/* all of this is still experimental -- feedback welcome */

/* 0.3
 *   - renamed WinStateSticky -> WinStateAllWorkspaces
 */
/* 0.2
 *   - added separate flags for horizonal/vertical maximized state
 */
/* 0.1 */

#define XA_WIN_PROTOCOLS       "WIN_PROTOCOLS"
/* Type: array of Atom
 *       set on Root window by the window manager.
 *
 * This property contains all of the protocols/hints supported by
 * the window manager (WM_HINTS, MWM_HINTS, WIN_*, etc.
 */

#define XA_WIN_ICONS           "WIN_ICONS"
/* Type: array of XID, alternating between Pixmap and Mask for icons
 *       set by applications on their toplevel windows.
 *
 * This property contains additional icons for the application.
 * if this property is set, the WM will ignore default X icon hints
 * and KWM_WIN_ICON hint.
 *
 * There are two values for each icon: Pixmap and Mask (Mask can be None
 * if transparency is not required).
 *
 * Icewm currenty needs/uses icons of size 16x16 and 32x32 with default
 * depth. Applications are recommended to set several icons sizes
 * (recommended 16x16,32x32,48x48 at least)
 *
 * TODO: WM should present a wishlist of desired icons somehow. (standard
 * WM_ICON_SIZES property is only a partial solution). 16x16 icons can be
 * quite small on large resolutions.
 */

/* workspace */
#define XA_WIN_WORKSPACE       "WIN_WORKSPACE"
/* Type: CARD32
 *       Root Window: current workspace, set by the window manager
 *
 *       Application Window: current workspace. The default workspace
 *       can be set by applications, but they must use ClientMessages to
 *       change them. When window is mapped, the propery should only be
 *       updated by the window manager.
 *
 * Applications wanting to change the current workspace should send
 * a ClientMessage to the Root window like this:
 *     xev.type = ClientMessage;
 *     xev.window = root_window or toplevel_window;
 *     xev.message_type = _XA_WIN_WORKSPACE;
 *     xev.format = 32;
 *     xev.data.l[0] = workspace;
 *     xev.data.l[1] = timeStamp;
 *     XSendEvent(display, root, False, SubstructureNotifyMask, (XEvent *) &xev);
 *
 */
 
#define XA_WIN_WORKSPACE_COUNT "WIN_WORKSPACE_COUNT"
/* Type: CARD32
 *       workspace count, set by window manager
 *
 * NOT FINALIZED/IMPLEMENTED YET
 */

#define XA_WIN_WORKSPACE_NAMES "_WIN_WORKSPACE_NAMES"
/* Type: StringList (TextPropery)
 *
 * 
 * IMPLEMENTED but not FINALIZED.
 * perhaps the name should be separate for each workspace (like KDE).
 * this where WIN_WORKSPACE_COUNT comes into play.
 */

#define WinWorkspaceInvalid    0xFFFFFFFFUL

/* layer */
#define XA_WIN_LAYER           "WIN_LAYER"
/* Type: CARD32
 *       window layer
 *
 * Window layer.
 * Windows with LAYER=WinLayerDock determine size of the Work Area
 * (WIN_WORKAREA). Windows below dock layer are resized to the size
 * of the work area when maximized. Windows above dock layer are
 * maximized to the entire screen space.
 *
 * The default can be set by application, but when window is mapped
 * only window manager can change this. If an application wants to change
 * the window layer it should send the ClientMessage to the root window
 * like this:
 *     xev.type = ClientMessage;
 *     xev.window = toplevel_window;
 *     xev.message_type = _XA_WIN_LAYER;
 *     xev.format = 32;
 *     xev.data.l[0] = layer;
 *     xev.data.l[1] = timeStamp;
 *     XSendEvent(display, root, False, SubstructureNotifyMask, (XEvent *) &xev);
 *
 * TODO: A few available layers could be used for WMaker menus/submenus
 * (comments?)
 *
 * Partially implemented. Currently requires all docked windows to be 
 * sticky (only one workarea for all workspaces). Otherwise non-docked sticky
 * windows could (?) move when switching workspaces (annoying).
 */
 
#define WinLayerCount          16
#define WinLayerInvalid        0xFFFFFFFFUL

#define WinLayerDesktop        0UL
#define WinLayerBelow          2UL
#define WinLayerNormal         4UL
#define WinLayerOnTop          6UL
#define WinLayerDock           8UL
#define WinLayerAboveDock      10UL

/* state */
#define XA_WIN_STATE           "WIN_STATE"

/* Type CARD32[2]
 *      window state. First CARD32 is the mask of set states,
 *      the second one is the state.
 *
 * The default value for this property can be set by applications.
 * When window is mapped, the property is updated by the window manager
 * as necessary. Applications can request the state change by sending
 * the client message to the root window like this:
 *
 *   xev.type = ClientMessage;
 *   xev.window = toplevel_window;
 *   xev.message_type = _XA_WIN_WORKSPACE;
 *   xev.format = 32;
 *   xev.data.l[0] = mask; // mask of the states to change
 *   xev.data.l[1] = state; // new state values
 *   xev.data.l[2] = timeStamp;
 *   XSendEvent(display, root, False, SubstructureNotifyMask, (XEvent *) &xev);
 */

#define WinStateAllWorkspaces  (1 << 0)   /* appears on all workspaces */
#define WinStateMinimized      (1 << 1)
#define WinStateMaximizedVert  (1 << 2)   /* maximized vertically */
#define WinStateMaximizedHoriz (1 << 3)   /* maximized horizontally */
#define WinStateHidden         (1 << 4)   /* not on taskbar if any, but still accessible */
#define WinStateRollup         (1 << 5)   /* only titlebar visible */
#define WinStateHidWorkspace   (1 << 6)   /* not on current workspace */
#define WinStateHidTransient   (1 << 7)   /* hidden due to invisible owner window */

#define WinStateDockHorizontal (1 << 8)
/*
 * This state is necessary for correct WORKAREA negotiation when
 * a window is positioned in a screen corner. If set, it determines how
 * the place where window is subtracted from workare.
 *
 * Imagine a square docklet in the corner of the screen (several WMaker docklets
 * are like this).
 *
 * HHHHD
 * ....V
 * ....V
 * ....V
 *
 * If WinStateDockHorizontal is set, the WORKAREA will consist of area
 * covered by '.' and 'V', otherwise the WORKAREA will consist of area
 * covered by '. and 'H';
 *
 * Implemented. Works pretty well. Comments/improvements welcome.
 */

/* hints */
#define XA_WIN_HINTS           "WIN_HINTS"
#define WinHintsSkipFocus      (1 << 0)
#define WinHintsSkipWindowMenu (1 << 1)
#define WinHintsSkipTaskBar    (1 << 2)
#define WinHintsGroupTransient (1 << 3)

/* Type CARD32[2]
 *      additional window hints
 *
 *      Handling of this propery is very similiar to WIN_STATE.
 *
 * NOT IMPLEMENTED YET.
 */

/* work area */
#define XA_WIN_WORKAREA        "WIN_WORKAREA"
/*
 * CARD32[4]
 *     minX, minY, maxX, maxY of workarea.
 *     set/updated only by the window manager
 *
 * When windows are maximized they occupy the entire workarea except for
 * the titlebar at the top (in icewm window frame is not visible).
 *
 * Note: when WORKAREA changes, the application window are automatically
 * repositioned and maximized windows are also resized.
 */

/*
 possible
 - WIN_MAXIMIZED_GEOMETRY
 - WIN_RESIZE (protocol that allows applications to start sizing the frame)
 - WIN_FOCUS (protocol for focusing)
 
 - there should be a way to coordinate corner/edge points of the frames
   (for placement). ICCCM is vague on this issue. icewm prefers to treat windows
   as client-area and titlebar when doing the placement (ignoring the frame)

 - handling of virtual (>screen) scrolling desktops and pages (fvwm like?)
 */

#endif
