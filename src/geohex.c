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
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "geohex/geohex.h"

#define H_KEY   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define H_BASE  20037508.34
#define H_DEG   M_PI * (30.0 / 180.0)
#define H_K     tan(H_DEG)

static inline bool is_in_upper_triangle(double h_x_q, double h_y_q) {
    return h_y_q > -h_x_q + 1 &&
           h_y_q < 2 * h_x_q &&
           h_y_q > 0.5 * h_x_q;
}

static inline bool is_in_lower_triangle(double h_x_q, double h_y_q) {
    return h_y_q < -h_x_q + 1 &&
           h_y_q > (2 * h_x_q) - 1 &&
           h_y_q < (0.5 * h_x_q) + 0.5;
}

static inline int index_of(const char *str, char c) {
    char *ptr = strchr(str, c);
    return ptr ? (int) (ptr - str) : -1;
}

double calc_hex_size(uint32_t level) {
    return H_BASE / pow(3, level + 3);
}

void loc2xy(double lat, double lon, xy_t *out) {
    if (out == NULL) {
        return;
    }

    out->x = lon * H_BASE / 180.0;
    out->y = log(tan((90.0 + lat) * M_PI / 360.0)) / (M_PI / 180.0);
    out->y *= H_BASE / 180.0;
    out->rev = false;
}

void xy2loc(double x, double y, loc_t *out) {
    if (out == NULL) {
        return;
    }

    out->lat = 180.0 / M_PI * (2.0 * atan(exp(((y / H_BASE) * 180.0) * M_PI / 180.0)) - M_PI / 2.0);
    out->lon = (x / H_BASE) * 180.0;
}

bool adjust_xy(double x, double y, uint32_t level, xy_t *out) {
    double max_h_steps;
    double h_steps;
    double dif, dif_x, dif_y;

    if (out == NULL) {
        return false;
    }

    max_h_steps = pow(3, level + 2);
    h_steps = fabs(x - y);

    if (h_steps == max_h_steps && x > y) {
        out->x = y;
        out->y = x;
        out->rev = true;
    } else if (h_steps > max_h_steps) {
        dif = h_steps - max_h_steps;
        dif_x = floor(dif / 2);
        dif_y = dif - dif_x;

        if (x > y) {
            out->x = y + dif_y + dif_x;
            out->y = x - dif_x - dif_y;
        } else {
            out->x = y - dif_y - dif_x;
            out->y = x + dif_x + dif_y;
        }
        out->rev = false;
    } else {
        out->x = x;
        out->y = y;
        out->rev = false;
    }

    return true;
}

bool get_xy_by_location(const loc_t *location, uint32_t level, xy_t *out) {
    double h_size, unit_x, unit_y;
    double lon_grid, lat_grid;
    double h_pos_x, h_pos_y;
    double h_x_0, h_y_0;
    double h_x_q, h_y_q;
    double h_x, h_y;
    xy_t z_xy, inner_xy;

    h_size = calc_hex_size(level);
    unit_x = 6 * h_size;
    unit_y = 6 * h_size * H_K;

    loc2xy(location->lat, location->lon, &z_xy);
    lon_grid = z_xy.x;
    lat_grid = z_xy.y;

    h_pos_x = (lon_grid + lat_grid / H_K) / unit_x;
    h_pos_y = (lat_grid - H_K * lon_grid) / unit_y;

    h_x_0 = floor(h_pos_x);
    h_y_0 = floor(h_pos_y);
    h_x_q = h_pos_x - h_x_0;
    h_y_q = h_pos_y - h_y_0;

    h_x = round(h_pos_x);
    h_y = round(h_pos_y);

    if (is_in_upper_triangle(h_x_q, h_y_q)) {
        h_x = h_x_0 + 1;
        h_y = h_y_0 + 1;
    } else if (is_in_lower_triangle(h_x_q, h_y_q)) {
        h_x = h_x_0;
        h_y = h_y_0;
    }

    adjust_xy(h_x, h_y, level, &inner_xy);

    *out = inner_xy;
    return true;
}

void convert_code(int *code3_x, int *code3_y, int count, char *result) {
    char h_code[GEOHEX_CODE_LENGTH] = "";
    char temp[GEOHEX_CODE_LENGTH];
    int code3 = 0, code9 = 0;

    for (int i = 0; i < count; i++) {
        code3 += code3_x[i] + code3_y[i];
        sprintf(temp, "%d", code3);
        code9 += (int)strtol(temp, NULL, 3);
        sprintf(temp, "%d", code9);
        strcat(h_code, temp);
        code9 = 0;
        code3 = 0;
    }

    int h_1 = atoi(strncpy(temp, h_code, 3));
    int h_a1 = h_1 / 30;
    int h_a2 = h_1 % 30;

    result[0] = H_KEY[h_a1];
    result[1] = H_KEY[h_a2];
    strcpy(result + 2, h_code + 3);
}

bool get_zone_by_xy(const xy_t *xy, uint32_t level, zone_t *out) {
    double h_size, h_x, h_y, unit_x, unit_y, h_lat, h_lon, z_loc_x, z_loc_y, mod_x, mod_y, h_pow;
    double max_hsteps, hsteps;
    int h_1_int, h_a1, h_a2;
    int code3_x[GEOHEX_CODE_LENGTH + 3] = {0};
    int code3_y[GEOHEX_CODE_LENGTH + 3] = {0};
    char h_code[GEOHEX_CODE_LENGTH] = "";
    char code3[3] = "";
    char code9[2] = "";
    char h_2[GEOHEX_CODE_LENGTH - 3];
    char h_1[4];
    char final_h_code[GEOHEX_CODE_LENGTH];
    loc_t z_loc;

    if (out == NULL) {
        return false;
    }

    h_size = calc_hex_size(level);
    h_x = xy->x;
    h_y = xy->y;

    unit_x = 6.0 * h_size;
    unit_y = 6.0 * h_size * H_K;

    h_lat = (H_K * h_x * unit_x + h_y * unit_y) / 2;
    h_lon = (h_lat - h_y * unit_y) / H_K;

    xy2loc(h_lon, h_lat, &z_loc);
    z_loc_x = z_loc.lon;
    z_loc_y = z_loc.lat;

    max_hsteps = pow(3.0, (double) level + 2.0);
    hsteps = fabs(h_x - h_y);

    if (hsteps == max_hsteps) {
        if (h_x > h_y) {
            double tmp = h_x;
            h_x = h_y;
            h_y = tmp;
        }
        z_loc_x = -180.0;
    }

    mod_x = h_x;
    mod_y = h_y;

    for (int i = 0; i <= level + 2; i++) {
        h_pow = pow(3.0, (double) level + 2.0 - (double) i);
        if (mod_x >= ceil(h_pow / 2.0)) {
            code3_x[i] = 2;
            mod_x -= h_pow;
        } else if (mod_x <= -ceil(h_pow / 2.0)) {
            code3_x[i] = 0;
            mod_x += h_pow;
        } else {
            code3_x[i] = 1;
        }
        if (mod_y >= ceil(h_pow / 2.0)) {
            code3_y[i] = 2;
            mod_y -= h_pow;
        } else if (mod_y <= -ceil(h_pow / 2.0)) {
            code3_y[i] = 0;
            mod_y += h_pow;
        } else {
            code3_y[i] = 1;
        }
        if (i == 2 && (z_loc_x == -180.0|| z_loc_x >= 0)) {
            if (code3_x[0] == 2 && code3_y[0] == 1 && code3_x[1] == code3_y[1] && code3_x[2] == code3_y[2]) {
                code3_x[0] = 1;
                code3_y[0] = 2;
            } else if (code3_x[0] == 1 && code3_y[0] == 0 && code3_x[1] == code3_y[1] && code3_x[2] == code3_y[2]) {
                code3_x[0] = 0;
                code3_y[0] = 1;
            }
        }
    }

    for (int i = 0; i < level + 3; i++) {
        snprintf(code3, sizeof(code3), "%d%d", code3_x[i], code3_y[i]);
        int code9_int = strtol(code3, NULL, 3);
        snprintf(code9, sizeof(code9), "%d", code9_int);
        strcat(h_code, code9);
    }

    strncpy(h_2, h_code + 3, sizeof(h_2) - 1);
    h_2[sizeof(h_2) - 1] = '\0';

    strncpy(h_1, h_code, 3);
    h_1[3] = '\0';

    h_1_int = atoi(h_1);
    h_a1 = h_1_int / 30;
    h_a2 = h_1_int % 30;

    snprintf(final_h_code, sizeof(final_h_code), "%c%c%s", H_KEY[h_a1], H_KEY[h_a2], h_2);

    out->latlon.lat = z_loc_y;
    out->latlon.lon = z_loc_x;
    out->xy.x = xy->x;
    out->xy.y = xy->y;
    strncpy(out->code, final_h_code, GEOHEX_CODE_LENGTH - 1);
    out->code[GEOHEX_CODE_LENGTH - 1] = '\0';

    return true;
}

bool get_xy_by_code(const geohex_code_t code, xy_t *out) {
    if (out == NULL || strlen(code) < 2) {
        return false;
    }

    uint32_t level = strlen(code) - 2;
    double h_size = calc_hex_size(level);
    double unit_x = 6 * h_size;
    double unit_y = 6 * h_size * H_K;
    int64_t h_x = 0;
    int64_t h_y = 0;

    char h_dec9[GEOHEX_CODE_LENGTH * 2] = {0};
    int h_dec9_len = snprintf(h_dec9, sizeof(h_dec9), "%d%s",
             index_of(H_KEY, code[0]) * 30 + index_of(H_KEY, code[1]),
             code + 2);

    if ((h_dec9[0] == '1' || h_dec9[0] == '5') &&
        (h_dec9[1] < '2' || h_dec9[1] > '5') &&
        (h_dec9[2] < '2' || h_dec9[2] > '5')) {
        h_dec9[0] = (h_dec9[0] == '5') ? '7' : '3';
    }

    char h_dec3[GEOHEX_CODE_LENGTH * 3] = {0};
    int h_dec3_pos = 0;
    for (int i = 0; i < h_dec9_len; i++) {
        int dec = h_dec9[i] - '0';
        h_dec3[h_dec3_pos++] = (dec / 3) + '0';
        h_dec3[h_dec3_pos++] = (dec % 3) + '0';
    }

    for (int i = 0; i < strlen(h_dec3); i += 2) {
        int64_t h_pow = (int64_t) pow(3, level + 2 - i/2);
        if (h_dec3[i] != '1')   h_x += (h_dec3[i] == '0') ? -h_pow : h_pow;
        if (h_dec3[i+1] != '1') h_y += (h_dec3[i+1] == '0') ? -h_pow : h_pow;
    }

    xy_t inner_xy;
    if (!adjust_xy(h_x, h_y, level, &inner_xy)) {
        return false;
    }

    out->x = inner_xy.x;
    out->y = inner_xy.y;
    out->rev = inner_xy.rev;

    return true;
}

bool get_zone_by_location(const loc_t *location, uint32_t level, zone_t *out) {
    if (location == NULL || out == NULL) {
        return false;
    }

    xy_t xy;

    if (!get_xy_by_location(location, level, &xy) || !get_zone_by_xy(&xy, level, out)) {
        return false;
    }

    return true;
}

bool get_zone_by_code(const geohex_code_t code, zone_t *out) {
    if (out == NULL || strlen(code) < 2) {
        return false;
    }

    xy_t xy;
    uint32_t level = strlen(code) - 2;
    zone_t zone;

    if (!get_xy_by_code(code, &xy) || !get_zone_by_xy(&xy, level, out)) {
        return false;
    }

    return true;
}
