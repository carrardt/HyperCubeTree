#include "HyperCube.h"

#include <iostream>
#include <cstdint>
using namespace std;
using namespace hct;

struct PrintCBitField
{
	inline PrintCBitField(ostream& o) : out(o) {}
	template<typename BF> inline void operator () (BF)
	{
		BF::toStream(out);
		out << ' ';
	}
	ostream& out;
};

struct PrintComponent
{
	ostream& out;
	inline PrintComponent(const PrintComponent& pc) : out(pc.out) {}
	inline PrintComponent(ostream& o) : out(o) {}
	template<typename T, typename Mask> inline void operator() (const HyperCube<T, 0, Mask>& component)
	{
		Mask::toStream(out);
		out << " : value " << component.value;
		out << " : " << (1 << Mask::N_FREE) << " point(s) : ";
		PrintCBitField printer(out);
		Mask::enumerate(printer);
		out << endl;
	}
};

int main()
{
	PrintComponent printComp(cout);

	{
		HyperCube<char, 2> ce2('a');
		ce2.self() = 'A';
		cout << "\n2-Cube enumeration (size=" << sizeof(ce2) << ")" << endl;
		ce2.forEachComponent(printComp);
	}

	{
		HyperCube<char, 3> ce3('b');
		ce3.self() = 'B';
		cout << "\n3-Cube enumeration (size=" << sizeof(ce3) << ")" << endl;
		ce3.forEachComponent(printComp);
	}

	{
		HyperCube<char, 4> ce4('c');
		ce4.self() = 'C';
		cout << "\n4-Cube enumeration (size=" << sizeof(ce4) << ")" << endl;
		ce4.forEachComponent(printComp);
	}

	{
		HyperCube<int64_t, 3> hc3(-1);
		hc3.forEachVertex([](size_t i, int64_t& c) { c=i; });
		cout << "test forEachValue on a 3-cube:\n";
		hc3.forEachValue([](int64_t c) { cout<<"value "<<c<<'\n'; });
		cout << "test forEachVertex on a 3-cube:\n";
		hc3.forEachVertex([](size_t i, int64_t c) { cout << "vertex " << i << " = " << c << '\n'; });
	}

	return 0;
}

