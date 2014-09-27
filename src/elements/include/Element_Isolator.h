/*                                              -*- mode:C++ -*-
  Element_Wall.h Basic immovable element
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
  \file Element_Isolator.h Surrounds other Elements with itself
  \author James Vickers
  \date (C) 2014 All rights reserved.
  \lgpl
 */
#ifndef ELEMENT_ISOLATOR_H
#define ELEMENT_ISOLATOR_H

#include "Element.h"
#include "EventWindow.h"
#include "ElementTable.h"
#include "itype.h"
#include "P1Atom.h"

namespace MFM
{

#define ISOLATOR_VERSION 2

  template <class CC>
  class Element_Isolator : public Element<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:
    ElementParameterS32<CC> m_cellRadius;
    ElementParameterS32<CC> m_deathProb;

  public:
    static Element_Isolator THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Isolator() : Element<CC>(MFM_UUID_FOR("Isolator", ISOLATOR_VERSION)), 
			 m_cellRadius(this, "cellRadius", "Cell Radius", "Isolator cell radius spacing", 
                                      1, R-1, R, 1),
                         m_deathProb(this, "deathProb", "Odds of dying", "Probability of dying for an Isolator",
                                     1, 100, 1000, 1) 
    {
      Element<CC>::SetAtomicSymbol("Is");
      Element<CC>::SetName("Isolator");
    }

    virtual const T & GetDefaultAtom() const
    {
      static T defaultAtom(TYPE(),0,0,0);
      return defaultAtom;
    }

    virtual u32 DefaultPhysicsColor() const
    {
      return 0xcc3399;  // purplish
    }

    virtual u32 DefaultLowlightColor() const
    {
      return 0xff66cc;  // lighter, pinklish-purple
    }

    virtual const char* GetDescription() const
    {
      return "Surrounds other Elements by writing itself to the edge of the Event Window";
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
       T self = window.GetCenterAtom();
       const MDist<R> md = MDist<R>::get();
       bool element_found = false;

       // Look in every site of the Event Window 
       for (u32 idx = md.GetFirstIndex(1); idx <= md.GetLastIndex(R); ++idx)
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
        
          // If a non-isolator and non-empty Element is found, surround it
          if(type != Element_Isolator<CC>::THE_INSTANCE.GetType() 
                             && 
             type != Element_Empty<CC>::THE_INSTANCE.GetType())
          {
            element_found = true;

            // Look at all sites within R of the found Element.  Any sites less than range (Cell Radius) away
            // should be removed of Isolators, and sites >= range in manhattan distance should become Isolator
            u32 range =  (u32) m_cellRadius.GetValue();
	    for (u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(R); ++i)
	      {  const SPoint baseSite = md.GetPoint(i);
                 const SPoint donutSite = baseSite + site;
		 if (donutSite.GetManhattanLength() > R)
                 {
                    continue;
                 }
		 type = window.GetRelativeAtom(donutSite).GetType();

                 if (baseSite.GetManhattanLength() < range)
		 { if (type == Element_Isolator<CC>::THE_INSTANCE.GetType())
		   {  window.SetRelativeAtom(donutSite, Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
		   }
		 }
                 else 
		 { if (type == Element_Empty<CC>::THE_INSTANCE.GetType())
		   {  window.SetRelativeAtom(donutSite, self);  // copy itself to donut location
		   }
		 }

	      }
            /*******************************************************************/

            // Do not break out, keep looking for more Elements: ensure radius is maintained for all!
            // return; 
          }
       }
       
       // haven't seen anything, die or do random walk
       if (! element_found)   
       {
          // Roll the dice and die if unlucky
          if (window.GetRandom().OneIn(m_deathProb.GetValue()))
          {  
             window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
          }
          else      // Didn't die, random walk
          {  randomWalk(window);
          }
       }
       /**********************************************/
    }

  private:

    s32 manhattanDist(const SPoint& p1, const SPoint& p2)
    {  s32 x1 = p1.GetX();
       s32 x2 = p2.GetX();

       s32 y1 = p1.GetY();
       s32 y2 = p2.GetY();

       return ABS(y1-y2) + ABS(x2-x1);
    }

    // Does a random walk given an EventWindow.  
    // Taken straight from AbstractElement_Wanderer.h
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
	   if(window.GetRelativeAtom(wanderPt).GetType() ==
	      Element_Empty<CC>::THE_INSTANCE.GetType())
	   {
	     window.SwapAtoms(wanderPt, SPoint(0, 0));
	   }
	 }
    
    }

  };

  template <class CC>
  Element_Isolator<CC> Element_Isolator<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_ISOLATOR_H */
