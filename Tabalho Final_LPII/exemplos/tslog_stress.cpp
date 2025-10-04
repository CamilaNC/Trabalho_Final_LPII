#include "../src/tslog.hpp"
#include <thread>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <string>

int main() {
    tslog::Config cfg;
    cfg.file = "logs/app.log";
    cfg.mirror_stdout = true;
    cfg.level = tslog::DEBUG;
    tslog::init(cfg);

    const int NTHREADS = 8;
    const int PER_THREAD = 5000;

    std::vector<std::thread> threads;
    for (int t = 0; t < NTHREADS; ++t) {
        threads.emplace_back([t, PER_THREAD]() {
            for (int i = 0; i < PER_THREAD; ++i) {
                char buf[128];
                snprintf(buf, sizeof(buf), "thread=%d msg=%d", t, i);
                tslog::debug(buf);
            }
        });
    }

    for (auto &th : threads) th.join();

    tslog::shutdown();
    return 0;
}
