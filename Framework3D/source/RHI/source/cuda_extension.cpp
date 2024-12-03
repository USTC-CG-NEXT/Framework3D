#include <RHI/internal/cuda_extension.hpp>

USTC_CG_NAMESPACE_OPEN_SCOPE

#define OPTIX_CHECK_LOG(call)                                              \
    do {                                                                   \
        OptixResult res = call;                                            \
        const size_t sizeof_log_returned = sizeof_log;                     \
        sizeof_log =                                                       \
            sizeof(optix_log); /* reset sizeof_log for future calls */     \
        if (res != OPTIX_SUCCESS) {                                        \
            std::stringstream ss;                                          \
            ss << "Optix call '" << #call << "' failed: " __FILE__ ":"     \
               << __LINE__ << ")\nLog:\n"                                  \
               << optix_log                                                \
               << (sizeof_log_returned > sizeof(optix_log) ? "<TRUNCATED>" \
                                                           : "")           \
               << "\n";                                                    \
            printf("%s", ss.str().c_str());                                \
        }                                                                  \
    } while (0)

#define OPTIX_CHECK(call)                                              \
    do {                                                               \
        OptixResult res = call;                                        \
        if (res != OPTIX_SUCCESS) {                                    \
            std::stringstream ss;                                      \
            ss << "Optix call '" << #call << "' failed: " __FILE__ ":" \
               << __LINE__ << ")\n";                                   \
            printf("%s", ss.str().c_str());                            \
        }                                                              \
    } while (0)

char optix_log[2048];

USTC_CG_NAMESPACE_CLOSE_SCOPE