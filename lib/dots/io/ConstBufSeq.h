#pragma once

#include <asio.hpp>

namespace dots {

struct ConstBufSeq
{
    const asio::const_buffer *arr = NULL;
    size_t len = 0;

    typedef const asio::const_buffer *const_iterator;

    ConstBufSeq(const asio::const_buffer *_arr, size_t _len) : arr(_arr), len(_len)
    {}

    const_iterator begin() const
    { return arr; }

    const_iterator end() const
    { return arr + len; }
};

}