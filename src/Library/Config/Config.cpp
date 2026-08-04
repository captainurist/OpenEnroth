#include "Config.h"

#include <cassert>
#include <string>
#include <vector>
#include <sstream>

#include <inicpp.h> // NOLINT: this is not a C system header.

#include "Library/Logger/Logger.h"

#include "Utility/Streams/FileInputStream.h"
#include "Utility/Streams/FileOutputStream.h"
#include "Utility/MapAccess.h"
#include "Utility/String/Format.h"
#include "Utility/String/Wrap.h"

Result<void> Config::load(std::string_view path) {
    FileInputStream stream;
    co_await stream.open(path); // Fails if the file doesn't exist.
    co_await load(&stream);
}

Result<void> Config::save(std::string_view path) const {
    FileOutputStream stream;
    co_await stream.open(path);
    co_await save(&stream);
    co_await stream.close();
}

Result<void> Config::load(InputStream *stream) {
    std::istringstream stdStream(co_await stream->readAll());

    ini::IniFile ini;
    ini.setCommentChar(';'); // Use ini comment char, not '#'.
    ini.decode(stdStream); // inicpp is external code and can throw - the coroutine catches, per our rules.

    for (const auto &[sectionName, iniSection] : ini) {
        if (ConfigSection *section = this->section(sectionName)) {
            for (const auto &[entryName, iniValue] : iniSection) {
                if (AnyConfigEntry *entry = section->entry(entryName)) {
                    try {
                        entry->setString(iniValue.as<std::string_view>());
                    } catch (const std::exception &e) {
                        logger->warning("Could not load config entry '[{}]/{}': {}. Value unchanged.",
                                        sectionName, entryName, e.what());
                    }
                }
            }
        }
    }
}

Result<void> Config::save(OutputStream *stream) const {
    // ini::IniFile doesn't support comments so we just write things out ourselves.
    for (ConfigSection *section : sections()) {
        co_await stream->write(fmt::format("[{}]\n", section->name()));

        for (AnyConfigEntry *entry : section->entries()) {
            if (!entry->description().empty())
                for (const std::string &line : wrapText(entry->description(), 78))
                    co_await stream->write(fmt::format("; {}\n", line));
            co_await stream->write(fmt::format("; Default is '{}'.\n", entry->defaultString()));
            co_await stream->write(fmt::format("{} = {}\n", entry->name(), entry->string()));
            co_await stream->write("\n");
        }

        co_await stream->write("\n");
    }
}

void Config::reset() {
    for (ConfigSection *section : sections())
        for (AnyConfigEntry *entry : section->entries())
            entry->reset();
}

void Config::registerSection(ConfigSection *section) {
    assert(section);
    assert(!_sectionByName.contains(section->name()));

    _sectionByName.emplace(section->name(), section);
}

ConfigSection *Config::section(std::string_view name) const {
    return valueOr(_sectionByName, name, nullptr);
}

std::vector<ConfigSection *> Config::sections() const {
    std::vector<ConfigSection *> result;
    for (const auto &[_, section] : _sectionByName)
        result.push_back(section);
    return result;
}
