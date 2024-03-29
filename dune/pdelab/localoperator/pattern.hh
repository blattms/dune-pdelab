// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:
#ifndef DUNE_PDELAB_PATTERN_HH
#define DUNE_PDELAB_PATTERN_HH

#include<dune/common/exceptions.hh>
#include <dune/common/fvector.hh>
#include <dune/common/static_assert.hh>

#include"../common/geometrywrapper.hh"
#include"../gridoperatorspace/gridoperatorspace.hh"
#include"../gridoperatorspace/gridoperatorspaceutilities.hh"


namespace Dune {
  namespace PDELab {

    //! sparsity pattern generator
    class FullVolumePattern
    {
    public:

      // define sparsity pattern of operator representation
      template<typename LFSU, typename LFSV>
      void pattern_volume (const LFSU& lfsu, const LFSV& lfsv,
                           LocalSparsityPattern& pattern) const
      {
        for (size_t i=0; i<lfsv.size(); ++i)
          for (size_t j=0; j<lfsu.size(); ++j)
            pattern.push_back(SparsityLink(lfsv.localIndex(i),lfsu.localIndex(j)));
      }
   };

    //! sparsity pattern generator
    class FullSkeletonPattern
    {
    public:

      // define sparsity pattern connecting self and neighbor dofs
      template<typename LFSU, typename LFSV>
      void pattern_skeleton (const LFSU& lfsu_s, const LFSV& lfsv_s, const LFSU& lfsu_n, const LFSV& lfsv_n,
                            LocalSparsityPattern& pattern_sn,
                            LocalSparsityPattern& pattern_ns) const
      {
        for (unsigned int i=0; i<lfsv_s.size(); ++i)
          for (unsigned int j=0; j<lfsu_n.size(); ++j)
            pattern_sn.push_back(SparsityLink(lfsv_s.localIndex(i),lfsu_n.localIndex(j)));

        for (unsigned int i=0; i<lfsv_n.size(); ++i)
          for (unsigned int j=0; j<lfsu_s.size(); ++j)
            pattern_ns.push_back(SparsityLink(lfsv_n.localIndex(i),lfsu_s.localIndex(j)));
      }
   };

    //! sparsity pattern generator
    class FullBoundaryPattern
    {
    public:

      // define sparsity pattern connecting dofs on boundary elements
      template<typename LFSU, typename LFSV>
      void pattern_boundary(const LFSU& lfsu_s, const LFSV& lfsv_s,
                            LocalSparsityPattern& pattern_ss) const
      {
        for (unsigned int i=0; i<lfsv_s.size(); ++i)
          for (unsigned int j=0; j<lfsu_s.size(); ++j)
            pattern_ss.push_back(SparsityLink(lfsv_s.localIndex(i),lfsu_s.localIndex(j)));
      }
   };

    //! \} group GridFunctionSpace
  } // namespace PDELab
} // namespace Dune

#endif
