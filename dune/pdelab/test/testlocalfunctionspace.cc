// -*- tab-width: 4; indent-tabs-mode: nil -*-
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include<iostream>
#include<vector>
#include<dune/common/parallel/mpihelper.hh>
#include<dune/common/exceptions.hh>
#include<dune/common/fvector.hh>
#include<dune/grid/yaspgrid.hh>
#include"../finiteelementmap/q22dfem.hh"
#include"../finiteelementmap/q12dfem.hh"
#include"../gridfunctionspace/gridfunctionspace.hh"
#include"../gridfunctionspace/localvector.hh"

// test function trees
template<class GV>
void test (const GV& gv)
{
  // instantiate finite element maps
  typedef Dune::PDELab::Q22DLocalFiniteElementMap<float,double> Q22DFEM;
  Q22DFEM q22dfem;
  typedef Dune::PDELab::Q12DLocalFiniteElementMap<float,double> Q12DFEM;
  Q12DFEM q12dfem;

  // make a grid function space
  typedef Dune::PDELab::GridFunctionSpace<GV,Q22DFEM> Q2GFS;
  Q2GFS q2gfs(gv,q22dfem);
  typedef Dune::PDELab::GridFunctionSpace<GV,Q12DFEM> Q1GFS;
  Q1GFS q1gfs(gv,q12dfem);

  // power grid function space
  typedef Dune::PDELab::PowerGridFunctionSpace<Q2GFS,2,
    Dune::PDELab::GridFunctionSpaceLexicographicMapper> PowerGFS;
  PowerGFS powergfs(q2gfs);

  // composite grid function space
  typedef Dune::PDELab::CompositeGridFunctionSpace<Dune::PDELab::GridFunctionSpaceLexicographicMapper,
    PowerGFS,Q1GFS> CompositeGFS;
  CompositeGFS compositegfs(powergfs,q1gfs);

  // make coefficent Vectors
  typedef typename Dune::PDELab::BackendVectorSelector<Q2GFS,double>::Type V;
  V x(q2gfs);
  x = 0.0;
  typedef typename Dune::PDELab::BackendVectorSelector<PowerGFS,double>::Type VP;
  VP xp(powergfs);
  xp = 0.0;

  // make local function space object
  typedef Dune::PDELab::AnySpaceTag Tag;
  typename Dune::PDELab::LocalFunctionSpace<Q2GFS> q2lfs(q2gfs);
  Dune::PDELab::LocalVector<double, Tag> xl(q2lfs.maxSize());
  typename Dune::PDELab::LocalFunctionSpace<PowerGFS> powerlfs(powergfs);
  Dune::PDELab::LocalVector<double, Tag> xlp(powerlfs.maxSize());
  typename Dune::PDELab::LocalFunctionSpace<CompositeGFS> compositelfs(compositegfs);
  //  std::vector<double> xlc(compositelfs.maxSize());

  // loop over elements
  typedef typename GV::Traits::template Codim<0>::Iterator ElementIterator;
  for (ElementIterator it = gv.template begin<0>();
	   it!=gv.template end<0>(); ++it)
	{
      q2lfs.bind(*it);
      q2lfs.debug();
      q2lfs.vread(x,xl);
      assert(q2lfs.size() ==
          q2lfs.localVectorSize());

      powerlfs.bind(*it);
      powerlfs.debug();
      powerlfs.vread(xp,xlp);
      assert(powerlfs.size() ==
          powerlfs.localVectorSize());
      assert(powerlfs.localVectorSize() ==
          powerlfs.template child<0>().localVectorSize());
      assert(powerlfs.localVectorSize() ==
          powerlfs.template child<1>().localVectorSize());

      compositelfs.bind(*it);
      compositelfs.debug();
      assert(compositelfs.size() ==
          compositelfs.localVectorSize());
      assert(compositelfs.localVectorSize() ==
          compositelfs.template child<0>().localVectorSize());
      assert(compositelfs.localVectorSize() ==
          compositelfs.template child<0>().template child<0>().localVectorSize());
      assert(compositelfs.localVectorSize() ==
          compositelfs.template child<0>().template child<1>().localVectorSize());
      assert(compositelfs.localVectorSize() ==
          compositelfs.template child<1>().localVectorSize());
	}
}

int main(int argc, char** argv)
{
  try{
    //Maybe initialize Mpi
    Dune::MPIHelper::instance(argc, argv);

	// need a grid in order to test grid functions
	Dune::FieldVector<double,2> L(1.0);
	Dune::FieldVector<int,2> N(1);
	Dune::FieldVector<bool,2> B(false);
	Dune::YaspGrid<2> grid(L,N,B,0);
    grid.globalRefine(1);

	test(grid.leafView());

	// test passed
	return 0;

  }
  catch (Dune::Exception &e){
    std::cerr << "Dune reported error: " << e << std::endl;
	return 1;
  }
  catch (...){
    std::cerr << "Unknown exception thrown!" << std::endl;
	return 1;
  }
}
