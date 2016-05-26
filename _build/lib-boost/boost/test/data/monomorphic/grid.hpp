//  (C) Copyright Gennadiy Rozental 2001.
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/test for the library home page.
//
///@file
/// Defines monomorphic dataset n+m dimentional *. Samples in this
/// dataset is grid of elements in DataSet1 and DataSet2. There will be total
/// |DataSet1| * |DataSet2| samples
// ***************************************************************************

#ifndef BOOST_TEST_DATA_MONOMORPHIC_GRID_HPP_101512GER
#define BOOST_TEST_DATA_MONOMORPHIC_GRID_HPP_101512GER

// Boost.Test
#include <boost/test/data/config.hpp>

#if !defined(BOOST_TEST_NO_GRID_COMPOSITION_AVAILABLE) || defined(BOOST_TEST_DOXYGEN_DOC__)

#include <boost/test/data/traits.hpp>
#include <boost/test/data/monomorphic/fwd.hpp>

#include <boost/test/detail/suppress_warnings.hpp>

//____________________________________________________________________________//

namespace boost {
namespace unit_test {
namespace data {
namespace monomorphic {

namespace ds_detail {

// !! ?? variadic template implementation; use forward_as_tuple?
template<typename T1, typename T2>
struct grid_traits {
    typedef std::tuple<T1,T2> type;
    typedef typename data::traits<type>::ref_type ref_type;

    static ref_type
    tuple_merge(T1 const& a1, T2 const& a2)
    {
        return ref_type(a1,a2);
    }
};

//____________________________________________________________________________//

template<typename T1, typename T2,typename T3>
struct grid_traits<T1,std::tuple<T2,T3>> {
    typedef std::tuple<T1,T2,T3> type;
    typedef typename data::traits<type>::ref_type ref_type;

    static ref_type
    tuple_merge(T1 const& a1, std::tuple<T2 const&,T3 const&> const& a2)
    {
        return ref_type(a1,get<0>(a2),get<1>(a2));
    }
};

//____________________________________________________________________________//

template<typename T1, typename T2,typename T3>
struct grid_traits<std::tuple<T1,T2>,T3> {
    typedef std::tuple<T1,T2,T3> type;
    typedef typename data::traits<type>::ref_type ref_type;

    static ref_type
    tuple_merge(std::tuple<T1 const&,T2 const&> const& a1, T3 const& a2)
    {
        return ref_type(get<0>(a1),get<1>(a1),a2);
    }
};

//____________________________________________________________________________//

} // namespace ds_detail

// ************************************************************************** //
// **************                       grid                    ************** //
// ************************************************************************** //


//! Implements the dataset resulting from a cartesian product/grid operation on datasets.
//!
//! The arity of the resulting dataset is the sum of the arity of its operands.
template<typename DataSet1, typename DataSet2>
class grid {
    typedef typename boost::decay<DataSet1>::type dataset1_decay;
    typedef typename boost::decay<DataSet2>::type dataset2_decay;

    typedef typename dataset1_decay::iterator     dataset1_iter;
    typedef typename dataset2_decay::iterator     dataset2_iter;

    typedef typename dataset1_decay::sample    sample1;
    typedef typename dataset2_decay::sample    sample2;

public:
    typedef typename ds_detail::grid_traits<sample1,sample2>::type sample;

    struct iterator {
        typedef typename data::traits<sample>::ref_type ref_type;

        // Constructor
        explicit    iterator( dataset1_iter iter1, DataSet2 const& ds2 )
        : m_iter1( std::move( iter1 ) )
        , m_iter2( std::move( ds2.begin() ) )
        , m_ds2( &ds2 )
        , m_ds2_pos( 0 )
        {}

        // forward iterator interface
        ref_type        operator*() const { return ds_detail::grid_traits<sample1,sample2>::tuple_merge( *m_iter1, *m_iter2 ); }
        void            operator++()
        {
            ++m_ds2_pos;
            if( m_ds2_pos != m_ds2->size() )
                ++m_iter2;
            else {
                m_ds2_pos = 0;
                ++m_iter1;
                m_iter2 = std::move( m_ds2->begin() );
            }
        }

    private:
        // Data members
        dataset1_iter   m_iter1;
        dataset2_iter   m_iter2;
        dataset2_decay const* m_ds2;
        data::size_t    m_ds2_pos;
    };

public:
    enum { arity = boost::decay<DataSet1>::type::arity + boost::decay<DataSet2>::type::arity };

    //! Constructor
    grid( DataSet1&& ds1, DataSet2&& ds2 )
    : m_ds1( std::forward<DataSet1>( ds1 ) )
    , m_ds2( std::forward<DataSet2>( ds2 ) )
    {}

    //! Move constructor
    grid( grid&& j )
    : m_ds1( std::forward<DataSet1>( j.m_ds1 ) )
    , m_ds2( std::forward<DataSet2>( j.m_ds2 ) )
    {}

    // dataset interface
    data::size_t    size() const    { return m_ds1.size() * m_ds2.size(); }
    iterator        begin() const   { return iterator( m_ds1.begin(), m_ds2 ); }

private:
    // Data members
    DataSet1             m_ds1;
    DataSet2             m_ds2;
};

//____________________________________________________________________________//

// A grid dataset is a dataset
template<typename DataSet1, typename DataSet2>
struct is_dataset<grid<DataSet1,DataSet2>> : mpl::true_ {};

//____________________________________________________________________________//

namespace result_of {

/// Result type of the grid operation on dataset.
template<typename DS1Gen, typename DS2Gen>
struct grid {
    typedef monomorphic::grid<typename DS1Gen::type,typename DS2Gen::type> type;
};

} // namespace result_of

//____________________________________________________________________________//

//! Grid operation
template<typename DataSet1, typename DataSet2>
inline typename boost::lazy_enable_if_c<is_dataset<DataSet1>::value && is_dataset<DataSet2>::value,
                                        result_of::grid<mpl::identity<DataSet1>,mpl::identity<DataSet2>>
>::type
operator*( DataSet1&& ds1, DataSet2&& ds2 )
{
    BOOST_TEST_DS_ASSERT( !ds1.size().is_inf() && !ds2.size().is_inf(), "Grid axes can't have infinite size" );

    return grid<DataSet1,DataSet2>( std::forward<DataSet1>( ds1 ),  std::forward<DataSet2>( ds2 ) );
}

//____________________________________________________________________________//

//! @overload boost::unit_test::data::operator*
template<typename DataSet1, typename DataSet2>
inline typename boost::lazy_enable_if_c<is_dataset<DataSet1>::value && !is_dataset<DataSet2>::value,
                                        result_of::grid<mpl::identity<DataSet1>,data::result_of::make<DataSet2>>
>::type
operator*( DataSet1&& ds1, DataSet2&& ds2 )
{
    return std::forward<DataSet1>(ds1) * data::make(std::forward<DataSet2>(ds2));
}

//____________________________________________________________________________//

//! @overload boost::unit_test::data::operator*
template<typename DataSet1, typename DataSet2>
inline typename boost::lazy_enable_if_c<!is_dataset<DataSet1>::value && is_dataset<DataSet2>::value,
                                        result_of::grid<data::result_of::make<DataSet1>,mpl::identity<DataSet2>>
>::type
operator*( DataSet1&& ds1, DataSet2&& ds2 )
{
    return data::make(std::forward<DataSet1>(ds1)) * std::forward<DataSet2>(ds2);
}

} // namespace monomorphic

} // namespace data
} // namespace unit_test
} // namespace boost

#include <boost/test/detail/enable_warnings.hpp>

#endif // BOOST_TEST_NO_GRID_COMPOSITION_AVAILABLE

#endif // BOOST_TEST_DATA_MONOMORPHIC_GRID_HPP_101512GER

