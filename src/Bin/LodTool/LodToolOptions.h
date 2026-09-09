#pragma once

#include <string>

struct LodToolOptions {
    enum class Subcommand {
        SUBCOMMAND_LS,
        SUBCOMMAND_DUMP,
        SUBCOMMAND_CAT,
        SUBCOMMAND_EXTRACT,
        SUBCOMMAND_CONVERT,
    };
    using enum Subcommand;

    struct CatOptions {
        std::string entry;
    };

    struct ExtractOptions {
        std::string output;
    };

    struct ConvertOptions {
        std::string input;
        std::string output;
    };

    Subcommand subcommand = SUBCOMMAND_DUMP;
    std::string path;
    bool helpPrinted = false; // True means that help message was already printed.
    CatOptions cat;
    ExtractOptions extract;
    ConvertOptions convert;
    bool raw = false; // Raw flag, shared by cat & extract.
    std::string palettesLodPath; // Path to bitmaps.lod for sprite palettes.

    static LodToolOptions parse(int argc, char **argv);
};
