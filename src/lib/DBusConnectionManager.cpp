#include "DBusConnectionManager.hpp"
#include "LoggerInternal.hpp"
#include <mutex>
#include <sdbus-c++/Error.h>

std::shared_ptr<sdbus::IConnection> DBusConnectionManager::getSystemBus() {
    static std::shared_ptr<sdbus::IConnection> sharedConnection = nullptr;
    static std::once_flag initFlag;

    std::call_once(initFlag, []() {
        try {
            sharedConnection = sdbus::createSystemBusConnection();
            
            sharedConnection->enterEventLoopAsync();
            
            CDM_INFO("System bus connection established and event loop started.");
        } catch (const sdbus::Error& err) {
            CDM_ERR("Failed to connect to system bus. {}", err.what());
            throw err;
        }
    });

    return sharedConnection;
}
