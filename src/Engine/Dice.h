#pragma once

#include <cassert>
#include <memory>
#include <string>

#include "Library/Random/RandomEngine.h"

#include "Utility/String/Format.h"

class Dice {
 public:
    constexpr Dice() = default;

    constexpr Dice(std::string_view string) {

    }

    constexpr Dice(int count, int sides, int bonus): _count(count), _sides(sides), _bonus(bonus) {
        assert(count >= 0);
        assert(sides >= 0);
        assert(bonus >= 0);
    }

    int roll(RandomEngine *rng) const {
        return rng->randomDice(_count, _sides) + _bonus;
    }

    int roll(const std::unique_ptr<RandomEngine> &rng) const {
        return roll(rng.get());
    }

    int count() const {
        return _count;
    }

    int sides() const {
        return _sides;
    }

    int bonus() const {
        return _bonus;
    }

    std::string toString() const {
        if (_bonus) {
            return fmt::format("{}d{}+{}", _count, _sides, _bonus);
        } else {
            return fmt::format("{}d{}", _count, _sides);
        }
    }

    explicit operator bool() const {
        return _count > 0;
    }

 private:
    int _count = 0;
    int _sides = 0;
    int _bonus = 0;
};
