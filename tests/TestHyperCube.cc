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

	{
		cout << "test bitfield [] operator :\n";
		HyperCube<int64_t, 3> hc3(-1);
		int i = 0;
		hc3.forEachValue([&i](int64_t& c) { c = i++; });
		hc3.forEachVertex([](int64_t i, int64_t& c) { c = -i; });
#		define TEST_OPERATOR(c,b,a) \
		using _##a##b##c = CBitField< Bit##a, CBitField< Bit##b, CBitField< Bit##c, NullBitField > > >; \
		cout << "Bitfield = "; _##a##b##c::toStream(cout); cout<<" / "<<_##a##b##c::BITFIELD << " => " << hc3[_##a##b##c()] << endl
		TEST_OPERATOR(0, 0, 0);
		TEST_OPERATOR(0, 0, 1);
		TEST_OPERATOR(0, 0, X);
		TEST_OPERATOR(0, 1, 0);
		TEST_OPERATOR(0, 1, 1);
		TEST_OPERATOR(0, 1, X);
		TEST_OPERATOR(0, X, 0);
		TEST_OPERATOR(0, X, 1);
		TEST_OPERATOR(0, X, X);
		TEST_OPERATOR(1, 0, 0);
		TEST_OPERATOR(1, 0, 1);
		TEST_OPERATOR(1, 0, X);
		TEST_OPERATOR(1, 1, 0);
		TEST_OPERATOR(1, 1, 1);
		TEST_OPERATOR(1, 1, X);
		TEST_OPERATOR(1, X, 0);
		TEST_OPERATOR(1, X, 1);
		TEST_OPERATOR(1, X, X);
		TEST_OPERATOR(X, 0, 0);
		TEST_OPERATOR(X, 0, 1);
		TEST_OPERATOR(X, 0, X);
		TEST_OPERATOR(X, 1, 0);
		TEST_OPERATOR(X, 1, 1);
		TEST_OPERATOR(X, 1, X);
		TEST_OPERATOR(X, X, 0);
		TEST_OPERATOR(X, X, 1);
		TEST_OPERATOR(X, X, X);
#		undef TEST_OPERATOR
	}

	return 0;
}

