#pragma once

namespace hct
{

	// Description des valeurs possibles d'un bit (l'information est entierement contenue dans le type)
	struct Bit1;
	struct Bit0
	{
		static constexpr size_t ONE = 0;
		static constexpr size_t ZERO = 1;
		static constexpr size_t UNDEF = 0;
		using Neg = Bit1;
		template<typename StreamT> static inline void toStream(StreamT& out) { out << '0'; }
	};

	struct Bit1
	{
		static constexpr size_t ONE = 1;
		static constexpr size_t ZERO = 0;
		static constexpr size_t UNDEF = 0;
		using Neg = Bit0;
		template<typename StreamT> static inline void toStream(StreamT& out) { out << '1'; }
	};

	struct BitX {
		static constexpr size_t ONE = 0;
		static constexpr size_t ZERO = 0;
		static constexpr size_t UNDEF = 1 ;
		using Neg = BitX;
		template<typename StreamT> static inline void toStream(StreamT& out) { out << 'X'; }
	};

	// prototype de la fonction de type, recursive, permettant de définir le reversement d'un empilement de bits
	// Exemple: 0X1X -> X1X0
	template<typename Bit, typename RevHead, typename Tail> struct CBitFieldInverter;

	// prototype de l'opérateur permettant d'appliquer un sous opérateur sur l'ensemble des empilements constant de bits
	// dans lesquels chaque valeur non-définie (BitX) prend toutes les valeurs possibles (Bit0 et Bit1).
	// Exemple: 0X1X -> 0010, 0011, 0110, 0111
	template<typename FuncT, typename RevHead, typename List> struct CBitFieldEnumerator;

	// description d'un empilement constant de bits (l'information est entierement contenue dans le type)
	struct NullBitField
	{
		static constexpr size_t N_ONES = 0;
		static constexpr size_t N_ZEROS = 0;
		static constexpr size_t N_FREE = 0;
		static constexpr size_t N_BITS = 0;
		static constexpr size_t BITFIELD = 0;
		static constexpr size_t UNDEF_BITFIELD = 0;
		static constexpr size_t DEF_BITFIELD = 0;

		using Negate = NullBitField;
		using Reverse = NullBitField;

		template<typename StreamT> static inline void toStream(StreamT& out) {}
		template<typename FuncT> static inline void enumerate(FuncT) {}
	};

	template<typename _Bit, typename _Tail = NullBitField> struct CBitField
	{
		using Bit = _Bit;
		using Tail = _Tail;

		static constexpr size_t N_ONES = Tail::N_ONES + Bit::ONE;
		static constexpr size_t N_ZEROS = Tail::N_ZEROS + Bit::ZERO;
		static constexpr size_t N_FREE = Tail::N_FREE + Bit::UNDEF;
		static constexpr size_t N_BITS = Tail::N_BITS + 1;
		static constexpr size_t BITFIELD = (Tail::N_FREE == 0) ? ((Bit::ONE << Tail::N_BITS) | Tail::BITFIELD) : 0;
		static constexpr size_t UNDEF_BITFIELD = (Bit::UNDEF << Tail::N_BITS) | Tail::UNDEF_BITFIELD;
		static constexpr size_t DEF_BITFIELD = ((1 - Bit::UNDEF) << Tail::N_BITS) | Tail::DEF_BITFIELD;

		using Reverse = typename CBitFieldInverter<Bit, NullBitField, Tail>::Reverse ;
		using Negate = CBitField<typename Bit::Neg, typename Tail::Negate>;

		template<typename StreamT>
		static inline void toStream(StreamT& out)
		{
			Tail::toStream(out);
			Bit::toStream(out);
		}

		template<typename FuncT>
		static inline void enumerate(FuncT f)
		{
			CBitFieldEnumerator<FuncT, NullBitField, CBitField>::enumerate(f);
		}
	}; // CBitField


	// definition concrete de la fonction de type, recursive, permettant d'inverser un empilement de bits
	template<typename Bit, typename RevHead> struct CBitFieldInverter<Bit, RevHead, NullBitField>
	{
		using Reverse = CBitField<Bit, RevHead>;
	};
	template<typename Bit, typename RevHead, typename Tail> struct CBitFieldInverter
	{
		using TailInverter = CBitFieldInverter< typename Tail::Bit, CBitField<Bit, RevHead>, typename Tail::Tail > ;
		using Reverse = typename TailInverter::Reverse;
	};


	// l'opération d'enumeration renverse l'empilement de bits par construction
	// cette opération utilise a la fois une fonction de type une fonction reelle.
	template<typename FuncT, typename RevHead> struct CBitFieldEnumerator<FuncT, RevHead, NullBitField>
	{
		using List = typename RevHead::Reverse;
		static inline void enumerate(FuncT f)
		{
			f(List());
		}
	}; // CBitFieldEnumerator

	template<typename FuncT, typename RevHead, typename List> struct CBitFieldEnumerator
	{
		using Bit = typename List::Bit;
		using Tail = typename List::Tail;
		static inline void enumerate(FuncT f)
		{
			if (Bit::UNDEF)
			{
				CBitFieldEnumerator<FuncT, CBitField<Bit0, RevHead>, Tail>::enumerate(f);
				CBitFieldEnumerator<FuncT, CBitField<Bit1, RevHead>, Tail>::enumerate(f);
			}
			else
			{
				CBitFieldEnumerator<FuncT, CBitField<Bit, RevHead>, Tail>::enumerate(f);
			}
		}
	}; // CBitFieldEnumerator



}; // namespace hct

