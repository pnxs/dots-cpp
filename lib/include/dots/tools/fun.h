#pragma once
#if defined(__cplusplus) || defined(c_plusplus)
/****************************************************************/

#include <functional>
#include <type_traits>

#define FUN(O, M) dots::tools::make_function(O, &std::remove_reference_t<decltype(O)>::M)

namespace dots::tools
{

template< class O, class M, class R >
inline auto
make_function(O &o, R (M::*m)()) { return std::bind(m, &o); }

template< class O, class M, class R, typename P1>
inline auto
make_function(O &o, R (M::*m)(P1)) { return std::bind(m, &o, std::placeholders::_1); }

template< class O, class M, class R,typename P1, typename P2>
inline auto
make_function(O &o, R (M::*m)(P1, P2)) { return std::bind(m, &o, std::placeholders::_1, std::placeholders::_2); }

template< class O, class M, class R, typename P1, typename P2, typename P3>
inline auto
make_function(O &o, R (M::*m)(P1, P2, P3)) { return std::bind(m, &o, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3); }

template< class O, class M, class R, typename P1, typename P2, typename P3, typename P4>
inline auto
make_function(O &o, R (M::*m)(P1, P2, P3, P4)) { return std::bind(m, &o, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4); }

}

#endif
