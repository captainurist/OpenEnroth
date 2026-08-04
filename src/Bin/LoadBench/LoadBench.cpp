#include <chrono>
#include <cstdlib>
#include <exception>
#include <string>
#include <utility>
#include <vector>

#include "Engine/Snapshots/CompositeSnapshots.h"

#include "Library/Lod/LodReader.h"
#include "Library/LodFormats/LodFormats.h"

#include "Utility/String/Format.h"
#include "Utility/UnicodeCrt.h"

// LoadBench: benchmarks the deserialization part of level loading.
//
// Reads every map in games.lod (.blv/.odm locations plus their .dlv/.ddm deltas), then repeatedly decompresses and
// deserializes all of them, reporting per-bucket timings. This is the deserialize-heavy path that the error
// handling migration (see ExceptionFreeErrorHandling.md) touches, so this tool is the yardstick for comparing
// master / Result / coroutine implementations, with and without LTO, across compilers.
//
// The LOD file is read into memory up front and every map gets a warm-up pass before the measurements, so the
// numbers exclude disk I/O and page cache effects. Single-threaded on purpose.
//
// Usage: LoadBench [path-to-games.lod] [iterations]
//        (path defaults to $OPENENROTH_MM7_PATH/data/games.lod, iterations default to 15)

namespace {

struct MapEntry {
    std::string name;
    bool isIndoor = false;
    Blob rawLocation; // Raw LOD entries, possibly compressed.
    Blob rawDelta;
};

struct Buckets {
    double decompressMs = 0;
    double locationsMs = 0;
    double deltasMs = 0;

    double totalMs() const {
        return decompressMs + locationsMs + deltasMs;
    }
};

double msSince(std::chrono::steady_clock::time_point start) {
    auto elapsed = std::chrono::steady_clock::now() - start;
    return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(elapsed).count();
}

/**
 * Runs decompression + deserialization for a single map, accumulating the time spent into `buckets`.
 *
 * @param map                           Map to process.
 * @param[in,out] buckets               Timing accumulators.
 * @return                              Total decompressed payload size in bytes.
 */
size_t processMap(const MapEntry &map, Buckets *buckets) {
    auto start = std::chrono::steady_clock::now();
    Blob location = lod::decodeMaybeCompressed(map.rawLocation).orThrow();
    Blob delta = lod::decodeMaybeCompressed(map.rawDelta).orThrow();
    buckets->decompressMs += msSince(start);

    if (map.isIndoor) {
        IndoorLocation_MM7 rawLocation;
        start = std::chrono::steady_clock::now();
        deserialize(location, &rawLocation);
        buckets->locationsMs += msSince(start);

        IndoorDelta_MM7 rawDelta;
        start = std::chrono::steady_clock::now();
        deserialize(delta, &rawDelta, tags::context(rawLocation));
        buckets->deltasMs += msSince(start);
    } else {
        OutdoorLocation_MM7 rawLocation;
        start = std::chrono::steady_clock::now();
        deserialize(location, &rawLocation);
        buckets->locationsMs += msSince(start);

        OutdoorDelta_MM7 rawDelta;
        start = std::chrono::steady_clock::now();
        deserialize(delta, &rawDelta, tags::context(rawLocation));
        buckets->deltasMs += msSince(start);
    }

    return location.size() + delta.size();
}

int runLoadBench(std::string_view lodPath, int iterations) {
    LodReader lod;
    lod.open(lodPath).orThrow();

    // Collect all maps, reading the raw entries into memory.
    std::vector<MapEntry> maps;
    for (const std::string &name : lod.ls()) {
        bool isIndoor = name.ends_with(".blv");
        if (!isIndoor && !name.ends_with(".odm"))
            continue;

        std::string baseName = name.substr(0, name.size() - 4);
        std::string deltaName = baseName + (isIndoor ? ".dlv" : ".ddm");
        if (!lod.exists(deltaName)) {
            fmt::println(stderr, "LoadBench: skipping '{}', no matching '{}'.", name, deltaName);
            continue;
        }

        MapEntry &map = maps.emplace_back();
        map.name = std::move(baseName);
        map.isIndoor = isIndoor;
        map.rawLocation = lod.read(name).orThrow();
        map.rawDelta = lod.read(deltaName).orThrow();
    }

    // Warm-up pass: fault everything in, and drop maps that don't survive a round trip so that the timed loop
    // never has to deal with errors.
    size_t payloadSize = 0;
    int indoorCount = 0;
    std::erase_if(maps, [&] (const MapEntry &map) {
        try {
            Buckets ignored;
            payloadSize += processMap(map, &ignored);
            indoorCount += map.isIndoor;
            return false;
        } catch (const std::exception &e) {
            fmt::println(stderr, "LoadBench: skipping '{}': {}", map.name, e.what());
            return true;
        }
    });

    // The measurements. Per-bucket minimum over all iterations - minimum is the noise-resistant statistic for
    // a fixed workload.
    Buckets best;
    double bestTotalMs = 0;
    for (int i = 0; i < iterations; i++) {
        Buckets current;
        for (const MapEntry &map : maps)
            processMap(map, &current);

        if (i == 0 || current.decompressMs < best.decompressMs)
            best.decompressMs = current.decompressMs;
        if (i == 0 || current.locationsMs < best.locationsMs)
            best.locationsMs = current.locationsMs;
        if (i == 0 || current.deltasMs < best.deltasMs)
            best.deltasMs = current.deltasMs;
        if (i == 0 || current.totalMs() < bestTotalMs)
            bestTotalMs = current.totalMs();
    }

    fmt::println("LoadBench: {} maps ({} indoor, {} outdoor), {:.1f} MB decompressed payload, {} iterations, best per bucket:",
                 maps.size(), indoorCount, maps.size() - indoorCount, static_cast<double>(payloadSize) / (1 << 20), iterations);
    fmt::println("  decompress  {:8.2f} ms", best.decompressMs);
    fmt::println("  locations   {:8.2f} ms", best.locationsMs);
    fmt::println("  deltas      {:8.2f} ms", best.deltasMs);
    fmt::println("  total       {:8.2f} ms  (best single iteration)", bestTotalMs);
    fmt::println("RESULT maps={} decompress_ms={:.2f} locations_ms={:.2f} deltas_ms={:.2f} total_ms={:.2f}",
                 maps.size(), best.decompressMs, best.locationsMs, best.deltasMs, bestTotalMs);
    return 0;
}

} // namespace

int main(int argc, char **argv) {
    try {
        UnicodeCrt _(argc, argv);

        std::string lodPath;
        if (argc >= 2) {
            lodPath = argv[1];
        } else if (const char *mm7Path = std::getenv("OPENENROTH_MM7_PATH")) {
            lodPath = fmt::format("{}/data/games.lod", mm7Path);
        } else {
            fmt::println(stderr, "Usage: LoadBench [path-to-games.lod] [iterations]");
            fmt::println(stderr, "       (path defaults to $OPENENROTH_MM7_PATH/data/games.lod)");
            return 1;
        }

        int iterations = argc >= 3 ? std::atoi(argv[2]) : 15;
        if (iterations <= 0) {
            fmt::println(stderr, "LoadBench: invalid iteration count '{}'.", argv[2]);
            return 1;
        }

        return runLoadBench(lodPath, iterations);
    } catch (const std::exception &e) {
        fmt::print(stderr, "{}\n", e.what());
        return 1;
    }
}
