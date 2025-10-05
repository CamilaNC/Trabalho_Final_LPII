#include "tslog.hpp"
#include "blocking_queue.hpp"

#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <memory>
#include <cstdio>

using namespace std::chrono;

namespace tslog {

struct LogRecord {
    Level lvl;
    std::string text;
    std::thread::id tid;
    system_clock::time_point ts;
};

static std::unique_ptr<BlockingQueue<LogRecord>> g_queue;
static std::unique_ptr<std::thread> g_worker;
static std::atomic<bool> g_running{false};
static Config g_cfg;

static const char* lvl_name(Level l) {
    switch (l) {
        case DEBUG: return "DEBUG";
        case INFO:  return "INFO";
        case WARN:  return "WARN";
        case ERROR: return "ERROR";
        default:    return "UNK";
    }
}

static std::string timestamp_str(system_clock::time_point tp) {
    auto t = system_clock::to_time_t(tp);
    std::tm tm;
#ifdef _WIN32
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif
    std::ostringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

static void worker_thread_func() {
    std::ofstream ofs;
    if (!g_cfg.file.empty()) {
        ofs.open(g_cfg.file, std::ios::app);
        if (!ofs.is_open()) {
            std::cerr << "tslog: cannot open file " << g_cfg.file << "\n";
        }
    }

    while (g_running.load()) {
        LogRecord rec;
        bool ok = g_queue->pop(rec);
        if (!ok) break; 

        std::ostringstream line;
        line << timestamp_str(rec.ts) << " [" << lvl_name(rec.lvl) << "]"
             << " (tid=" << rec.tid << ") " << rec.text << "\n";
        std::string s = line.str();

        if (ofs.is_open()) {
            ofs << s;
            ofs.flush();
        }
        if (g_cfg.mirror_stdout) {
            std::fwrite(s.data(), 1, s.size(), stdout);
            std::fflush(stdout);
        }
    }

    while (true) {
        LogRecord rec;
        if (!g_queue->pop(rec)) break; 
        std::ostringstream line;
        line << timestamp_str(rec.ts) << " [" << lvl_name(rec.lvl) << "]"
             << " (tid=" << rec.tid << ") " << rec.text << "\n";
        std::string s = line.str();
        if (ofs.is_open())  ofs << s;
        if (g_cfg.mirror_stdout) std::fwrite(s.data(), 1, s.size(), stdout);
    }

    if (ofs.is_open()) ofs.close();
}

void init(const Config &cfg) {
    if (g_running.load()) return;
    g_cfg = cfg;
    g_queue.reset(new BlockingQueue<LogRecord>());
    g_running.store(true);
    g_worker.reset(new std::thread(worker_thread_func));
}

void shutdown() {
    if (!g_running.load()) return;
    g_running.store(false);
    if (g_queue) g_queue->notify_shutdown();
    if (g_worker && g_worker->joinable()) g_worker->join();
    g_worker.reset();
    g_queue.reset();
}

void log(Level lvl, const char *msg) {
    if (!g_running.load()) return;
    if (lvl < g_cfg.level) return;
    LogRecord rec;
    rec.lvl = lvl;
    rec.text = msg;
    rec.tid = std::this_thread::get_id();
    rec.ts = system_clock::now();
    g_queue->push(std::move(rec));
}

} // namespace tslog
