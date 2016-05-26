// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010-2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

// If defined, tests are run without rescaling-to-integer or robustness policy
// This multi_union currently contains no tests for double which then fail
// #define BOOST_GEOMETRY_NO_ROBUSTNESS

#include "test_union.hpp"
#include <algorithms/test_overlay.hpp>
#include <algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/read.hpp>


template <typename Ring, typename Polygon, typename MultiPolygon>
void test_areal()
{
    test_one<Polygon, MultiPolygon, MultiPolygon>("simplex_multi",
        case_multi_simplex[0], case_multi_simplex[1],
        1, 0, 20, 14.58);

    test_one<Polygon, Polygon, MultiPolygon>("simplex_multi_p_mp",
        case_single_simplex, case_multi_simplex[0],
        1, 0, 20, 14.58);
    test_one<Polygon, MultiPolygon, Polygon>("simplex_multi_mp_p",
        case_multi_simplex[0], case_single_simplex,
        1, 0, 20, 14.58);

    test_one<Polygon, Ring, MultiPolygon>("simplex_multi_r_mp",
        case_single_simplex, case_multi_simplex[0],
        1, 0, 20, 14.58);
    test_one<Ring, MultiPolygon, Polygon>("simplex_multi_mp_r",
        case_multi_simplex[0], case_single_simplex,
        1, 0, 20, 14.58);


    // Normal test cases
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_no_ip",
        case_multi_no_ip[0], case_multi_no_ip[1],
        4, 0, 16, 66.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_2",
        case_multi_2[0], case_multi_2[1],
        3, 0, 16, 59.1);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_58_multi_a",
        case_58_multi[0], case_58_multi[3],
        2, 0, 21, 19.83333333);

    // (#holes should be 3)
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_58_multi_b",
        case_58_multi[1], case_58_multi[2],
        1, 1, 17, 48.333333);

    // Constructed cases for multi/touch/equal/etc
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_61_multi",
        case_61_multi[0], case_61_multi[1],
        1, 0, 11, 4.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_62_multi",
        case_62_multi[0], case_62_multi[1],
        2, 0, 10, 2.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_63_multi",
        case_63_multi[0], case_63_multi[1],
        2, 0, 10, 2.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_64_multi",
        case_64_multi[0], case_64_multi[1],
        1, 0, 9, 3.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_65_multi",
        case_65_multi[0], case_65_multi[1],
        3, 0, 15, 4.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_66_multi",
        case_66_multi[0], case_66_multi[1],
        3, 0, 23, 7.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_72_multi",
        case_72_multi[0], case_72_multi[1],
        1, 0, 13, 10.65);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_75_multi",
        case_75_multi[0], case_75_multi[1],
        5, 0, 25, 5.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_76_multi",
        case_76_multi[0], case_76_multi[1],
        4, 0, 31, 8.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_89_multi",
        case_89_multi[0], case_89_multi[1],
        1, 0, 13, 6);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_101_multi",
        case_101_multi[0], case_101_multi[1],
        1, 0, 32, 22.25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_103_multi",
        case_103_multi[0], case_103_multi[1],
        1, 0, 7, 25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_104_multi",
        case_104_multi[0], case_104_multi[1],
        1, 0, 8, 25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_105_multi",
        case_105_multi[0], case_105_multi[1],
        1, 0, 5, 25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_106_multi",
        case_106_multi[0], case_106_multi[1],
        1, 0, 12, 25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_107_multi",
        case_107_multi[0], case_107_multi[1],
        1, 0, 15, 6.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_108_multi",
        case_108_multi[0], case_108_multi[1],
        1, 0, 20, 22.75);

    // Should have 2 holes
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_109_multi",
        case_109_multi[0], case_109_multi[1],
        1, 1, 14, 1400);

    // Should have 9 holes, they are all separate and touching
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_110_multi",
       case_110_multi[0], case_110_multi[1],
       1, 1, 37, 1250);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_1",
        case_recursive_boxes_1[0], case_recursive_boxes_1[1],
        1, 1, 36, 97.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_2",
        case_recursive_boxes_2[0], case_recursive_boxes_2[1],
        1, 0, 14, 100.0); // Area from SQL Server
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_3",
        case_recursive_boxes_3[0], case_recursive_boxes_3[1],
        14, 0, 159, 56.5); // Area from SQL Server

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_4",
        case_recursive_boxes_4[0], case_recursive_boxes_4[1],
        1, 1, 42, 96.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_5",
        case_recursive_boxes_5[0], case_recursive_boxes_5[1],
        3, 2, 110, 70.0);

    // TODO: fix self touching interior ring (should get 3 interior rings)
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_6",
        case_recursive_boxes_6[0], case_recursive_boxes_6[1],
        1, 2, 25, 24.0);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_7",
        case_recursive_boxes_7[0], case_recursive_boxes_7[1],
        2, 0, 20, 7.0);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_8",
        case_recursive_boxes_8[0], case_recursive_boxes_8[1],
        1, 0, 13, 12.0);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_9",
        case_recursive_boxes_9[0], case_recursive_boxes_9[1],
        1, 1, 16, 8.25);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_10",
        case_recursive_boxes_10[0], case_recursive_boxes_10[1],
            1, 0, -1, 2.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_11",
        case_recursive_boxes_11[0], case_recursive_boxes_11[1],
            1, 0, -1, 8.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_12",
        case_recursive_boxes_12[0], case_recursive_boxes_12[1],
            3, 0, -1, 6.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_13",
        case_recursive_boxes_13[0], case_recursive_boxes_13[1],
            3, 0, -1, 10.25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_14",
        case_recursive_boxes_14[0], case_recursive_boxes_14[1],
            4, 0, -1, 4.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_15",
        case_recursive_boxes_15[0], case_recursive_boxes_15[1],
            2, 0, -1, 6.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_16",
        case_recursive_boxes_16[0], case_recursive_boxes_16[1],
            1, 1, -1, 22.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_17",
        case_recursive_boxes_17[0], case_recursive_boxes_17[1],
            3, 1, -1, 21.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_18",
        case_recursive_boxes_18[0], case_recursive_boxes_18[1],
            3, 0, -1, 2.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_19",
        case_recursive_boxes_19[0], case_recursive_boxes_19[1],
            3, 0, -1, 2.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_20",
        case_recursive_boxes_20[0], case_recursive_boxes_20[1],
            2, 0, -1, 2.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_21",
        case_recursive_boxes_21[0], case_recursive_boxes_21[1],
            1, 0, -1, 2.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_22",
        case_recursive_boxes_22[0], case_recursive_boxes_22[1],
            2, 0, -1, 3.25);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_23",
        case_recursive_boxes_23[0], case_recursive_boxes_23[1],
            3, 0, -1, 1.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_24",
        case_recursive_boxes_24[0], case_recursive_boxes_24[1],
            5, 0, -1, 5.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_25",
        case_recursive_boxes_25[0], case_recursive_boxes_25[1],
            2, 0, -1, 5.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_26",
        case_recursive_boxes_26[0], case_recursive_boxes_26[1],
            3, 0, -1, 6.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_27",
        case_recursive_boxes_27[0], case_recursive_boxes_27[1],
            4, 0, -1, 4.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_28",
        case_recursive_boxes_28[0], case_recursive_boxes_28[1],
            2, 0, -1, 6.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_29",
        case_recursive_boxes_29[0], case_recursive_boxes_29[1],
            1, 0, -1, 15.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_30",
        case_recursive_boxes_30[0], case_recursive_boxes_30[1],
            1, 2, -1, 17.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_31",
        case_recursive_boxes_31[0], case_recursive_boxes_31[1],
            3, 0, -1, 5.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_32",
        case_recursive_boxes_32[0], case_recursive_boxes_32[1],
            2, 0, -1, 5.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_33",
        case_recursive_boxes_33[0], case_recursive_boxes_33[1],
            1, 0, -1, 11.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_34",
        case_recursive_boxes_34[0], case_recursive_boxes_34[1],
            1, 0, -1, 25.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_35",
        case_recursive_boxes_35[0], case_recursive_boxes_35[1],
            1, 1, -1, 24.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_36",
        case_recursive_boxes_36[0], case_recursive_boxes_36[1],
            2, 0, -1, 3.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_37",
        case_recursive_boxes_37[0], case_recursive_boxes_37[1],
            2, 1, -1, 7.75);

    test_one<Polygon, MultiPolygon, MultiPolygon>("ggl_list_20120915_h2_a",
         ggl_list_20120915_h2[0], ggl_list_20120915_h2[1],
         1, 0, 12, 23.0); // Area from SQL Server
    test_one<Polygon, MultiPolygon, MultiPolygon>("ggl_list_20120915_h2_b",
         ggl_list_20120915_h2[0], ggl_list_20120915_h2[2],
         1, 0, 12, 23.0); // Area from SQL Server

    test_one<Polygon, MultiPolygon, MultiPolygon>("ggl_list_20140212_sybren",
         ggl_list_20140212_sybren[0], ggl_list_20140212_sybren[1],
         2, 0, 16, 0.002471626);

    test_one<Polygon, MultiPolygon, MultiPolygon>("ticket_9081",
        ticket_9081[0], ticket_9081[1],
#if defined(BOOST_GEOMETRY_NO_ROBUSTNESS)
        3,
#else
        4,
#endif
        0, 31, 0.2187385);

    test_one<Polygon, MultiPolygon, MultiPolygon>("ticket_10803",
        ticket_10803[0], ticket_10803[1],
        1, 0, 9, 2663736.07038);
    test_one<Polygon, MultiPolygon, MultiPolygon>("ticket_11984",
        ticket_11984[0], ticket_11984[1],
        1, 2, 134, 60071.08077);

#ifdef BOOST_GEOMETRY_ENABLE_FAILING_TESTS
    test_one<Polygon, MultiPolygon, MultiPolygon>("mysql_1",
        mysql_1[0], mysql_1[1],
        1, 1, 19, 99.19494);
#endif
}

// Test cases (generic)
template <typename Point, bool ClockWise, bool Closed>
void test_all()
{

    typedef bg::model::ring<Point, ClockWise, Closed> ring;
    typedef bg::model::polygon<Point, ClockWise, Closed> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;
    test_areal<ring, polygon, multi_polygon>();
}

// Test cases for integer coordinates / ccw / open
template <typename Point, bool ClockWise, bool Closed>
void test_specific()
{
    typedef bg::model::polygon<Point, ClockWise, Closed> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;

    ut_settings settings;
    settings.test_validity = true;

    test_one<polygon, multi_polygon, multi_polygon>("ticket_10803",
        ticket_10803[0], ticket_10803[1],
        1, 0, 9, 2664270, settings);
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double>, true, true>();
    test_all<bg::model::d2::point_xy<double>, false, false>();

    test_specific<bg::model::d2::point_xy<int>, false, false>();

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
    test_all<bg::model::d2::point_xy<float>, true, true>();

#if defined(HAVE_TTMATH)
    std::cout << "Testing TTMATH" << std::endl;
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

#endif

    return 0;
}
