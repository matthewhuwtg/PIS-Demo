#include "../include/pis_ipc.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <chrono>
#include <algorithm>

namespace pis {

IpcServer::IpcServer() = default;
IpcServer::~IpcServer() { stop(); }

bool IpcServer::setupSocket() {
    unlink(config_.socket_path.c_str());
    server_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd_ < 0) { std::cerr << "[IPC] socket: " << strerror(errno) << std::endl; return false; }
    struct sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, config_.socket_path.c_str(), sizeof(addr.sun_path) - 1);
    if (bind(server_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) { std::cerr << "[IPC] bind: " << strerror(errno) << std::endl; close(server_fd_); server_fd_ = -1; return false; }
    if (listen(server_fd_, 5) < 0) { std::cerr << "[IPC] listen: " << strerror(errno) << std::endl; close(server_fd_); server_fd_ = -1; return false; }
    std::cout << "[IPC] Server listening on " << config_.socket_path << std::endl;
    return true;
}

bool IpcServer::initialize(const IpcConfig& config) {
    config_ = config;
    if (!config_.enabled) { std::cout << "[IPC] Disabled" << std::endl; return true; }
    return setupSocket();
}

void IpcServer::start() {
    if (!config_.enabled || server_fd_ < 0 || running_) return;
    running_ = true;
    accept_thread_ = std::make_unique<std::thread>([this]() {
        struct timeval tv{0, 500000};
        setsockopt(server_fd_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        while (running_) {
            int client_fd = accept(server_fd_, nullptr, nullptr);
            if (client_fd < 0) { if (errno == EAGAIN || errno == EWOULDBLOCK) continue; break; }
            std::cout << "[IPC] Client connected (fd=" << client_fd << ")" << std::endl;
            client_fds_.push_back(client_fd);
        }
    });
}

void IpcServer::stop() {
    running_ = false;
    for (int fd : client_fds_) { if (fd >= 0) close(fd); }
    client_fds_.clear();
    if (server_fd_ >= 0) { close(server_fd_); unlink(config_.socket_path.c_str()); server_fd_ = -1; }
    if (accept_thread_ && accept_thread_->joinable()) accept_thread_->join();
}

std::string IpcServer::serializeEvent(const ScheduleEvent& event) const {
    std::ostringstream j;
    j << "{\"type\":\"" << event.typeString() << "\",\"station_id\":\"" << event.station_id << "\",\"station_name_en\":\"" << event.station_name_en << "\",\"elapsed_sec\":" << event.elapsed_sec << ",\"message\":\"" << event.message << "\"}";
    return j.str();
}

void IpcServer::broadcast(const ScheduleEvent& event) {
    if (!config_.enabled || server_fd_ < 0) return;
    std::string payload = serializeEvent(event);
    IpcMessageHeader hdr{0x50495331, 1, static_cast<uint32_t>(event.type), static_cast<uint32_t>(payload.size()), static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())};
    auto it = client_fds_.begin();
    while (it != client_fds_.end()) {
        if (write(*it, &hdr, sizeof(hdr)) <= 0 || write(*it, payload.data(), payload.size()) <= 0) {
            close(*it); it = client_fds_.erase(it);
        } else { ++it; }
    }
}

IpcClient::IpcClient() = default;
IpcClient::~IpcClient() { disconnect(); }

bool IpcClient::connect(const std::string& socket_path) {
    client_fd_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd_ < 0) return false;
    struct sockaddr_un addr{};
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path.c_str(), sizeof(addr.sun_path) - 1);
    if (::connect(client_fd_, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(client_fd_); client_fd_ = -1; return false; }
    connected_ = true;
    reading_ = true;
    reader_thread_ = std::make_unique<std::thread>([this]() { readLoop(); });
    return true;
}

void IpcClient::disconnect() {
    reading_ = false; connected_ = false;
    if (reader_thread_ && reader_thread_->joinable()) reader_thread_->join();
    if (client_fd_ >= 0) { close(client_fd_); client_fd_ = -1; }
}

void IpcClient::onMessage(std::function<void(const ScheduleEvent&)> cb) { callback_ = std::move(cb); }

void IpcClient::readLoop() {
    char buf[IPC_MAX_MSG];
    while (reading_ && client_fd_ >= 0) {
        IpcMessageHeader hdr;
        if (read(client_fd_, &hdr, sizeof(hdr)) <= 0) break;
        if (hdr.payload_len > 0 && hdr.payload_len < IPC_MAX_MSG) {
            ssize_t total = 0;
            while (total < static_cast<ssize_t>(hdr.payload_len)) {
                ssize_t n = read(client_fd_, buf + total, hdr.payload_len - total);
                if (n <= 0) break;
                total += n;
            }
            buf[total] = '\0';
            if (callback_) callback_(deserializeEvent(std::string(buf, total)));
        }
    }
}

ScheduleEvent IpcClient::deserializeEvent(const std::string& json) const {
    ScheduleEvent ev;
    auto ext = [&](const std::string& k) -> std::string {
        auto p = json.find("\"" + k + "\""); if (p == std::string::npos) return "";
        p = json.find(':', p); if (p == std::string::npos) return ""; p++;
        while (p < json.size() && isspace(json[p])) p++;
        if (p < json.size() && json[p] == '"') { p++; std::string v; while (p < json.size() && json[p] != '"') v += json[p++]; return v; }
        std::string n; while (p < json.size() && (isdigit(json[p]) || json[p] == '-')) n += json[p++]; return n;
    };
    ev.message = ext("message");
    ev.station_id = ext("station_id");
    ev.station_name_en = ext("station_name_en");
    try { ev.elapsed_sec = std::stoi(ext("elapsed_sec")); } catch (...) {}
    std::string t = ext("type");
    if (t == "DEPARTURE") ev.type = ScheduleEventType::DEPARTURE;
    else if (t == "ARRIVAL") ev.type = ScheduleEventType::ARRIVAL;
    else if (t == "APPROACHING") ev.type = ScheduleEventType::APPROACHING;
    else if (t == "NEXT_STOP") ev.type = ScheduleEventType::NEXT_STOP;
    else if (t == "TERMINUS") ev.type = ScheduleEventType::TERMINUS;
    else ev.type = ScheduleEventType::CUSTOM;
    return ev;
}

} // namespace pis
