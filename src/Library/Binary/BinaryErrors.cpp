#include "BinaryErrors.h"

#include "Utility/Error/Error.h"
#include "Utility/Streams/InputStream.h"

void setBinarySerializationNoMoreDataError(InputStream &stream, size_t bytesRead, size_t bytesExpected,
                                           std::string_view typeName) {
    stream.setFailed(Error("Could not read '{}' from binary stream '{}': expected {} bytes, got only {}",
                           typeName, stream.displayPath(), bytesExpected, bytesRead));
}

void setBinarySerializationLeftoverDataError(InputStream &stream, size_t bytesRead, size_t bytesLeft,
                                             std::string_view typeName) {
    stream.setFailed(Error("Unexpected data left in binary stream '{}' after reading '{}': {} bytes read, {} bytes left",
                           stream.displayPath(), typeName, bytesRead, bytesLeft));
}
