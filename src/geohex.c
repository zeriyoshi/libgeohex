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
#define H_K         0.5773502691896257 /* tan(M_PI / 6.0) */

const uint32_t pow3_table[] = {
    1,          /* pow(3, 0) */
    3,          /* pow(3, 1) */
    9,          /* pow(3, 2) */
    27,         /* pow(3, 3) */
    81,         /* pow(3, 4) */
    243,        /* pow(3, 5) */
    729,        /* pow(3, 6) */
    2187,       /* pow(3, 7) */
    6561,       /* pow(3, 8) */
    19683,      /* pow(3, 9) */
    59049,      /* pow(3, 10) */
    177147,     /* pow(3, 11) */
    531441,     /* pow(3, 12) */
    1594323,    /* pow(3, 13) */
    4782969,    /* pow(3, 14) */
    14348907,   /* pow(3, 15) */
    43046721,   /* pow(3, 16) */
    129140163,  /* pow(3, 17) */
    387420489,  /* pow(3, 18) */
    1162261467, /* pow(3, 19) */
    3486784401  /* pow(3, 20) */
};

static inline int char_to_index(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A';
    } else if (c >= 'a' && c <= 'z') {
        return c - 'a' + 26;
    } else {
        return -1;
    }
}

double calc_hex_size(uint32_t level) {
    return H_BASE / pow3_table[level + 3];
}

void loc2xy(double lon, double lat, double *dx, double *dy) {
    *dx = lon * H_BASE / 180.0;
    *dy = log(tan((90.0 + lat) * M_PI / 360.0)) * (H_BASE / M_PI);
}

void xy2loc(double dx, double dy, double *lon, double *lat) {
    *lon = (dx / H_BASE) * 180.0;
    double lat_rad = (dy / H_BASE) * M_PI;
    *lat = (2.0 * atan(exp(lat_rad)) - M_PI / 2.0) * 180.0 / M_PI;
}


bool adjust_xy(int32_t x, int32_t y, uint32_t level, xy_t *out) {
    if (!out) {
        return false;
    }

    int32_t max_hsteps = pow3_table[level + 2];
    int32_t hsteps = abs(x - y);
    bool rev = false;

    if (hsteps == max_hsteps && x > y) {
        int32_t tmp = x;
        x = y;
        y = tmp;
        rev = true;
    } else if (hsteps > max_hsteps) {
        int32_t dif = hsteps - max_hsteps;
        int32_t dif_x = dif / 2;
        int32_t dif_y = dif - dif_x;

        if (x > y) {
            int32_t edge_x = x - dif_x;
            int32_t edge_y = y + dif_y;
            int32_t temp = edge_x;
            edge_x = edge_y;
            edge_y = temp;
            x = edge_x + dif_x;
            y = edge_y - dif_y;
        } else if (y > x) {
            int32_t edge_x = x + dif_x;
            int32_t edge_y = y - dif_y;
            int32_t temp = edge_x;
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

    int32_t h_x = (int32_t) round(h_pos_x);
    int32_t h_y = (int32_t) round(h_pos_y);

    double h_x_q = h_pos_x - floor(h_pos_x);
    double h_y_q = h_pos_y - floor(h_pos_y);

    if (h_y_q > -h_x_q + 1) {
        if (h_y_q < 2 * h_x_q && h_y_q > 0.5 * h_x_q) {
            h_x = ((int32_t) floor(h_pos_x)) + 1;
            h_y = ((int32_t) floor(h_pos_y)) + 1;
        }
    } else if (h_y_q < -h_x_q + 1) {
        if (h_y_q > 2 * h_x_q - 1 && h_y_q < 0.5 * h_x_q + 0.5) {
            h_x = (int32_t) floor(h_pos_x);
            h_y = (int32_t) floor(h_pos_y);
        }
    }

    return adjust_xy(h_x, h_y, level, out);
}

bool get_xy_by_code(const geohex_code_t code, xy_t *out) {
    if (!out) {
        return false;
    }

    uint32_t code_len = strlen(code);
    uint32_t level = code_len - 2;
    int32_t h_x = 0, h_y = 0;

    int32_t c1_idx = char_to_index(code[0]);
    int32_t c2_idx = char_to_index(code[1]);

    if (c1_idx == -1 || c2_idx == -1) {
        return false;
    }

    int32_t code3 = c1_idx * 30 + c2_idx;

    char code3_str[5];
    snprintf(code3_str, sizeof(code3_str), "%d", code3);

    if ((code3_str[0] == '1' || code3_str[0] == '5') &&
        code3_str[1] != '1' && code3_str[1] != '2' && code3_str[1] != '5' &&
        code3_str[2] != '1' && code3_str[2] != '2' && code3_str[2] != '5') {
        code3_str[0] = (code3_str[0] == '5') ? '7' : '3';
    }

    char h_dec9[MAX_H_DEC9_LEN] = {0};
    snprintf(h_dec9, sizeof(h_dec9), "%s%s", code3_str, code + 2);

    int32_t d9xlen = strlen(h_dec9);
    int32_t target_len = level + 3;
    if (d9xlen < target_len) {
        memmove(h_dec9 + (target_len - d9xlen), h_dec9, d9xlen + 1);
        memset(h_dec9, '0', target_len - d9xlen);
        d9xlen = target_len;
    }

    int32_t h_decx[MAX_H_DEC3_LEN] = {0};
    int32_t h_decy[MAX_H_DEC3_LEN] = {0};
    for (int32_t i = 0; i < d9xlen; i++) {
        int32_t digit = h_dec9[i] - '0';
        h_decx[i] = digit / 3;
        h_decy[i] = digit % 3;
    }

    for (int32_t i = 0; i <= (int32_t)(level + 2); i++) {
        int32_t h_pow = pow3_table[level + 2 - i];

        if (h_decx[i] == 0) {
            h_x -= h_pow;
        } else if (h_decx[i] == 2) {
            h_x += h_pow;
        }

        if (h_decy[i] == 0) {
            h_y -= h_pow;
        } else if (h_decy[i] == 2) {
            h_y += h_pow;
        }
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
    int32_t h_x = xy->x, h_y = xy->y;

    double unit_x = 6.0 * h_size;
    double unit_y = 6.0 * h_size * H_K;

    double h_lat = (H_K * h_x * unit_x + h_y * unit_y) / 2.0;
    double h_lon = (h_lat - h_y * unit_y) / H_K;

    double z_loc_x, z_loc_y;
    xy2loc(h_lon, h_lat, &z_loc_x, &z_loc_y);

    int32_t max_hsteps = pow3_table[level + 2];
    if (abs(h_x - h_y) == max_hsteps && h_x > h_y) {
        int32_t tmp = h_x;
        h_x = h_y;
        h_y = tmp;
        z_loc_x = -180.0;
    }

    int32_t code3_x[MAX_CODE_LEN + 2], code3_y[MAX_CODE_LEN + 2];
    int32_t mod_x = h_x, mod_y = h_y;

    for (int32_t i = 0; i <= level + 2; i++) {
        int32_t h_pow = pow3_table[level + 2 - i];
        int32_t half_h_pow = (h_pow + 1) / 2;

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

    int32_t h_code_digits[MAX_CODE_LEN + 2];
    for (int32_t i = 0; i <= level + 2; i++) {
        h_code_digits[i] = code3_x[i] * 3 + code3_y[i];
    }

    int32_t h_1_int = h_code_digits[0] * 100 + h_code_digits[1] * 10 + h_code_digits[2];
    int32_t h_a1 = h_1_int / 30;
    int32_t h_a2 = h_1_int % 30;

    out->code[0] = GEOHEX_KEY[h_a1];
    out->code[1] = GEOHEX_KEY[h_a2];

    for (int32_t i = 3; i <= level + 2; i++) {
        out->code[i - 1] = '0' + h_code_digits[i];
    }
    out->code[level + 2] = '\0';

    out->latlon.lat = z_loc_y;
    out->latlon.lon = z_loc_x;
    out->xy = *xy;

    return true;
}
