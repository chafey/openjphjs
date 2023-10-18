// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

struct FrameInfo {
    /// <summary>
    /// Width of the image, range [1, 65535].
    /// </summary>
    uint16_t width {0};

    /// <summary>
    /// Height of the image, range [1, 65535].
    /// </summary>
    uint16_t height {0};

    /// <summary>
    /// Number of bits per sample, range [2, 16]
    /// </summary>
    uint8_t bitsPerSample {0};

    /// <summary>
    /// Number of components contained in the frame, range [1, 255]
    /// </summary>
    uint8_t componentCount {0};

    /// <summary>
    /// true if signed, false if unsigned
    /// </summary>
    bool isSigned {false};

    /// <summary>
    /// true if color transform is used, false if not
    /// </summary>
    bool isUsingColorTransform {false};
};