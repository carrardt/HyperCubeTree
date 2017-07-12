#include "TreeLevelStorage.h"

#include <iostream>

int main()
{
	hct::TreeLevelStorage store;

	std::cout << "Initital store state :" << std::endl;
	store.toStream(std::cout);

	store.setNumberOfLevels(3);

	std::cout << "Store after setNumberOfLevels :" << std::endl;
	store.toStream(std::cout);

	hct::TreeLevelArray<size_t> cellIndices;
	store.addArray(&cellIndices);

	std::cout << "cellIndices before resize :" << std::endl;
	cellIndices.toStream(std::cout);

	store.resize(0,10);
	store.resize(1,100);
	store.resize(2,1000);

	std::cout << "cellIndices after resize :" << std::endl;
	cellIndices.toStream(std::cout);

	hct::TreeLevelArray<double> cellValues;
	store.addArray(&cellValues);

	std::cout << "cellValues after addArray :" << std::endl;
	cellValues.toStream(std::cout);

	return 0;
}
