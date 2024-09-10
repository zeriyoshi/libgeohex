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
#include "code2hex_data.h"

void setUp(void) {}
void tearDown(void) {}

void test_calc_hex_size(void)
{
    uint32_t i;
    const double cases[] = {
        247376.6461728395,
        82458.88205761317,
        27486.29401920439,
        9162.098006401464,
        3054.0326688004875,
        1018.0108896001626,
        339.3369632000542,
        113.11232106668473,
        37.70410702222824,
        12.56803567407608,
        4.189345224692027,
        1.3964484082306756,
        0.4654828027435586,
        0.15516093424785285,
        0.05172031141595095,
        0.017240103805316983,
        0.005746701268438995
    };

    for (i = 0; i < (sizeof(cases) / sizeof(cases[0])); i++) {
        TEST_ASSERT_DOUBLE_WITHIN(15, cases[i], calc_hex_size(i + 1));
    }
}

void test_loc2xy(void)
{
    xy_t r;

    loc2xy(35.65858, 139.745433, &r);

    TEST_ASSERT_DOUBLE_WITHIN(15, 15556390.440080063, r.x);
    TEST_ASSERT_DOUBLE_WITHIN(15, 4253743.631945749, r.y);
    TEST_ASSERT_FALSE(r.rev);
}

void test_xy2loc(void)
{
    loc_t r;

    xy2loc(15556390.440080063, 4253743.631945749, &r);

    TEST_ASSERT_DOUBLE_WITHIN(15, 35.65858, r.lat);
    TEST_ASSERT_DOUBLE_WITHIN(15, 139.745433, r.lon);
}

void test_adjust_xy(void)
{
    xy_t r;

    adjust_xy(15556390.440080063, 4253743.631945749, 1, &r);
    TEST_ASSERT_DOUBLE_WITHIN(15, 15556363.440080062, r.x);
    TEST_ASSERT_DOUBLE_WITHIN(15, 4253770.63194575, r.y);
    TEST_ASSERT_FALSE(r.rev);

    adjust_xy(15556390.440080063, 4253743.631945749, 17, &r);
    TEST_ASSERT_DOUBLE_WITHIN(15, 15556390.440080063, r.x);
    TEST_ASSERT_DOUBLE_WITHIN(15, 4253743.631945749, r.y);
    TEST_ASSERT_FALSE(r.rev);

    TEST_ASSERT_TRUE(true);
}

void test_get_xy_by_location(void)
{
    loc_t location = {.lat = 35.65858, .lon = 139.745433};
    xy_t r;

    TEST_ASSERT_TRUE(get_xy_by_location(&location, 11, &r));
    TEST_ASSERT_DOUBLE_WITHIN(15, 912000.0, r.x);
    TEST_ASSERT_DOUBLE_WITHIN(15, -325774.0, r.y);
}

void test_get_zone_by_xy(void)
{
    xy_t xy = {.x = 912000.0, .y = -325774.0, .rev = false};
    zone_t r;

    TEST_ASSERT_TRUE(get_zone_by_xy(&xy, 11, &r));
    TEST_ASSERT_EQUAL_STRING("XM48854457273", r.code);
    TEST_ASSERT_DOUBLE_WITHIN(15, 35.658618718910624, r.latlon.lat);
    TEST_ASSERT_DOUBLE_WITHIN(15, 139.7454091799466, r.latlon.lon);
    TEST_ASSERT_DOUBLE_WITHIN(15, 912000.0, r.xy.x);
    TEST_ASSERT_DOUBLE_WITHIN(15, -325774.0, r.xy.y);
}

void test_get_xy_by_code(void)
{
    xy_t r;

    TEST_ASSERT_TRUE(get_xy_by_code("XM48854457273", &r));
    TEST_ASSERT_DOUBLE_WITHIN(15, 912000.0, r.x);
    TEST_ASSERT_DOUBLE_WITHIN(15, -325774.0, r.y);
    TEST_ASSERT_FALSE(r.rev);
}

void test_get_zone_by_location(void)
{
    loc_t location;
    zone_t r;
    uint32_t i;

    for (i = 0; i < (sizeof(entries) / sizeof(entries[0])); i++) {
        location.lat = entries[i].lat;
        location.lon = entries[i].lon;

        TEST_ASSERT_TRUE(get_zone_by_location(&location, strlen(entries[i].code) - 2, &r));
        TEST_ASSERT_EQUAL_STRING(entries[i].code, r.code);
    }
}

void test_get_zone_by_code(void)
{
    zone_t r;
    uint32_t i;

    for (i = 0; i < (sizeof(entries) / sizeof(entries[0])); i++) {
        char *msg;

        sprintf(msg, "%s|%s", entries[i].code, r.code);

        TEST_ASSERT_TRUE(get_zone_by_code(entries[i].code, &r));
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(entries[i].lat, r.latlon.lat, msg);
        TEST_ASSERT_EQUAL_DOUBLE_MESSAGE(entries[i].lon, r.latlon.lon, msg);
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
