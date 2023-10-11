// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

struct Point {
    Point(uint32_t x = 0, uint32_t y = 0) : x(x), y(y) {}
    uint32_t x;
    uint32_t y;
};