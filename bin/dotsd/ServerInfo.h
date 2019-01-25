#pragma once

namespace dots {

class AuthManager;

class ServerInfo
{
public:
    virtual ~ServerInfo()
    {}

    virtual const string &name() const = 0;
    virtual AuthManager& authManager() = 0;
    virtual const ClientId& id() const = 0;
};

}
