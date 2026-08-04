#include "MemoryFileSystemInputStream.h"

#include <cassert>
#include <memory>
#include <utility>

#include "MemoryFileSystem.h"

namespace detail {

MemoryFileSystemInputStream::MemoryFileSystemInputStream(std::shared_ptr<MemoryFileData> data) : _data(std::move(data)) {
    assert(_data);
    assert(_data->writerCount == 0);

    _data->readerCount++;
    base_type::open(_data->blob);
}

MemoryFileSystemInputStream::~MemoryFileSystemInputStream() {
    destroy();
}

Result<void> MemoryFileSystemInputStream::_close() {
    assert(_data);
    _data->readerCount--;
    _data.reset();
    return base_type::_close();
}

} // namespace detail
