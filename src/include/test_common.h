#ifndef TEST_COMMON_H      /* -*- C++ -*- */
#define TEST_COMMON_H

#include "grid.h"
#include "p1atom.h"
#include "elementtable.h"
#include "eventwindow.h"
#include "tile.h"

/* Some types for us to test */
typedef Grid<P1Atom,4> GridP1Atom;
typedef ElementTable<P1Atom,4> ElementTableP1Atom;
typedef EventWindow<P1Atom,4> EventWindowP1Atom;
typedef Tile<P1Atom,4> TileP1Atom;

#endif /*TEST_COMMON_H*/
