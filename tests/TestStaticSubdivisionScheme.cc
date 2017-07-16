#include "StaticSubdivisionScheme.h"

#include <iostream>

template <typename T, unsigned int D>
inline std::ostream & operator << (std::ostream& out, const hct::Vec<T, D>& v)
{
	v.toStream(out);
	return out;
}

int main()
{
	using Grid0 = hct::StaticGridDim<1, 2, 3>;
	using Grid1 = hct::StaticGridDim<4, 5, 6>;
	using Grid2 = hct::StaticGridDim<7, 8, 9>;
	using Grid3 = hct::StaticGridDim<10, 11>;

	std::cout << "Grid0 = " << Grid0::value() << std::endl;
	std::cout << "Grid1 = " << Grid1::value() << std::endl;
	std::cout << "Grid2 = " << Grid2::value() << std::endl;
	std::cout << "Grid3 = " << Grid3::value() << std::endl;

	std::cout << "Set(Grid0) valid = "<< hct::StaticGridDimSet< Grid0 >::is_valid << std::endl;
	std::cout << "Set(Grid0,Grid1) valid = " << hct::StaticGridDimSet< Grid0,Grid1 >::is_valid << std::endl;
	std::cout << "Set(Grid0,Grid1,Grid2) valid = " << hct::StaticGridDimSet< Grid0, Grid1, Grid2 >::is_valid << std::endl;
	std::cout << "Set(Grid0,Grid1,Grid2,Grid3) valid = " << hct::StaticGridDimSet< Grid0, Grid1, Grid2, Grid3 >::is_valid << std::endl;
	std::cout << "Set(Grid3,Grid0) valid = " << hct::StaticGridDimSet< Grid3, Grid0 >::is_valid << std::endl;

	using InitialSubdiv = hct::StaticSubdivisionScheme< 1, 4, 4, 20 >;
	using Subdiv = hct::StaticSubdivisionScheme< 4, 3, 3, 3 >;
	InitialSubdiv init_subdiv;
	init_subdiv.toStream(std::cout);
	Subdiv subdiv;
	subdiv.toStream(std::cout);

	hct::StaticSubdivisionSchemeCombo< InitialSubdiv, Subdiv > finalSubdivCheme;
	finalSubdivCheme.toStream(std::cout);
}
