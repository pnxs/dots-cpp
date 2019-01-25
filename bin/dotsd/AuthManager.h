#pragma once

#include <cstdint>

namespace dots {

class AuthManager
{
public:
    typedef uint64_t Challenge;

    Challenge newChallenge();

};

}