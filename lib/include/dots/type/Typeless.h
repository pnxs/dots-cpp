// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once

namespace dots::type
{
    struct Typeless
    {
        Typeless() = delete;
        Typeless(const Typeless& other) = delete;
        Typeless(Typeless&& other) = delete;
        ~Typeless() = delete;

        Typeless& operator = (const Typeless& rhs) = delete;
        Typeless& operator = (Typeless&& rhs) = delete;

        template <typename T>
        const T& to() const &
        {
            return reinterpret_cast<const T&>(*this);
        }

        template <typename T>
        T& to() &
        {
            return reinterpret_cast<T&>(*this);
        }

        template <typename T>
        T&& to() &&
        {
            return reinterpret_cast<T&&>(*this);
        }

        template <typename T>
        static const Typeless& From(const T& t)
        {
            return reinterpret_cast<const Typeless&>(t);
        }

        template <typename T>
        static Typeless& From(T& t)
        {
            return reinterpret_cast<Typeless&>(t);
        }

        template <typename T>
        static Typeless&& From(T&& t)
        {
            return reinterpret_cast<Typeless&&>(t);
        }

        template <typename T>
        static const Typeless* From(const T* t)
        {
            return reinterpret_cast<const Typeless*>(t);
        }

        template <typename T>
        static Typeless* From(T* t)
        {
            return reinterpret_cast<Typeless*>(t);
        }
    };
}
