#include "ConstBits.h"

#include <iostream>
using namespace std;

using namespace hct;

struct PrintCBitField
{
  inline PrintCBitField(ostream& o) : out(o) {}
  template<typename BF> inline void process ( BF )
  {
    BF::toStream(out);
    out<<' ';
  }
  ostream& out;
};

typedef CBitField<BitX, CBitField<Bit1, CBitField<BitX, CBitField<Bit0> > > > X1X0;
typedef CBitField<Bit1, CBitField<Bit0, CBitField<Bit1, CBitField<Bit1> > > > _1011;

int main()
{
  PrintCBitField printer(cout);

  cout<<"X1X0:"<<endl;
  cout<<"self = "; X1X0::toStream( cout ); cout<<endl;
  cout<<"negate = "; X1X0::Negate::toStream( cout ); cout<<endl;
  cout<<"reverse = "; X1X0::Reverse::toStream( cout ); cout<<endl;
  cout<<"enumerate = "; X1X0::enumerate( printer ); cout<<endl;
  cout<<"reverse.enumerate = "; X1X0::Reverse::enumerate( printer ); cout<<endl;

  cout<<"X1X0 bitfield = "<<X1X0::BITFIELD<<endl;
  cout<<"1011 bitfield = "<<_1011::BITFIELD<<endl;

  return 0;
}
