#pragma once
#include <stdexcept>
#include <string_view>
#include <algorithm>

namespace dots::tools
{
    inline size_t find_delimiter(std::string_view str, std::string_view delimiter, bool assertNotNpos = true)
    {
        size_t delimiterPos = str.find(delimiter);

        if (assertNotNpos && delimiterPos == std::string_view::npos)
        {
            throw std::runtime_error{ "string '" + std::string{ str } + "' does not contain delimiter '" + std::string{ delimiter } + "'" };
        }

        return delimiterPos;
    }

    inline auto split_before_first_of(std::string_view str, std::string_view delimiter, bool assertNotNpos = true) -> std::pair<std::string_view, std::string_view>
    {
        if (size_t delimiterPos = find_delimiter(str, delimiter, assertNotNpos); delimiterPos == std::string_view::npos)
        {
            return { str, {} };
        }
        else
        {
            return { str.substr(0, delimiterPos), str.substr(delimiterPos) };
        }
    }

    inline auto split_left_at_first_of(std::string_view str, std::string_view delimiter, bool assertNotNpos = true) -> std::pair<std::string_view, std::string_view>
    {
        if (size_t delimiterPos = find_delimiter(str, delimiter, assertNotNpos); delimiterPos == std::string_view::npos)
        {
            return { str, {} };
        }
        else
        {
            return { str.substr(0, delimiterPos), str.substr(delimiterPos + delimiter.size()) };
        }
    }

    inline auto split_right_at_first_of(std::string_view str, std::string_view delimiter, bool assertNotNpos = true) -> std::pair<std::string_view, std::string_view>
    {
        if (size_t delimiterPos = find_delimiter(str, delimiter, assertNotNpos); delimiterPos == std::string_view::npos)
        {
            return { {}, str };
        }
        else
        {
            return { str.substr(0, delimiterPos), str.substr(delimiterPos + delimiter.size()) };
        }
    }

    inline auto split_after_first_of(std::string_view str, std::string_view delimiter, bool assertNotNpos = true) -> std::pair<std::string_view, std::string_view>
    {
        if (size_t delimiterPos = find_delimiter(str, delimiter, assertNotNpos); delimiterPos == std::string_view::npos)
        {
            return { {}, str };
        }
        else
        {
            return { str.substr(0, delimiterPos + delimiter.size()), str.substr(delimiterPos + delimiter.size()) };
        }
    }

    inline bool starts_with(std::string_view str, std::string_view prefix)
    {
        if (str.size() < prefix.size())
        {
            return false;
        }
        else
        {
            return std::mismatch(std::begin(prefix), std::end(prefix), std::begin(str)).first == std::end(prefix);
        }
    }
}