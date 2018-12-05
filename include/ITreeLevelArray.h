#pragma once

#include "HyperCubeTreeCell.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace hct
{

	class ITreeLevelArray
	{
		public:
			/*virtual  ~ITreeLevelArray()
			{
				std::cout << "ITreeLevelArray being deleted\n";
			}*/
			virtual std::string name() const = 0;
			virtual void setNumberOfLevels(size_t nLevels) =0;
			virtual size_t numberOfLevels() const =0;
			virtual size_t size(size_t level) const = 0;
			virtual void resize(size_t level, size_t nElems) =0;
			virtual void erase(size_t level, size_t position, size_t nElems) =0;
			virtual size_t numberOfComponents() const = 0;
			virtual std::ostream& printCell(std::ostream&, HyperCubeTreeCell cell) const =0;
			virtual inline std::ostream& print(std::ostream& out) 
			{
				out << "Number of levels : " << numberOfLevels() << '\n';
				for (size_t i = 0; i < numberOfLevels(); i++)
				{
					out << "\tLevel " << i << " : size = " << size(i) << '\n';
				}
				return out;
			}

	};

}


