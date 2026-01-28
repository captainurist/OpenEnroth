#include "BlobOutputStream.h"

#include <cassert>
#include <string>
#include <utility>

BlobOutputStream::BlobOutputStream() {}


BlobOutputStream::BlobOutputStream(Blob *target, std::string_view displayPath) : BlobOutputStream() {
    open(target, displayPath);
}

BlobOutputStream::~BlobOutputStream() {
    closeInternal();
}

void BlobOutputStream::open(Blob *target, std::string_view displayPath) {
    assert(target);

    closeInternal();

    _target = target;
    _displayPath = displayPath;
}

void BlobOutputStream::write(const void *data, size_t size) {
    assert(_target);

    _buffer.resize(_buffer.size() + size);
    memcpy(_buffer.data() + _buffer.size() - size, data, size); // TODO(captainurist): #cpp23 resize_and_overwrite
}

void BlobOutputStream::flush() {
    assert(_target); // Should be open.

    // Flushing does the only sane thing, which is just making a copy. Shouldn't really be necessary in any of the
    // possible use cases.
    *_target = Blob::fromString(_buffer).withDisplayPath(_displayPath);
}

void BlobOutputStream::close() {
    closeInternal();
}

std::string BlobOutputStream::displayPath() const {
    return _displayPath;
}

void BlobOutputStream::closeInternal() {
    if (!_target)
        return;

    *_target = Blob::fromString(std::move(_buffer)).withDisplayPath(_displayPath);
    _target = nullptr;
    _buffer = {};
    _displayPath = {};
}
