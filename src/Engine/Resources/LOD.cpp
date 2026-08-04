#include "LOD.h"

#include <memory>

#include "EngineFileSystem.h"

std::unique_ptr<LodReader> pGames_LOD;

bool Initialize_GamesLOD_NewLOD() {
    pGames_LOD = std::make_unique<LodReader>();
    // TODO(captainurist): #exceptions Should be surfaced to the user as "please reinstall the game", see
    //                     ResourceManager::open.
    mustSucceed(pGames_LOD->open(dfs->read("data/games.lod")));
    return true;
}
