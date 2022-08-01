// SPDX-License-Identifier: LGPL-3.0-only
// Copyright 2015-2022 Thomas Schaetzlein <thomas@pnxs.de>, Christopher Gerlach <gerlachch@gmx.com>
#pragma once
#if defined(__cplusplus) || defined(c_plusplus)
/****************************************************************/

#include <functional>
#include <type_traits>

#define FUN(O, M) dots::tools::make_function(O, &std::remove_reference_t<decltype(O)>::M)

namespace dots::tools
{

template< class O, class M, class R >
auto
make_function(O &o, R (M::*m)()) { return std::bind(m, &o); }

template< class O, class M, class R, typename P1>
auto
make_function(O &o, R (M::*m)(P1)) { return std::bind(m, &o, std::placeholders::_1); }

template< class O, class M, class R,typename P1, typename P2>
auto
make_function(O &o, R (M::*m)(P1, P2)) { return std::bind(m, &o, std::placeholders::_1, std::placeholders::_2); }

template< class O, class M, class R, typename P1, typename P2, typename P3>
auto
make_function(O &o, R (M::*m)(P1, P2, P3)) { return std::bind(m, &o, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3); }

template< class O, class M, class R, typename P1, typename P2, typename P3, typename P4>
auto
make_function(O &o, R (M::*m)(P1, P2, P3, P4)) { return std::bind(m, &o, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4); }

}

#endif
