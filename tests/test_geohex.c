/* SPDX-License-Identifier: MIT */
/*
 * libgeohex
 *
 * Copyright (c) 2024 Go Kudo (https://github.com/zeriyoshi)
 *
 * GeoHex original implementation by @sa2da (http://twitter.com/sa2da)
 * https://www.geohex.org/
 *
 * Original test case by HarikiTech (https://github.com/harikitech)
 * https://github.com/harikitech/py-geohex3/blob/master/test/geohex_test.py
 *
 * Released under the MIT license.
 * see https://opensource.org/licenses/MIT
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "unity.h"

#include "geohex/geohex.h"

#include "geohex_private.h"
#include "json_data.h"

void setUp(void) {}
void tearDown(void) {}

void test_calc_hex_size(void)
{
    uint32_t i;

    for (i = 0; i < (sizeof(calc_hex_size_data) / sizeof(calc_hex_size_data[0])); i++) {
        TEST_ASSERT_DOUBLE_WITHIN(15, calc_hex_size_data[i], calc_hex_size(i + 1));
    }
}

void test_loc2xy(void)
{
    double dx, dy;

    loc2xy(139.745433, 35.65858, &dx, &dy);

    TEST_ASSERT_DOUBLE_WITHIN(15, 15556390.440080063, dx);
    TEST_ASSERT_DOUBLE_WITHIN(15, 4253743.631945749, dy);
}

void test_xy2loc(void)
{
    double lat, lon;

    xy2loc(15556390.440080063, 4253743.631945749, &lon, &lat);

    TEST_ASSERT_DOUBLE_WITHIN(15, 35.65858, lat);
    TEST_ASSERT_DOUBLE_WITHIN(15, 139.745433, lon);
}

void test_adjust_xy(void)
{
    xy_t xy;

    adjust_xy(15556390, 4253743, 1, &xy);
    TEST_ASSERT_EQUAL_INT32(15556363, xy.x);
    TEST_ASSERT_EQUAL_INT32(4253770, xy.y);
    TEST_ASSERT_FALSE(xy.rev);

    adjust_xy(15556390, 4253743, 17, &xy);
    TEST_ASSERT_EQUAL_INT32(15556390, xy.x);
    TEST_ASSERT_EQUAL_INT32(4253743, xy.y);
    TEST_ASSERT_FALSE(xy.rev);
}

void test_get_xy_by_location(void)
{
    xy_t out;

    for (uint32_t i = 0; i < (sizeof(coord2xy_data) / sizeof(coord2xy_data[0])); i++) {
        loc_t loc = {
            .lat = coord2xy_data[i].lat,
            .lon = coord2xy_data[i].lon,
        };

        TEST_ASSERT_TRUE(get_xy_by_location(&loc, coord2xy_data[i].level, &out));
        TEST_ASSERT_DOUBLE_WITHIN(15, coord2xy_data[i].x, out.x);
        TEST_ASSERT_DOUBLE_WITHIN(15, coord2xy_data[i].y, out.y);
    }
}

void test_get_zone_by_xy(void)
{
    zone_t out;

    for (uint32_t i = 0; i < (sizeof(xy2hex_data) / sizeof(xy2hex_data[0])); i++) {
        xy_t xy = {
            .x = xy2hex_data[i].x,
            .y = xy2hex_data[i].y,
            .rev = false,
        };

        TEST_ASSERT_TRUE(get_zone_by_xy(&xy, xy2hex_data[i].level, &out));
        TEST_ASSERT_EQUAL_STRING(xy2hex_data[i].code, out.code);
    }
}

void test_get_xy_by_code(void)
{
    xy_t out;

    for (uint32_t i = 0; i < (sizeof(code2xy_data) / sizeof(code2xy_data[0])); i++) {
        TEST_ASSERT_TRUE(get_xy_by_code(code2xy_data[i].code, &out));
        TEST_ASSERT_EQUAL_INT32(code2xy_data[i].x, out.x);
        TEST_ASSERT_EQUAL_INT32(code2xy_data[i].y, out.y);
    }
}

void test_get_zone_by_location(void)
{
    zone_t out;

    for (uint32_t i = 0; i < (sizeof(code2hex_data) / sizeof(code2hex_data[0])); i++) {
        loc_t loc = {
            .lat = code2hex_data[i].lat,
            .lon = code2hex_data[i].lon,
        };

        TEST_ASSERT_TRUE(get_zone_by_location(&loc, strlen(code2hex_data[i].code) - 2, &out));
        TEST_ASSERT_EQUAL_STRING(code2hex_data[i].code, out.code);
    }

    for (uint32_t i = 0; i < sizeof(coord2hex_data) / sizeof(coord2hex_data[0]); i++) {
        loc_t loc = {
            .lat = coord2hex_data[i].lat,
            .lon = coord2hex_data[i].lon,
        };

        TEST_ASSERT_TRUE(get_zone_by_location(&loc, coord2hex_data[i].level, &out));
        TEST_ASSERT_EQUAL_STRING(coord2hex_data[i].code, out.code);
    }
}

void test_get_zone_by_code(void)
{
    zone_t out;

    for (uint32_t i = 0; i < (sizeof(code2hex_data) / sizeof(code2hex_data[0])); i++) {
        TEST_ASSERT_TRUE(get_zone_by_code(code2hex_data[i].code, &out));
        TEST_ASSERT_DOUBLE_WITHIN(15, code2hex_data[i].lat, out.latlon.lat);
        TEST_ASSERT_DOUBLE_WITHIN(15, code2hex_data[i].lon, out.latlon.lon);
    }
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_calc_hex_size);
    RUN_TEST(test_loc2xy);
    RUN_TEST(test_xy2loc);

    RUN_TEST(test_adjust_xy);
    RUN_TEST(test_get_xy_by_location);
    RUN_TEST(test_get_zone_by_xy);
    RUN_TEST(test_get_xy_by_code);
    RUN_TEST(test_get_zone_by_location);
    RUN_TEST(test_get_zone_by_code);

    return UNITY_END();
}
