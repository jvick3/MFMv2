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
#include "Element_Eraser.h"
#include "Element_Data.h"

namespace MFM
{

#define ISOLATOR_VERSION 5

  template <class CC>
  class Element_Isolator : public Element<CC>
  {
    // Short names for params
    typedef typename CC::ATOM_TYPE T;
    typedef typename CC::PARAM_CONFIG P;
    enum { R = P::EVENT_WINDOW_RADIUS };

  private:
    ElementParameterS32<CC> m_cellRadius;

    s32 manhattanDist(const SPoint& p1, const SPoint& p2) const
    {  s32 x1 = p1.GetX();
       s32 x2 = p2.GetX();

       s32 y1 = p1.GetY();
       s32 y2 = p2.GetY();

       return ABS(y1-y2) + ABS(x2-x1);
    }

    /* Given an Event Window, the center of it (origin), and a site, this method returns the distance of 
       the nearest non-Empty Element to site, or the max distance visible to site if no Element is seen.
     */
    s32 nearest_elem_dist(EventWindow<CC>& window, const SPoint& origin, 
                          const SPoint& site, u32 ignore_type) const
    {  	    
       const MDist<R>& md = MDist<R>::get();
       s32 nearest_dist = 0;

       for (u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(R); ++i)
       {  const SPoint baseSite = md.GetPoint(i); 
          const SPoint donutSite = baseSite + site;
	  if ((donutSite - origin).GetManhattanLength() > R)  // site not visible in Event Window
          {
             continue;
          }
          if (! window.IsLiveSite(donutSite))
	  {
	     continue;
	  }
          nearest_dist = donutSite.GetX() + donutSite.GetY();  // manhattan distance of donutSite 
	  const u32 type = window.GetRelativeAtom(donutSite).GetType();
	  //          if (type == ignore_type)  // ignore type is visible, do not swap
	  //   return 1000;
          if(type  !=  Element_Empty<CC>::THE_INSTANCE.GetType())  // Empty and the ignore_type not considered
	    {  return nearest_dist;
	    }           
       }
             
       return nearest_dist;
    }

    s32 unit_reduce(s32 num) const
    { if (num < 0) 
	 return -1;
      else if (num > 0)
	 return 1;
      
      return 0;  
    }  

  public:
    static Element_Isolator THE_INSTANCE;
    static const u32 TYPE() {
      return THE_INSTANCE.GetType();
    }

    Element_Isolator() : Element<CC>(MFM_UUID_FOR("Isolator", ISOLATOR_VERSION)), 
			 m_cellRadius(this, "cellRadius", "Cell Radius", "Isolator cell radius spacing", 
                                      1, R-1, R)

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

     virtual u32 Diffusability(EventWindow<CC> & ew, SPoint nowAt, SPoint maybeAt) const
     {
      return nowAt.Equals(maybeAt) ? 
                                    Element<CC>::COMPLETE_DIFFUSABILITY 
                                    : 0;
     }

    virtual u32 PercentMovable(const T& you,
                               const T& me, const SPoint& offset) const
    {
      return 0;
    }

    virtual void Behavior(EventWindow<CC>& window) const
    {  
       T self = window.GetCenterAtom();
       const MDist<R>& md = MDist<R>::get();
       const SPoint center_point = md.GetPoint(md.GetFirstIndex(0));
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
        
          // Non-empty, non-isolator Element found
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
		 if (manhattanDist(center_point, donutSite) > R || !window.IsLiveSite(donutSite))
                 {  
                    continue;
                 }
		 type = window.GetRelativeAtom(donutSite).GetType();

                 if (baseSite.GetManhattanLength() < range)  
		 { 	    
                   if (type == Element_Isolator<CC>::THE_INSTANCE.GetType())
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

            // If this Isolator is on the edge of the 'bubble' (R-1), look at all sites A of at least distance
            // R to this Isolator; If A is of greater distance than R-1 (further from seen Element than this isolator)
            // and has a non-empty non-isolator Atom in it, Isolator will try to swap the seen Element
            // away from itself (and thus away from the other seen element at A).  The distance it swaps 
            // the Element is at most 2; 1 unit in the X and Y directions.
            s32 length_to_E = manhattanDist(center_point, site);
            //if (length_to_E == R-1)  
	    // { 
               for (u32 i = md.GetFirstIndex(1); i <= md.GetLastIndex(R); ++i) 
	       {  SPoint adj_site = md.GetPoint(i);
                  u32 seen_type = window.GetRelativeAtom(adj_site).GetType();
		  if (manhattanDist(adj_site, site) > length_to_E)
		  {
		    // now looking for other non-isolator elements
		     if ( seen_type != Element_Isolator<CC>::THE_INSTANCE.GetType() 
                          && seen_type != Element_Empty<CC>::THE_INSTANCE.GetType())
		     {  
		        SPoint away_offset, away_site;
			s32 site_x = site.GetX();
			s32 site_y = site.GetY();
			LOG.Message("swapping: dist = %d\n", length_to_E);

                        // If either x or y offsets are zero, increase the non-zero one
			// If both x and y offsets are non-zero, flip coin; 
                        // either move Element in X or Y one unit

			if (site_x != 0 && site_y == 0)
			  {  away_offset.SetX(unit_reduce(site_x));
			     away_offset.SetY(0);
			  }
			else if (site_y != 0 && site_x == 0)
			  {  away_offset.SetX(0);
                             away_offset.SetY(unit_reduce(site_y));
			  }
			else   // both X and Y offsets are non-zero; randomly increase the magnitude of one of them
			  {  if (window.GetRandom().OneIn(2))
			      {  away_offset.SetX(unit_reduce(site_x));
				 away_offset.SetY(0);
			      }
			     else
			      {  away_offset.SetX(0);
				 away_offset.SetY(unit_reduce(site_y));
			      } 
			  } 

			away_site = site + away_offset;

			if (window.IsLiveSite(away_site))
			  {   window.SwapAtoms(site, away_site);
			  }
		     }
		  }
	       }
 
	       //}
	    /***********************************************************************/

	  
         } // end brace for seeing an element

       }  // end brace for overall search loop
       
       // haven't seen anything, die
       if (! element_found)   
       {
          window.SetCenterAtom(Element_Empty<CC>::THE_INSTANCE.GetDefaultAtom());
       }
       /**********************************************/
    }

  };

  template <class CC>
  Element_Isolator<CC> Element_Isolator<CC>::THE_INSTANCE;
}

#endif /* ELEMENT_ISOLATOR_H */
