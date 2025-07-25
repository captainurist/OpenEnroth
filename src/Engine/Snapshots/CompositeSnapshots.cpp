#include "CompositeSnapshots.h"

#include <string>
#include <algorithm>
#include <tuple>

#include "Engine/Graphics/Indoor.h"
#include "Engine/Graphics/Outdoor.h"
#include "Engine/Objects/Decoration.h"
#include "Engine/Objects/DecorationList.h"
#include "Engine/Graphics/Overlays.h"
#include "Engine/Graphics/Sprites.h"
#include "Engine/Objects/SpriteObject.h"
#include "Engine/Objects/ObjectList.h"
#include "Engine/Objects/Chest.h"
#include "Engine/Objects/Actor.h"
#include "Engine/Tables/ItemTable.h"
#include "Engine/Engine.h"
#include "Engine/Party.h"
#include "Engine/Data/TileEnumFunctions.h"
#include "Engine/Tables/TileTable.h"

#include "GUI/GUIFont.h"

#include "Library/Snapshots/CommonSnapshots.h"
#include "Library/Lod/LodWriter.h"
#include "Library/Lod/LodReader.h"

void reconstruct(const IndoorLocation_MM7 &src, IndoorLocation *dst) {
    reconstruct(src.vertices, &dst->pVertices);
    reconstruct(src.faces, &dst->pFaces);
    reconstruct(src.faceData, &dst->pLFaces);

    for (size_t i = 0, j = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];

        pFace->pVertexIDs = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        // Skipping pXInterceptDisplacements.
        j += pFace->uNumVertices + 1;

        // Skipping pYInterceptDisplacements.
        j += pFace->uNumVertices + 1;

        // Skipping pZInterceptDisplacements.
        j += pFace->uNumVertices + 1;

        pFace->pVertexUIDs = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        pFace->pVertexVIDs = dst->pLFaces.data() + j;
        j += pFace->uNumVertices + 1;

        assert(j <= dst->pLFaces.size());
    }

    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];

        std::string texName;
        reconstruct(src.faceTextures[i], &texName);
        pFace->SetTexture(texName);
    }

    reconstruct(src.faceExtras, &dst->pFaceExtras);

    std::string textureName;
    for (unsigned i = 0; i < dst->pFaceExtras.size(); ++i) {
        reconstruct(src.faceExtraTextures[i], &textureName);

        if (textureName.empty())
            dst->pFaceExtras[i].uAdditionalBitmapID = -1;
        else
            dst->pFaceExtras[i].uAdditionalBitmapID = -1; //pBitmaps_LOD->loadTexture(textureName); // TODO(captainurist): unused for some reason.
    }

    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        BLVFace *pFace = &dst->pFaces[i];
        BLVFaceExtra *pFaceExtra = &dst->pFaceExtras[pFace->uFaceExtraID];

        if (pFaceExtra->uEventID) {
            if (pFaceExtra->HasEventHint())
                pFace->uAttributes |= FACE_HAS_EVENT;
            else
                pFace->uAttributes &= ~FACE_HAS_EVENT;
        }
    }

    reconstruct(src.sectors, &dst->pSectors);
    reconstruct(src.sectorData, &dst->ptr_0002B0_sector_rdata);

    for (size_t i = 0, j = 0; i < dst->pSectors.size(); ++i) {
        BLVSector *pSector = &dst->pSectors[i];

        pSector->pFloors = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumFloors;

        pSector->pWalls = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumWalls;

        pSector->pCeilings = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumCeilings;

        pSector->pFluids = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumFluids;

        pSector->pPortals = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumPortals;

        pSector->pFaceIDs = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumFaces;

        pSector->pCogs = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumCogs;

        pSector->pDecorationIDs = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumDecorations;

        pSector->pMarkers = dst->ptr_0002B0_sector_rdata.data() + j;
        j += pSector->uNumMarkers;

        assert(j <= dst->ptr_0002B0_sector_rdata.size());
    }

    reconstruct(src.sectorLightData, &dst->ptr_0002B8_sector_lrdata);

    for (unsigned i = 0, j = 0; i < dst->pSectors.size(); ++i) {
        BLVSector *pSector = &dst->pSectors[i];

        pSector->pLights = dst->ptr_0002B8_sector_lrdata.data() + j;
        j += pSector->uNumLights;

        assert(j <= dst->ptr_0002B8_sector_lrdata.size());
    }

    reconstruct(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        reconstruct(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    reconstruct(src.lights, &dst->pLights);
    reconstruct(src.bspNodes, &dst->pNodes);
    reconstruct(src.spawnPoints, &dst->pSpawnPoints);
    reconstruct(src.mapOutlines, &dst->pMapOutlines);
}

void deserialize(InputStream &src, IndoorLocation_MM7 *dst) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->vertices);
    deserialize(src, &dst->faces);
    deserialize(src, &dst->faceData, tags::presized(dst->header.uFaces_fdata_Size / sizeof(uint16_t)));
    deserialize(src, &dst->faceTextures, tags::presized(dst->faces.size()));
    deserialize(src, &dst->faceExtras);
    deserialize(src, &dst->faceExtraTextures, tags::presized(dst->faceExtras.size()));
    deserialize(src, &dst->sectors);
    deserialize(src, &dst->sectorData, tags::presized(dst->header.uSector_rdata_Size / sizeof(uint16_t)));
    deserialize(src, &dst->sectorLightData, tags::presized(dst->header.uSector_lrdata_Size / sizeof(uint16_t)));
    deserialize(src, &dst->doorCount);
    deserialize(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    deserialize(src, &dst->lights);
    deserialize(src, &dst->bspNodes);
    deserialize(src, &dst->spawnPoints);
    deserialize(src, &dst->mapOutlines);
}

void snapshot(const IndoorLocation &src, IndoorDelta_MM7 *dst) {
    snapshot(src.dlv, &dst->header.info);
    dst->header.totalFacesCount = src.pFaces.size();
    dst->header.bmodelCount = 0;
    dst->header.decorationCount = pLevelDecorations.size();

    snapshot(src._visible_outlines, &dst->visibleOutlines);

    // Symmetric to what's happening in reconstruct - not all of the attributes need to be saved in a delta.
    dst->faceAttributes.clear();
    for (const BLVFace &pFace : pIndoor->pFaces)
        dst->faceAttributes.push_back(std::to_underlying(pFace.uAttributes & ~(FACE_HAS_EVENT | FACE_TEXTURE_FRAME)));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    snapshot(pActors, &dst->actors);
    snapshot(pSpriteObjects, &dst->spriteObjects);
    snapshot(vChests, &dst->chests);
    snapshot(src.pDoors, &dst->doors);
    snapshot(src.ptr_0002B4_doors_ddata, &dst->doorsData);
    snapshot(engine->_persistentVariables, &dst->eventVariables);
    snapshot(src.stru1, &dst->locationTime);
}

void reconstruct(const IndoorDelta_MM7 &src, IndoorLocation *dst) {
    reconstruct(src.header.info, &dst->dlv); // XXX
    reconstruct(src.visibleOutlines, &dst->_visible_outlines);

    for (size_t i = 0; i < dst->pMapOutlines.size(); ++i) {
        BLVMapOutline *pVertex = &dst->pMapOutlines[i];
        if ((uint8_t)(1 << (7 - i % 8)) & dst->_visible_outlines[i / 8])
            pVertex->uFlags |= 1;
    }

    // Not all of the attributes need to be restored.
    size_t attributeIndex = 0;
    for (BLVFace &face : dst->pFaces) {
        face.uAttributes &= FACE_TEXTURE_FRAME | FACE_HAS_EVENT;
        face.uAttributes |= FaceAttributes(src.faceAttributes[attributeIndex++]) & ~(FACE_HAS_EVENT | FACE_TEXTURE_FRAME);
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    reconstruct(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    reconstruct(src.spriteObjects, &pSpriteObjects);

    for (size_t i = 0; i < pSpriteObjects.size(); ++i) {
        if (pSpriteObjects[i].containing_item.itemId != ITEM_NULL && !(pSpriteObjects[i].uAttributes & SPRITE_MISSILE)) {
            pSpriteObjects[i].uType = static_cast<SpriteId>(pItemTable->items[pSpriteObjects[i].containing_item.itemId].spriteId);
            pSpriteObjects[i].uObjectDescID = pObjectList->ObjectIDByItemID(pSpriteObjects[i].uType);
        }
    }

    vChests.resize(src.chests.size());
    for (size_t i = 0; i < src.chests.size(); ++i)
        reconstruct(src.chests[i], &vChests[i], tags::context<int>(i));

    reconstruct(src.doors, &dst->pDoors);
    reconstruct(src.doorsData, &dst->ptr_0002B4_doors_ddata);

    for (unsigned i = 0, j = 0; i < dst->pDoors.size(); ++i) {
        BLVDoor *pDoor = &dst->pDoors[i];

        pDoor->pVertexIDs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumVertices;

        pDoor->pFaceIDs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumFaces;

        pDoor->pSectorIDs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumSectors;

        pDoor->pDeltaUs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumFaces;

        pDoor->pDeltaVs = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumFaces;

        pDoor->pXOffsets = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumOffsets;

        pDoor->pYOffsets = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumOffsets;

        pDoor->pZOffsets = dst->ptr_0002B4_doors_ddata.data() + j;
        j += pDoor->uNumOffsets;

        assert(j <= dst->ptr_0002B4_doors_ddata.size());
    }

    for (size_t i = 0; i < dst->pDoors.size(); ++i) {
        BLVDoor *pDoor = &dst->pDoors[i];

        for (unsigned j = 0; j < pDoor->uNumFaces; ++j) {
            BLVFace *pFace = &dst->pFaces[pDoor->pFaceIDs[j]];
            BLVFaceExtra *pFaceExtra = &dst->pFaceExtras[pFace->uFaceExtraID];

            pDoor->pDeltaUs[j] = pFaceExtra->sTextureDeltaU;
            pDoor->pDeltaVs[j] = pFaceExtra->sTextureDeltaV;
        }
    }

    reconstruct(src.eventVariables, &engine->_persistentVariables);
    reconstruct(src.locationTime, &dst->stru1);
}

void serialize(const IndoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.visibleOutlines, dst);
    serialize(src.faceAttributes, dst, tags::unsized);
    serialize(src.decorationFlags, dst, tags::unsized);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.doors, dst, tags::unsized);
    serialize(src.doorsData, dst, tags::unsized);
    serialize(src.eventVariables, dst);
    serialize(src.locationTime, dst);
}

void deserialize(InputStream &src, IndoorDelta_MM7 *dst, ContextTag<IndoorLocation_MM7> ctx) {
    deserialize(src, &dst->header);
    deserialize(src, &dst->visibleOutlines);
    deserialize(src, &dst->faceAttributes, tags::presized(ctx->faces.size()));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->doors, tags::presized(ctx->doorCount));
    deserialize(src, &dst->doorsData, tags::presized(ctx->header.uDoors_ddata_Size / sizeof(int16_t)));
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->locationTime);
}

void reconstruct(std::tuple<const BSPModelData_MM7 &, const BSPModelExtras_MM7 &> src, BSPModel *dst) {
    const auto &[srcData, srcExtras] = src;

    // dst->index is set externally.
    reconstruct(srcData.pModelName, &dst->pModelName);
    reconstruct(srcData.pModelName2, &dst->pModelName2);
    dst->field_40 = srcData.field_40;
    dst->sCenterX = srcData.sCenterX;
    dst->sCenterY = srcData.sCenterY;
    dst->vPosition = srcData.vPosition.toFloat();
    dst->pBoundingBox.x1 = srcData.sMinX;
    dst->pBoundingBox.y1 = srcData.sMinY;
    dst->pBoundingBox.z1 = srcData.sMinZ;
    dst->pBoundingBox.x2 = srcData.sMaxX;
    dst->pBoundingBox.y2 = srcData.sMaxY;
    dst->pBoundingBox.z2 = srcData.sMaxZ;
    dst->sSomeOtherMinX = srcData.sSomeOtherMinX;
    dst->sSomeOtherMinY = srcData.sSomeOtherMinY;
    dst->sSomeOtherMinZ = srcData.sSomeOtherMinZ;
    dst->sSomeOtherMaxX = srcData.sSomeOtherMaxX;
    dst->sSomeOtherMaxY = srcData.sSomeOtherMaxY;
    dst->sSomeOtherMaxZ = srcData.sSomeOtherMaxZ;
    dst->vBoundingCenter = srcData.vBoundingCenter.toFloat();
    dst->sBoundingRadius = srcData.sBoundingRadius;

    reconstruct(srcExtras.vertices, &dst->pVertices);
    reconstruct(srcExtras.faces, &dst->pFaces);

    for (size_t i = 0; i < dst->pFaces.size(); i++)
        dst->pFaces[i].index = i;

    dst->pFacesOrdering = srcExtras.faceOrdering;

    reconstruct(srcExtras.bspNodes, &dst->pNodes);

    std::string textureName;
    for (size_t i = 0; i < dst->pFaces.size(); ++i) {
        reconstruct(srcExtras.faceTextures[i], &textureName);
        dst->pFaces[i].SetTexture(textureName);

        if (dst->pFaces[i].sCogTriggeredID) {
            if (dst->pFaces[i].HasEventHint())
                dst->pFaces[i].uAttributes |= FACE_HAS_EVENT;
            else
                dst->pFaces[i].uAttributes &= ~FACE_HAS_EVENT;
        }
    }
}

static int mapToGlobalTileId(const std::array<int, 4> &baseIds, int localTileId) {
    // Tiles in tilemap:
    // [0..90) are mapped as-is, but seem to be mostly invalid. Only global tile ids [1..12] are valid (all are dirt),
    //         the rest are "pending", effectively invalid.
    // [90..126) map to tileset #1.
    // [126..162) map to tileset #2.
    // [162..198) map to tileset #3.
    // [198..234) map to tileset #4 (road).
    // [234..255) are invalid.
    if (localTileId < 90)
        return localTileId;

    if (localTileId >= 234)
        return 0;

    int tilesetIndex = (localTileId - 90) / 36;
    int tilesetOffset = (localTileId - 90) % 36;
    return baseIds[tilesetIndex] + tilesetOffset;
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorTerrain *dst) {
    std::array<int, 4> baseTileIds;
    for (int i = 0; i < 4; i++) {
        reconstruct(src.tileTypes[i].tileset, &dst->_tilesets[i]);
        baseTileIds[i] = pTileTable->tileId(dst->_tilesets[i], isRoad(dst->_tilesets[i]) ? TILE_VARIANT_ROAD_N_S_E_W : TILE_VARIANT_BASE1);
    }

    for (int y = 0; y < 128; y++)
        for (int x = 0; x < 128; x++)
            dst->_heightMap[y][x] = src.heightMap[y * 128 + x];

    for (int y = 0; y < 127; y++)
        for (int x = 0; x < 127; x++)
            dst->_originalTileMap[y][x] = mapToGlobalTileId(baseTileIds, src.tileMap[y * 128 + x]);

    dst->recalculateNormals();
    dst->recalculateTransitions(&dst->_tileMap);

    dst->_tileMap = Image<int16_t>::copy(dst->_originalTileMap);
}

void reconstruct(const OutdoorLocation_MM7 &src, OutdoorLocation *dst) {
    reconstruct(src.name, &dst->level_filename);
    reconstruct(src.fileName, &dst->location_filename);
    reconstruct(src.desciption, &dst->location_file_description);
    reconstruct(src.skyTexture, &dst->sky_texture_filename);
    reconstruct(src, &dst->pTerrain);

    dst->pBModels.clear();
    for (size_t i = 0; i < src.models.size(); i++) {
        BSPModel &model = dst->pBModels.emplace_back();
        model.index = i;
        reconstruct(std::forward_as_tuple(src.models[i], src.modelExtras[i]), &model);

        // Recalculate bounding spheres, the ones stored in data files are borked.
        model.vBoundingCenter = model.pBoundingBox.center().toFloat();
        model.sBoundingRadius = model.pBoundingBox.size().toFloat().length() / 2.0f;
    }

    reconstruct(src.decorations, &pLevelDecorations);

    std::string decorationName;
    for (size_t i = 0; i < pLevelDecorations.size(); ++i) {
        reconstruct(src.decorationNames[i], &decorationName);
        pLevelDecorations[i].uDecorationDescID = pDecorationList->GetDecorIdByName(decorationName);
    }

    reconstruct(src.decorationPidList, &dst->pFaceIDLIST);
    reconstruct(src.decorationMap, &dst->pOMAP);
    reconstruct(src.spawnPoints, &dst->pSpawnPoints);
}

void deserialize(InputStream &src, OutdoorLocation_MM7 *dst) {
    deserialize(src, &dst->name);
    deserialize(src, &dst->fileName);
    deserialize(src, &dst->desciption);
    deserialize(src, &dst->skyTexture);
    deserialize(src, &dst->groundTilesetUnused);
    deserialize(src, &dst->tileTypes);
    deserialize(src, &dst->heightMap);
    deserialize(src, &dst->tileMap);
    deserialize(src, &dst->attributeMap);
    deserialize(src, &dst->normalCount);
    deserialize(src, &dst->someOtherMap);
    deserialize(src, &dst->normalMap);
    deserialize(src, &dst->normals, tags::presized(dst->normalCount));
    deserialize(src, &dst->models);

    dst->modelExtras.clear();
    for (const BSPModelData_MM7 &model : dst->models) {
        BSPModelExtras_MM7 &extra = dst->modelExtras.emplace_back();
        deserialize(src, &extra.vertices, tags::presized(model.uNumVertices));
        deserialize(src, &extra.faces, tags::presized(model.uNumFaces));
        deserialize(src, &extra.faceOrdering, tags::presized(model.uNumFaces));
        deserialize(src, &extra.bspNodes, tags::presized(model.uNumNodes));
        deserialize(src, &extra.faceTextures, tags::presized(model.uNumFaces));
    }

    deserialize(src, &dst->decorations);
    deserialize(src, &dst->decorationNames, tags::presized(dst->decorations.size()));
    deserialize(src, &dst->decorationPidList);
    deserialize(src, &dst->decorationMap);
    deserialize(src, &dst->spawnPoints);
}

void snapshot(const OutdoorLocation &src, OutdoorDelta_MM7 *dst) {
    snapshot(src.ddm, &dst->header.info);
    dst->header.totalFacesCount = 0;
    for (const BSPModel &model : src.pBModels)
        dst->header.totalFacesCount += model.pFaces.size();
    dst->header.bmodelCount = src.pBModels.size();
    dst->header.decorationCount = pLevelDecorations.size();

    snapshot(src.uFullyRevealedCellOnMap, &dst->fullyRevealedCells);
    snapshot(src.uPartiallyRevealedCellOnMap, &dst->partiallyRevealedCells);

    // Symmetric to what's happening in reconstruct - no all attributes need to be saved in a delta.
    dst->faceAttributes.clear();
    for (const BSPModel &model : src.pBModels)
        for (const ODMFace &face : model.pFaces)
            dst->faceAttributes.push_back(std::to_underlying(face.uAttributes & ~FACE_HAS_EVENT));

    dst->decorationFlags.clear();
    for (const LevelDecoration &decoration : pLevelDecorations)
        dst->decorationFlags.push_back(std::to_underlying(decoration.uFlags));

    snapshot(pActors, &dst->actors);
    snapshot(pSpriteObjects, &dst->spriteObjects);
    snapshot(vChests, &dst->chests);
    snapshot(engine->_persistentVariables, &dst->eventVariables);
    snapshot(src.loc_time, &dst->locationTime);
}

void reconstruct(const OutdoorDelta_MM7 &src, OutdoorLocation *dst) {
    reconstruct(src.header.info, &dst->ddm);
    reconstruct(src.fullyRevealedCells, &dst->uFullyRevealedCellOnMap);
    reconstruct(src.partiallyRevealedCells, &dst->uPartiallyRevealedCellOnMap);

    // Not all of the attributes need to be restored.
    size_t attributeIndex = 0;
    for (BSPModel &model : dst->pBModels) {
        for (ODMFace &face : model.pFaces) {
            face.uAttributes &= FACE_HAS_EVENT; // TODO(captainurist): skip FACE_TEXTURE_FRAME here too?
            face.uAttributes |= FaceAttributes(src.faceAttributes[attributeIndex++]) & ~FACE_HAS_EVENT;
        }
    }

    for (size_t i = 0; i < pLevelDecorations.size(); ++i)
        pLevelDecorations[i].uFlags = LevelDecorationFlags(src.decorationFlags[i]);

    reconstruct(src.actors, &pActors);
    for(size_t i = 0; i < pActors.size(); i++)
        pActors[i].id = i;

    reconstruct(src.spriteObjects, &pSpriteObjects);

    vChests.resize(src.chests.size());
    for (size_t i = 0; i < src.chests.size(); ++i)
        reconstruct(src.chests[i], &vChests[i], tags::context<int>(i));

    reconstruct(src.eventVariables, &engine->_persistentVariables);
    reconstruct(src.locationTime, &dst->loc_time);
}

void serialize(const OutdoorDelta_MM7 &src, OutputStream *dst) {
    serialize(src.header, dst);
    serialize(src.fullyRevealedCells, dst);
    serialize(src.partiallyRevealedCells, dst);
    serialize(src.faceAttributes, dst, tags::unsized);
    serialize(src.decorationFlags, dst, tags::unsized);
    serialize(src.actors, dst);
    serialize(src.spriteObjects, dst);
    serialize(src.chests, dst);
    serialize(src.eventVariables, dst);
    serialize(src.locationTime, dst);
}

void deserialize(InputStream &src, OutdoorDelta_MM7 *dst, ContextTag<OutdoorLocation_MM7> ctx) {
    size_t totalFaces = 0;
    for (const BSPModelData_MM7 &model : ctx->models)
        totalFaces += model.uNumFaces;

    deserialize(src, &dst->header);
    deserialize(src, &dst->fullyRevealedCells);
    deserialize(src, &dst->partiallyRevealedCells);
    deserialize(src, &dst->faceAttributes, tags::presized(totalFaces));
    deserialize(src, &dst->decorationFlags, tags::presized(ctx->decorations.size()));
    deserialize(src, &dst->actors);
    deserialize(src, &dst->spriteObjects);
    deserialize(src, &dst->chests);
    deserialize(src, &dst->eventVariables);
    deserialize(src, &dst->locationTime);
}

void snapshot(const SaveGameHeader &src, SaveGame_MM7 *dst) {
    snapshot(src, &dst->header);
    snapshot(*pParty, &dst->party);
    snapshot(*pEventTimer, &dst->eventTimer);
    snapshot(*pActiveOverlayList, &dst->overlays);
    snapshot(pNPCStats->pNPCData, &dst->npcData);
    snapshot(pNPCStats->pGroups, &dst->npcGroups);
}

void reconstruct(const SaveGame_MM7 &src, SaveGameHeader *dst) {
    reconstruct(src.header, dst);
    reconstruct(src.party, pParty);
    reconstruct(src.eventTimer, pEventTimer);
    reconstruct(src.overlays, pActiveOverlayList);
    reconstruct(src.npcData, &pNPCStats->pNPCData);
    reconstruct(src.npcGroups, &pNPCStats->pGroups);
}

void serialize(const SaveGame_MM7 &src, LodWriter *dst) {
    dst->write("header.bin", toBlob(src.header));
    dst->write("party.bin", toBlob(src.party));
    dst->write("clock.bin", toBlob(src.eventTimer));
    dst->write("overlay.bin", toBlob(src.overlays));
    dst->write("npcdata.bin", toBlob(src.npcData));
    dst->write("npcgroup.bin", toBlob(src.npcGroups));
}

void deserialize(const LodReader &src, SaveGame_MM7 *dst) {
    deserialize(src.read("header.bin"), &dst->header);
    deserialize(src.read("party.bin"), &dst->party);
    deserialize(src.read("clock.bin"), &dst->eventTimer);
    deserialize(src.read("overlay.bin"), &dst->overlays);
    deserialize(src.read("npcdata.bin"), &dst->npcData);
    deserialize(src.read("npcgroup.bin"), &dst->npcGroups);
}

void reconstruct(const SpriteFrameTable_MM7 &src, SpriteFrameTable *dst) {
    reconstruct(src.frames, &dst->pSpriteSFrames);
    reconstruct(src.eframes, &dst->pSpriteEFrames);
}

void deserialize(InputStream &src, SpriteFrameTable_MM7 *dst) {
    deserialize(src, &dst->frameCount);
    deserialize(src, &dst->eframeCount);
    deserialize(src, &dst->frames, tags::presized(dst->frameCount));
    deserialize(src, &dst->eframes, tags::presized(dst->eframeCount));
}
