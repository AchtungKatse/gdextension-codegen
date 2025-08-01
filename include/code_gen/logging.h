enum log_level {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_TRACE,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_FATAL,
};

const char* level_strings[6] = {
    "\x1B[32m[DEBUG]: ",
    "\x1B[36m[TRACE]: ",
    "\x1B[37m[INFO]:  ",
    "\x1B[33m[WARN]:  ",
    "\x1B[91m[ERROR]: ",
    "\x1B[31m[FATAL]: ",
};

#define log_debug(...) printf("%s", level_strings[LOG_LEVEL_DEBUG]); printf(__VA_ARGS__); printf("\x1B[0m\n");
#define log_trace(...) printf("%s", level_strings[LOG_LEVEL_TRACE]); printf(__VA_ARGS__); printf("\x1B[0m\n");
#define log_info(...)  printf("%s", level_strings[LOG_LEVEL_INFO]); printf(__VA_ARGS__); printf("\x1B[0m\n");
#define log_warn(...)  printf("%s", level_strings[LOG_LEVEL_WARN]); printf(__VA_ARGS__); printf("\x1B[0m\n");
#define log_error(...) printf("%s", level_strings[LOG_LEVEL_ERROR]); printf(__VA_ARGS__); printf("\x1B[0m\n");
#define log_fatal(...) printf("%s", level_strings[LOG_LEVEL_FATAL]); printf(__VA_ARGS__); printf("\x1B[0m\n"); fflush(stdout); abort();
