// -*- tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=8 sw=2 sts=2:

#ifndef DUNE_PDELAB_GRIDFUNCTIONSPACE_LEXICOGRAPHICORDERING_HH
#define DUNE_PDELAB_GRIDFUNCTIONSPACE_LEXICOGRAPHICORDERING_HH

#include <cstddef>
#include <ostream>
#include <string>

#include <dune/common/classname.hh>
#include <dune/common/exceptions.hh>
#include <dune/common/stdstreams.hh>

#include <dune/pdelab/common/typetree/compositenodemacros.hh>
#include <dune/pdelab/common/typetree/powernode.hh>
#include <dune/pdelab/common/typetree/traversal.hh>
#include <dune/pdelab/common/typetree/visitor.hh>
#include <dune/pdelab/gridfunctionspace/nonleaforderingbase.hh>
#include <dune/pdelab/gridfunctionspace/orderingbase.hh>
#include <dune/pdelab/gridfunctionspace/compositeorderingutilities.hh>

namespace Dune {
  namespace PDELab {

    //! \addtogroup GridFunctionSpace
    //! \ingroup PDELab
    //! \{

    //! \brief Indicate lexicographic ordering of the unknowns of non-leaf
    //!        grid function spaces.
    /**
     * This class instructs the non-leaf GridFunctionSpaces to order the dofs
     * of the child-GridFunctionSpaces in a lexicographic manner in the
     * combined dof-vector, i.e. first all dofs of child 0, then all dofs of
     * child 1 and so on.
     */
    struct LexicographicOrderingTag { };
    typedef LexicographicOrderingTag GridFunctionSpaceLexicographicMapper;

    namespace LexicographicOrderingImp {

      template<class SizeType>
      class CollectSizesVisitor :
        public TypeTree::DirectChildrenVisitor,
        public TypeTree::DynamicTraversal
      {
        SizeType *sizes;

      public:
        CollectSizesVisitor(SizeType *sizes_) : sizes(sizes_) { }

        template<class T, class Child, class TreePath, class ChildIndex>
        void beforeChild(const T &t, const Child& child, TreePath, ChildIndex childIndex) const
        { sizes[childIndex] = child.size(); }
      };

      //! Interface for merging index spaces
      template<class SizeType, class Node, class Imp>
      class Base :
        public NonLeafOrderingBase<SizeType, Imp>
      {
      protected:
        using NonLeafOrderingBase<SizeType, Imp>::asImp;
        using NonLeafOrderingBase<SizeType, Imp>::printInfo;

        SizeType childOffsets[Node::CHILDREN+1];

      public:
        //! Construct ordering object
        /**
         * In general, an ordering object is not properly setup after
         * construction.  This must be done by a seperate call to update()
         * after all the children have been properly set up.
         */
        Base() {
          childOffsets[0] = 0;
          asImp().update();
        }

        //! update internal data structures
        /**
         * This method must be called after initialization and every time the
         * structure of the dof-vector of one of gfs's children changes.  All
         * the children must have been set up properly before the call to
         * update().
         */
        void update() {
          Dune::dinfo << asImp().name() << ":" << std::endl;

          TypeTree::applyToTree(asImp(),
                                CollectSizesVisitor<SizeType>(childOffsets+1));
          // childOffset[i+1] now contains the *size* of child[i].
          // Convert to offsets...
          for(std::size_t child = 1; child < Node::CHILDREN; ++child)
            childOffsets[child+1] += childOffsets[child];

          printInfo(dinfo);
        }

        //! whether dofs are blocked per entity/intersection (they are not)
        bool blocked() const { return false; }

        //! map a global dof index from a child
        /**
         * Given the index of a dof in the global dof-vector of one of the
         * children, compute the index of the same dof in the global
         * dof-vector of this ordering.
         *
         * \note update() must have been called before this method may be
         *       used.
         */
        SizeType subMap(SizeType child, SizeType indexInChild) const
        { return childOffsets[child] + indexInChild; }

        //! number of indices in this ordering
        SizeType size() const { return childOffsets[Node::CHILDREN]; }

        //! \brief offset of the block of dofs attached to a given entity (of
        //!        arbitrary codimension)
        /**
         * This implementation just throws NotImplemented since there are no
         * per-entity blocks for lexicographic ordering.
         *
         * \throw NotImplemented If this EntityType is not supported by the
         *                       ordering.
         */
        template<class Entity>
        SizeType entityOffset(const Entity &e) const {
          DUNE_THROW(NotImplemented, className<Imp>() << "::entityOffset() "
                     "does not make sense since the ordering is non-blocking");
        }
        //! \brief offset of the blocks of dofs attached to a given subentity
        //!        of an element
        /**
         * This implementation just throws NotImplemented since there are no
         * per-entity blocks for lexicographic ordering.
         *
         * \throw NotImplemented If this EntityType is not supported by the
         *                       ordering.
         */
        template<class Element>
        SizeType entityOffset(const Element &e, std::size_t codim,
                              std::size_t subentity) const {
          DUNE_THROW(NotImplemented, className<Imp>() << "::entityOffset() "
                     "does not make sense since the ordering is non-blocking");
        }
        //! offset of the block of dofs attached to a given intersection
        /**
         * This implementation just throws NotImplemented since there are no
         * per-intersection blocks for lexicographic ordering.
         *
         * \throw NotImplemented If this EntityType is not supported by the
         *                       ordering.
         */
        template<class Intersection>
        SizeType intersectionOffset(const Intersection &i) const {
          DUNE_THROW(NotImplemented,
                     className<Imp>() << "::intersectionOffset() does not "
                     "make sense since the ordering is non-blocking");
        }
      };
    }

    //! Interface for merging index spaces
    template<class SizeType, class Child, std::size_t k>
    class PowerLexicographicOrdering :
      public TypeTree::PowerNode<Child, k>,
      public LexicographicOrderingImp::Base<
        SizeType, TypeTree::PowerNode<Child, k>,
        PowerLexicographicOrdering<SizeType, Child, k>
      >
    {
      typedef TypeTree::PowerNode<Child, k> Node;

    public:
      //! Construct ordering object
      /**
       * In general, an ordering object is not properly setup after
       * construction.  This must be done by a seperate call to update() after
       * all the children have been properly set up.
       *
       * \note This constructor must be present for ordering objects not at
       *       the leaf of the tree.
       */
      template<class GFS>
      PowerLexicographicOrdering(const GFS &gfs,
                                 const typename Node::NodeStorage &children) :
        Node(children)
      { }

      std::string name() const { return "PowerLexicographicOrdering"; }
    };

    template<>
    struct TransformPowerGFSToOrdering<LexicographicOrderingTag> {
      template<class GFSTraits, class TransformedChild, std::size_t k>
      struct result {
        typedef PowerLexicographicOrdering<typename GFSTraits::SizeType,
                                           TransformedChild, k
                                           > type;
      };
    };

    //! Interface for merging index spaces
    template<class SizeType, DUNE_TYPETREE_COMPOSITENODE_TEMPLATE_CHILDREN>
    class CompositeLexicographicOrdering :
      public DUNE_TYPETREE_COMPOSITENODE_BASETYPE,
      public LexicographicOrderingImp::Base<
        SizeType, DUNE_TYPETREE_COMPOSITENODE_BASETYPE,
        CompositeLexicographicOrdering<
          SizeType, DUNE_TYPETREE_COMPOSITENODE_CHILDTYPES
          >
      >
    {
      typedef DUNE_TYPETREE_COMPOSITENODE_BASETYPE Node;

    public:
      //! Construct ordering object
      /**
       * In general, an ordering object is not properly setup after
       * construction.  This must be done by a seperate call to update() after
       * all the children have been properly set up.
       *
       * \note This constructor must be present for ordering objects not at
       *       the leaf of the tree.
       */
      template<class GFS>
      CompositeLexicographicOrdering
      ( const GFS &gfs,
        DUNE_TYPETREE_COMPOSITENODE_STORAGE_CONSTRUCTOR_SIGNATURE) :
        Node(DUNE_TYPETREE_COMPOSITENODE_CHILDVARIABLES)
      { }

      std::string name() const { return "CompositeLexicographicOrdering"; }
    };


#if HAVE_VARIADIC_TEMPLATES

    //! Node transformation descriptor for CompositeGridFunctionSpace -> LexicographicOrdering (with variadic templates).
    template<typename GFSNode, typename Transformation>
    struct VariadicCompositeGFSToLexicographicOrderingTransformation
    {

      static const bool recursive = true;

      template<typename... TC>
      struct result
      {
        typedef CompositeLexicographicOrdering<typename Transformation::GridFunctionSpace::Traits::SizeType,
                                               TC...> type;
        typedef shared_ptr<type> storage_type;
      };

      template<typename... TC>
      static typename result<TC...>::type transform(const GFSNode& s, const Transformation& t, shared_ptr<TC>... children)
      {
        return typename result<TC...>::type(t.asGridFunctionSpace(s),children...);
      }

      template<typename... TC>
      static typename result<TC...>::storage_type transform_storage(shared_ptr<const GFSNode> s, const Transformation& t, shared_ptr<TC>... children)
      {
        return make_shared<typename result<TC...>::type>(t.asGridFunctionSpace(*s),children...);
      }

    };

    // Register transformation.
    template<typename GFSNode, typename GFS>
    VariadicCompositeGFSToLexicographicOrderingTransformation<GFSNode,gfs_to_ordering<GFS,LexicographicOrderingTag> >
    lookupNodeTransformation(GFSNode*, gfs_to_ordering<GFS,LexicographicOrderingTag>*, CompositeGridFunctionSpaceBaseTag);

#else // HAVE_VARIADIC_TEMPLATES

    //! Node transformation descriptor for CompositeGridFunctionSpace -> LexicographicOrdering (without variadic templates).
    template<typename GFSNode, typename Transformation>
    struct CompositeGFSToLexicographicOrderingTransformation
    {

      static const bool recursive = true;

      template<typename TC0,
               typename TC1,
               typename TC2,
               typename TC3,
               typename TC4,
               typename TC5,
               typename TC6,
               typename TC7,
               typename TC8,
               typename TC9>
      struct result
      {
        typedef CompositeLexicographicOrdering<typename Transformation::GridFunctionSpace::Traits::SizeType,
                                               TC0,TC1,TC2,TC3,TC4,TC5,TC6,TC7,TC8,TC9> type;
        typedef shared_ptr<type> storage_type;
      };

      template<typename TC0,
               typename TC1,
               typename TC2,
               typename TC3,
               typename TC4,
               typename TC5,
               typename TC6,
               typename TC7,
               typename TC8,
               typename TC9>
      static typename result<TC0,TC1,TC2,TC3,TC4,TC5,TC6,TC7,TC8,TC9>::type
      transform(const GFSNode& s,
                const Transformation& t,
                shared_ptr<TC0> c0,
                shared_ptr<TC1> c1,
                shared_ptr<TC2> c2,
                shared_ptr<TC3> c3,
                shared_ptr<TC4> c4,
                shared_ptr<TC5> c5,
                shared_ptr<TC6> c6,
                shared_ptr<TC7> c7,
                shared_ptr<TC8> c8,
                shared_ptr<TC9> c9)
      {
        return typename result<TC0,TC1,TC2,TC3,TC4,TC5,TC6,TC7,TC8,TC9>::type(t.asGridFunctionSpace(s),c0,c1,c2,c3,c4,c5,c6,c7,c8,c9);
      }

      template<typename TC0,
               typename TC1,
               typename TC2,
               typename TC3,
               typename TC4,
               typename TC5,
               typename TC6,
               typename TC7,
               typename TC8,
               typename TC9>
      static typename result<TC0,TC1,TC2,TC3,TC4,TC5,TC6,TC7,TC8,TC9>::storage_type
      transform_storage(shared_ptr<const GFSNode> s,
                        const Transformation& t,
                        shared_ptr<TC0> c0,
                        shared_ptr<TC1> c1,
                        shared_ptr<TC2> c2,
                        shared_ptr<TC3> c3,
                        shared_ptr<TC4> c4,
                        shared_ptr<TC5> c5,
                        shared_ptr<TC6> c6,
                        shared_ptr<TC7> c7,
                        shared_ptr<TC8> c8,
                        shared_ptr<TC9> c9)
      {
        return make_shared<typename result<TC0,TC1,TC2,TC3,TC4,TC5,TC6,TC7,TC8,TC9>::type>(t.asGridFunctionSpace(s),c0,c1,c2,c3,c4,c5,c6,c7,c8,c9);
      }

    };

    // Register transformation
    template<typename GFSNode, typename GFS>
    CompositeGFSToLexicographicOrderingTransformation<GFSNode,gfs_to_ordering<GFS,LexicographicOrderingTag> >
    lookupNodeTransformation(GFSNode*, gfs_to_ordering<GFS,LexicographicOrderingTag>*, CompositeGridFunctionSpaceBaseTag);

#endif // HAVE_VARIADIC_TEMPLATES

   //! \} group GridFunctionSpace
  } // namespace PDELab
} // namespace Dune

#endif // DUNE_PDELAB_GRIDFUNCTIONSPACE_LEXICOGRAPHICORDERING_HH
