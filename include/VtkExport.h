#pragma once

#include "HyperCubeTreeLocatedCursor.h"

namespace hct
{
	namespace vtk
	{
		template<typename Tree, typename StreamT>
		static inline void exportHyperCubeTreeToVtk(const Tree& tree, StreamT out)
		{
			out << "# vtk DataFile Version 2.0\n";
			out << "Exported from an HyperCubeTree object\n";
			out << "ASCII\n";
		}
	}
}

