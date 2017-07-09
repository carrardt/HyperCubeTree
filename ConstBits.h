#ifndef __CONST_BITS_H
#define __CONST_BITS_H


namespace AmrReconstruction3
{

  // Description des valeurs possibles d'un bit (l'information est entierement contenue dans le type)
  struct Bit1;
  struct Bit0 {
    enum { ONE=0, ZERO=1, UNDEF=0 };
    typedef Bit1 Neg;
    template<typename StreamT> static inline void toStream(StreamT& out){ out<<'0'; }
  };
  struct Bit1 {
    enum { ONE=1, ZERO=0, UNDEF=0 };
    typedef Bit0 Neg;
    template<typename StreamT> static inline void toStream(StreamT& out){ out<<'1'; }
  };
  struct BitX {
    enum { ONE=0, ZERO=0, UNDEF=1 };
    typedef BitX Neg;
    template<typename StreamT> static inline void toStream(StreamT& out){ out<<'X'; }
  };


  // prototype de la fonction de type, recursive, permettant de définir le reversement d'un empilement de bits
  // Exemple: 0X1X -> X1X0
  template<typename Bit, typename RevHead, typename Tail> struct CBitFieldInverter;

  // prototype de l'opérateur permettant d'appliquer un sous opérateur sur l'ensemble des empilements constant de bits
  // dans lesquels chaque valeur non-définie (BitX) prend toutes les valeurs possibles (Bit0 et Bit1).
  // Exemple: 0X1X -> 0010, 0011, 0110, 0111
  template<typename ProcObj, typename RevHead, typename List> struct CBitFieldEnumerator;

  // description d'un empilement constant de bits (l'information est entierement contenue dans le type)
  struct NullBitField
  {
    enum {
      N_ONES  = 0,
      N_ZEROS = 0,
      N_FREE  = 0,
      N_BITS  = 0,
      BITFIELD = 0,
      UNDEF_BITFIELD = 0,
      DEF_BITFIELD = 0
   };
    
    typedef NullBitField Negate;
    typedef NullBitField Reverse;

    template<typename StreamT> static inline void toStream(StreamT& out) {}
    template<typename ProcObj> static inline void enumerate(ProcObj& proc) {}
  };

  template<typename _Bit, typename _Tail=NullBitField> struct CBitField
  {
    typedef _Bit Bit;
    typedef _Tail Tail;
    enum {
      N_ONES  = Tail::N_ONES  + Bit::ONE  ,
      N_ZEROS = Tail::N_ZEROS + Bit::ZERO ,
      N_FREE  = Tail::N_FREE  + Bit::UNDEF,
      N_BITS  = Tail::N_BITS  + 1,
      BITFIELD = (Tail::N_FREE==0) ? ( (Bit::ONE << Tail::N_BITS) | Tail::BITFIELD ) : 0,
      UNDEF_BITFIELD = (Bit::UNDEF << Tail::N_BITS) | Tail::UNDEF_BITFIELD,
      DEF_BITFIELD = ((1-Bit::UNDEF) << Tail::N_BITS) | Tail::DEF_BITFIELD 
    };
    typedef typename CBitFieldInverter<Bit,NullBitField,Tail>::Reverse Reverse;
    typedef CBitField<typename Bit::Neg,typename Tail::Negate> Negate;
    template<typename StreamT> static inline void toStream(StreamT& out)
    {
      Bit::toStream(out);
      Tail::toStream(out);
    }
    template<typename ProcObj> static inline void enumerate(ProcObj& proc)
    {
      CBitFieldEnumerator<ProcObj,NullBitField,CBitField>::enumerate( proc );
    }
  }; // CBitField


  // definition concrete de la fonction de type, recursive, permettant d'inverser un empilement de bits
  template<typename Bit, typename RevHead> struct CBitFieldInverter<Bit,RevHead,NullBitField>
  {
    typedef CBitField<Bit,RevHead> Reverse;
  };
  template<typename Bit, typename RevHead, typename Tail> struct CBitFieldInverter
  {
    typedef CBitFieldInverter< typename Tail::Bit, CBitField<Bit,RevHead>, typename Tail::Tail > TailInverter;
    typedef typename TailInverter::Reverse Reverse;
  };


  // l'opération d'enumeration renverse l'empilement de bits par construction
  // cette opération utilise a la fois une fonction de type une fonction reelle.
  template<typename ProcObj,typename RevHead> struct CBitFieldEnumerator<ProcObj,RevHead,NullBitField>
  {
    typedef typename RevHead::Reverse List;
    static inline void enumerate(ProcObj& proc)
    {
      proc.process( List () );
    }
  }; // CBitFieldEnumerator
  template<typename ProcObj, typename RevHead, typename List> struct CBitFieldEnumerator
  {
    typedef typename List::Bit Bit;
    typedef typename List::Tail Tail;
    static inline void enumerate(ProcObj& proc)
    {
      if( Bit::UNDEF )
	{
	  CBitFieldEnumerator<ProcObj,CBitField<Bit0,RevHead>,Tail>::enumerate( proc );
	  CBitFieldEnumerator<ProcObj,CBitField<Bit1,RevHead>,Tail>::enumerate( proc );
	}
      else
	{
	  CBitFieldEnumerator<ProcObj,CBitField<Bit,RevHead>,Tail>::enumerate( proc );
	}
    }
  }; // CBitFieldEnumerator



}; // namespace AmrReconstruction3

#endif
