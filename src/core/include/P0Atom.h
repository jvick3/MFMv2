#ifndef P0ATOM_H      /* -*- C++ -*- */
#define P0ATOM_H

#include <stdio.h>
#include "itype.h"
#include "Point.h"
#include "BitField.h"
#include "MDist.h"
#include "Atom.h"
#include "Element.h"
#include "CoreConfig.h"
#include "ParamConfig.h"

#define P0ATOM_HEADER_LENGTH_SIZE 2
#define P0ATOM_TYPE_WIDTH_INCREMENT 12
#define P0ATOM_STATE_SIZE 48

namespace MFM {

  template <class PC>
  class P0Atom : public Atom< CoreConfig< P0Atom<PC>, PC> >
  {
    enum { 
      BITS = 64,
      // For now we insist on exact match.  Possibly longer could be
      // supported relatively easily.
      CONFIGURED_BITS_PER_ATOM_IS_INCOMPATIBLE_WITH_P0ATOM = 1/((PC::BITS_PER_ATOM==BITS)?1:0)
    };

    typedef BitField<BitVector<BITS>,P0ATOM_HEADER_LENGTH_SIZE,0> AFTypeLengthCode;
    typedef BitField<BitVector<BITS>,0*P0ATOM_TYPE_WIDTH_INCREMENT,AFTypeLengthCode::END> AFTypeLength0;
    typedef BitField<BitVector<BITS>,1*P0ATOM_TYPE_WIDTH_INCREMENT,AFTypeLengthCode::END> AFTypeLength1;
    typedef BitField<BitVector<BITS>,2*P0ATOM_TYPE_WIDTH_INCREMENT,AFTypeLengthCode::END> AFTypeLength2;
    typedef BitField<BitVector<BITS>,28,AFTypeLengthCode::END> AFTypeLength3;

  protected:

    typedef MDist<4> MDist4;

    /* We really don't want to allow the public to change the type of a
       P0Atom, since the type doesn't mean much without the atomic
       header as well */

    void SetType(u32 lengthCode, u32 type) {
      AFTypeLengthCode::Write(this->m_bits,lengthCode);
      switch (lengthCode) {
      case 0: AFTypeLength0::Write(this->m_bits,type); break;
      case 1: AFTypeLength1::Write(this->m_bits,type); break;
      case 2: AFTypeLength2::Write(this->m_bits,type); break;
      case 3: AFTypeLength3::Write(this->m_bits,type); break;
      default:
        FAIL(ILLEGAL_ARGUMENT);
      }
    }

    u32 GetLengthCodeForType(u32 type) 
    {
      for (u32 i = 0; i <= 3; ++i) {
        u32 typebits = i*P0ATOM_TYPE_WIDTH_INCREMENT;
        u32 maxtype = 1u<<typebits;
        if (type < maxtype) {
          return i;
        }
      }
      FAIL(ILLEGAL_ARGUMENT);
    }

  public:

    u32 GetType() const {
      u32 lengthCode = AFTypeLengthCode::Read(this->m_bits);
      switch (lengthCode) {
      case 0: return AFTypeLength0::Read(this->m_bits);
      case 1: return AFTypeLength1::Read(this->m_bits);
      case 2: return AFTypeLength2::Read(this->m_bits);
      case 3: return AFTypeLength3::Read(this->m_bits); 
      default:
        FAIL(UNREACHABLE_CODE);
      }
    }

    u32 GetMaxStateSize(u32 lengthCode) const {
      if (lengthCode >= 1<<P0ATOM_HEADER_LENGTH_SIZE)
        FAIL(ILLEGAL_ARGUMENT);
      return BITS-P0ATOM_HEADER_LENGTH_SIZE-P0ATOM_TYPE_WIDTH_INCREMENT*lengthCode;
    }

    P0Atom()
    {
      InitAtom(ELEMENT_EMPTY,0,0,P0ATOM_STATE_SIZE);
    }

    P0Atom(u32 type, u32 longc, u32 shortc, u32 statec) 
    { 
      InitAtom(type,longc,shortc,statec);
    }

    void InitAtom(u32 type, u32 longc, u32 shortc, u32 statec) 
    {
      if (longc != 0)
        FAIL(ILLEGAL_ARGUMENT);
      if (shortc != 0)
        FAIL(ILLEGAL_ARGUMENT);

      u32 lengthCode =
        GetLengthCodeForType(type);
      u32 maxState = 
        GetMaxStateSize(lengthCode);

      if (statec > maxState)
        FAIL(ILLEGAL_ARGUMENT);

      SetType(lengthCode, type);

    }

    void PrintBits(FILE* ostream) const
    { this->m_bits.Print(ostream); }

    void Print(FILE* ostream) const
    { 
      u32 lengthCode = AFTypeLengthCode::Read();
      u32 type = GetType();
      fprintf(ostream,"P0[%x/",type);
      u32 length = GetMaxStateSize(lengthCode);
      for (int i = 0; i < length; i += 4) {
        u32 nyb = this->m_bits.Read(length+i,4);
        fprintf(ostream,"%x",nyb);
      }
      fprintf(ostream,"]");
    }

    P0Atom& operator=(const P0Atom & rhs);
  };
} /* namespace MFM */

#include "P0Atom.tcc"

#endif /*P0ATOM_H*/

