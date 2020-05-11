// Copyright (c) Chris Hafey.
// SPDX-License-Identifier: MIT

#pragma once

#include <exception>
#include <memory>

#include <ojph_arch.h>
#include <ojph_file.h>
#include <ojph_mem.h>
#include <ojph_params.h>
#include <ojph_codestream.h>

#include <vector>

/**
 * EncodedBuffer implements the ojph::outfile_base using a 
 * std::vector<uint8_t>.  This allows OpenJPEG to write
 * directly to std::vector<> and avoid a memory copy that
 * would otherwise be needed when using ojph::mem_outfile.
 */
class EncodedBuffer : public ojph::outfile_base
  {
  public:
    /**  A constructor */
    OJPH_EXPORT
    EncodedBuffer() {}
    /**  A destructor */
    OJPH_EXPORT
    ~EncodedBuffer() {}

    /**  Call this function to open a memory file.
	 *
     *  This function creates a memory buffer to be used for storing
     *  the generated j2k codestream.
     *
     *  @param initial_size is the initial memory buffer size.
     *         The default value is 2^16.
     */
    OJPH_EXPORT
    void open(size_t initial_size = 65536) {
        buffer_.resize(0);
        buffer_.reserve(initial_size);
    }

    /**  Call this function to write data to the memory file.
	 *
     *  This function adds new data to the memory file.  The memory buffer
     *  of the file grows as needed.
     *
     *  @param ptr is the address of the new data.
     *  @param size the number of bytes in the new data.
     */
    OJPH_EXPORT
    virtual size_t write(const void *ptr, size_t size) {
        auto bytes = reinterpret_cast<uint8_t const*>(ptr);
        buffer_.insert(buffer_.end(), bytes, bytes + size);
        return size;
    }

    /** Call this function to know the file size (i.e., number of bytes used
     *  to store the file).
     *
     *  @return the file size.
     */
    OJPH_EXPORT
    virtual ojph::si64 tell() { return buffer_.size(); }

    /** Call this function to close the file and deallocate memory
	 *
     *  The object can be used again after calling close
     */
    OJPH_EXPORT
    virtual void close() {}

    /** Call this function to access memory file data.
	 *
     *  It is not recommended to store the returned value because buffer
     *  storage address can change between write calls.
     *
     *  @return a constant pointer to the data.
     */
    OJPH_EXPORT
    const ojph::ui8* get_data() { return buffer_.data(); }

    /** Call this function to access memory file data (for const objects)
	 *
     *  This is similar to the above function, except that it can be used
     *  with constant objects.
     *
     *  @return a constant pointer to the data.
     */
    OJPH_EXPORT
    const ojph::ui8* get_data() const { return buffer_.data(); }
    
    /**
     * Returns the underlying buffer
     */
    OJPH_EXPORT
    const std::vector<uint8_t>& getBuffer() const {return buffer_;}

  private:
    std::vector<uint8_t> buffer_;
  };