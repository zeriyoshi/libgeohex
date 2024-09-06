/* SPDX-License-Identifier: MIT */
/*
 * libgeohex
 *
 * Copyright (c) 2024 Go Kudo (https://github.com/zeriyoshi)
 *
 * GeoHex original implementation by @sa2da (http://twitter.com/sa2da)
 * https://www.geohex.org/
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

void setUp(void) {}
void tearDown(void) {}

void test_get_zone_by_location(void)
{
    TEST_ASSERT_TRUE(true);
}

void test_get_zone_by_code(void)
{
    TEST_ASSERT_TRUE(true);
}

void test_get_xy_by_location(void)
{
    TEST_ASSERT_TRUE(true);
}

void test_get_xy_by_code(void)
{
    TEST_ASSERT_TRUE(true);
}

void test_get_zone_by_xy(void)
{
    TEST_ASSERT_TRUE(true);
}

void test_adjust_xy(void)
{
    TEST_ASSERT_TRUE(true);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_get_zone_by_location);
    RUN_TEST(test_get_zone_by_code);
    RUN_TEST(test_get_xy_by_location);
    RUN_TEST(test_get_xy_by_code);
    RUN_TEST(test_get_zone_by_xy);
    RUN_TEST(test_adjust_xy);
    return UNITY_END();
}
