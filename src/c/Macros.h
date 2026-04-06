/// Generic C macros.

// Macros
#define MACRO_START do {
#define MACRO_END } while (0)

// Maths
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define ABS(a) (((a) >= 0) ? (a) : ((a) * -1))
#define ABSDIFF(a, b) ((a) >= (b) ? (a) - (b) : (b) - (a))

// Debugging
#if DEBUG
    #define LOG(...) APP_LOG(APP_LOG_LEVEL_INFO, __VA_ARGS__)
    #define TRACE(...) APP_LOG(APP_LOG_LEVEL_DEBUG, __VA_ARGS__)
    #define ASSERT(condition) if (!(condition)) APP_LOG(APP_LOG_LEVEL_ERROR, \
        "ASSERTION FAILED AT %s:%d - "#condition, __FILE__, __LINE__)
#else // !DEBUG
    #define LOG(...)
    #define TRACE(...)
    #define ASSERT(condition) (void)(condition)
#endif // !DEBUG

// Constants
#define TIME_MAX INT32_MAX  // max value of time_t
#define MS_PER_S (1000)  // milliseconds per second