#include "BlobInputStream.h"

#include <cassert>
#include <algorithm>
#include <utility>

BlobInputStream::BlobInputStream(Blob &&blob) {
    open(std::move(blob));
}

BlobInputStream::BlobInputStream(const Blob &blob) {
    open(blob);
}

void BlobInputStream::open(Blob &&blob) {
    _blob = std::move(blob);
    const char *start = static_cast<const char *>(_blob.data());
    base_type::open(Buffer(start, start, start + _blob.size()), _blob.size(), _blob.displayPath());
}

void BlobInputStream::open(const Blob &blob) {
    open(Blob::share(blob));
}

Blob BlobInputStream::readAsBlob(size_t size) {
    assert(isOpen());

    size_t bytes = skip(size);
    return _blob.subBlob(position() - bytes, bytes);
}

Blob BlobInputStream::readAsBlobOrFail(size_t size) {
    assert(isOpen());

    if (failed() || size > this->size() - position()) [[unlikely]] {
        setReadError(size, failed() ? 0 : this->size() - position());
        return Blob();
    }

    return readAsBlob(size);
}

Blob BlobInputStream::readAllAsBlob() {
    assert(isOpen());

    size_t bytes = skip(size());
    return _blob.subBlob(position() - bytes);
}
