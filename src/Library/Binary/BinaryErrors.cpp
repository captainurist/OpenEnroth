#include "BinaryErrors.h"

Error binarySerializationNoMoreDataError(size_t bytesRead, size_t bytesExpected, std::string_view typeName) {
    return Error("Could not read '{}' from binary stream: expected {} bytes, got only {}", typeName, bytesExpected, bytesRead);
}

Error binarySerializationLeftoverDataError(size_t bytesRead, size_t bytesLeft, std::string_view typeName) {
    return Error("Unexpected data left in binary stream after reading '{}': {} bytes read, {} bytes left", typeName, bytesRead, bytesLeft);
}
