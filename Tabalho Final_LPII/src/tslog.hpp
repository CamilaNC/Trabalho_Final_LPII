#pragma once


#include <string>


namespace tslog {


enum Level { DEBUG=0, INFO=1, WARN=2, ERROR=3 };


struct Config {
std::string file = "logs/app.log";
bool mirror_stdout = true;
Level level = INFO;
};


// init logger; must be called before using macros
void init(const Config &cfg);
// shut down logger (flush + join worker)
void shutdown();


// log a formatted message (simple API)
void log(Level lvl, const char *msg);


// convenience helpers
inline void debug(const char *msg) { log(DEBUG, msg); }
inline void info(const char *msg) { log(INFO, msg); }
inline void warn(const char *msg) { log(WARN, msg); }
inline void error(const char *msg) { log(ERROR, msg); }


} // namespace tslog