#ifndef __NBH_H
#define __NBH_H

#include "HyperCube.h"

namespace AmrReconstruction3
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

}; // namespace AmrReconstruction3

#endif
/* ===================================================================================================================*/


#ifdef _TEST_Nbh
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

using namespace AmrReconstruction3;

char tmp[255];

template <unsigned int D> struct NbhTest;
template <> struct NbhTest<0>
{
  static inline void getUIVec(Vec<unsigned int,0>)
  {
    cin.getline(tmp,255,'\n');
  }
};

struct NbhNodeInfo
{
  int nodeId;
  unsigned int left; // masque qui indique si on est au plancher selon tel ou tel axe.
  unsigned int right; // masque qui indique si on est au max selon tel ou tel axe.
  inline NbhNodeInfo() : nodeId(-1), left(0), right(0) {}
  inline bool isLeaf() { return nodeId==-1; }
};

struct TestObj
{
  template<unsigned int D, typename M1, typename M2>
    inline void operator () (const CubeEnum<NbhNodeInfo,0,M1>& parent, CubeEnum<NbhNodeInfo,0,M2>& child, const Vec<unsigned int,D>& grid,const Vec<unsigned int,D>&, Vec<unsigned int,D> coord)
  {
    // calculer les flags left et right
    cout<<"child/"; CubeEnum<NbhNodeInfo,0,M2>::Mask::toStream(cout); cout<<" -> ";
    cout<<"parent/"; CubeEnum<NbhNodeInfo,0,M1>::Mask::toStream(cout); cout<<" @ ("; coord.toStream(cout); cout<<")\n";
  }
  int _;
};

template <unsigned int D> struct NbhTest
{
  static inline void getUIVec( Vec<unsigned int,D>& v )
  {
    cin>>v.val;
    NbhTest<D-1>::getUIVec(v);
  }

  static inline void test()
  {
    CubeEnum<NbhNodeInfo,D> root;
    CubeEnum<NbhNodeInfo,D> inner;

    Vec<unsigned int,D> grid;
    Vec<unsigned int,D> coord;

    root.self().nodeId = 0; // seule le centre n'est pas une feuille

    cout<<"Grid ("<<D<<" value(s)) ? "; cout.flush();
    getUIVec(grid);
    cout<<"Position ? "; cout.flush();
    getUIVec(coord);
    cout<<"grid=("; grid.toStream(cout); cout<<"), coord=("; coord.toStream(cout); cout<<')'<<endl;

    TestObj proc;
    Nbh<NbhNodeInfo,D>::dig(grid,proc,root,inner,coord);
  }

};

int main(int argc, char* argv[])
{
  unsigned int dim;

  cout<<"Dimension ? ";cout.flush();
  cin>>dim;
  cin.getline(tmp,255,'\n');

  switch(dim)
    {
    case 1: NbhTest<1>::test(); break;
    case 2: NbhTest<2>::test(); break;
    case 3: NbhTest<3>::test(); break;
      //case 4: NbhTest<4>::test(); break;
    }

  return 0;
}

#endif
