// -*- tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:

#ifndef DUNE_PDELAB_COMMON_TYPETREE_ACCUMULATE_STATIC_HH
#define DUNE_PDELAB_COMMON_TYPETREE_ACCUMULATE_STATIC_HH

#include <dune/pdelab/common/typetree/nodetags.hh>
#include <dune/pdelab/common/typetree/treepath.hh>


namespace Dune {
  namespace PDELab {
    namespace TypeTree {

      /** \addtogroup Tree Traversal
       *  \ingroup TypeTree
       *  \{
       */

      //! Statically combine two values of type result_type using ||.
      template<typename result_type>
      struct or_
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 || r2;
        };
      };

      //! Statically combine two values of type result_type using &&.
      template<typename result_type>
      struct and_
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 && r2;
        };
      };

      //! Statically combine two values of type result_type using +.
      template<typename result_type>
      struct plus
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 + r2;
        };
      };

      //! Statically combine two values of type result_type using -.
      template<typename result_type>
      struct minus
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 - r2;
        };
      };

      //! Statically combine two values of type result_type using *.
      template<typename result_type>
      struct multiply
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 * r2;
        };
      };

      //! Statically combine two values of type result_type by returning their minimum.
      template<typename result_type>
      struct min
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 < r2 ? r1 : r2;
        };
      };

      //! Statically combine two values of type result_type by returning their maximum.
      template<typename result_type>
      struct max
      {
        template<result_type r1, result_type r2>
        struct reduce
        {
          static const result_type result = r1 > r2 ? r1 : r2;
        };
      };


      namespace {

        // implementation of the traversal algoritm

        //! helper struct to decide whether or not to perform the per-node calculation on the current node. Default case: ignore the node.
        template<typename Node, typename Functor, typename Reduction, typename Functor::result_type current_value, typename TreePath, bool doVisit>
        struct accumulate_node_helper
        {

          typedef typename Functor::result_type result_type;

          static const result_type result = current_value;

        };

        //! helper struct to decide whether or not to perform the per-node calculation on the current node. Specialization: Perform the calculation.
        template<typename Node, typename Functor, typename Reduction, typename Functor::result_type current_value, typename TreePath>
        struct accumulate_node_helper<Node,Functor,Reduction,current_value,TreePath,true>
        {

          typedef typename Functor::result_type result_type;

          static const result_type result = Reduction::template reduce<current_value,Functor::template visit<Node,TreePath>::result>::result;

        };

        //! Per node type algorithm struct. Prototype.
        template<typename Tree, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath, typename Tag>
        struct accumulate_value;

        //! Leaf node specialization.
        template<typename LeafNode, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath>
        struct accumulate_value<LeafNode,Functor,Reduction,ParentChildReduction,current_value,TreePath,LeafNodeTag>
        {

          typedef typename Functor::result_type result_type;

          static const result_type result =

accumulate_node_helper<LeafNode,Functor,Reduction,current_value,TreePath,Functor::template doVisit<LeafNode,TreePath>::value>::result;

        };

        //! Iteration over children of a composite node.
        template<typename Node, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath, std::size_t i, std::size_t n>
        struct accumulate_over_children
        {

          typedef typename Functor::result_type result_type;

          typedef typename TreePathPushBack<TreePath,i>::type child_tree_path;

          typedef typename Node::template Child<i>::Type child;

          static const result_type child_result = accumulate_value<child,Functor,Reduction,ParentChildReduction,current_value,child_tree_path,typename child::NodeTag>::result;

          static const result_type result = accumulate_over_children<Node,Functor,Reduction,ParentChildReduction,child_result,TreePath,i+1,n>::result;

        };

        //! end of iteration.
        template<typename Node, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath, std::size_t n>
        struct accumulate_over_children<Node,Functor,Reduction,ParentChildReduction,current_value,TreePath,n,n>
        {

          typedef typename Functor::result_type result_type;

          static const result_type result = current_value;

        };

        //! Generic composite node specialization. We are doing the calculation at compile time and thus have to use static iteration for
        //! the PowerNode as well.
        template<typename Node, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath>
        struct accumulate_value_generic_composite_node
        {

          typedef typename Functor::result_type result_type;

          static const result_type child_result = accumulate_over_children<Node,Functor,Reduction,ParentChildReduction,current_value,TreePath,0,Node::CHILDREN>::result;

          static const result_type result =
            accumulate_node_helper<Node,Functor,ParentChildReduction,child_result,TreePath,Functor::template doVisit<Node,TreePath>::value>::result;


        };

        //! PowerNode specialization.
        template<typename PowerNode, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath>
        struct accumulate_value<PowerNode,Functor,Reduction,ParentChildReduction,current_value,TreePath,PowerNodeTag>
          : public accumulate_value_generic_composite_node<PowerNode,Functor,Reduction,ParentChildReduction,current_value,TreePath>
        {};

        //! CompositeNode specialization.
        template<typename CompositeNode, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath>
        struct accumulate_value<CompositeNode,Functor,Reduction,ParentChildReduction,current_value,TreePath,CompositeNodeTag>
          : public accumulate_value_generic_composite_node<CompositeNode,Functor,Reduction,ParentChildReduction,current_value,TreePath>
        {};

#if HAVE_VARIADIC_TEMPLATES

        //! VariadicCompositeNode specialization.
        template<typename VariadicCompositeNode, typename Functor, typename Reduction, typename ParentChildReduction, typename Functor::result_type current_value, typename TreePath>
        struct accumulate_value<VariadicCompositeNode,Functor,Reduction,ParentChildReduction,current_value,TreePath,VariadicCompositeNodeTag>
          : public accumulate_value_generic_composite_node<VariadicCompositeNode,Functor,Reduction,ParentChildReduction,current_value,TreePath>
        {};

#endif // HAVE_VARIADIC_TEMPLATES

      } // anonymous namespace

      //! Statically accumulate a value over the nodes of a TypeTree.
      /**
       * This struct implements an algorithm for iterating over a tree and
       * calculating an accumulated value at compile time.
       *
       * \tparam Tree        The tree to iterate over.
       * \tparam Functor     The compile-time functor used for visiting each node.
       *
       * This functor must implement the following interface:
       *
       * \code
       * struct AccumulationFunctor
       * {
       *
       *   // The result type of the overall computation.
       *   typedef ... result_type;
       *
       *   // Decide whether to include the given node in the calculation
       *   // or to skip it.
       *   template<typename Node, typename TreePath>
       *   struct doVisit
       *   {
       *     static const bool value = true;
       *   };
       *
       *   // Calculate the per-node result.
       *   template<typename Node, typename TreePath>
       *   struct visit
       *   {
       *     static const result_type result = ...;
       *   };
       *
       * };
       * \endcode
       *
       * \tparam Reduction   The reduction operator used to accumulate the per-node
       *                     results.
       *
       * The reduction operator must implement the following interface:
       *
       * \code
       * template<typename result_type>
       * struct ReductionOperator
       * {
       *
       *   // combine two per-node results
       *   template<result_type r1, result_type r2>
       *   struct reduce
       *   {
       *     static const result_type result = ...;
       *   };
       *
       * };
       * \endcode
       *
       * \tparam startValue  The starting value fed into the initial accumulation step.
       */
      template<typename Tree, typename Functor, typename Reduction, typename Functor::result_type startValue, typename ParentChildReduction = Reduction>
      struct AccumulateValue
      {

        //! The result type of the computation.
        typedef typename Functor::result_type result_type;

        //! The accumulated result of the computation.
        static const result_type result = accumulate_value<Tree,Functor,Reduction,ParentChildReduction,startValue,TreePath<>,typename Tree::NodeTag>::result;

      };


      namespace {

        // implementation of the traversal algoritm

        //! helper struct to decide whether or not to perform the per-node calculation on the current node. Default case: ignore the node.
        template<typename Node, typename Functor, typename Reduction, typename current_type, typename TreePath, bool doVisit>
        struct accumulate_type_node_helper
        {

          typedef current_type type;

        };

        //! helper struct to decide whether or not to perform the per-node calculation on the current node. Specialization: Perform the calculation.
        template<typename Node, typename Functor, typename Reduction, typename current_type, typename TreePath>
        struct accumulate_type_node_helper<Node,Functor,Reduction,current_type,TreePath,true>
        {

          typedef typename Reduction::template reduce<current_type,typename Functor::template visit<Node,TreePath>::type>::type type;

        };

        //! Per node type algorithm struct. Prototype.
        template<typename Tree, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath, typename Tag>
        struct accumulate_type;

        //! Leaf node specialization.
        template<typename LeafNode, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath>
        struct accumulate_type<LeafNode,Functor,Reduction,ParentChildReduction,current_type,TreePath,LeafNodeTag>
        {

          typedef typename accumulate_type_node_helper<LeafNode,Functor,Reduction,current_type,TreePath,Functor::template doVisit<LeafNode,TreePath>::value>::type type;

        };

        //! Iteration over children of a composite node.
        template<typename Node, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath, std::size_t i, std::size_t n>
        struct accumulate_type_over_children
        {

          typedef typename TreePathPushBack<TreePath,i>::type child_tree_path;

          typedef typename Node::template Child<i>::Type child;

          typedef typename accumulate_type<child,Functor,Reduction,ParentChildReduction,current_type,child_tree_path,typename child::NodeTag>::type child_result_type;

          typedef typename accumulate_type_over_children<Node,Functor,Reduction,ParentChildReduction,child_result_type,TreePath,i+1,n>::type type;

        };

        //! end of iteration.
        template<typename Node, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath, std::size_t n>
        struct accumulate_type_over_children<Node,Functor,Reduction,ParentChildReduction,current_type,TreePath,n,n>
        {

          typedef current_type type;

        };

        //! Generic composite node specialization. We are doing the calculation at compile time and thus have to use static iteration for
        //! the PowerNode as well.
        template<typename Node, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath>
        struct accumulate_type_generic_composite_node
        {

          typedef typename accumulate_type_over_children<Node,Functor,Reduction,ParentChildReduction,current_type,TreePath,0,Node::CHILDREN>::type children_result_type;

          typedef typename accumulate_type_node_helper<Node,Functor,ParentChildReduction,children_result_type,TreePath,Functor::template doVisit<Node,TreePath>::value>::type type;


        };

        //! PowerNode specialization.
        template<typename PowerNode, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath>
        struct accumulate_type<PowerNode,Functor,Reduction,ParentChildReduction,current_type,TreePath,PowerNodeTag>
          : public accumulate_type_generic_composite_node<PowerNode,Functor,Reduction,ParentChildReduction,current_type,TreePath>
        {};

        //! CompositeNode specialization.
        template<typename CompositeNode, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath>
        struct accumulate_type<CompositeNode,Functor,Reduction,ParentChildReduction,current_type,TreePath,CompositeNodeTag>
          : public accumulate_type_generic_composite_node<CompositeNode,Functor,Reduction,ParentChildReduction,current_type,TreePath>
        {};

#if HAVE_VARIADIC_TEMPLATES

        //! VariadicCompositeNode specialization.
        template<typename VariadicCompositeNode, typename Functor, typename Reduction, typename ParentChildReduction, typename current_type, typename TreePath>
        struct accumulate_type<VariadicCompositeNode,Functor,Reduction,ParentChildReduction,current_type,TreePath,VariadicCompositeNodeTag>
          : public accumulate_type_generic_composite_node<VariadicCompositeNode,Functor,Reduction,ParentChildReduction,current_type,TreePath>
        {};

#endif // HAVE_VARIADIC_TEMPLATES

      } // anonymous namespace

      //! Statically accumulate a type over the nodes of a TypeTree.
      /**
       * This struct implements an algorithm for iterating over a tree and
       * calculating an accumulated type at compile time.
       *
       * \tparam Tree        The tree to iterate over.
       * \tparam Functor     The compile-time functor used for visiting each node.
       *
       * This functor must implement the following interface:
       *
       * \code
       * struct AccumulationFunctor
       * {
       *
       *   // Decide whether to include the given node in the calculation
       *   // or to skip it.
       *   template<typename Node, typename TreePath>
       *   struct doVisit
       *   {
       *     static const bool value = true;
       *   };
       *
       *   // Calculate the per-node result.
       *   template<typename Node, typename TreePath>
       *   struct visit
       *   {
       *     typedef ... type;
       *   };
       *
       * };
       * \endcode
       *
       * \tparam Reduction   The reduction operator used to accumulate the per-node
       *                     results.
       *
       * The reduction operator must implement the following interface:
       *
       * \code
       * struct ReductionOperator
       * {
       *
       *   // combine two per-node results
       *   template<typename T1, typename T2>
       *   struct reduce
       *   {
       *     typedef ... type;
       *   };
       *
       * };
       * \endcode
       *
       * \tparam startType  The start type fed into the initial accumulation step.
       */
      template<typename Tree, typename Functor, typename Reduction, typename startType, typename ParentChildReduction = Reduction>
      struct AccumulateType
      {

        //! The accumulated result of the computation.
        typedef typename accumulate_type<Tree,Functor,Reduction,ParentChildReduction,startType,TreePath<>,typename Tree::NodeTag>::type type;

      };

      //! \} group Tree Traversal

    } // namespace TypeTree
  } // namespace PDELab
} //namespace Dune

#endif // DUNE_PDELAB_COMMON_TYPETREE_ACCUMULATE_STATIC_HH
