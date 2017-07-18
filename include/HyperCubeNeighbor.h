#pragma once

#include "HyperCube.h"
#include "GridDimension.h"

#include <type_traits>
#include <assert.h>

namespace hct
{
	template<typename T, unsigned int DecD, unsigned int IncD = 0> struct HyperCubeNeighbor;

	// Arret de la récursivité, on appel la procédure passée en paramètre
	template<typename T, unsigned int Dim>
	struct HyperCubeNeighbor<T, 0, Dim>
	{
		template<typename FuncT, typename M1, typename M2>
		static inline void rdig(
			GridDimension<Dim> grid,		// parent's subdivision grid dimensions
			const HyperCube<T, 0, M1>& parent,		// parent's neighbor containing child's neighbor
			HyperCube<T, 0, M2> &child,				// child component involved
			Vec<unsigned int, Dim> inCoord,	// parent subdivision grid coordinates where we digged
			Vec<unsigned int, Dim> outCoord,		// parent's neighbor subdivision grid coordinates where to find child's neighbor
			FuncT f )								// operator to apply
		{
			// seul outCoord est genere a l'envers, d'ou le .reverse()
			f(parent, child, grid, inCoord, outCoord.reverse());
		}
	};

	template<typename T, unsigned int DecD, unsigned int IncD>
	struct HyperCubeNeighbor
	{
		static constexpr unsigned int Dim = DecD + IncD;
		using Next = HyperCubeNeighbor<T, DecD - 1, IncD + 1>;

		template<typename FuncT, typename M1, typename M2>
		static inline void
			rdig(
				GridDimension<Dim> grid,
				const HyperCube<T, DecD, M1> & parent,
				HyperCube<T, DecD, M2> & result,
				Vec<unsigned int, Dim> inCoord,
				Vec<unsigned int, IncD> outCoord,
				FuncT f )
		{
			// !!! outCoord est généré en inversé

			// Nous gardons grid dans sa totalité, car nous en auront besoin à la fin pour calculer l'index du noeud enfant.
			// C'est pourquoi on le cast, pour faire comme si nous avions pris en paramètre de type Vec<DecD> et non Vec<Dim>
			int gridVal = grid.Vec<unsigned int, DecD>::val;
			// idem pour inCoord
			int inCoordVal = inCoord.Vec<unsigned int, DecD>::val;

			// voisinage gauche : ensemble des voisins dont le Dième bit des numéros des points partagés ne peut être que 0
			if (inCoordVal == 0)
			{
				Next::rdig(grid, parent._0, result._0, inCoord, Vec<unsigned int, IncD + 1>(gridVal - 1, outCoord), f);
			}
			else
			{
				Next::rdig(grid, parent._X, result._0, inCoord, Vec<unsigned int, IncD + 1>(inCoordVal - 1, outCoord), f);
			}

			// voisinage central : ensemble des voisins dont le Dième bit des numéros des points partagés peut être 0 ou 1
			{
				Next::rdig(grid, parent._X, result._X, inCoord, Vec<unsigned int, IncD + 1>(inCoordVal, outCoord), f);
			}

			// voisinage droite : ensemble des voisins dont le Dième bit des numéros des points partagés ne peut être que 1
			if (inCoordVal == (gridVal - 1))
			{
				Next::rdig(grid, parent._1, result._1, inCoord, Vec<unsigned int, IncD + 1>(0, outCoord), f);
			}
			else
			{
				Next::rdig(grid, parent._X, result._1, inCoord, Vec<unsigned int, IncD + 1>(inCoordVal + 1, outCoord), f);
			}
		}

		/* méthode frontale */
		template<typename FuncT>
		static inline void dig(
			GridDimension<Dim> grid,	// subdivision grid dimensions of parent
			const HyperCube<T, Dim>& parent,	// parent hypercube
			HyperCube<T, Dim> &result,			// destination hypercube
			Vec<unsigned int, Dim> inCoord,		// dig coordinate, inside the subdivision grid
			FuncT f )							// operator to apply
		{
			rdig(grid, parent, result, inCoord, Vec<unsigned int, 0>(), f);
		}
	};


// ==================================== new version ======================================

	template<typename T, unsigned int DecD, unsigned int IncD = 0, typename ParentMask = NullBitField, typename ChildMask = NullBitField> struct HyperCubeNeighbor2;

	// Arret de la récursivité, on appel la procédure passée en paramètre
	template<typename T, unsigned int Dim, typename ParentMask, typename ChildMask>
	struct HyperCubeNeighbor2<T, 0, Dim, ParentMask, ChildMask>
	{
		template<typename FuncT>
		static inline void rdig(
			GridDimension<Dim> grid,		// parent's subdivision grid dimensions
			const HyperCube<T,Dim>& parent,		// parent's neighbor containing child's neighbor
			HyperCube<T,Dim> &child,				// child component involved
			Vec<unsigned int, Dim> inCoord,	// parent subdivision grid coordinates where we digged
			Vec<unsigned int, Dim> outCoord,		// parent's neighbor subdivision grid coordinates where to find child's neighbor
			FuncT f)								// operator to apply
		{
			// seul outCoord est genere a l'envers, d'ou le .reverse()
			f(parent, child, grid, inCoord, outCoord.reverse(), ParentMask::Reverse(), ChildMask::Reverse() );
		}
	};

	template<typename T, unsigned int DecD, unsigned int IncD, typename ParentMask, typename ChildMask>
	struct HyperCubeNeighbor2
	{
		static constexpr unsigned int Dim = DecD + IncD;
		template<typename FuncT>
		static inline void
			rdig(
				GridDimension<Dim> grid,
				const HyperCube<T,Dim> & parent,
				HyperCube<T,Dim> & result,
				Vec<unsigned int, Dim> inCoord,
				Vec<unsigned int, IncD> outCoord,
				FuncT f)
		{
			// !!! outCoord est généré en inversé

			// Nous gardons grid dans sa totalité, car nous en auront besoin à la fin pour calculer l'index du noeud enfant.
			// C'est pourquoi on le cast, pour faire comme si nous avions pris en paramètre de type Vec<DecD> et non Vec<Dim>
			int gridVal = grid.Vec<unsigned int, DecD>::val;
			// idem pour inCoord
			int inCoordVal = inCoord.Vec<unsigned int, DecD>::val;

			// voisinage gauche : ensemble des voisins dont le Dième bit des numéros des points partagés ne peut être que 0
			if (inCoordVal == 0)
			{
				HyperCubeNeighbor2<T, DecD - 1, IncD + 1, CBitField<Bit0,ParentMask> , CBitField<Bit0, ChildMask> >
					::rdig(grid, parent, result, inCoord, Vec<unsigned int, IncD + 1>(gridVal - 1, outCoord), f);
			}
			else
			{
				HyperCubeNeighbor2<T, DecD - 1, IncD + 1, CBitField<BitX, ParentMask>, CBitField<Bit0, ChildMask> >
					::rdig(grid, parent, result, inCoord, Vec<unsigned int, IncD + 1>(inCoordVal - 1, outCoord), f);
			}

			// voisinage central : ensemble des voisins dont le Dième bit des numéros des points partagés peut être 0 ou 1
			{
				HyperCubeNeighbor2<T, DecD - 1, IncD + 1, CBitField<BitX, ParentMask>, CBitField<BitX, ChildMask> >
					::rdig(grid, parent, result, inCoord, Vec<unsigned int, IncD + 1>(inCoordVal, outCoord), f);
			}

			// voisinage droite : ensemble des voisins dont le Dième bit des numéros des points partagés ne peut être que 1
			if (inCoordVal == (gridVal - 1))
			{
				HyperCubeNeighbor2<T, DecD - 1, IncD + 1, CBitField<Bit1, ParentMask>, CBitField<Bit1, ChildMask> >
					::rdig(grid, parent, result, inCoord, Vec<unsigned int, IncD + 1>(0, outCoord), f);
			}
			else
			{
				HyperCubeNeighbor2<T, DecD - 1, IncD + 1, CBitField<BitX, ParentMask>, CBitField<Bit1, ChildMask> >
					::rdig(grid, parent, result, inCoord, Vec<unsigned int, IncD + 1>(inCoordVal + 1, outCoord), f);
			}
		}

		/* méthode frontale */
		template<typename FuncT>
		static inline void dig(
			GridDimension<Dim> grid,	// subdivision grid dimensions of parent
			const HyperCube<T, Dim>& parent,	// parent hypercube
			HyperCube<T, Dim> &result,			// destination hypercube
			Vec<unsigned int, Dim> inCoord,		// dig coordinate, inside the subdivision grid
			FuncT f)							// operator to apply
		{
			rdig(grid, parent, result, inCoord, Vec<unsigned int, 0>(), f);
		}
	};


}; // namespace hct
