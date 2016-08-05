//
// "$Id: Fl_Menu_Window.cxx 10095 2014-02-07 00:09:52Z AlbrechtS $"
//
// Menu window code for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     http://www.fltk.org/COPYING.php
//
// Please report all bugs and problems on the following page:
//
//     http://www.fltk.org/str.php
//

// This is the window type used by Fl_Menu to make the pop-ups.
// It draws in the overlay planes if possible.

// Also here is the implementation of the mouse & keyboard grab,
// which are used so that clicks outside the program's windows
// can be used to dismiss the menus.

#include <config.h>
#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Menu_Window.H>

// WIN32 note: HAVE_OVERLAY is false
#if HAVE_OVERLAY
extern XVisualInfo *fl_find_overlay_visual();
extern XVisualInfo *fl_overlay_visual;
extern Colormap fl_overlay_colormap;
extern unsigned long fl_transparent_pixel;
static GC gc;	// the GC used by all X windows
extern uchar fl_overlay; // changes how fl_color(x) works
#endif

#include <stdio.h>

void Fl_Menu_Window::show() {
#if HAVE_OVERLAY
  if (!shown() && overlay() && fl_find_overlay_visual()) {
    XInstallColormap(fl_display, fl_overlay_colormap);
    fl_background_pixel = int(fl_transparent_pixel);
    Fl_X::make_xid(this, fl_overlay_visual, fl_overlay_colormap);
    fl_background_pixel = -1;
  } else
#endif
    Fl_Single_Window::show();
}

void Fl_Menu_Window::flush() {
  if (!shown()) return;
#if HAVE_OVERLAY
  if (!fl_overlay_visual || !overlay()) {Fl_Single_Window::flush(); return;}
  Fl_X *myi = Fl_X::i(this);
  fl_window = myi->xid;
  if (!gc) {
	  gc = XCreateGC(fl_display, myi->xid, 0, 0);
# if defined(FLTK_USE_CAIRO)
	  if(Fl::autolink_context()) Fl::cairo_make_current(gc); // capture gc changes automatically to update the cairo context adequately
# endif
  }
  fl_gc = gc;
  fl_overlay = 1;
  fl_clip_region(myi->region); myi->region = 0; current_ = this;
  draw();
  fl_overlay = 0;
#else
  Fl_Single_Window::flush();
#endif
}

/** Erases the window, does nothing if HAVE_OVERLAY is not defined config.h */
void Fl_Menu_Window::erase() {
#if HAVE_OVERLAY
  if (!gc || !shown()) return;
//XSetForeground(fl_display, gc, 0);
//XFillRectangle(fl_display, fl_xid(this), gc, 0, 0, w(), h());
  XClearWindow(fl_display, fl_xid(this));
#endif
}

// Fix the colormap flashing on Maximum Impact Graphics by erasing the
// menu before unmapping it:
void Fl_Menu_Window::hide() {
  erase();
  Fl_Single_Window::hide();
}

/**  Destroys the window and all of its children.*/
Fl_Menu_Window::~Fl_Menu_Window() {
  hide();
}


Fl_Menu_Window::Fl_Menu_Window(int W, int H, const char *l)
: Fl_Single_Window(W,H,l) 
{ 
  image(0); 
}


Fl_Menu_Window::Fl_Menu_Window(int X, int Y, int W, int H, const char *l)
: Fl_Single_Window(X,Y,W,H,l) { 
  image(0); 
}


//
// End of "$Id: Fl_Menu_Window.cxx 10095 2014-02-07 00:09:52Z AlbrechtS $".
//
