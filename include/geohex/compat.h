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
#ifndef GEOHEX_COMPAT_H
#define GEOHEX_COMPAT_H

#if defined(_WIN32) || defined(__CYGWIN__)
#  if defined(GEOHEX_STATIC_DEFINE)
#    define GEOHEX_API
#  elif defined(GEOHEX_BUILDING_LIBRARY)
#    define GEOHEX_API __declspec(dllexport)
#  else
#    define GEOHEX_API __declspec(dllimport)
#  endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#  define GEOHEX_API __attribute__((visibility("default")))
#else
#  define GEOHEX_API
#endif

#define GEOHEX_PI 3.14159265358979323846

#endif /* GEOHEX_COMPAT_H */
