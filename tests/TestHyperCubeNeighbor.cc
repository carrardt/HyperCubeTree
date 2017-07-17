#include "HyperCubeNeighbor.h"
#include "HyperCube.h"
#include "Vec.h"

#include <iostream>
#include <sstream>
#include <string>
using namespace std;
using namespace hct;

template <unsigned int D> struct NbhTest;
template <> struct NbhTest<0>
{
  static inline void getUIVec(Vec<unsigned int,0>)
  {
	  char tmp[255];
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
    inline void operator () (const HyperCube<NbhNodeInfo,0,M1>& parent, HyperCube<NbhNodeInfo,0,M2>& child, const Vec<unsigned int,D> grid,const Vec<unsigned int,D>, Vec<unsigned int,D> coord)
  {
    // calculer les flags left et right
    cout<<"child/"; HyperCube<NbhNodeInfo,0,M2>::Mask::toStream(cout); cout<<" -> ";
    cout<<"parent/"; HyperCube<NbhNodeInfo,0,M1>::Mask::toStream(cout); cout<<" @ ("; coord.toStream(cout); cout<<")\n";
  }
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
	  HyperCube<NbhNodeInfo,D> root;
	  HyperCube<NbhNodeInfo,D> inner;

    Vec<unsigned int,D> grid;
    Vec<unsigned int,D> coord;

    root.self().nodeId = 0; // seule le centre n'est pas une feuille

    cout<<"Grid ("<<D<<" value(s)) ? "; cout.flush();
    getUIVec(grid);
    cout<<"Position ? "; cout.flush();
    getUIVec(coord);
    cout<<"grid=("; grid.toStream(cout); cout<<"), coord=("; coord.toStream(cout); cout<<')'<<endl;

    TestObj proc;
	HyperCubeNeighbor<NbhNodeInfo,D>::dig(grid,root,inner,coord, proc);
  }

};

int main()
{
  unsigned int dim;

  cout<<"Dimension ? ";cout.flush();
  cin>>dim;

  switch(dim)
    {
    case 1: NbhTest<1>::test(); break;
    case 2: NbhTest<2>::test(); break;
    case 3: NbhTest<3>::test(); break;
    case 4: NbhTest<4>::test(); break;
    }

  return 0;
}
