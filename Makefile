EXE=_TEST_ConstBits _TEST_PointIds _TEST_Vec _TEST_GridEnum _TEST_readMesh _TEST_PathBits _TEST_HyperCube  _TEST_Nbh _TEST_MeshInfo _TEST_AmrLevels _TEST_AmrTree _TEST_AmrConnect _TEST_AmrSidePoints

#CXX=colorgcc
CXX=g++
#CFLAGS=-g3 -DDEBUG=1
CFLAGS=-march=native -mtune=native -O3 -finline-limit=1000000000 -DPROFILING=1

all: $(EXE)

_TEST_%: %.h
	$(CXX) -xc++ $(CFLAGS) -D$@=1 $< -o $@

test.%: data/%.bin $(EXE)
	$(DBG) ./_TEST_MeshInfo      $(<D)/`basename $< .bin`
	$(DBG) ./_TEST_AmrLevels     $(<D)/`basename $< .bin`
	$(DBG) ./_TEST_AmrTree       $(<D)/`basename $< .bin`
	$(DBG) ./_TEST_AmrConnect    $(<D)/`basename $< .bin`
	$(DBG) ./_TEST_AmrSidePoints $(<D)/`basename $< .bin`

clean:
	rm -f $(EXE) data/*.cs data/*.cc data/*.bnd data/*.dpt data/*.con data/*.geom data/*.lvl data/*.scal data/*.tree data/*.m2t data/*.nbh *~
