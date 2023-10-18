// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

struct Size {
    Size(uint32_t width = 0, uint32_t height = 0) : width(width), height(height) {}
    uint32_t width;
    uint32_t height;
};