#ifndef __NBH_H
#define __NBH_H

#include "HyperCube.h"

namespace hct
{
   template<typename T, unsigned int DecD, unsigned int IncD=0> struct Nbh;

   // Arret de la récursivité, on appel la procédure passée en paramètre
   template<typename T,unsigned int Dim>
   struct Nbh<T,0,Dim>
   {
     template<typename ProcObj, typename M1, typename M2> 
       static inline void rdig( const Vec<unsigned int,Dim>& grid,
				ProcObj& proc,
				const CubeEnum<T,0,M1>& parent,
				CubeEnum<T,0,M2> &child,
				const Vec<unsigned int,Dim>& inCoord,
				Vec<unsigned int,Dim> outCoord )
     {
       // seul outCoord est genere a l'envers, d'ou le .reverse()
       proc( parent, child, grid, inCoord, outCoord.reverse() );
     }
   };

   template<typename T, unsigned int DecD, unsigned int IncD>
   struct Nbh
   {
     enum {
	Dim = DecD+IncD
     };

     typedef Nbh<T,DecD-1,IncD+1> Next;

     template<typename Func, typename M1, typename M2>
     static inline void
     rdig(
	const Vec<unsigned int,Dim>& grid,
	Func& func,
	const CubeEnum<T,DecD,M1> & parent,
	CubeEnum<T,DecD,M2> & result,
	const Vec<unsigned int,Dim> & inCoord,
	Vec<unsigned int,IncD> outCoord )
     {
	// !!! outCoord est généré en inversé

	// Nous gardons grid dans sa totalité, car nous en auront besoin à la fin pour calculer l'index du noeud enfant.
	// C'est pourquoi on le cast, pour faire comme si nous avions pris en paramètre de type Vec<DecD> et non Vec<Dim>
	int gridVal = grid.Vec<unsigned int,DecD>::val;
	// idem pour inCoord
	int inCoordVal = inCoord.Vec<unsigned int,DecD>::val;

	// voisinage gauche : ensemble des voisins dont le Dième bit des numéros des points partagés ne peut être que 0
	if( inCoordVal == 0 )
	{
	   Next::rdig(grid,func,parent._0,result._0,inCoord, Vec<unsigned int,IncD+1>(gridVal-1   ,outCoord));
	}
	else
	{
	   Next::rdig(grid,func,parent._X,result._0,inCoord, Vec<unsigned int,IncD+1>(inCoordVal-1,outCoord));
	}

	// voisinage central : ensemble des voisins dont le Dième bit des numéros des points partagés peut être 0 ou 1
	{
	   Next::rdig(grid,func,parent._X,result._X,inCoord, Vec<unsigned int,IncD+1>(inCoordVal  ,outCoord));
	}

	// voisinage droite : ensemble des voisins dont le Dième bit des numéros des points partagés ne peut être que 1
	if( inCoordVal == (gridVal-1) )
	{
	   Next::rdig(grid,func,parent._1,result._1,inCoord, Vec<unsigned int,IncD+1>(0           ,outCoord));
	}
	else
	{
	   Next::rdig(grid,func,parent._X,result._1,inCoord, Vec<unsigned int,IncD+1>(inCoordVal+1,outCoord));
	}
     }

     /* méthode frontale */
     template<typename Func>
     static inline void dig(
			    const Vec<unsigned int,Dim>& grid,
			    Func& func,
			    const CubeEnum<T,Dim,NullBitField>& parent,
			    CubeEnum<T,Dim,NullBitField> &result,
			    Vec<unsigned int,Dim> inCoord )
     {
       rdig(grid,func,parent,result,inCoord,Vec<unsigned int,0>());
     }
   };

}; // namespace hct

#endif
