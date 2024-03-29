// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_PDELAB_CONSTRAINTS_HH
#define DUNE_PDELAB_CONSTRAINTS_HH

#include<dune/common/exceptions.hh>
#include <dune/common/float_cmp.hh>

#include"../common/function.hh"
#include"../common/geometrywrapper.hh"
#include"../common/typetraits.hh"

#include"../gridfunctionspace/gridfunctionspace.hh"

namespace Dune {
  namespace PDELab {

    //! \addtogroup GridFunctionSpace
    //! \ingroup PDELab
    //! \{

    namespace { // hide internals
      // do method invocation only if class has the method

      template<typename C, bool doIt>
      struct ConstraintsCallBoundary
      {
        template<typename F, typename IG, typename LFS, typename T>
        static void boundary (const C& c, const F& f, const IG& ig, const LFS& lfs, T& trafo)
        {
        }
      };
      template<typename C, bool doIt>
      struct ConstraintsCallProcessor
      {
        template<typename IG, typename LFS, typename T>
        static void processor (const C& c, const IG& ig, const LFS& lfs, T& trafo)
        {
        }
      };
      template<typename C, bool doIt>
      struct ConstraintsCallSkeleton
      {
        template<typename IG, typename LFS, typename T>
        static void skeleton (const C& c,  const IG& ig,
                              const LFS& lfs_e, const LFS& lfs_f,
                              T& trafo_e, T& trafo_f)
        {
        }
      };
      template<typename C, bool doIt>
      struct ConstraintsCallVolume
      {
        template<typename EG, typename LFS, typename T>
        static void volume (const C& c, const EG& eg, const LFS& lfs, T& trafo)
        {
        }
      };


      template<typename C>
      struct ConstraintsCallBoundary<C,true>
      {
        template<typename F, typename IG, typename LFS, typename T>
        static void boundary (const C& c, const F& f, const IG& ig, const LFS& lfs, T& trafo)
        {
          c.boundary(f,ig,lfs,trafo);
        }
      };
      template<typename C>
      struct ConstraintsCallProcessor<C,true>
      {
        template<typename IG, typename LFS, typename T>
        static void processor (const C& c, const IG& ig, const LFS& lfs, T& trafo)
        {
          c.processor(ig,lfs,trafo);
        }
      };
      template<typename C>
      struct ConstraintsCallSkeleton<C,true>
      {
        template<typename IG, typename LFS, typename T>
        static void skeleton (const C& c, const IG& ig,
                              const LFS& lfs_e, const LFS& lfs_f,
                              T& trafo_e, T& trafo_f)
        {
          c.skeleton(ig, lfs_e, lfs_f, trafo_e, trafo_f);
        }
      };
      template<typename C>
      struct ConstraintsCallVolume<C,true>
      {
        template<typename EG, typename LFS, typename T>
        static void volume (const C& c, const EG& eg, const LFS& lfs, T& trafo)
        {
          c.volume(eg,lfs,trafo);
        }
      };


      struct BoundaryConstraintsBase
        : public TypeTree::TreePairVisitor
      {
        // This acts as a catch-all for unsupported leaf- / non-leaf combinations in the two
        // trees. It is necessary because otherwise, the visitor would fall back to the default
        // implementation in TreeVisitor, which simply does nothing. The resulting bugs would
        // probably be hell to find...
        template<typename F, typename LFS, typename TreePath>
        void leaf(const F& f, const LFS& lfs, TreePath treePath) const
        {
          dune_static_assert((AlwaysFalse<F>::Value),
                             "unsupported combination of function and LocalFunctionSpace");
        }
      };


      template<typename F, typename IG, typename CG>
      struct BoundaryConstraintsForParametersLeaf
        : public TypeTree::TreeVisitor
        , public TypeTree::DynamicTraversal
      {

        template<typename LFS, typename TreePath>
        void leaf(const LFS& lfs, TreePath treePath) const
        {
          // allocate local constraints map
          CG cl;

          // extract constraints type
          typedef typename LFS::Traits::ConstraintsType C;

          // iterate over boundary, need intersection iterator
          ConstraintsCallBoundary<C,C::doBoundary>::boundary(lfs.constraints(),f,ig,lfs,cl);

          // write coefficients into local vector
          lfs.mwrite(cl,cg);
        }

        BoundaryConstraintsForParametersLeaf(const F& f_, const IG& ig_, CG& cg_)
          : f(f_)
          , ig(ig_)
          , cg(cg_)
        {}

        const F& f;
        const IG& ig;
        CG& cg;

      };


      template<typename IG, typename CG>
      struct BoundaryConstraints
        : public BoundaryConstraintsBase
        , public TypeTree::DynamicTraversal
      {

        // standard case - leaf in both trees
        template<typename F, typename LFS, typename TreePath>
        typename enable_if<F::isLeaf && LFS::isLeaf>::type
        leaf(const F& f, const LFS& lfs, TreePath treePath) const
        {
          // allocate local constraints map
          CG cl;

          // extract constraints type
          typedef typename LFS::Traits::ConstraintsType C;

          // iterate over boundary, need intersection iterator
          ConstraintsCallBoundary<C,C::doBoundary>::boundary(lfs.constraints(),f,ig,lfs,cl);

          // write coefficients into local vector
          lfs.mwrite(cl,cg);
        }

        // reuse constraints parameter information from f for all LFS children
        template<typename F, typename LFS, typename TreePath>
        typename enable_if<F::isLeaf && (!LFS::isLeaf)>::type
        leaf(const F& f, const LFS& lfs, TreePath treePath) const
        {
          // traverse LFS tree and reuse parameter information
          TypeTree::applyToTree(lfs,BoundaryConstraintsForParametersLeaf<F,IG,CG>(f,ig,cg));
        }

        BoundaryConstraints(const IG& ig_, CG& cg_)
          : ig(ig_)
          , cg(cg_)
        {}

      private:
        const IG& ig;
        CG& cg;

      };


      template<typename IG, typename CG>
      struct ProcessorConstraints
        : public TypeTree::TreeVisitor
        , public TypeTree::DynamicTraversal
      {

        template<typename LFS, typename TreePath>
        void leaf(const LFS& lfs, TreePath treePath) const
        {
          CG cl;

          // extract constraints type
          typedef typename LFS::Traits::ConstraintsType C;

          // iterate over boundary, need intersection iterator
          ConstraintsCallProcessor<C,C::doProcessor>::processor(lfs.constraints(),ig,lfs,cl);

          // write coefficients into local vector
          lfs.mwrite(cl,cg);
        }

        ProcessorConstraints(const IG& ig_, CG& cg_)
          : ig(ig_)
          , cg(cg_)
        {}

      private:
        const IG& ig;
        CG& cg;

      };


      template<typename IG, typename CG>
      struct SkeletonConstraints
        : public TypeTree::TreePairVisitor
        , public TypeTree::DynamicTraversal
      {

        template<typename LFS, typename TreePath>
        void leaf(const LFS& lfs_e, const LFS& lfs_f, TreePath treePath) const
        {
          // allocate local constraints map for both elements adjacent
          // to this intersection
          CG cl_e;
          CG cl_f;

          // extract constraints type
          typedef typename LFS::Traits::ConstraintsType C;

          // as LFS::constraints() just returns the constraints of the
          // GridFunctionSpace, lfs_e.constraints() is equivalent to
          // lfs_f.constraints()
          const C & c = lfs_e.constraints();

          // iterate over boundary, need intersection iterator
          ConstraintsCallSkeleton<C,C::doSkeleton>::skeleton(c,ig,lfs_e,lfs_f,cl_e,cl_f);

          // write coefficients into local vector
          lfs_e.mwrite(cl_e,cg);
          lfs_f.mwrite(cl_f,cg);
        }

        SkeletonConstraints(const IG& ig_, CG& cg_)
          : ig(ig_)
          , cg(cg_)
        {}

      private:
        const IG& ig;
        CG& cg;

      };


      template<typename EG, typename CG>
      struct VolumeConstraints
        : public TypeTree::TreeVisitor
        , public TypeTree::DynamicTraversal
      {

        template<typename LFS, typename TreePath>
        void leaf(const LFS& lfs, TreePath treePath) const
        {
          // allocate local constraints map
          CG cl;

          // extract constraints type
          typedef typename LFS::Traits::ConstraintsType C;
          const C & c = lfs.constraints();

          // iterate over boundary, need intersection iterator
          ConstraintsCallVolume<C,C::doVolume>::volume(c,eg,lfs,cl);

          // write coefficients into local vector
          lfs.mwrite(cl,cg);
        }

        VolumeConstraints(const EG& eg_, CG& cg_)
          : eg(eg_)
          , cg(cg_)
        {}

      private:
        const EG& eg;
        CG& cg;

      };


    } // anonymous namespace

    template<DUNE_TYPETREE_COMPOSITENODE_TEMPLATE_CHILDREN_FOR_SPECIALIZATION>
    struct CompositeConstraintsOperator
      : public DUNE_TYPETREE_COMPOSITENODE_BASETYPE
    {
      typedef DUNE_TYPETREE_COMPOSITENODE_BASETYPE BaseT;

      CompositeConstraintsOperator(DUNE_TYPETREE_COMPOSITENODE_CONSTRUCTOR_SIGNATURE)
        : BaseT(DUNE_TYPETREE_COMPOSITENODE_CHILDVARIABLES)
      {}

      CompositeConstraintsOperator(DUNE_TYPETREE_COMPOSITENODE_STORAGE_CONSTRUCTOR_SIGNATURE)
        : BaseT(DUNE_TYPETREE_COMPOSITENODE_CHILDVARIABLES)
      {}

      // aggregate flags

      // forward methods to childs

    };

    template<DUNE_TYPETREE_COMPOSITENODE_TEMPLATE_CHILDREN_FOR_SPECIALIZATION>
    struct CompositeConstraintsParameters
      : public DUNE_TYPETREE_COMPOSITENODE_BASETYPE
    {
      typedef DUNE_TYPETREE_COMPOSITENODE_BASETYPE BaseT;

      CompositeConstraintsParameters(DUNE_TYPETREE_COMPOSITENODE_CONSTRUCTOR_SIGNATURE)
        : BaseT(DUNE_TYPETREE_COMPOSITENODE_CHILDVARIABLES)
      {}

      CompositeConstraintsParameters(DUNE_TYPETREE_COMPOSITENODE_STORAGE_CONSTRUCTOR_SIGNATURE)
        : BaseT(DUNE_TYPETREE_COMPOSITENODE_CHILDVARIABLES)
      {}
    };

    template<typename T, std::size_t k>
    struct PowerConstraintsParameters
      : public TypeTree::PowerNode<T,k>
    {
      typedef TypeTree::PowerNode<T,k> BaseT;

      PowerConstraintsParameters()
        : BaseT()
      {}

      PowerConstraintsParameters(T& c)
        : BaseT(c)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1)
        : BaseT(c0,c1)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2)
        : BaseT(c0,c1,c2)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3)
        : BaseT(c0,c1,c2,c3)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3,
                              T& c4)
        : BaseT(c0,c1,c2,c3,c4)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3,
                              T& c4,
                              T& c5)
        : BaseT(c0,c1,c2,c3,c4,c5)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3,
                              T& c4,
                              T& c5,
                              T& c6)
        : BaseT(c0,c1,c2,c3,c4,c5,c6)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3,
                              T& c4,
                              T& c5,
                              T& c6,
                              T& c7)
        : BaseT(c0,c1,c2,c3,c4,c5,c6,c7)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3,
                              T& c4,
                              T& c5,
                              T& c6,
                              T& c7,
                              T& c8)
        : BaseT(c0,c1,c2,c3,c4,c5,c6,c7,c8)
      {}

      PowerConstraintsParameters (T& c0,
                              T& c1,
                              T& c2,
                              T& c3,
                              T& c4,
                              T& c5,
                              T& c6,
                              T& c7,
                              T& c8,
                              T& c9)
        : BaseT(c0,c1,c2,c3,c4,c5,c6,c7,c8,c9)
      {}

      PowerConstraintsParameters (const array<shared_ptr<T>,k>& children)
        : BaseT(children)
      {}
    };

#ifndef DOXYGEN

    //! wraps a BoundaryGridFunction in a OldStyleConstraintsParameter class
    template<typename F>
    class OldStyleConstraintsWrapper
      : public TypeTree::LeafNode
    {
      shared_ptr<const F> _f;
      unsigned int _i;
    public:

      template<typename Transformation>
      OldStyleConstraintsWrapper(shared_ptr<const F> f, const Transformation& t, unsigned int i=0)
        : _f(f)
        , _i(i)
      {}

      template<typename Transformation>
      OldStyleConstraintsWrapper(const F & f, const Transformation& t, unsigned int i=0)
        : _f(stackobject_to_shared_ptr(f))
        , _i(i)
      {}

      template<typename I>
      bool isDirichlet(const I & intersection, const FieldVector<typename I::ctype, I::dimension-1> & coord) const
      {
        typename F::Traits::RangeType bctype;
        _f->evaluate(intersection,coord,bctype);
        return bctype[_i] > 0;
      }

      template<typename I>
      bool isNeumann(const I & intersection, const FieldVector<typename I::ctype, I::dimension-1> & coord) const
      {
        typename F::Traits::RangeType bctype;
        _f->evaluate(intersection,coord,bctype);
        return bctype[_i] == 0;
      }
    };

    //! empty ConstraintsParameters class, needed for the TMP without any parameters
    class NoConstraintsParameters : public TypeTree::LeafNode {};

    // Tag to name trafo GridFunction -> OldStyleConstraintsWrapper
    struct gf_to_constraints {};

    // register trafos GridFunction -> OldStyleConstraintsWrapper
    template<typename F, typename Transformation>
    struct MultiComponentOldStyleConstraintsWrapperDescription
    {

      static const bool recursive = false;

      enum { dim = F::Traits::dimRange };
      typedef OldStyleConstraintsWrapper<F> node_type;
      typedef PowerConstraintsParameters<node_type, dim> transformed_type;
      typedef shared_ptr<transformed_type> transformed_storage_type;

      static transformed_type transform(const F& s, const Transformation& t)
      {
        shared_ptr<const F> sp = stackobject_to_shared_ptr(s);
        array<shared_ptr<node_type>, dim> childs;
        for (int i=0; i<dim; i++)
          childs[i] = make_shared<node_type>(sp,t,i);
        return transformed_type(childs);
      }

      static transformed_storage_type transform_storage(shared_ptr<const F> s, const Transformation& t)
      {
        array<shared_ptr<node_type>, dim> childs;
        for (int i=0; i<dim; i++)
          childs[i] = make_shared<node_type>(s,t,i);
        return make_shared<transformed_type>(childs);
      }

    };
    // trafos for leaf nodes
    template<typename GridFunction>
    typename SelectType<
      (GridFunction::Traits::dimRange == 1),
      // trafo for scalar leaf nodes
      Dune::PDELab::TypeTree::GenericLeafNodeTransformation<GridFunction,gf_to_constraints,OldStyleConstraintsWrapper<GridFunction> >,
      // trafo for multi component leaf nodes
      MultiComponentOldStyleConstraintsWrapperDescription<GridFunction,gf_to_constraints>
      >::Type
    lookupNodeTransformation(GridFunction*, gf_to_constraints*, GridFunctionTag);

    // trafo for power nodes
    template<typename PowerGridFunction>
    Dune::PDELab::TypeTree::SimplePowerNodeTransformation<PowerGridFunction,gf_to_constraints,PowerConstraintsParameters>
    lookupNodeTransformation(PowerGridFunction*, gf_to_constraints*, PowerGridFunctionTag);

    // trafos for composite nodes
#if HAVE_VARIADIC_TEMPLATES
    template<typename CompositeGridFunction>
    Dune::PDELab::TypeTree::SimpleVariadicCompositeNodeTransformation<CompositeGridFunction,gf_to_constraints,CompositeConstraintsParameters>
    lookupNodeTransformation(CompositeGridFunction*, gf_to_constraints*, CompositeGridFunctionTag);
#else
    template<typename CompositeGridFunction>
    Dune::PDELab::TypeTree::SimpleCompositeNodeTransformation<CompositeGridFunction,gf_to_constraints,CompositeConstraintsParameters>
    lookupNodeTransformation(CompositeGridFunction*, gf_to_constraints*, CompositeGridFunctionTag);
#endif

    //! construct constraints
    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     * \tparam P   Type implementing a constraints parameter tree
     * \tparam GFS Type implementing the model GridFunctionSpace
     * \tparam CG  Type implementing the model
     *             GridFunctionSpace::ConstraintsContainer::Type
     * \tparam isFunction bool to identify old-style parameters, which were implemented the Dune::PDELab::FunctionInterface
     */
    template<typename P, typename GFS, typename CG, bool isFunction>
    struct ConstraintsAssemblerHelper
    {
      //! construct constraints from given boundary condition function
      /**
       * \code
       * #include <dune/pdelab/gridfunctionspace/constraints.hh>
       * \endcode
       * \tparam P   Type implementing a constraints parameter tree
       * \tparam GFS Type implementing the model GridFunctionSpace
       * \tparam CG  Type implementing the model
       *             GridFunctionSpace::ConstraintsContainer::Type
       *
       * \param p       The parameter object
       * \param gfs     The gridfunctionspace
       * \param cg      The constraints container
       * \param verbose Print information about the constaints at the end
       */
      static void
      assemble(const P& p, const GFS& gfs, CG& cg, const bool verbose)
      {
        // clear global constraints
        cg.clear();

        // get some types
        typedef typename GFS::Traits::GridViewType GV;
        typedef typename GV::Traits::template Codim<0>::Entity Element;
        typedef typename GV::Traits::template Codim<0>::Iterator ElementIterator;
        typedef typename GV::IntersectionIterator IntersectionIterator;
        typedef typename IntersectionIterator::Intersection Intersection;

        // make local function space
        typedef LocalFunctionSpace<GFS> LFS;
        LFS lfs_e(gfs);
        LFS lfs_f(gfs);

        // get index set
        const typename GV::IndexSet& is=gfs.gridView().indexSet();

        // helper to compute offset dependent on geometry type
        const int chunk=1<<28;
        int offset = 0;
        std::map<Dune::GeometryType,int> gtoffset;

        // loop once over the grid
        for (ElementIterator it = gfs.gridView().template begin<0>();
             it!=gfs.gridView().template end<0>(); ++it)
        {
          // assign offset for geometry type;
          if (gtoffset.find(it->type())==gtoffset.end())
          {
            gtoffset[it->type()] = offset;
            offset += chunk;
          }

          const typename GV::IndexSet::IndexType id = is.index(*it)+gtoffset[it->type()];

          // bind local function space to element
          lfs_e.bind(*it);

          // TypeTree::applyToTreePair(p,lfs_e,VolumeConstraints<Element,CG>(ElementGeometry<Element>(*it),cg));
          typedef ElementGeometry<Element> ElementWrapper;
          TypeTree::applyToTree(lfs_e,VolumeConstraints<ElementWrapper,CG>(ElementWrapper(*it),cg));

          // iterate over intersections and call metaprogram
          unsigned int intersection_index = 0;
          IntersectionIterator endit = gfs.gridView().iend(*it);
          for (IntersectionIterator iit = gfs.gridView().ibegin(*it); iit!=endit; ++iit, ++intersection_index)
          {
            if (iit->boundary())
            {
              typedef IntersectionGeometry<Intersection> IntersectionWrapper;
              TypeTree::applyToTreePair(p,lfs_e,BoundaryConstraints<IntersectionWrapper,CG>(IntersectionWrapper(*iit,intersection_index),cg));
            }

            // ParallelStuff: BEGIN support for processor boundaries.
            if ((!iit->boundary()) && (!iit->neighbor()))
            {
              typedef IntersectionGeometry<Intersection> IntersectionWrapper;
              TypeTree::applyToTree(lfs_e,ProcessorConstraints<IntersectionWrapper,CG>(IntersectionWrapper(*iit,intersection_index),cg));
            }
            // END support for processor boundaries.

            if (iit->neighbor()){

              Dune::GeometryType gtn = iit->outside()->type();
              const typename GV::IndexSet::IndexType idn = is.index(*(iit->outside()))+gtoffset[gtn];

              if(id>idn){
                // bind local function space to element in neighbor
                lfs_f.bind( *(iit->outside()) );
                typedef IntersectionGeometry<Intersection> IntersectionWrapper;
                TypeTree::applyToTreePair(lfs_e,lfs_f,SkeletonConstraints<IntersectionWrapper,CG>(IntersectionWrapper(*iit,intersection_index),cg));
              }
            }
          }
        }

        // print result
        if(verbose){
          std::cout << "constraints:" << std::endl;
          typedef typename CG::iterator global_col_iterator;
          typedef typename CG::value_type::second_type global_row_type;
          typedef typename global_row_type::iterator global_row_iterator;

          std::cout << cg.size() << " constrained degrees of freedom" << std::endl;

          for (global_col_iterator cit=cg.begin(); cit!=cg.end(); ++cit)
          {
            std::cout << cit->first << ": ";
            for (global_row_iterator rit=(cit->second).begin(); rit!=(cit->second).end(); ++rit)
              std::cout << "(" << rit->first << "," << rit->second << ") ";
            std::cout << std::endl;
          }
        }
      }
    }; // end ConstraintsAssemblerHelper

    template<typename F, typename GFS, typename CG>
    struct ConstraintsAssemblerHelper<F, GFS, CG, true>
    {
      static void
      assemble(const F& f, const GFS& gfs, CG& cg, const bool verbose)
      {
        // type of transformed tree
        typedef typename Dune::PDELab::TypeTree::TransformTree<F,gf_to_constraints> Transformation;
        typedef typename Transformation::Type P;
        // transform tree
        P p = Transformation::transform(f);
        // call parameter based implementation
        ConstraintsAssemblerHelper<P, GFS, CG, IsGridFunction<P>::value>::assemble(p,gfs,cg, verbose);
      }
    };
#endif

    //! construct constraints
    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     * \tparam GFS Type implementing the model GridFunctionSpace
     * \tparam CG  Type implementing the model
     *             GridFunctionSpace::ConstraintsContainer::Type
     *
     * \param gfs     The gridfunctionspace
     * \param cg      The constraints container
     * \param verbose Print information about the constaints at the end
     */
    template<typename GFS, typename CG>
    void constraints(const GFS& gfs, CG& cg,
                     const bool verbose = false)
    {
      NoConstraintsParameters p;
      ConstraintsAssemblerHelper<NoConstraintsParameters, GFS, CG, false>::assemble(p,gfs,cg, verbose);
    }

    //! construct constraints from given constraits parameter tree
    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     * \tparam P   Type implementing a constraits parameter tree
     * \tparam GFS Type implementing the model GridFunctionSpace
     * \tparam CG  Type implementing the model
     *             GridFunctionSpace::ConstraintsContainer::Type
     *
     * \param p       The condition parameters
     * \param gfs     The gridfunctionspace
     * \param cg      The constraints container
     * \param verbose Print information about the constaints at the end
     *
     * \note For backwards compatibility you can implement the parameter tree as an Implemention Dune::PDELab::FunctionInterface
     *
     */
    template<typename P, typename GFS, typename CG>
    void constraints(const P& p, const GFS& gfs, CG& cg,
                     const bool verbose = false)
    {
      ConstraintsAssemblerHelper<P, GFS, CG, IsGridFunction<P>::value>::assemble(p,gfs,cg, verbose);
    }

    //! construct constraints from given boundary condition function
    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     * \tparam CG Type of ConstraintsContainer
     * \tparam XG Type of coefficients container
     *
     * \param cg The ConstraintsContainer
     * \param x  The value to assign
     * \param xg The container with the coefficients
     */
    template<typename CG, typename XG>
    void set_constrained_dofs(const CG& cg, typename XG::ElementType x,
                              XG& xg)
    {
      typedef typename XG::Backend B;
      typedef typename CG::const_iterator global_col_iterator;
      for (global_col_iterator cit=cg.begin(); cit!=cg.end(); ++cit)
        B::access(xg,cit->first) = x;
    }

    //! check that constrained dofs match a certain value
    /**
     * as if they were set by set_constrained_dofs()
     *
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     *
     * \tparam CG  Type of ConstraintsContainer
     * \tparam XG  Type of coefficients container
     * \tparam Cmp Type of Comparison object to use (something with the
     *             interface of Dune::FloatCmpOps)
     *
     * \param cg  The ConstraintsContainer
     * \param x   The value to compare with
     * \param xg  The container with the coefficients
     * \param cmp The comparison object to use.
     *
     * \returns true if all constrained dofs match the given value, false
     *          otherwise.
     */
    template<typename CG, typename XG, typename Cmp>
    bool check_constrained_dofs(const CG& cg, typename XG::ElementType x,
                                XG& xg, const Cmp& cmp = Cmp())
    {
      typedef typename XG::Backend B;
      typedef typename CG::const_iterator global_col_iterator;
      for (global_col_iterator cit=cg.begin(); cit!=cg.end(); ++cit)
        if(cmp.ne(B::access(xg,cit->first), x))
          return false;
      return true;
    }
    //! check that constrained dofs match a certain value
    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     *
     * This just calls check_constrained_dofs(cg, x, xg, cmp) with a default
     * comparison object FloatCmpOps<typename XG::ElementType>().
     *
     * \tparam CG  Type of ConstraintsContainer
     * \tparam XG  Type of coefficients container
     *
     * \param cg  The ConstraintsContainer
     * \param x   The value to compare with
     * \param xg  The container with the coefficients
     *
     * \returns true if all constrained dofs match the given value, false
     *          otherwise.
     */
    template<typename CG, typename XG>
    bool check_constrained_dofs(const CG& cg, typename XG::ElementType x,
                                XG& xg)
    {
      return check_constrained_dofs(cg, x, xg,
                                    FloatCmpOps<typename XG::ElementType>());
    }

    //! transform residual into transformed basis: r -> r~
    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     */
    template<typename CG, typename XG>
    void constrain_residual (const CG& cg, XG& xg)
    {
      typedef typename CG::const_iterator global_col_iterator;
      typedef typename CG::value_type::second_type::const_iterator global_row_iterator;
      typedef typename XG::Backend B;

      for (global_col_iterator cit=cg.begin(); cit!=cg.end(); ++cit)
        for(global_row_iterator rit = cit->second.begin(); rit!=cit->second.end(); ++rit)
          B::access(xg,rit->first) += rit->second * B::access(xg,cit->first);

      // extra loop because constrained dofs might have contributions
      // to constrained dofs
      for (global_col_iterator cit=cg.begin(); cit!=cg.end(); ++cit)
        B::access(xg,cit->first) = 0;
    }

    //! Modify coefficient vector based on constrained dofs as given
    //! in the constraints container
    //! @{

    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     */
    template<typename CG, typename XG>
    void copy_constrained_dofs (const CG& cg, const XG& xgin, XG& xgout)
    {
      typedef typename XG::Backend B;
      typedef typename CG::const_iterator global_col_iterator;
      for (global_col_iterator cit=cg.begin(); cit!=cg.end(); ++cit)
        B::access(xgout,cit->first) = B::access(xgin,cit->first);
    }

    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     */
    template<typename CG, typename XG>
    void set_nonconstrained_dofs (const CG& cg, typename XG::ElementType x, XG& xg)
    {
      typedef typename XG::Backend B;
      for (typename XG::size_type i=0; i<xg.flatsize(); ++i)
        if (cg.find(i)==cg.end())
          B::access(xg,i) = x;
    }


    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     */
    template<typename CG, typename XG>
    void copy_nonconstrained_dofs (const CG& cg, const XG& xgin, XG& xgout)
    {
      typedef typename XG::Backend B;
      for (typename XG::size_type i=0; i<xgin.flatsize(); ++i)
        if (cg.find(i)==cg.end())
          B::access(xgout,i) = B::access(xgin,i);
    }

    /**
     * \code
     * #include <dune/pdelab/gridfunctionspace/constraints.hh>
     * \endcode
     */
    template<typename CG, typename XG>
    void set_shifted_dofs (const CG& cg, typename XG::ElementType x, XG& xg)
    {
      typedef typename XG::Backend B;
      typedef typename CG::const_iterator global_col_iterator;
      for (typename XG::size_type i=0; i<xg.flatsize(); ++i){
        global_col_iterator it = cg.find(i);
        if (it == cg.end() || it->second.size() > 0)
          B::access(xg,i) = x;
      }
    }

    //! @}

    //! \} group GridFunctionSpace
  } // namespace PDELab
} // namespace Dune

#endif
