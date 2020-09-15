#pragma once

#ifdef APOLLO_API
# undef APOLLO_API
#endif
#define APOLLO_API __attribute__((visibility("default")))

#include <string>

namespace apollo {
namespace v2x {
namespace obu {
namespace conv {

/**
 * Decode a SSM data to serialized proto data
 *
 * @param buf       SSM data buffer
 * @param buflen    SSM buffer length
 * @param res       serialized proto data
 * @return          true  if success and res changed
 *                  false if failed
 */
APOLLO_API bool DecodeSSM(const void *buf, size_t buflen, std::string *res);

} // namespace conv
} // namespace obu
} // namespace v2x
} // namespace apollo
