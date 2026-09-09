#pragma once

#include <algorithm>

#include "Types.h"

/**
 * Convenience function for finding an index of a value in any container. Search is performed with `std::find` in O(N).
 *
 * @param container                     Container to look in.
 * @param value                         Value to look up.
 * @param def                           Default index to return if the value is not found.
 * @return                              Index of the provided `value` in the container, or `def` if the value was not
 *                                      found.
 */
template<class Container, class Value>
ssize_t indexOf(const Container &container, const Value &value, ssize_t def = -1) {
    auto pos = std::find(std::begin(container), std::end(container), value);
    return pos == std::end(container) ? -1 : std::distance(std::begin(container), pos);
}
