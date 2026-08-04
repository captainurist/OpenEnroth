#pragma once

#include <cstddef>
#include <string_view>

#include "Utility/Error/Error.h"

/**
 * @param bytesRead                     Number of bytes that were actually read.
 * @param bytesExpected                 Number of bytes that were expected.
 * @param typeName                      Name of the type that was being deserialized.
 * @return                              Error reporting that there wasn't enough data in the stream to deserialize
 *                                      a value.
 */
[[nodiscard]] Error binarySerializationNoMoreDataError(size_t bytesRead, size_t bytesExpected, std::string_view typeName);

/**
 * @param bytesRead                     Number of bytes that were read.
 * @param bytesLeft                     Number of bytes that were left in the stream.
 * @param typeName                      Name of the type that was being deserialized.
 * @return                              Error reporting that there was unexpected data left in the stream after
 *                                      deserialization.
 */
[[nodiscard]] Error binarySerializationLeftoverDataError(size_t bytesRead, size_t bytesLeft, std::string_view typeName);
