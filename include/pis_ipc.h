#ifndef PIS_IPC_H
#define PIS_IPC_H

#include "pis_schedule.h"
#include "pis_config.h"
#include <string>
#include <memory>
#include <atomic>
#include <thread>
#include <vector>
#include <functional>

namespace pis {

constexpr size_t IPC_MAX_MSG = 4096;

struct __attribute__((packed)) IpcMessageHeader {
    uint32_t magic;
    uint32_t version;
    uint32_t msg_type;
    uint32_t payload_len;
    uint64_t timestamp_ms;
};

struct IpcMessage {
    IpcMessageHeader header;
    std::string payload;
};

class IpcServer {
public:
    IpcServer();
    ~IpcServer();
    bool initialize(const IpcConfig& config);
    void start();
    void stop();
    void broadcast(const ScheduleEvent& event);
    bool isRunning() const { return running_; }

private:
    IpcConfig config_;
    std::atomic<bool> running_{false};
    std::unique_ptr<std::thread> accept_thread_;
    int server_fd_{-1};
    std::vector<int> client_fds_;
    std::string serializeEvent(const ScheduleEvent& event) const;
    bool setupSocket();
    void acceptLoop();
};

class IpcClient {
public:
    IpcClient();
    ~IpcClient();
    bool connect(const std::string& socket_path);
    void disconnect();
    void onMessage(std::function<void(const ScheduleEvent&)> cb);
    bool isConnected() const { return connected_; }

private:
    int client_fd_{-1};
    bool connected_{false};
    std::function<void(const ScheduleEvent&)> callback_;
    std::unique_ptr<std::thread> reader_thread_;
    std::atomic<bool> reading_{false};
    ScheduleEvent deserializeEvent(const std::string& json) const;
    void readLoop();
};

} // namespace pis

#endif // PIS_IPC_H
