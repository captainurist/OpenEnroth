#pragma once

#include <cstddef>
#include <string_view>

class InputStream;

/**
 * Puts `stream` into a failed state, reporting that there wasn't enough data in it to deserialize a value.
 *
 * @param stream                        Stream to put into a failed state.
 * @param bytesRead                     Number of bytes that were actually read.
 * @param bytesExpected                 Number of bytes that were expected.
 * @param typeName                      Name of the type that was being deserialized.
 */
void setBinarySerializationNoMoreDataError(InputStream &stream, size_t bytesRead, size_t bytesExpected,
                                           std::string_view typeName);

/**
 * Puts `stream` into a failed state, reporting that there was unexpected data left in it after deserialization.
 *
 * @param stream                        Stream to put into a failed state.
 * @param bytesRead                     Number of bytes that were read.
 * @param bytesLeft                     Number of bytes that were left in the stream.
 * @param typeName                      Name of the type that was being deserialized.
 */
void setBinarySerializationLeftoverDataError(InputStream &stream, size_t bytesRead, size_t bytesLeft,
                                             std::string_view typeName);
