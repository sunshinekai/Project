.. Algorithms/Transformation Algorithms//reverse_remove_if |170

reverse_remove_if
=================

Synopsis
--------

.. parsed-literal::
    
    template<
          typename Sequence
        , typename Pred
        , typename In = |unspecified|
        >
    struct reverse_remove_if
    {
        typedef |unspecified| type;
    };


Description
-----------

Returns a new sequence that contains all the elements from |begin/end<Sequence>| range 
in reverse order except those that satisfy the predicate ``Pred``.

|transformation algorithm disclaimer|

Header
------

.. parsed-literal::
    
    #include <boost/mpl/remove_if.hpp>


Model of
--------

|Reversible Algorithm|


Parameters
----------

+---------------+-----------------------------------+-------------------------------+
| Parameter     | Requirement                       | Description                   |
+===============+===================================+===============================+
| ``Sequence``  | |Forward Sequence|                | An original sequence.         |
+---------------+-----------------------------------+-------------------------------+
| ``Pred``      | Unary |Lambda Expression|         | A removal condition.          |
+---------------+-----------------------------------+-------------------------------+
| ``In``        | |Inserter|                        | An inserter.                  |
+---------------+-----------------------------------+-------------------------------+


Expression semantics
--------------------

|Semantics disclaimer...| |Reversible Algorithm|.

For any |Forward Sequence| ``s``, and an |Inserter| ``in``, and an unary 
|Lambda Expression| ``pred``:


.. parsed-literal::

    typedef reverse_remove_if<s,pred,in>::type r; 

:Return type:
    A type.

:Semantics:
    Equivalent to 

    .. parsed-literal::

        typedef lambda<pred>::type p;
        typedef lambda<in::operation>::type op;
        
        typedef reverse_fold<
              s
            , in::state
            , eval_if<
                  apply_wrap\ ``1``\<p,_2>
                , identity<_1>
                , apply_wrap\ ``2``\<op,_1,_2>
                >
            >::type r;


Complexity
----------

Linear. Performs exactly ``size<s>::value`` applications of ``pred``, and at 
most ``size<s>::value`` insertions.


Example
-------

.. parsed-literal::
    
    typedef vector_c<int,1,4,5,2,7,5,3,5>::type numbers;
    typedef reverse_remove_if< numbers, greater<_,int_<4> > >::type result;
    
    BOOST_MPL_ASSERT(( equal< result,vector_c<int,3,2,4,1>,equal_to<_,_> > ));


See also
--------

|Transformation Algorithms|, |Reversible Algorithm|, |remove_if|, |reverse_remove|, 
|reverse_copy_if|, |replace_if|


.. copyright:: Copyright ?  2001-2009 Aleksey Gurtovoy and David Abrahams
   Distributed under the Boost Software License, Version 1.0. (See accompanying
   file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
