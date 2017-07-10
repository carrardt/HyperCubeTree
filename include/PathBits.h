#ifndef __PATH_BITS_H
#define __PATH_BITS_H

#include "Vec.h"

namespace hct
{
  // la seule coherence a verifier est celle entre PathBits<>::fromPath() et GridInfo<>::branch()

  // transformation d'un empilement de booleens en serie de bits
  template<unsigned int D> struct PathBits;
  template<> struct PathBits<0>
  {
    static inline unsigned int fromPath(Vec<bool,0>) {return 0;}
  };
  template<unsigned int D> struct PathBits
  {
    static inline unsigned int fromPath(const Vec<bool,D>& path)
    {
      //return ( PathBits<D-1>::fromPath(path) << 1 ) | ( path.val ? 1 : 0 ) ;
      return ( path.val ? (1<<(D-1)) : 0 ) | PathBits<D-1>::fromPath(path);
    }
  };

}; // namespace hct

#endif
