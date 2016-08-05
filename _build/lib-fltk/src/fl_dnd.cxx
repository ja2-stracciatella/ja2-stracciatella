//
// "$Id: fl_dnd.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $"
//
// Drag & Drop code for the Fast Light Tool Kit (FLTK).
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

#ifdef WIN32
#  include "fl_dnd_win32.cxx"
#elif defined(__APPLE__)
//#  include "fl_dnd_mac.cxx"
#else
#  include "fl_dnd_x.cxx"
#endif

//
// End of "$Id: fl_dnd.cxx 8864 2011-07-19 04:49:30Z greg.ercolano $".
//
