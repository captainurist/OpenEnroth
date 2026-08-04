#pragma once

#include <map>
#include <string>
#include <vector>

#include "Utility/Error/Result.h"
#include "Utility/String/TransparentFunctors.h"
#include "ConfigFwd.h"
#include "ConfigSection.h"
#include "ConfigEntry.h"

class OutputStream;
class InputStream;

class Config {
 public:
    Config() = default;
    Config(const Config &other) = delete; // non-copyable
    Config(Config &&other) = delete; // non-movable

    Result<void> load(std::string_view path);
    Result<void> save(std::string_view path) const;
    Result<void> load(InputStream *stream);
    Result<void> save(OutputStream *stream) const;

    void reset();

    void registerSection(ConfigSection *section);

    ConfigSection *section(std::string_view name) const;

    std::vector<ConfigSection *> sections() const;

 private:
    std::map<std::string, ConfigSection *, TransparentStringLess> _sectionByName;
};
