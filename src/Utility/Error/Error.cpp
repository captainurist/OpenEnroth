#include "Error.h"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <exception>
#include <string>
#include <utility>
#include <vector>

namespace {

void defaultFatalErrorHandler(const Error &error) {
    fmt::print(stderr, "Fatal error: {}\n", error.message());
}

FatalErrorHandler globalFatalErrorHandler = &defaultFatalErrorHandler;

} // namespace

Error Error::fromErrno(std::string_view arg) {
    assert(errno != 0);

    std::error_code code = std::error_code(errno, std::system_category());
    return Error(code, "{}: {}", arg, code.message());
}

Error Error::fromErrc(std::errc error, std::string_view arg) {
    assert(error != std::errc());

    std::error_code code = std::make_error_code(error);
    return Error(code, "{}: {}", arg, code.message());
}

Error Error::fromCurrentException() {
    assert(std::current_exception());

    try {
        throw;
    } catch (const std::system_error &e) {
        return Error(e.code(), "{}", e.what());
    } catch (const std::exception &e) {
        return Error("{}", e.what());
    } catch (...) {
        return Error("Unknown exception");
    }
}

std::string Error::message() const {
    if (!_node->cause)
        return _node->message;

    std::string result;
    for (const Node *node = _node.get(); node; node = node->cause.get()) {
        if (!result.empty())
            result += ": ";
        result += node->message;
    }
    return result;
}

std::string_view Error::innerMessage() const {
    const Node *node = _node.get();
    while (node->cause)
        node = node->cause.get();
    return node->message;
}

std::error_code Error::code() const {
    std::error_code result;
    for (const Node *node = _node.get(); node; node = node->cause.get())
        if (node->code)
            result = node->code;
    return result;
}

FatalErrorHandler setFatalErrorHandler(FatalErrorHandler handler) {
    return std::exchange(globalFatalErrorHandler, handler ? handler : &defaultFatalErrorHandler);
}

void fatalError(const Error &error) {
    globalFatalErrorHandler(error);
    std::abort();
}
