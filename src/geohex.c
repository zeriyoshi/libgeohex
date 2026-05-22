/* SPDX-License-Identifier: MIT */
/*
 * libgeohex
 *
 * Copyright (c) 2024-2026 Go Kudo Kudo (https://github.com/zeriyoshi)
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
#define H_K         0.5773502691896257 /* tan(GEOHEX_PI / 6.0) */

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

GEOHEX_API double calc_hex_size(uint32_t level) {
    return H_BASE / pow3_table[level + 3];
}

GEOHEX_API void loc2xy(double lon, double lat, double *dx, double *dy) {
    *dx = lon * H_BASE / 180.0;
    *dy = log(tan((90.0 + lat) * GEOHEX_PI / 360.0)) * (H_BASE / GEOHEX_PI);
}

GEOHEX_API void xy2loc(double dx, double dy, double *lon, double *lat) {
    double lat_rad;

    *lon = (dx / H_BASE) * 180.0;
    lat_rad = (dy / H_BASE) * GEOHEX_PI;
    *lat = (2.0 * atan(exp(lat_rad)) - GEOHEX_PI / 2.0) * 180.0 / GEOHEX_PI;
}

GEOHEX_API bool adjust_xy(int32_t x, int32_t y, uint32_t level, xy_t *out) {
    int32_t tmp, max_hsteps, hsteps, dif, dif_x, dif_y, edge_x, edge_y;
    bool rev;

    if (!out) {
        return false;
    }

    max_hsteps = pow3_table[level + 2];
    hsteps = abs(x - y);
    rev = false;

    if (hsteps == max_hsteps && x > y) {
        tmp = x;
        x = y;
        y = tmp;
        rev = true;
    } else if (hsteps > max_hsteps) {
        dif = hsteps - max_hsteps;
        dif_x = dif / 2;
        dif_y = dif - dif_x;

        if (x > y) {
            edge_x = x - dif_x;
            edge_y = y + dif_y;
            tmp = edge_x;
            edge_x = edge_y;
            edge_y = tmp;
            x = edge_x + dif_x;
            y = edge_y - dif_y;
        } else if (y > x) {
            edge_x = x + dif_x;
            edge_y = y - dif_y;
            tmp = edge_x;
            edge_x = edge_y;
            edge_y = tmp;
            x = edge_x - dif_x;
            y = edge_y + dif_y;
        }
    }

    out->x = x;
    out->y = y;
    out->rev = rev;

    return true;
}

GEOHEX_API bool get_xy_by_location(const loc_t *location, uint32_t level, xy_t *out) {
    int32_t h_x, h_y;
    double h_size, lon_grid, lat_grid, unit_x, unit_y, h_pos_x, h_pos_y,
        h_x_q, h_y_q;

    if (!location || !out) {
        return false;
    }

    h_size = calc_hex_size(level);
    loc2xy(location->lon, location->lat, &lon_grid, &lat_grid);

    unit_x = 6.0 * h_size;
    unit_y = 6.0 * h_size * H_K;

    h_pos_x = (lon_grid + lat_grid / H_K) / unit_x;
    h_pos_y = (lat_grid - H_K * lon_grid) / unit_y;

    h_x = (int32_t) round(h_pos_x);
    h_y = (int32_t) round(h_pos_y);

    h_x_q = h_pos_x - floor(h_pos_x);
    h_y_q = h_pos_y - floor(h_pos_y);

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

GEOHEX_API bool get_xy_by_code(const geohex_code_t code, xy_t *out) {
    uint32_t code_len, level;
    int32_t i, h_x, h_y, c1_idx, c2_idx, code3, d9xlen, target_len,
        h_decx[MAX_H_DEC3_LEN] = {0}, h_decy[MAX_H_DEC3_LEN] = {0},
        digit, h_pow;
    char code3_str[5], h_dec9[MAX_H_DEC9_LEN] = {0};

    if (!out) {
        return false;
    }

    code_len = strlen(code);
    level = code_len - 2;
    h_x = 0;
    h_y = 0;

    c1_idx = char_to_index(code[0]);
    c2_idx = char_to_index(code[1]);

    if (c1_idx == -1 || c2_idx == -1) {
        return false;
    }

    code3 = c1_idx * 30 + c2_idx;

    snprintf(code3_str, sizeof(code3_str), "%d", code3);

    if ((code3_str[0] == '1' || code3_str[0] == '5') &&
        code3_str[1] != '1' && code3_str[1] != '2' && code3_str[1] != '5' &&
        code3_str[2] != '1' && code3_str[2] != '2' && code3_str[2] != '5') {
        code3_str[0] = (code3_str[0] == '5') ? '7' : '3';
    }

    snprintf(h_dec9, sizeof(h_dec9), "%s%s", code3_str, code + 2);

    d9xlen = strlen(h_dec9);
    target_len = level + 3;
    if (d9xlen < target_len) {
        memmove(h_dec9 + (target_len - d9xlen), h_dec9, d9xlen + 1);
        memset(h_dec9, '0', target_len - d9xlen);
        d9xlen = target_len;
    }

    for (i = 0; i < d9xlen; i++) {
        digit = h_dec9[i] - '0';
        h_decx[i] = digit / 3;
        h_decy[i] = digit % 3;
    }

    for (i = 0; i <= (int32_t)(level + 2); i++) {
        h_pow = pow3_table[level + 2 - i];

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

GEOHEX_API bool get_zone_by_location(const loc_t *location, uint32_t level, zone_t *out) {
    xy_t xy;

    if (!out) {
        return false;
    }

    if (!get_xy_by_location(location, level, &xy)) {
        return false;
    }

    return get_zone_by_xy(&xy, level, out);
}

GEOHEX_API bool get_zone_by_code(const geohex_code_t code, zone_t *out) {
    xy_t xy;

    if (!out) {
        return false;
    }

    if (!get_xy_by_code(code, &xy)) {
        return false;
    }

    return get_zone_by_xy(&xy, (strlen(code) - 2), out);
}

GEOHEX_API bool get_zone_by_xy(const xy_t *xy, uint32_t level, zone_t *out) {
    uint32_t i;
    int32_t tmp, h_x, h_y, max_hsteps,
        code3_x[MAX_CODE_LEN + 2], code3_y[MAX_CODE_LEN + 2],
        h_code_digits[MAX_CODE_LEN + 2],
        mod_x, mod_y, h_1_int, h_a1, h_a2,
        h_pow, half_h_pow;
    double h_size, unit_x, unit_y, h_lat, h_lon, z_loc_x, z_loc_y;

    if (!xy || !out) {
        return false;
    }

    h_size = calc_hex_size(level);
    h_x = xy->x;
    h_y = xy->y;

    unit_x = 6.0 * h_size;
    unit_y = 6.0 * h_size * H_K;

    h_lat = (H_K * h_x * unit_x + h_y * unit_y) / 2.0;
    h_lon = (h_lat - h_y * unit_y) / H_K;

    z_loc_x = 0.0;
    z_loc_y = 0.0;
    xy2loc(h_lon, h_lat, &z_loc_x, &z_loc_y);

    max_hsteps = pow3_table[level + 2];
    if (abs(h_x - h_y) == max_hsteps && h_x > h_y) {
        tmp = h_x;
        h_x = h_y;
        h_y = tmp;
        z_loc_x = -180.0;
    }

    mod_x = h_x;
    mod_y = h_y;

    for (i = 0; i <= level + 2; i++) {
        h_pow = pow3_table[level + 2 - i];
        half_h_pow = (h_pow + 1) / 2;

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
                code3_x[1] == code3_y[1] && code3_x[2] == code3_y[2]
            ) {
                code3_x[0] = 1;
                code3_y[0] = 2;
            } else if (code3_x[0] == 1 && code3_y[0] == 0 &&
                       code3_x[1] == code3_y[1] && code3_x[2] == code3_y[2]
            ) {
                code3_x[0] = 0;
                code3_y[0] = 1;
            }
        }
    }

    for (i = 0; i <= level + 2; i++) {
        h_code_digits[i] = code3_x[i] * 3 + code3_y[i];
    }

    h_1_int = h_code_digits[0] * 100 + h_code_digits[1] * 10 + h_code_digits[2];
    h_a1 = h_1_int / 30;
    h_a2 = h_1_int % 30;

    out->code[0] = GEOHEX_KEY[h_a1];
    out->code[1] = GEOHEX_KEY[h_a2];

    for (i = 3; i <= level + 2; i++) {
        out->code[i - 1] = '0' + h_code_digits[i];
    }
    out->code[level + 2] = '\0';

    out->latlon.lat = z_loc_y;
    out->latlon.lon = z_loc_x;
    out->xy = *xy;

    return true;
}
