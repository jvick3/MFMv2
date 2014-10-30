/*                                              -*- mode:C++ -*-
  Copyright (C) 2014 The Regents of the University of New Mexico.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
  USA
*/

/**
  \file Element_Eraser.h - Destroys anything within parameter "Radius" of itself
  \author James Vickers
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_ERASER_H
#define ELEMENT_ERASER_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

#define ERASER_VERSION 1

  template <class CC>
  class Element_Eraser : public Element<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:
    ElementParameterS32<CC> m_eraseRadius;

  private:

    // Does a random walk given an EventWindow.  
    // Taken straight from AbstractElement_Wanderer.h, 
    // except it will swap with non-Empty Elements as well
    void randomWalk(EventWindow<CC>& window) const
    {  SPoint wanderPt;
       Random& rand = window.GetRandom();
       const MDist<R> md = MDist<R>::get();
       Dir d = (Dir)rand.Create(Dirs::DIR_COUNT); 
       const int wanderDistance = 1;

       Dirs::FillDir(wanderPt, d);

       wanderPt *= Dirs::IsCorner(d) ? (wanderDistance / 2) : wanderDistance;

       if(window.IsLiveSite(wanderPt))
	 {
	     window.SwapAtoms(wanderPt, SPoint(0, 0));
	 }
    
    }

  public:
    static Element_Eraser THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Eraser() : Element<CC>(MFM_UUID_FOR("Eraser", ERASER_VERSION)), 
			 m_eraseRadius(this, "eraseRadius", "Erase Radius", "Eraser effective distance", 
                                      1, 2, R, 1)
    {
      Element<CC>::SetAtomicSymbol("Er");
      Element<CC>::SetName("Eraser");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0x58584c;  // gray
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0x58584c;  // gray
    }

    virtual const char* GetDescription() const
    {
      return "Erases any non-Eraser Element within 'Erase Radius' of itself, wanders";
    }

    //  virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
    // {
    //  return nowAt.Equals(maybeAt)?Element<CC>::COMPLETE_DIFFUSABILITY:0;
    //}

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 100;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {  
       const MDist<R> md = MDist<R>::get();

       // Look in every site of the Event Window within distance m_eraseRadius
       for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(m_eraseRadius.GetValue()); ++idx)
       {
	  // Get site, skip if invalid, get type of Element at site
          const SPoint site = md.GetPoint(idx);
          if (!window.IsLiveSite(site))
          {
            continue;
          }

          T other = window.GetRelativeAtom(site);
          u32 type = other.GetType();
          /********************************************************/
        
          // If a non-Empty and non-Eraser Element is found, destroy it (set to Empty)
          if(type != Element_Eraser<CC>::THE_INSTANCE.GetType() 
                             && 
             type != Element_Empty<CC>::THE_INSTANCE.GetType())
          {
              window.SetRelativeAtom(site, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());             
	  }            
          /*******************************************************************/
       }
       
       randomWalk(window);

    }
  
 };

  template <class CC>
  Element_Eraser<CC> Element_Eraser<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_ERASER_H */
