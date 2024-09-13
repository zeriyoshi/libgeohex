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
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "geohex/geohex.h"

#define GEOHEX_KEY  "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
#define H_BASE      20037508.34
#define H_DEG       (M_PI / 6.0)
#define H_K         tan(H_DEG)

double calc_hex_size(uint32_t level) {
    return H_BASE / pow(3.0, level + 3);
}

void loc2xy(double lon, double lat, double *x, double *y) {
    *x = lon * H_BASE / 180.0;
    *y = log(tan((90.0 + lat) * M_PI / 360.0)) * (H_BASE / M_PI);
}

void xy2loc(double x, double y, double *lon, double *lat) {
    *lon = (x / H_BASE) * 180.0;
    *lat = (y / H_BASE) * 180.0;
    *lat = 180.0 / M_PI * (2.0 * atan(exp(*lat * M_PI / 180.0)) - M_PI / 2.0);
}

bool adjust_xy(double x, double y, uint32_t level, xy_t *out) {
    if (!out) {
        return false;
    }

    double max_hsteps = pow(3.0, level + 2);
    double hsteps = fabs(x - y);
    bool rev = false;

    if (hsteps == max_hsteps && x > y) {
        double tmp = x;
        x = y;
        y = tmp;
        rev = true;
    } else if (hsteps > max_hsteps) {
        double dif = hsteps - max_hsteps;
        double dif_x = floor(dif / 2.0);
        double dif_y = dif - dif_x;

        if (x > y) {
            double edge_x = x - dif_x;
            double edge_y = y + dif_y;
            double temp = edge_x;
            edge_x = edge_y;
            edge_y = temp;
            x = edge_x + dif_x;
            y = edge_y - dif_y;
        } else if (y > x) {
            double edge_x = x + dif_x;
            double edge_y = y - dif_y;
            double temp = edge_x;
            edge_x = edge_y;
            edge_y = temp;
            x = edge_x - dif_x;
            y = edge_y + dif_y;
        }
    }

    out->x = x;
    out->y = y;
    out->rev = rev;
    return true;
}

bool get_xy_by_location(const loc_t *location, uint32_t level, xy_t *out) {
    if (!location || !out) {
        return false;
    }

    double h_size = calc_hex_size(level);
    double lon_grid, lat_grid;
    loc2xy(location->lon, location->lat, &lon_grid, &lat_grid);

    double unit_x = 6.0 * h_size;
    double unit_y = 6.0 * h_size * H_K;

    double h_pos_x = (lon_grid + lat_grid / H_K) / unit_x;
    double h_pos_y = (lat_grid - H_K * lon_grid) / unit_y;

    double h_x = round(h_pos_x);
    double h_y = round(h_pos_y);

    double h_x_q = h_pos_x - floor(h_pos_x);
    double h_y_q = h_pos_y - floor(h_pos_y);

    if (h_y_q > -h_x_q + 1) {
        if (h_y_q < 2 * h_x_q && h_y_q > 0.5 * h_x_q) {
            h_x = floor(h_pos_x) + 1;
            h_y = floor(h_pos_y) + 1;
        }
    } else if (h_y_q < -h_x_q + 1) {
        if (h_y_q > 2 * h_x_q - 1 && h_y_q < 0.5 * h_x_q + 0.5) {
            h_x = floor(h_pos_x);
            h_y = floor(h_pos_y);
        }
    }

    return adjust_xy(h_x, h_y, level, out);
}

bool get_xy_by_code(const geohex_code_t code, xy_t *out) {
    if (!out) {
        return false;
    }

    uint32_t level = strlen(code) - 2;
    double h_x = 0, h_y = 0;

    char *ptr_c1 = strchr(GEOHEX_KEY, code[0]);
    char *ptr_c2 = strchr(GEOHEX_KEY, code[1]);
    if (!ptr_c1 || !ptr_c2) return false;

    int32_t code3 = (ptr_c1 - GEOHEX_KEY) * 30 + (ptr_c2 - GEOHEX_KEY);
    char h_dec9[64];
    snprintf(h_dec9, sizeof(h_dec9), "%d%s", code3, code + 2);

    if ((h_dec9[0] == '1' || h_dec9[0] == '5') &&
        (h_dec9[1] != '1' && h_dec9[1] != '2' && h_dec9[1] != '5') &&
        (h_dec9[2] != '1' && h_dec9[2] != '2' && h_dec9[2] != '5')) {
        h_dec9[0] = (h_dec9[0] == '5') ? '7' : '3';
    }

    int32_t d9xlen = strlen(h_dec9);
    int32_t target_len = level + 3;
    while (d9xlen < target_len) {
        memmove(h_dec9 + 1, h_dec9, d9xlen + 1);
        h_dec9[0] = '0';
        d9xlen++;
    }

    char h_dec3[128] = {0};
    const char *base9_to_base3[9] = {"00", "01", "02", "10", "11", "12", "20", "21", "22"};
    int32_t idx = 0;

    for (int32_t i = 0; i < d9xlen; i++) {
        int32_t digit = h_dec9[i] - '0';
        h_dec3[idx++] = base9_to_base3[digit][0];
        h_dec3[idx++] = base9_to_base3[digit][1];
    }
    h_dec3[idx] = '\0';

    int32_t h_decx_len = idx / 2;
    char h_decx[64], h_decy[64];
    for (int32_t i = 0; i < h_decx_len; i++) {
        h_decx[i] = h_dec3[2 * i];
        h_decy[i] = h_dec3[2 * i + 1];
    }
    h_decx[h_decx_len] = '\0';
    h_decy[h_decx_len] = '\0';

    for (int32_t i = 0; i <= level + 2; i++) {
        double h_pow = pow(3.0, level + 2 - i);
        if (h_decx[i] == '0') h_x -= h_pow;
        else if (h_decx[i] == '2') h_x += h_pow;

        if (h_decy[i] == '0') h_y -= h_pow;
        else if (h_decy[i] == '2') h_y += h_pow;
    }

    return adjust_xy(h_x, h_y, level, out);
}

bool get_zone_by_location(const loc_t *location, uint32_t level, zone_t *out) {
    if (!out) {
        return false;
    }

    xy_t xy;

    if (!get_xy_by_location(location, level, &xy)) {
        return false;
    }

    return get_zone_by_xy(&xy, level, out);
}

bool get_zone_by_code(const geohex_code_t code, zone_t *out) {
    if (!out) {
        return false;
    }

    xy_t xy;

    if (!get_xy_by_code(code, &xy)) {
        return false;
    }

    uint32_t level = strlen(code) - 2;

    return get_zone_by_xy(&xy, level, out);
}

bool get_zone_by_xy(const xy_t *xy, uint32_t level, zone_t *out) {
    if (!xy || !out) {
        return false;
    }

    double h_size = calc_hex_size(level);
    double h_x = xy->x, h_y = xy->y;

    double unit_x = 6.0 * h_size;
    double unit_y = 6.0 * h_size * H_K;

    double h_lat = (H_K * h_x * unit_x + h_y * unit_y) / 2.0;
    double h_lon = (h_lat - h_y * unit_y) / H_K;

    double z_loc_x, z_loc_y;
    xy2loc(h_lon, h_lat, &z_loc_x, &z_loc_y);

    double max_hsteps = pow(3.0, level + 2);
    if (fabs(h_x - h_y) == max_hsteps && h_x > h_y) {
        double tmp = h_x;
        h_x = h_y;
        h_y = tmp;
        z_loc_x = -180.0;
    }

    int32_t code3_len = level + 3;
    int32_t code3_x[code3_len], code3_y[code3_len];
    double mod_x = h_x, mod_y = h_y;

    for (int32_t i = 0; i <= level + 2; i++) {
        double h_pow = pow(3.0, level + 2 - i);
        double half_h_pow = ceil(h_pow / 2.0);

        if (mod_x >= half_h_pow) {
            code3_x[i] = 2;
            mod_x -= h_pow;
        } else if (mod_x <= -half_h_pow) {
            code3_x[i] = 0;
            mod_x += h_pow;
        } else {
            code3_x[i] = 1;
        }

        if (mod_y >= half_h_pow) {
            code3_y[i] = 2;
            mod_y -= h_pow;
        } else if (mod_y <= -half_h_pow) {
            code3_y[i] = 0;
            mod_y += h_pow;
        } else {
            code3_y[i] = 1;
        }

        if (i == 2 && (z_loc_x == -180.0 || z_loc_x >= 0.0)) {
            if (code3_x[0] == 2 && code3_y[0] == 1 &&
                code3_x[1] == code3_y[1] && code3_x[2] == code3_y[2]) {
                code3_x[0] = 1;
                code3_y[0] = 2;
            } else if (code3_x[0] == 1 && code3_y[0] == 0 &&
                       code3_x[1] == code3_y[1] && code3_x[2] == code3_y[2]) {
                code3_x[0] = 0;
                code3_y[0] = 1;
            }
        }
    }

    char h_code[64] = {0};
    for (int32_t i = 0; i <= level + 2; i++) {
        int32_t code3_int32_t = code3_x[i] * 3 + code3_y[i];
        char code9 = '0' + code3_int32_t;
        strncat(h_code, &code9, 1);
    }

    char h_1_str[4] = {0};
    strncpy(h_1_str, h_code, 3);
    int32_t h_1_int32_t = atoi(h_1_str);
    int32_t h_a1 = h_1_int32_t / 30;
    int32_t h_a2 = h_1_int32_t % 30;

    out->code[0] = GEOHEX_KEY[h_a1];
    out->code[1] = GEOHEX_KEY[h_a2];
    strcpy(out->code + 2, h_code + 3);

    out->latlon.lat = z_loc_y;
    out->latlon.lon = z_loc_x;
    out->xy = *xy;

    return true;
}
