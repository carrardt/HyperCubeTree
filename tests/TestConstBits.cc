#include "ConstBits.h"
#include "Vec.h"

#include <iostream>
using namespace std;

using namespace hct;

struct PrintCBitField
{
  inline PrintCBitField(ostream& o) : out(o) {}
  template<typename BF> inline void operator () ( BF )
  {
	out << ' ';
    BF::toStream(out);
  }
  ostream& out;
};

template<typename BF>
static inline void testCBitField(BF)
{
	PrintCBitField printer(cout);
	cout << "self = "; BF::toStream(cout); cout << endl;
	cout << "negate = "; BF::Negate::toStream(cout); cout << endl;
	cout << "reverse = "; BF::Reverse::toStream(cout); cout << endl;
	cout << "enumerate ="; BF::enumerate(printer); cout << endl;
	cout << "reverse.enumerate ="; BF::Reverse::enumerate(printer); cout << endl;
	cout << "negate.enumerate ="; BF::Negate::enumerate(printer); cout << endl;
	cout << "bitfield = " << BF::BITFIELD << ", Vec="; bitfield_vec<BF::N_BITS>(BF::BITFIELD).toStream(cout); cout << endl;
	cout << "undef bitfield = " << BF::UNDEF_BITFIELD << ", Vec="; bitfield_vec<BF::N_BITS>(BF::UNDEF_BITFIELD).toStream(cout); cout << endl;
	cout << "def bitfield = " << BF::DEF_BITFIELD << ", Vec="; bitfield_vec<BF::N_BITS>(BF::DEF_BITFIELD).toStream(cout); cout << endl << endl;
}

typedef CBitField<BitX, CBitField<Bit1, CBitField<BitX, CBitField<Bit0> > > > _X1X0;
typedef CBitField<Bit1, CBitField<Bit0, CBitField<Bit1, CBitField<Bit1> > > > _1011;
typedef CBitField<Bit0, CBitField<Bit0, CBitField<Bit0, CBitField<Bit0> > > > _0000;
typedef CBitField<Bit1, CBitField<Bit1, CBitField<Bit1, CBitField<Bit1> > > > _1111;
typedef CBitField<Bit0, CBitField<BitX, CBitField<Bit1, CBitField<Bit0> > > > _0X10;
typedef CBitField<Bit0, CBitField<Bit1, CBitField<BitX, CBitField<Bit0> > > > _01X0;

int main()
{
	testCBitField(_X1X0());
	testCBitField(_1011());
	testCBitField(_0000());
	testCBitField(_1111());
	testCBitField(_0X10());
	testCBitField(_01X0());

  return 0;
}
