#pragma once

#include <memory>
#include <sdbus-c++/IConnection.h>

class DBusConnectionManager {
public:
    // Returns a ready-to-use, shared connection with a running event loop
    static std::shared_ptr<sdbus::IConnection> getSystemBus();
};
