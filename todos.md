# TODOs

Every `TODO` in `src/` and `test/` as of `1d460fb7b41` (Reset particles on level load (#2563)), 697 in total.
Each one was read in context and rated by how much work fixing it actually is.
This file is a worklist - tick entries off as they land.

The rating is a starting point, not a verdict. An entry rated easy can still turn out to hide a design
question once you open the file, and a design one can turn out to have an obvious answer. Line numbers
are as of `1d460fb7b41` and drift as the code changes; `git grep` for the TODO text if one does not match.

## Counts

| tier | count | what it means |
|---|---:|---|
| Trivial | 36 | Mechanical, minutes of work, no behavior risk. |
| Easy | 166 | Localized fix in one file or function, clear what to do, low risk. |
| Stale | 7 | Already done or obsolete - the comment itself is the thing to delete. |
| Medium | 184 | Clear goal, but touches several files or needs care and tests. |
| Hard | 21 | Significant refactor or deep investigation. |
| Design | 282 | Blocked on a decision, missing data, or original-game research. |
| Unclear | 1 | Meaning could not be determined from the code. |
| **total** | **697** | |

By subsystem:

| subsystem | count |
|---|---:|
| `src/Engine` | 500 |
| `src/GUI` | 92 |
| `src/Application` | 29 |
| `src/Library` | 24 |
| `src/Media` | 12 |
| `src/Io` | 11 |
| `src/Utility` | 8 |
| `src/Arcomage` | 7 |
| `src/Scripting` | 7 |
| `test/Bin` | 3 |
| `src/Bin` | 2 |
| `test/Testing` | 2 |

## Trivial (36)

_Mechanical, minutes of work, no behavior risk._

### `src/Arcomage/Arcomage.cpp`

- [ ] **2685** - TODO(pskelton): translate comments to English
  - GameResultsApply is full of Russian comments (e.g. 'проверка построена ли башня'); mechanically translate them in place, no code change.

### `src/Engine/Evt/EvtInstruction.cpp`

- [ ] **311** - TODO
  - In getVariableSetStr, replace the numeric placeholder for VAR_CircusPrises with a named string matching sibling cases, e.g. fmt::format("CircusPrises, {}", value).
- [ ] **319** - TODO
  - For VAR_Unknown1 in getVariableSetStr, emit fmt::format("Unknown1, {}", value) to match sibling style; identifying what the variable actually is would be a separate research task.
- [ ] **634** - TODO
  - In getVariableCompareStr, replace the placeholder for VAR_CircusPrises with e.g. fmt::format("CircusPrises >= {}", value) matching sibling cases.
- [ ] **642** - TODO
  - For VAR_Unknown1 in getVariableCompareStr, emit fmt::format("Unknown1 >= {}", value) to match sibling style; the variable's real meaning is a separate research question.

### `src/Engine/Evt/EvtInterpreter.cpp`

- [ ] **582** - TODO: seems unused
  - EVENT_SpecialJump is already confirmed absent from game data at EvtInstruction.cpp:1185 ("not present in used MM7 data"); replace the tentative comment with that definitive note and keep the assert(false).
- [ ] **586** - TODO: seems unused
  - EVENT_IsTotalBountyHuntingAwardInRange is confirmed unused at EvtInstruction.cpp:1188; reword the comment to the definitive "not present in used MM7 data" note, keeping the assert.
- [ ] **590** - TODO: seems unused
  - EVENT_IsNPCInParty is confirmed unused at EvtInstruction.cpp:1191; same fix - replace "seems unused" with the definitive note matching EvtInstruction.cpp.

### `src/Engine/Graphics/Camera.cpp`

- [ ] **308** - TODO(yoctozepto): just have this as a global constant instead of random vars/4 around
  - Add a constexpr int (e.g. kFrustumPlaneCount = 4) in Camera.h and replace the local NumFrustumPlanes const and the magic 4s at frustum call sites/array sizes.

### `src/Engine/Graphics/Indoor.cpp`

- [ ] **442** - TODO(pskelton): these arent face normals - they are texture shift vectors
  - Pure rename: BLVFace::_get_normals has a single caller (Indoor.cpp:713) and one declaration (Indoor.h:93) — rename to something like getTextureAxes(outU, outV) and fix the comment.

### `src/Engine/Graphics/LightmapBuilder.cpp`

- [ ] **3** - TODO(pskelton): rename - lighting functions
  - Nothing in this file builds lightmaps any more (it holds GetActorTintColor, GetLightLevelAtPoint, the light stacks); git mv LightmapBuilder.cpp/.h to e.g. Lighting.cpp/.h and update the 10 #include sites plus src/Engine/Graphics/CMakeLists.txt.

### `src/Engine/Graphics/LightmapBuilder.h`

- [ ] **3** - TODO(pskelton): rename - lighting functions
  - Header half of the same rename as LightmapBuilder.cpp:3; the header only declares the light stacks and three lighting helpers, so renaming it to Lighting.h costs one CMakeLists edit and 10 include updates.

### `src/Engine/Graphics/LocationTime.h`

- [ ] **8** - TODO(captainurist): rename to smth like MapTime.
  - Target name is already given: rename struct LocationTime to MapTime and the file to MapTime.h, touching ~11 files (Indoor.h/Outdoor.h members, LocationTime_MM7 snapshot pair, SaveLoad.cpp, Evt/Processor.cpp) with no behaviour change.

### `src/Engine/Graphics/Outdoor.cpp`

- [ ] **1862** - TODO(pskelton): drop this
  - sub_47C3D7_get_fog_specular has zero callers - the only two hits in the tree are its definition here and the declaration in Outdoor.h:116; delete both.
- [ ] **1945** - TODO(pskelton): move this - used both indoors and out
  - TeleportToStartingPoint has exactly two callers (Outdoor.cpp:1820 and Indoor.cpp:1063); move the body plus the Outdoor.h:119 declaration into a level-neutral home such as src/Engine/TeleportPoint.cpp/.h and fix the two includes.

### `src/Engine/Objects/Character.cpp`

- [ ] **3615** - TODO(_) change pValue to long long
  - experience is uint64_t and pValue is int, but line 3570 already asserts pValue >= 0 and the value originates from a 32-bit EVT script field (EvtInterpreter.cpp:138/301), so widening buys nothing; either add an explicit cast for clarity or just delete the comment.

### `src/Engine/Objects/Item.cpp`

- [ ] **638** - TODO(captainurist): what is this code about? ^
  - Item::IsRegularEnchanmentForAttribute has zero callers and a fully commented-out body that unconditionally returns false - delete the function (Item.cpp:632-640) together with its declaration at Item.h:42.

### `src/Engine/Objects/ItemEnums.h`

- [ ] **1038** - TODO(captainurist): come up with a better name for this value.
  - ITEM_TYPE_NONE has only 4 references in the tree (its definition, the ITEM_TYPE_LAST alias, ItemTable.cpp:146 fallback, ItemEnumFunctions.h:265); rename to ITEM_TYPE_MISC (ores/quest items are 'miscellaneous', not 'none') with a global find/replace.

### `src/Engine/Objects/Monsters.cpp`

- [ ] **513** - TODO(captainurist): get rid of magic numbers in txt deserialization.
  - 264 is exactly MONSTER_ULTRA_DRAGON_C (the last real monster; rows 265-276 in monsters.txt are the unused cat/chicken/dog/rat); replace the literal with `std::to_underlying(MONSTER_ULTRA_DRAGON_C)` or a named MONSTER_LAST_REAL alias in MonsterEnums.h.

### `src/Engine/Objects/SpriteObject.cpp`

- [ ] **772** - TODO(Nik-RE-dev): unreachable, these cases does not process this sprite type
  - Confirmed dead: the case only covers the nine elemental bolts, and updateSpriteOnImpact() at line 766 has already rewritten object->spriteId to the impact sprite, so the comparison can never be true - delete lines 771-774 (the always-true twin check at line 811 deserves the same treatment).

### `src/Engine/Party.cpp`

- [ ] **838** - drop unsigned
  - Change `GivePartyExp(unsigned int pEXPNum)` to `int` in Party.h:123 and Party.cpp:840 and drop the `static_cast<int>(pEXPNum)` at line 851; the existing `if (pEXPNum > 0)` already guards negatives, and all 5 call sites pass ints.

### `src/Engine/Spells/CastSpellInfo.cpp`

- [ ] **996** - TODO(Nik-RE-dev): condition is always false
  - j,k are in [-512,511] and originHeight-2500 is in [-2500,-1501], so the vector length is always >= 1501 > 1.0f; delete the if-branch and keep only the atan2 else-branch.
- [ ] **1200** - TODO(Nik-RE-dev): condition is always false
  - Identical copy of the meteor-shower code for Starburst; same bounds argument, so delete the dead if-branch and keep the atan2 computation.

### `src/Engine/Tables/NPCTable.cpp`

- [ ] **58** - TODO(Nik-RE-dev): move out of table back to Engine/Objects/NPC.cpp
  - Move setNPCNamesOnLoad() to NPC.cpp as a free function alongside the other pNPCStats-based helpers (getNPCData, PartyHasDragon), qualifying the two member accesses as pNPCStats->pNPCData / pNPCStats->pNPCUnicNames; the only caller is Engine.cpp:560.
- [ ] **78** - TODO(captainurist): just make this 1-based too?
  - pNPCUnicNames has exactly three uses, all in NPCTable.cpp: grow the array in NPCTable.h:128 from 500 to 501 and drop the `- 1` at lines 61, 77 and 78.

### `src/Engine/Time/Timer.h`

- [ ] **51** - TODO(captainurist): pAnimTimer?
  - Pure rename: pMiscTimer has 59 references and is only used for UI/portrait/icon animation (UIGame.cpp, UIPartyCreation.cpp, its own tick() at Game.cpp:1575 is commented "used for animations"), so sed it to pAnimTimer.
- [ ] **54** - TODO(captainurist): pGameTimer?
  - Pure rename of the global driving game logic/physics dt: 119 references, mechanical sed of pEventTimer -> pGameTimer with no semantic change.

### `src/Engine/mm7_data.cpp`

- [ ] **8** - TODO(pskelton): move me - to spellfxrendere.h?
  - Move the sphereVertPos and sphereVertInd definitions out of mm7_data.cpp into SpellFxRenderer.cpp — they are already declared in SpellFxRenderer.h:19-20 and used only by SpellFxRenderer.cpp:119-128.

### `src/GUI/GUIButton.h`

- [ ] **35** - TODO(Nik-RE-dev): rename properly. In most cases it is a hover hint for status bar.
  - Rename the field to `label` (it is not only a hint — UIDialogue.cpp:292-313 draws it as the visible dialogue option text) across its 14 references in GUIWindow.cpp, UIGame.cpp, UIDialogue.cpp and UIHouses.cpp.

### `src/GUI/UI/NPCTopics.cpp`

- [ ] **285** - TODO(pskelton) :: extract this common teleport to func
  - The identical 5-line block (pos = Vec3f(3849, 5770, 1); velocity = {}; uFallStartZ = 1; _viewYaw = 512; _viewPitch = 0) appears at lines 255-259 and 285-289 in the same file; hoist it into a file-local `static void teleportPartyToArena()` and call it from both.

### `src/GUI/UI/UIBooks.cpp`

- [ ] **100** - TODO(pskelton): make a constant for this magic number
  - The 10 is a frame countdown for the pressed-button overlay; add e.g. `static constexpr int BOOK_BUTTON_CLICK_FRAMES = 10;` next to `_bookButtonClicked` in UIBooks.h and use it here and at the four `_bookButtonClicked == 10` comparisons in Books/AutonotesBook.cpp:205, JournalBook.cpp:85/89, MapBook.cpp:111, QuestBook.cpp:72/79.

### `src/GUI/UI/UIDialogue.cpp`

- [ ] **189** - TODO(pskelton): nothing done with fame here?
  - Party::getPartyFame() (Party.cpp:369-377) only sums character experience and returns a value — no side effects — so the bare `pParty->getPartyFame();` statement is dead; delete the line (it is a leftover of the `#if 0`-ed fame gate above).

### `src/GUI/UI/UIGame.cpp`

- [ ] **1716** - TODO(pskelton): this used to check if character had the spell activated - no longer required here ??
  - The question is answered by GUIWindow_Spellbook::openSpellbook (src/GUI/UI/UISpellbook.cpp:111-122): the button that sends UIMSG_Spellbook_ShowHightlightedSpellInfo is only created for spells with player.bHaveSpell (or debug.AllMagic), so the check is redundant - delete the comment.

### `src/GUI/UI/UIHouses.cpp`

- [ ] **671** - TODO(Nik-RE-dev): looks like this function is not needed anymore
  - BackToHouseMenu()'s whole body is an unused `pMouse` local plus an `#if 0` block, so it is a no-op: delete the function, its declaration in UIHouses.h:24, and the 9 call sites (Game.cpp:302, Arcomage.cpp:965, UIHouses.cpp:477/486/502/509/514/571/597).
- [ ] **674** - TODO(Nik-RE-dev): Looks like it's artifact of MM6
  - This annotates the `#if 0` block inside BackToHouseMenu that hardcodes houseId() == 165 and HOUSE_BODY_GUILD_MASTER_ERATHIA; it is unreachable dead code and goes away with the same deletion as the previous item.
- [ ] **693** - TODO(captainurist): encapsulate
  - Wrap the magic formula SoundId(type + 100 * (roomSoundId + 300)) in a named helper (e.g. `SoundId houseSoundId(HouseId, HouseSoundType)` or a method on the room descriptor) and reuse it for the duplicated `pAnimatedRooms[houseTable[h].uAnimationID].uRoomSoundId` guards in UITransition.cpp:164/177.

## Easy (166)

_Localized fix in one file or function, clear what to do, low risk._

### `src/Application/Game.cpp`

- [ ] **171** - TODO(Nik-RE-dev): should not be an assert but an exception or error message.
  - Replace the assert with `throw Exception("Invalid starting map '{}'", ...)` — the MAP_INVALID case only happens when gameplay.StartingMap config is bad, and Exception is already used for fatal startup errors (see GameStarter.cpp:172), so a localized one-line change.
- [ ] **1685** - TODO(Nik-RE-dev): should not be an assert but an exception or error message.
  - Same as line 171: replace the assert on the party-wipe respawn path with `throw Exception(...)` naming the invalid gameplay.StartingMap config value — one-line localized change.

### `src/Application/GameConfig.h`

- [ ] **99** - TODO(captainurist): Move to [audio]?
  - Move the NoSound entry from the Debug section into the Audio section class and update the `config->debug.NoSound` references; it is a debug flag so silently losing its saved ini value on the section change is acceptable — only needs the maintainer's nod on the question mark.

### `src/Application/GameTraceHandler.cpp`

- [ ] **29** - TODO(captainurist): do this properly, trace00001.json, etc.
  - Localized: probe ufs for the first free traceNNNNN.json/traceNNNNN.mm7 pair (formatted with fmt) and write the recording there instead of the fixed trace.json/trace.mm7 names; all in this one lambda.
- [ ] **56** - TODO(captainurist) : make configurable
  - Add a KeyConfigEntry (e.g. debug.TraceTriggerKey) to GameConfig and read it here instead of the hardcoded KEY_R; small plumbing to give GameTraceHandler config access (constructor arg or engine->config), and note the broader hotkey-system TODO in the header would subsume this.
- [ ] **61** - TODO(captainurist) : make configurable
  - Same config entry as line 56 covers this: store the trigger key plus its Ctrl+Shift modifiers in one configurable binding and consult it in both isTriggerKey and isTriggerKeySequence.

### `src/Application/GameWindowHandler.cpp`

- [ ] **588** - TODO: deadzone should be configurable and default should be lowered once we implement proper axis event processing.
  - The actionable half is small: add a Float config entry (e.g. gamepad deadzone, default 0.5) in GameConfig and compare `value` against it here; lowering the default stays gated on the axis-handling rework from line 573.

### `src/Application/GameWindowHandler.h`

- [ ] **24** - TODO(captainurist): this probably doesn't even belong here. Find a place to move to.
  - UpdateWindowFromConfig/UpdateConfigFromWindow and the two position helpers are window<->config bridging with few callers (GameStarter and this class); extract them into a small helper file under Application (e.g. WindowConfig.h/cpp) as free functions and update the two call sites.

### `src/Arcomage/Arcomage.cpp`

- [ ] **408** - TODO(pskelton): Hardcoded limit checks need changing
  - Replace the magic numbers in DrawSparks (10 effect slots, 150 sparks, 639/479 bounds) with named constants tied to the am_effects_array/effect_sparks array sizes and the 640x480 logical viewport (or render dimensions if resolution independence is wanted) — mechanical, contained to this file.
- [ ] **1191** - TODO(pskelton): was 1 - what was this meant to do? Check for dual key press??
  - The whole branch is dead: ARCO_MSG_FORCEQUIT is never produced anywhere in src, and ArcomageGame_InputMSG::field_4 is never assigned (declared 'unsused' in Arcomage.h:159), so field_4 == 129 can never hold. Delete the case (together with the Arcomage.h:136 unused-enum cleanup) instead of researching the original intent.
- [ ] **2864** - TODO(pskelton): stop audio comment?
  - The '// stop all audio' comment in PrepareArcomage has no code behind it. Check whether game music/sounds keep playing when Arcomage starts; if so add a pAudioPlayer stop/pause call here, otherwise delete the orphan comment and the TODO.

### `src/Arcomage/Arcomage.h`

- [ ] **136** - TODO(pskelton): cleanup unused
  - ARCO_MSG_LM_UP/RM_UP/LM_DOWN/RM_DOWN/SWITCH_FULLSCREEN are referenced nowhere, and ARCO_MSG_KEYDOWN/FORCEQUIT are only consumed but never produced; delete those enumerators, their dead consumers (Arcomage.cpp:883-886 and the FORCEQUIT case at 1190), and the unused field_4.

### `src/Engine/Data/HouseData.h`

- [ ] **10** - TODO(captainurist): drop everything that's not used here.
  - Grep each suspicious field (field_14, field_1E, field_32, _state, _rep, _per, uExitPicID, uAnimationID...), delete the unused ones together with their parse assignments in src/Engine/Tables/HouseTable.cpp; struct is only consumed via houseTable and PriceCalculator.

### `src/Engine/Engine.cpp`

- [ ] **301** - TODO(pskelton): set this on level load
  - The outdoor TorchLightDistance=1024 override is recomputed every frame in StackPartyTorchLight; hoist it into level-load code (or cache an effective-distance member updated on load/config change) and verify traces still pass.
- [ ] **328** - TODO(pskelton): move this
  - Hoist the outdoor-daytime check to the top of StackPartyTorchLight as an early-out; only care point is that the current code still stores TorchLightLastIntensity=0 and pushes a zero-distance light, so keep the lastIntensity reset.
- [ ] **446** - TODO(captainurist): Right now we can have popups for monsters that are not reachable with a bow, and this is OK. However, such monsters also don't get a hint displayed on mouseover. Probably should fix this?
  - Make the mouseover-hint code path pick with pCamera3D->GetMouseInfoDepth() (as PickMouseInfoPopup does at Engine.cpp:448) instead of the RangedAttackDepth used by PickMouseNormal/PickMouseTarget; one call-site depth change, verifiable in game.
- [ ] **551** - TODO(captainurist): drop?
  - floor_face_id feeds pressure-plate edge detection (Indoor.cpp:1539, Collisions.cpp:1008, Outdoor.cpp:987), so resetting it on map load prevents a stale id from the previous map suppressing a plate trigger; conclusion is likely 'keep the line, delete the TODO', confirmable by reading those three users and running game tests.
- [ ] **566** - TODO(captainurist): shouldn't we also set uTreasureLevel = ITEM_TREASURE_LEVEL_INVALID?
  - Yes - loot generation gates on treasureLevel != ITEM_TREASURE_LEVEL_INVALID (Actor.cpp:207, 3561), so without it monsters on these no-loot maps can still drop items; add the line mirroring Actor.cpp:1761 and retrace any traces covering Breeding Zone / Walls of Mist.
- [ ] **937** - TODO(pskelton): starts at 1?
  - Faces are matched by cogNumber and sCogNumber==0 is already excluded by the guard, so starting the loop at 0 is safe and matches the outdoor branch; change it here and in the identical sub_44861E_set_texture_indoor loop (Engine.cpp:900), then run game tests.

### `src/Engine/Evt/EvtInstruction.cpp`

- [ ] **874** - TODO(yoctozepto): this downcasts a DWORD to WORD
  - SoundId's underlying type is int16_t, so the static_cast from the uint32_t read silently truncates; add a range check that throws an Exception (mirroring the requireSize style) before casting, or a small checked-narrowing helper shared with the two sibling downcast sites in this function.
- [ ] **1042** - TODO(yoctozepto): this downcasts a DWORD to WORD
  - SpriteId's underlying type is uint16_t, so the cast from the uint32_t read truncates; add the same throw-on-out-of-range guard as for the SoundId site (a shared checked-narrowing helper covers all three downcast TODOs in this function).
- [ ] **1162** - TODO(yoctozepto): this downcasts a DWORD to WORD
  - ChestFlag's underlying type is uint16_t, so the C-style cast from the uint32_t read truncates; add the same throw-on-out-of-range guard as the SoundId/SpriteId sites (one shared checked-narrowing helper resolves all three).

### `src/Engine/Evt/EvtInstruction.h`

- [ ] **40** - TODO(yoctozepto): only a byte is read in
  - The parser (EvtInstruction.cpp:998) reads fromStream<uint8_t> into each element, so change std::array<int, 6> to std::array<uint8_t, 6>; the only consumers are fmt::join in toString (still prints as integers) and an int-returning jump-target lookup in EvtInterpreter.cpp:364, both fine with uint8_t.

### `src/Engine/Evt/EvtInterpreter.cpp`

- [ ] **171** - TODO: magic number
  - 167 is the jail house animation ID: enterHouse (UIHouses.cpp:347-350) redirects to HOUSE_JAIL and sets uCurrentHouse_Animation from houseTable[HOUSE_JAIL].uAnimationID when the party has a fine; replace the literal with houseTable[HOUSE_JAIL].uAnimationID (or have enterHouse report the actual house entered).

### `src/Engine/Evt/Processor.cpp`

- [ ] **154** - TODO: rename and contain in this module or better remove it altogether
  - dword_5B65C4_cancelEventProcessing has only 4 usage sites (EvtInterpreter.cpp:611, Character.cpp:5214/5648, plus this reset); rename it, move the definition from mm7_data.cpp/h into the Evt module with a declaration in Processor.h, and update the sites. Full removal would need Character::SubtractVariable to signal cancellation another way, but rename+contain alone satisfies the TODO.

### `src/Engine/Graphics/BspRenderer.cpp`

- [ ] **116** - TODO(yoctozepto): remove it from here
  - pPortalBounding is pure scratch for CalcPortalShapePoly (single caller, output never read here); make the 4-element bounding buffer a local inside CalcPortalShapePoly, drop the pOutBounding parameter, and delete the static array.

### `src/Engine/Graphics/Camera.cpp`

- [ ] **61** - TODO(captainurist): function belongs to stru314
  - GetFacetOrientation is static with one external caller (DecalBuilder.cpp:71) that immediately stores results into stru314 fields; move it into stru314 (e.g. a method filling field_10/field_1C from Normal in stru314.h) and drop it from Camera3D.
- [ ] **166** - TODO(pskelton): fov calcs only need recalculating on level change or if we add config option
  - Hoist the fov/projection block (halfFovTan, ViewPlaneDistPixels, fov_y_deg, screen center, aspect) out of the per-frame CreateViewMatrixAndProjectionScale into a recalc method invoked on level load/viewport change, or cache it keyed on level type; localized to Camera3D.
- [ ] **217** - TODO(pskelton): does this func need to copy verts or could it be eliminated
  - CullFaceToCameraFrustum only culls then memcpy's, and its single caller is the debug-only do_draw_debug_line_sw (Camera.cpp:127); drop the copy output parameter and use the input buffer at the call site (which also fixes the caller's use of a possibly-unfilled pVertices when the cull fails).

### `src/Engine/Graphics/Collisions.cpp`

- [ ] **465** - TODO: this doesn't respect ignore_ethereal parameter
  - Change the condition to skip ethereal faces only when ignore_ethereal is set (CollideBodyWithFace at line 101 already re-checks it), i.e. drop the unconditional mface.Ethereal() skip; the only caller passing false is SpriteObject.cpp:228, so verify projectile behavior with game tests.
- [ ] **916** - TODO: whaaa?
  - The `return` is the last statement path of the 5-attempt loop (loop ends at line 1025, function at 1026), so it is behaviourally identical to `break`; fix by replacing it with `break` plus the explanation already used for the identical actor-side check in ProcessActorCollisionsBLV (Collisions.cpp:631-632, "New pos is out of bounds, running more iterations won't help").
- [ ] **932** - TODO(pskelton): common to odm/blv so extract
  - Lines 933-949 are character-for-character identical to lines 1124-1140 in ProcessPartyCollisionsODM; extract a file-local `static Vec3f decorationSlideDirection()` returning newDirection and leave only `speed = dir * dot(dir, speed)` at each site.
- [ ] **1123** - TODO(pskelton): common to odm/blv so extract
  - Same duplicated decoration-slide block as Collisions.cpp:932 (lines 1124-1140 vs 933-949); one shared helper removes both copies, with only the final assignment target (pParty->velocity vs *partyInputSpeed) differing.

### `src/Engine/Graphics/DecalBuilder.h`

- [ ] **38** - TODO(captainurist): doesn't belong to this struct, should be moved out.
  - faceDist is a per-face scratch value written in ApplyBloodsplatDecalsToFace (DecalBuilder.cpp:198) and ApplyBloodSplatToTerrain (:235) and read once in BuildAndApplyDecals (:86); add a `std::array<float, 1024> splatDistsThisFace` parallel to DecalBuilder::WhichSplatsOnThisFace, write/read it there and delete the field plus its zeroing at :37.

### `src/Engine/Graphics/FaceEnums.h`

- [ ] **60** - TODO(captainurist): most closed doors are in DOOR_OPEN, and most open doors are in DOOR_CLOSED. Rename states?
  - Only ~29 usages and the names are never serialized (EntitySnapshots.cpp:1539 casts the raw int), so it is a pure symbol rename to neutral names like DOOR_AT_INITIAL/DOOR_AT_ALTERNATIVE matching the existing per-enumerator comments.

### `src/Engine/Graphics/ImageLoader.h`

- [ ] **111** - TODO(captainurist): this is the next level of ugly, redo.
  - The templated ctor + `std::function<Blob()>` has exactly one instantiation, AssetsManager.cpp:144 with `pIcons_LOD` (a LodTextureCache*), and LoadCompressedTexture only exists on LodTextureCache — replace with a plain `LodTextureCache *lod` member like the sibling loaders.

### `src/Engine/Graphics/Indoor.cpp`

- [ ] **205** - TODO(captainurist): using pEventTimer here is weird. This means that e.g. cleric in the haunted mansion is not animated in turn-based mode. Use misc timer?
  - One-line change to `pMiscTimer->time()`, which is already the timer used for decoration sprite animation in BaseRenderer::PrepareDecorationsRenderList_ODM (BaseRenderer.cpp:169); GetTexture() is only consumed by renderers plus the door texture-shift code at Indoor.cpp:736/742, so it is purely visual.
- [ ] **379** - TODO(captainurist): we do get here sometimes (e.g. in dragon cave), increase limit?
  - Bump `int FoundFaceStore[5]` and the two `>= 5` guards (or switch to a small vector) in IndoorLocation::GetSector; low risk but it can change which sector is picked in the overflow case, so recorded game-test traces should be re-checked.
- [ ] **942** - TODO(Nik-RE-dev): why there's logic for loading maps that are not listed in info?
  - The branch is dead: MAP_INVALID == 0 and isMapIndoor(MAP_INVALID) is false (MapEnumFunctions.h:15), so Engine.cpp:555 always routes MAP_INVALID to loadAndPrepareODM — replace this else-branch with an assert and drop the other `mapid == MAP_INVALID` checks at lines 954 and 1025.
- [ ] **1015** - TODO(captainurist): merge with ArrangeSpriteObjects?
  - OutdoorLocation::ArrangeSpriteObjects (Outdoor.cpp:610-622) is this exact loop plus terrain-height snapping and a progress-bar tick; hoist the shared `postGenerate(ITEM_SOURCE_MAP)` loop into one free function taking a flag (or let the ODM version call it after snapping).
- [ ] **1054** - TODO(captainurist): can drop this? Was loaded because light elementals can be summoned.
  - Delete the three lines: the ODM counterpart is already commented out as "no use for this" (Outdoor.cpp:681-684), and every summon path calls actor->PrepareSprites(0) itself (Actor.cpp:3245, 4183, 4332), which does the same pSpriteFrameTable->InitializeSprite work lazily.
- [ ] **1185** - TODO(pskelton): common emit fire code
  - The same particle setup is duplicated verbatim in BaseRenderer::PrepareDecorationsRenderList_ODM (BaseRenderer.cpp:243-255); extract e.g. `void emitDecorationFireParticle(const Vec3f &pos)` and call it from both.
- [ ] **1314** - TODO(captainurist): what's going on in this check?
  - It is algebraically `|NegFacePlaceDist / dirDotNormal| <= 16384`, i.e. an original fixed-point overflow guard capping the intersection distance at 2^14; replace the TODO with that explanation (same check duplicated at Indoor.cpp:1366) and optionally drop it as subsumed by the `IntersectionDist <= dist` test two lines below.
- [ ] **1386** - TODO(Nik-RE-dev): does not belong here, it's common function for interaction for both indoor/outdoor
  - Move DoInteractionWithTopmostZObject out of Indoor.cpp/Indoor.h:273 into Viewport.cpp/Viewport.h, where the ItemInteraction/InteractWithActor/DecorationInteraction helpers it dispatches to already live (Viewport.cpp:158-202); the only external caller is Game.cpp:1519.
- [ ] **1387** - TODO(Nik-RE-dev): get rid of external function declaration inside
  - ItemInteraction, CanInteractWithActor, InteractWithActor and DecorationInteraction are defined in Viewport.cpp:158-202 but declared nowhere; add their prototypes to Viewport.h and include it here, deleting the four inline `extern` declarations at lines 1403, 1414, 1415, 1427.
- [ ] **1895** - TODO(pskelton): configurable thresholds for treasure
  - Replace the literal 20 and 60 in SpawnRandomTreasure with two new Int entries in GameConfig::Gameplay (same pattern as ChestTryPlaceItems / ArtifactLimit, with a 0..100 validator), keeping current values as defaults so RNG-sensitive game tests are unaffected.

### `src/Engine/Graphics/Indoor.h`

- [ ] **188** - TODO(captainurist): #enum
  - Only two bits are ever read (`flags & 8` in Indoor.cpp:1120 and `flags & 0x10` in BspRenderer.cpp:235), so declare an enum class SectorAttribute with MM_DECLARE_FLAGS in FaceEnums.h style, change the field type, and cast in reconstruct(BLVSector_MM7) where flags comes in as int32_t.
- [ ] **262** - TODO(pskelton): move to party?
  - uPartySectorID/uPartyEyeSectorID are pure party state with ~13 direct accesses across 8 files; move them onto Party, move the two assignments in BLVRenderParams::Reset() to where the party position updates, and update Collisions.cpp:921 — the only judgement call is that they stay non-serialized runtime state.

### `src/Engine/Graphics/LightmapBuilder.cpp`

- [ ] **21** - TODO(pskelton): this needs reworking if we want lights to be outlined
  - The body is entirely commented out and references StationaryLight::pVertices/NumVertices, which no longer exist (lights are now pos + radius); re-implement by iterating pStationaryLightsStack/pMobileLightsStack and drawing a sphere/cylinder via pCamera3D->do_draw_debug_line_sw, like drawDebugCylinder in Engine.cpp — debug-only path behind config->debug.LightmapDecals, so no gameplay risk.

### `src/Engine/Graphics/LocationFunctions.h`

- [ ] **8** - TODO(captainurist): move to Engine/ and drop the Location- prefix, should be MapSmth.
  - Move the file out of Engine/Graphics into Engine/ under a new name (e.g. MapFunctions.h/.cpp), update the 8 includes and CMakeLists; only real work is picking a name that does not collide with the existing Engine/MapInfo.h and MapEnumFunctions.h.

### `src/Engine/Graphics/LocationInfo.h`

- [ ] **3** - TODO(captainurist): we also have MapInfo, rename this into something more sane with Map- prefix.
  - Pure rename of a 4-field POD (e.g. to MapState/MapRespawnState) across ~15 files including the LocationInfo_MM7 snapshot type and its snapshot/reconstruct declarations in EntitySnapshots.h; mechanical once the name is chosen so it does not clash with Engine/MapInfo.h.

### `src/Engine/Graphics/Outdoor.cpp`

- [ ] **688** - TODO: move to actors?
  - OutdoorLocation::PrepareActorsDrawList() handles both level types and is even called as pOutdoor->PrepareActorsDrawList() from PrepareDrawLists_BLV (Indoor.cpp:153); make it a free function in Actor.cpp/a render file, replacing the two spell_fx_renderer member uses with EngineIocContainer::ResolveSpellFxRenderer(), then update the two call sites and Outdoor.h.
- [ ] **791** - TODO(pskelton): what is this for?
  - BILLBOARD_0X200 is write-only dead: it is set here and at BaseRenderer.cpp:238 and never tested anywhere (grep for the flag and for 0x200 confirms no reads), so delete both assignments and the enumerator in SpriteEnums.h:12.
- [ ] **931** - TODO(pskelton): Pass party as param
  - Mechanical: change the signature to ODM_ProcessPartyActions(Party *party) (declared in Outdoor.h), replace the pParty uses inside the function body, and update the single caller; no behaviour change, though it is a large diff and is naturally done together with the split TODO above.
- [ ] **1956** - TODO: (Chaosit) dummy variables created for the sake of passing pointers
  - Give ODM_GetFloorLevel's out-params defaults (bool *pIsOnWater = nullptr, int *faceId = nullptr) and null-guard the three writes at Outdoor.cpp:804/849/(final store); only Outdoor.cpp+Outdoor.h change and the dummy locals here disappear.

### `src/Engine/Graphics/ParticleEngine.cpp`

- [ ] **183** - TODO(Nik-RE-dev): check colour format use in particles
  - Effectively already answered: Color is now a channel-named struct (r/g/b/a) and the value flows straight to vertex diffuse via toColorf(), so there is no bgr/rgb swap left - drop the TODO and rename the misleading Particle::uLightColor_bgr field (ParticleEngine.h:72, 3 use sites).
- [ ] **226** - TODO(pskelton): reinstate this guard check
  - The commented-out screen-space cull used pBLVRenderParams, which is invalid outdoors; replace `if (true)` with a check of the projected p->uScreenSpaceX/Y against the global `pViewport` rect (pBLVRenderParams->viewportRect is just a copy of it, Indoor.cpp:177).

### `src/Engine/Graphics/Renderer/BaseRenderer.cpp`

- [ ] **62** - TODO: Move this to sprites ?
  - DrawSpriteObjects only uses free functions/globals (IsCylinderInFrustum, pBspRenderer, render->AddBillboardIfVisible), so it can become a free function in Sprites.cpp/SpriteObject.cpp; delete the pure-virtual at Renderer.h:122 plus the BaseRenderer.h:25 override and fix 2 call sites (Indoor.cpp:152, Outdoor.cpp:207).
- [ ] **143** - TODO(pskelton): Move to outdoors - clean up
  - Same shape as DrawSpriteObjects: move the body to Outdoor.cpp as a free function, drop Renderer.h:121 and BaseRenderer.h:26, update the single caller Outdoor.cpp:205; the 'clean up' part is local (the leftover v6/v7/v13/v37 decompiler names and the uninitialised v38 BillboardFlags).
- [ ] **237** - TODO(pskelton): what is this for?
  - Answer: nothing - BILLBOARD_0X200 is only ever written (here and Outdoor.cpp:792) and never tested anywhere; delete both writes and the enum member in SpriteEnums.h:12.
- [ ] **307** - TODO(captainurist): what's going on here?
  - Color::c32 is a memcpy of {r,g,b,a} so `0x007F7F7F & (c32 >> 1)` is just per-channel halving with the inter-channel bleed masked off and alpha forced to 0; replace with the explicit `Color(diffuse.r / 2, diffuse.g / 2, diffuse.b / 2, 0)`.
- [ ] **365** - TODO(pskelton): replace asserts with warning and return false
  - Turn the three asserts in AddBillboardIfVisible into `logger->warning(...); return false;` - the function already returns bool and every caller checks it (Outdoor.cpp:786, BaseRenderer.cpp:135/234).
- [ ] **411** - TODO(captainurist): taking cos of an INT angle? WTF?
  - Particle::angle is in TrigLUT units (seeded with vrng->random(TrigLUT.uIntegerDoublePi) and advanced by rotation_speed), so std::cos/std::sin are wrong; swap to TrigLUT.cos(p.angle)/TrigLUT.sin(p.angle) - it changes rotating-particle visuals, so verify with a screenshot diff.
- [ ] **527** - TODO(captainurist): Old code used BR - TL (one less than actual width/height), preserving that behavior for now.
  - Drop the `- 1`s so the quad is Recti(pViewport.x, pViewport.y, pViewport.w, pViewport.h); one line, but it shifts the special-effect overlay by a pixel, so confirm with a screenshot diff on a prismatic-light/effect frame.
- [ ] **538** - TODO: should this be combined / moved out of render
  - getActorsInViewport is pure hit-testing over the billboard list plus vis->DoesRayIntersectBillboard; move it to Vis (or a picking helper), remove Renderer.h:137 / BaseRenderer.h:35 and update the 6 `render->getActorsInViewport` call sites in CastSpellInfo.cpp.

### `src/Engine/Graphics/Renderer/OpenGLRenderer.cpp`

- [ ] **128** - TODO(pskelton): clean up and move out of here
  - SkyBillboardStruct is declared in Renderer.h and only its method body lives in the GL backend; move CalcSkyFrustumVec (and the `SkyBillboard` global, currently in Outdoor.cpp:67) into a dedicated Sky/SkyBillboard translation unit - pure code motion, no GL calls involved.
- [ ] **783** - TODO(captainurist): subImage().scale()
  - Neither subImage nor scale exists in Library/Image (only makeRgbaImage/flipVertically in ImageFunctions.h); add a subImage(RgbaImageView, Recti) and a nearest-neighbour scale(RgbaImageView, Sizei) there and replace the manual double loop, keeping the built-in vertical flip (index outputRender.h - (y+1)*interval_y).
- [ ] **1145** - TODO(captainurist): terrible, terrible hack, redo this.
  - terraintexmap is map<name, unit|layer> so the generated-tile flag is lost and re-guessed from the 'generated/tiles/...' name prefix (TileGenerator.cpp:39); change the map's value to carry TILE_GENERATED_TRANSITION (or store the GraphicsImage* fetched at line 1035) and pass that to getBitmap instead of starts_with.
- [ ] **1227** - TODO(pskelton): this should be a separate function
  - The ~50-line torch/stationary/mobile light-stacking block is byte-for-byte duplicated at line 2603 (TerrainUniforms vs OutBuildUniforms, both std::array<PointLightUniform,20>); extract a helper taking std::span<PointLightUniform> and call it from both (the indoor copy at 3117 has extra sector/frustum culling and stays separate).
- [ ] **1413** - TODO(pskelton): do we need this?
  - No: sky_texture is always assigned during outdoor load (OutdoorLocation::Load, Outdoor.cpp:538) and by CreateDebugLocation (Outdoor.cpp:413), assets->getBitmap never returns nullptr, and DrawOutdoorSky is only reachable from OutdoorLocation::Draw - drop the plansky3 fallback and turn the following null check into an assert.
- [ ] **2603** - TODO(pskelton): this should be a separate function
  - Duplicate of line 1227 - the same light-stacking block; both call sites collapse into one helper filling std::span<PointLightUniform> from pMobileLightsStack/pStationaryLightsStack.
- [ ] **2832** - TODO(pskelton): make this a pair/struct rather than encoding
  - Change `std::map<std::string, int> bsptexmap`/`outbuildtexmap` in OpenGLRenderer.h to map to a small `struct {int unit; int layer;}` and drop the `(unit << 8) | layer` pack/unpack at the ~6 sites in this file (2796, 2832, 2864-2868, 2915-2930 and the outbuild twins at 2273, 2336, 2367, 2427).
- [ ] **2966** - TODO(pskelton): check tickcount usage here
  - This is the only place in the renderer using wall-clock `platform->tickCount()` for animation; every other animated path (lines 2584, 3099, 705, 1469) uses `pMiscTimer->time().realtimeMilliseconds()`, so swap the two skymodtime lines to pMiscTimer so indoor sky scroll pauses with the game and stays deterministic.
- [ ] **3362** - TODO(captainurist): this is probably the place to check OpenGL version & exit. openenroth requires opengl core 4.1 or opengles 3.2 capable gpu to run.
  - Right after the successful gladLoad, compare GLAD_VERSION_MAJOR/MINOR(version) against 4.1 (or 3.2 when `OpenGLES`), log an error and `return false` from OpenGLRenderer::Initialize, which already propagates a startup failure.

### `src/Engine/Graphics/TurnBasedOverlay.cpp`

- [ ] **89** - TODO(captainurist): get rid of this dependency.
  - The only TurnEngine reference in the overlay; pass the value in from the single caller (Game.cpp:1604 already passes `pTurnEngine->turn_stage`) as e.g. `update(dt, newStep, pTurnEngine->uActionPointsLeft)`, store the computed movement icon index in a member, and drop the TurnEngine.h include.

### `src/Engine/Localization.h`

- [ ] **28** - TODO(captainurist): what if fmt throws?
  - Format strings come from game data (mm7text.txt), so a malformed/edited localization makes fmt::sprintf throw fmt::format_error straight into ~54 UI call sites; wrap the call in try/catch, log the LstrId plus e.what() via logger->error (same pattern as SaveLoad.cpp:218) and return str(index) unformatted.

### `src/Engine/Objects/Actor.cpp`

- [ ] **1553** - TODO(captainurist): yaw/pitch angles are actually initialized to 0 despite the name!
  - AIDirection (Actor.h:34) has default member initializers, so doNotInitializeBecauseShouldBeRandom is always {0,0} and AI_Stand copies that into yawAngle/pitchAngle; either rename the variable to zeroDirection (4 mechanical uses, no behavior change) or, to honour the name, seed it with grng->random(TrigLUT.uIntegerDoublePi) - the rename is the safe resolution.
- [ ] **2414** - TODO(captainurist): encapsulate monster level arithmetic properly.
  - Add a helper next to monsterTierForMonsterId in MonsterEnumFunctions.h, e.g. monsterIdForTypeAndTier(MonsterType, MonsterTier), and replace static_cast<MonsterId>(to_underlying(base) + extraSummonLevel - 1) with it.
- [ ] **2419** - TODO(captainurist): encapsulate monster level arithmetic properly.
  - Same fix as the previous one: the +1/+2 tier bumps become monsterIdForTypeAndTier(monsterTypeForMonsterId(base), MONSTER_TIER_B/C) using the new helper in MonsterEnumFunctions.h.
- [ ] **4316** - TODO(captainurist): MONSTER_ANGEL_A is monster #1, why do we even need this check?
  - The check exists because FindMonsterByInternalName returns MONSTER_INVALID (=0) for unknown names while pMonsterStats->infos is an IndexedArray starting at MONSTER_FIRST=1, so indexing would assert; replace the silent Angel substitution with a logger->warning plus `continue` to skip the bad spawn (the loop already uses continue at 4347), or at minimum document the reason.

### `src/Engine/Objects/ActorEnums.h`

- [ ] **74** - TODO(captainurist): rename to Summoning? This is a transient state, takes 2s.
  - Pure rename of AIState::Summoned to Summoning; ~18 usages (Actor.cpp:174/181/188/2229/2454/2549/2583/3375, Outdoor.cpp, Indoor.cpp, Collisions.cpp, Engine.cpp, BaseRenderer.cpp). Careful not to touch the unrelated LSTR_SUMMONED string constant.

### `src/Engine/Objects/Character.cpp`

- [ ] **132** - TODO(captainurist): #enum
  - pBaseHealthByClass/pBaseManaByClass are raw int[12] indexed by std::to_underlying(classType)/4 at lines 1780 and 1850 (with 3 trailing unused zeros). Replace with IndexedArray<int, CLASS_FIRST, CLASS_LAST> spelling out all 36 classes (mirroring pBaseHealthPerLevelByClass right below) and index by classType directly, dropping the /4.
- [ ] **540** - TODO(captainurist): this doesn't belong to a getter!!!
  - Character::HasSkill fires engine->_statusBar->setEvent(LSTR_S_DOES_NOT_HAVE_THE_SKILL) as a side effect, which is actively wrong in GameBindings.cpp:395 where it is called in a loop over every skill. Make HasSkill a pure const getter and move the setEvent call to the four equip call sites in UICharacter.cpp (1778, 1881, 1914, 1965).
- [ ] **3041** - TODO(Nik-RE-dev): this looks like some artifact from MM6 (where you can eat reagents) In MM7 these item IDs are invalid (plus index 161 used twice which is wrong)
  - The branch is entered only when isReagent() (ItemId 200-219) yet compares against ITEM_161/ITEM_162, so all three arms are unreachable and every reagent already falls through to the 'can not be used that way' else. Collapse the whole isReagent() block to that error message + sound + return, deleting the dead eat/recovery code below it (note the MM6 support goal if the engine ever loads MM6 data).
- [ ] **3210** - TODO(Nik-RE-dev): why not playerAffected?
  - ITEM_POTION_DIVINE_RESTORATION saves playerAffected's DEAD/PETRIFIED/ERADICATED times, then calls this->conditions.resetAll() and restores them onto playerAffected - so via the portrait path (UIPopup.cpp:99, where this != playerAffected) it wipes the drinker's conditions and does nothing for the target. Fix is `playerAffected->conditions.resetAll();`.
- [ ] **3617** - TODO(captainurist): values coming from scripts should be bound-checked.
  - pValue comes straight from EVT data and is cast to QuestBit indexing IndexedBitset<QBIT_FIRST=1, QBIT_LAST=512>, whose operator[] only asserts (UB in release). Guard with Segment(QBIT_FIRST, QBIT_LAST).contains(bit) plus a logger warning and return false; same fix applies to lines 4070, 4682 and 5196.
- [ ] **4070** - TODO(captainurist): qbits value is coming from a script, need to bound-check.
  - Same fix as line 3617, but here the unchecked cast also indexes pQuestTable (IndexedArray<std::string, QBIT_FIRST, QBIT_LAST>) before _questBits.set(); add the Segment(QBIT_FIRST, QBIT_LAST).contains() guard and bail out with a warning.
- [ ] **4682** - TODO(captainurist): quest bit is coming from a script, do range checking here.
  - Identical to lines 3617/4070 in the AddVariable path - `val` from the script indexes both pQuestTable and _questBits unchecked; add the same Segment contains() guard.
- [ ] **5196** - TODO(captainurist): quest bit is coming from a script, do range checking here.
  - Same guard needed in the SubtractVariable path, where pParty->_questBits.reset(static_cast<QuestBit>(pValue)) uses std::bitset::set and would throw out_of_range on a bad script value; wrap with the Segment(QBIT_FIRST, QBIT_LAST).contains() check.
- [ ] **5599** - TODO(Nik-RE-dev): decreasing 1 seems wrong, also bits indexing was changed
  - Party::_autonoteBits is an IndexedBitset<1, 208> and every other path (Character.cpp:3839 get, 4325/4922 set) indexes it with the raw event value, so replace the assert(false) plus commented-out `reset(pValue - 1)` with `pParty->_autonoteBits.reset(pValue);`.
- [ ] **5947** - TODO(captainurist): doesn't look like a proper default.
  - This else-branch is unreachable: it sits inside `if (uActorType == OBJECT_Sprite)` (line 5920), so `uActorType != OBJECT_Character` is always true and CalcSpellDamage always wins - delete the condition and the dead else block (lines 5939, 5945-5948), keeping the spell-damage path.
- [ ] **6530** - TODO(captainurist): encapsulate the logic here.
  - Extract the variant pick plus the magic `byte_4ECF08[pickedVariant - 1][uVoiceID]` / `2 * (pickedVariant + 50 * uVoiceID) + 4998` sound-id math into a named helper (e.g. `soundIdForSpeech(SpeechId, voiceId)`) next to the tables in mm7_data; pure refactor, no behavior change.
- [ ] **6610** - TODO(captainurist): just implement properly?
  - The only callers are NPCTopics.cpp:540/543 with the four honorary promotion classes, so replace the asserting default with a plain `return false` (or a tiny Class -> AWARD_PROMOTION_*_HONORARY lookup) since no other class has an honorary award.
- [ ] **6617** - TODO(_): probably move this somewhere else, not really Character:: stuff
  - Move SelectPhrasesTransaction (shop/house logic, uses houseTable + PriceCalculator) into GUI/UI/Houses/Shops as a function taking the Character, and update the 9 call sites in Shops.cpp/MagicGuild.cpp; mechanical, no behavior change.

### `src/Engine/Objects/Character.h`

- [ ] **359** - TODO(captainurist): #enum
  - bodyType is 0..3 = {non-dwarf male, non-dwarf female, dwarf male, dwarf female} (Character.cpp:5728); introduce a small enum (e.g. PaperdollBodyType) with FIRST/LAST, use it in the signature and in the single caller loop at UICharacter.cpp:1395, indexing the paperdoll arrays via std::to_underlying or IndexedArray.

### `src/Engine/Objects/Chest.cpp`

- [ ] **132** - TODO(Nik-RE-dev): chest is originator in this case
  - There is no OBJECT_Chest Pid type, but the trap explosion sprite created one line above is the natural originator: capture `int id = pSpellObject.Create(0, 0, 0, 0);` and call playSound(SOUND_fireBall, SOUND_MODE_PID, Pid(OBJECT_Sprite, id)) instead of SOUND_MODE_UI so the blast is positional.
- [ ] **162** - TODO(captainurist): need to use mouse->pickedItemOffset here?
  - Yes - the click handler at Chest.cpp:241 uses `mapToInventoryGrid(mousePos + mouse->pickedItemOffset, offset, &pParty->pPickedItem)`, so the hover status string should use the same expression, otherwise the text describes a different cell than the one a click would hit while carrying an item.
- [ ] **336** - TODO(captainurist): GenerateArtifact calls Reset on failure, this messes up inventory state. Rewrite properly.
  - Item::GenerateArtifact (Item.cpp:236) resets *this before deciding success; convert it into a factory (e.g. `static std::optional<Item> Item::randomUnfoundArtifact()`) and fix the only two call sites - here and Indoor.cpp:1912, which also has a suspicious post-success Reset() at 1916; RNG consumption stays identical.

### `src/Engine/Objects/Item.cpp`

- [ ] **160** - TODO(captainurist): can drop potionPower?
  - For potions standardEnchantmentStrength is 0 and specialEnchantment is NULL, so dropping `potionPower ||` makes GetValue fall through to the same `return uBaseValue`; just confirm no potion ever carries a nonzero standardEnchantmentStrength (they share a save slot only with potionPower, see EntitySnapshots.cpp:451-458).

### `src/Engine/Objects/Monsters.cpp`

- [ ] **73** - TODO(captainurist): this does happen, investigate.
  - The TODO predates the 2025 rewrite of parseSpellEntry: scanning the spell columns of the en/de/fr/ru monsters.txt shows every non-empty cell is either "<spell>,<NEMG>,<digits>" or the special "Lightning Bolt,M10", both of which now parse; replace the silent CombinedSkillValue::none() with a logger->warning (or assert) and drop the TODO.
- [ ] **146** - TODO(captainurist): this is broken, we get "FireAr" for flaming arrow here.
  - monsters.txt only ever contains 0/Arrow/FireAr/Fire/Air/Water/Earth/Light/Dark/Ener, so the "ARROWF" branch is dead - change it to "FireAr"; this gives Archer/Bowman/Elite Archer a real attack1MissileType (they currently get MONSTER_PROJECTILE_NONE and so never make ranged attacks), which is a gameplay change requiring `OpenEnroth retrace`.
- [ ] **441** - TODO(captainurist): makes no sense.
  - Real bug: the explode cells are "explode,4D10,earth", so the damage type is props[2], not props[0] - ParseAttackType("explode") falls through to DAMAGE_PHYSICAL, meaning Gogs/golems/light elementals all explode for physical damage; change to props[2] (guarding props.size() >= 3) and retrace.

### `src/Engine/Objects/Monsters.h`

- [ ] **41** - TODO(captainurist): that's... a weird default.
  - DAMAGE_FIRE is just enum value 0; the field is unconditionally overwritten by ParseAttackType at Monsters.cpp:538, so change the in-class initializer to DAMAGE_PHYSICAL (matching ParseAttackType's own fallback) - only the 12 never-spawned unused-monster slots would ever observe the default.
- [ ] **47** - TODO(captainurist): and here, weird default.
  - Same as attack1Type on line 41 - always overwritten at Monsters.cpp:542; default it to DAMAGE_PHYSICAL and drop both comments in one edit.

### `src/Engine/Objects/NPC.cpp`

- [ ] **98** - TODO(captainurist): This looks broken. pNPCTopics[407].pTopic = "Mind Guild Membership" pNPCTopics[407].pText = "With Expert Air Magic you can learn all of the Expert spells for this element...." Double broken!
  - The default branch is unreachable: the sole caller (NPCTopics.cpp:615) is only reached for DIALOGUE_USE_HIRED_NPC_ABILITY, which UIDialogue.cpp:129-134 only offers for exactly the 11 professions the switch already lists - replace the pNPCTopics[407] fallback with assert(false) plus a static empty string.

### `src/Engine/Objects/NPC.h`

- [ ] **21** - TODO(Nik-RE-dev): remove
  - CheckPortretAgainstSex is a stub that always returns true; delete it, the stale mismatched forward declaration at NPC.cpp:22, and collapse NPCTable.cpp:238 (`if (Check...) break_gen = true;` becomes unconditional, which also makes the gen_attempts retry dead) - RNG consumption is unchanged so traces stay valid.

### `src/Engine/Objects/NPCEnums.h`

- [ ] **26** - TODO(captainurist): #enum renamings needed
  - NpcProfession's 59 enumerators are CamelCase (Smith, ExpertHealer, ChimneySweep) instead of the HACKING.md rule of type-prefixed SNAKE_CASE_ALL_CAPS - the FIRST/LAST aliases already use NPC_PROFESSION_*; a mechanical, compiler-verified rename across NPCTable/UIDialogue/NPCTopics/NPC.cpp.

### `src/Engine/Objects/SpriteObject.cpp`

- [ ] **236** - TODO: why pActors.size() - 1? Should just check for .size()
  - The guard is just an actorId range check, so `pActors.size() - 1` makes projectiles fired by the last actor in the array skip actor collision entirely; change to `actorId < pActors.size()` and retrace (the empty-vector underflow is harmless today because the inner loop runs zero times).
- [ ] **371** - TODO(Nik-RE-dev): check purpose of inner loop
  - Answerable from the code: the loop re-runs collision detection after a portal crossing moved the sprite into a new sector, and it is dead today because CollideIndoorWithPortals() is hard-stubbed to `return true` at Collisions.cpp:500-503, so it always breaks after one iteration - replace the TODO with that comment (same pattern as Collisions.cpp:619 and :901).

### `src/Engine/Pid.h`

- [ ] **7** - rename according to codestyle
  - HACKING.md requires SNAKE_CASE_ALL_CAPS enum values, so OBJECT_None/OBJECT_Door/... become OBJECT_NONE/OBJECT_DOOR/...; a mechanical sed over ~396 occurrences in src/ and test/ plus a rebuild, no behavior risk.

### `src/Engine/Resources/LodSpriteCache.cpp`

- [ ] **13** - dependency doesn't belong here
  - The include exists only for the `assets->getSprite()` call at line 59; drop both once that call is moved to the 4 callers in Engine/Graphics/Sprites.cpp (lines 52/75/107/148), which already hold the sprite name.
- [ ] **59** - very weird dependency here.
  - The resource-layer cache calls up into AssetsManager to fill Sprite::texture; move `sprite->texture = assets->getSprite(name)` into the only 4 callers of loadSprite() (Engine/Graphics/Sprites.cpp:52/75/107/148) so the cache stays pure data-loading.

### `src/Engine/Resources/LodTextureCache.h`

- [ ] **27** - doesn't belong here.
  - LoadCompressedTexture is a one-line forwarder to `lod::decodeMaybeCompressed(_reader.read(...))` with only two users (GUIFont.cpp:36 and the PCX_LOD_Compressed_Loader template in ImageLoader.h:117 constructed at AssetsManager.cpp:144); expose `LodReader &reader()` and move the decode call to those two sites.
- [ ] **28** - doesn't belong here.
  - `read()` is a pure forwarder to _reader.read() with a single user (TileGenerator.cpp:93); delete it in favour of a `reader()` accessor on the cache.

### `src/Engine/SpellFxRenderer.cpp`

- [ ] **72** - TODO (mcgreentn): should we use an epsilon here?
  - pOnPlane (exact float ==) is only ever used as `pInFront | pOnPlane` (lines 76 and 84), i.e. plain `p[1].x >= ViewPlaneDistPixels`; collapse the two bools into one `>=` comparison and the epsilon question disappears with zero behaviour change.
- [ ] **506** - TODO(Nik-RE-dev): check colour format
  - History shows the line replaced `vrng->random(0x10000) | (vrng->random(0x10000) << 16)`, i.e. a fully random 32-bit diffuse; since all four Color components are now independent uniform bytes, channel order cannot matter, so the only real question is whether alpha should be random or 255 - fix alpha (or confirm) and delete the comment.

### `src/Engine/Spells/CastSpellInfo.cpp`

- [ ] **568** - TODO(captainurist): why transparent black?
  - Color() is a sentinel: sparklesOnActorAfterItCastsBuff (SpellFxRenderer.cpp:503) treats a default-constructed Color as "randomize each particle's colour"; make the parameter std::optional<Color> (or add a no-colour overload) and update the three sentinel call sites (568, 611, 1676) - the ten Actor.cpp callers pass real colours and stay as is.
- [ ] **611** - TODO(captainurist): why transparent black?
  - Same sentinel-as-Color() issue as line 568 (Earth Slow); fixed by the same std::optional<Color> change to SpellFxRenderer::sparklesOnActorAfterItCastsBuff.
- [ ] **667** - TODO: spell_id different?
  - initSpellSprite already sets pSpellSprite.uSpellID = SPELL_DARK_SHRINKING_RAY, then line 676 overwrites it with SPELL_FIRE_PROTECTION_FROM_FIRE; uSpellID only feeds playSpellSound, so the sprite currently plays sound 10021 instead of 18031 on the shrink-ray AoE path (SpriteObject.cpp:1074) - delete line 676.
- [ ] **1676** - TODO(captainurist): why transparent black?
  - Third occurrence of the Color()-as-"random colour" sentinel (Spirit Fate on an actor); resolved by the same signature change to sparklesOnActorAfterItCastsBuff.
- [ ] **2204** - TODO(captainurist): investigate, that's a very weird std::to_underlying call.
  - The condition indexes pLevelDecorations with the item id of an unrelated sprite object (pSpriteObjects[obj_id].containing_item.itemId) - an out-of-bounds read in a decoration branch whose body uses pLevelDecorations[obj_id]; replace it with pLevelDecorations[obj_id].IsInteractive().
- [ ] **2653** - TODO: why call SetCondition and then conditions.set?
  - Character::SetCondition already does conditions.set(condition, GetPlayingTime()) at its end, so the extra call only matters when SetCondition bailed out early (lich, eradicated, already zombie) - i.e. it force-zombifies characters the game refuses to zombify; delete line 2652 (and consider guarding the portrait reload the same way).

### `src/Engine/Spells/SpellBuff.h`

- [ ] **7** - TODO(pskelton): style
  - Methods violate HACKING.md ("method names should start with a lowercase letter"): Apply/Reset/IsBuffExpiredToTime/Active/Inactive/Expired/GetExpireTime, plus Hungarian params uSkillMastery/uPower/uOverlayID; a purely mechanical, compiler-checked rename over ~90 call sites in 8 files.

### `src/Engine/Spells/Spells.cpp`

- [ ] **486** - TODO(captainurist): #enum, use enum serialization
  - Add MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(DamageType, CASE_INSENSITIVE, {...}) to src/Engine/Objects/ItemEnumFunctions.cpp plus MM_DECLARE_SERIALIZATION_FUNCTIONS(DamageType) in ItemEnums.h, then drop the local spellSchoolMaps and do `DamageType t = DAMAGE_PHYSICAL; tryDeserialize(tokens[3], &t);` — the serializer already supports case-insensitive lookup and alias names.

### `src/Engine/Tables/ItemTable.cpp`

- [ ] **92** - TODO(captainurist): #enum use enum serialization
  - Same mechanical change as the spell school map: declare serialization for ItemType in ItemEnums.h / ItemEnumFunctions.cpp with the alias rows (weapon1or2, missile, herb...) — the EnumSerializer explicitly supports repeated values as deserialize-only aliases — then replace valueOr(equipStatMap, ...) with tryDeserialize into an ITEM_TYPE_NONE-initialized variable.

### `src/Engine/Tables/ItemTable.h`

- [ ] **51** - TODO(captainurist): do a proper struct here
  - Replace the ItemId payload with e.g. `struct PotionCombination { ItemId result = ITEM_NULL; int damageLevel = 0; }`; only two places need updating — the parser at ItemTable.cpp:266-275 which stuffs the "E{n}" damage level into an ItemId, and UIPopup.cpp:2077-2089 which decodes it back via isPotion()/to_underlying.

### `src/Engine/Time/Duration.h`

- [ ] **95** - TODO(captainurist): #time add unit tests.
  - Add src/Engine/Time/Tests/Duration_ut.cpp covering roundedUp/roundedDown, toCivilDuration/toLongCivilDuration and the from*/realtime* conversions, and wire it up in src/Engine/Time/CMakeLists.txt with the same OE_BUILD_TESTS block used by src/Engine/Spells/CMakeLists.txt.

### `src/GUI/GUIEnums.h`

- [ ] **285** - TODO(captainurist): #enum class
  - Do exactly what MenuType/ScreenType above already do: make it `enum class WindowType {...}` followed by `using enum WindowType;`, then drop the elaborated `enum` keyword at src/GUI/GUIWindow.cpp:70 and src/GUI/GUIWindow.h:217; no arithmetic or int conversions on WINDOW_* exist, and MM_DEFINE_ENUM_MAGIC_SERIALIZATION_FUNCTIONS works either way.

### `src/GUI/GUIFont.cpp`

- [ ] **42** - TODO(pskelton): Save built atlas so it doesnt get recalcualted on reload?
  - CreateFontTex() re-packs all glyphs into a new RgbaImage every time AssetsManager::ReloadFonts() runs (src/Engine/AssetsManager.cpp:19-40); since GraphicsImage keeps its _rgba and re-uploads lazily via renderId(), the fix is to add a GUIFont method that just calls _texture->releaseRenderId() and use it in ReloadFonts instead of CreateFontTex().

### `src/GUI/GUIWindow.cpp`

- [ ] **157** - TODO(captainurist): Sus. Comparing height to width?
  - Change `pWindow->frameRect.h == width` to compare against the height, and use render->GetRenderDimensions() (windows are constructed with render dims, not present dims); note the break currently never fires at 640x480, so hotkey scanning will start stopping at the first fullscreen window - needs a game test pass.
- [ ] **185** - TODO(captainurist): logger can be NULL here if we're called from cxa_finalize.
  - `logger` is a plain global pointer initialized to nullptr (src/Library/Logger/Logger.cpp:9); wrap the trace call in `if (logger)` in ~GUIWindow (the same guard the constructor's trace at line 446 would want), or ensure the window globals are torn down before the logger.
- [ ] **942** - TODO(captainurist): encapsulate this logic in PriceCalculator
  - Add e.g. `PriceCalculator::itemSellingPriceForPlayerCheap(player, item, mult)` doing the `/2` with the min-1 clamp and call it here and at line 920 (base price variant); the path is MM6-only (SHOP_SCREEN_SELL_FOR_CHEAP is unused in MM7 per GUIEnums.h:357), so risk is nil.

### `src/GUI/GUIWindow.h`

- [ ] **48** - TODO(pskelton): move to button
  - ButtonType belongs in GUIButton.h, but GUIButton.h currently includes GUIWindow.h while GUIWindow::CreateButton takes a ButtonType, so the move needs the include cycle broken (drop GUIButton.h's include of GUIWindow.h, or park the enum in GUIEnums.h) - otherwise mechanical.
- [ ] **84** - TODO(pskelton): string_view or ref?
  - DrawMessageBox's `std::string hint` is only read, so change it to std::string_view; the one catch is `callObserver->notify(CALL_DRAW_MESSAGE_BOX, sHint)` deduces T from the parameter and test/Testing/Game/CommonTapeRecorder.cpp:180 records `std::string`, so pass `std::string(hint)` (or update the recorder).
- [ ] **85** - TODO(pskelton): inside_game_viewport used anywhere?
  - No: all 21 call sites (UIPopup.cpp, UIGameOver, UIPartyCreation, UIMessageScroll, LoadStep2State) pass 0/false, so delete the parameter and the dead pViewport branch in GUIWindow.cpp:213-217 and drop the leading argument at each call.

### `src/GUI/UI/Books/TownPortalBook.cpp`

- [ ] **210** - render->DrawQuad2D(game_ui_statusbar, {0, 352}); // TODO(captainurist): engine->_statusBar->smthSmth()???
  - Add a small `StatusBar::drawBackground()` that does the DrawQuad2D, use it from StatusBar::draw()/drawForced() (UIStatusBar.cpp:25,34) and call it here and at UIBranchlessDialogue.cpp:39 so game_ui_statusbar stops being poked directly from UI code.

### `src/GUI/UI/NPCTopics.cpp`

- [ ] **777** - TODO(captainurist): #unicode
  - ascii::noCaseEquals only case-folds ASCII, so it degrades to exact compare for localized (e.g. Russian) NPC names; the hireling entry is a copy of the same NPCData, and the same identity check at line 794 already uses plain `==`, so replace the call with `pParty->pHirelings[hirelingId].name == npcData->name`.
- [ ] **799** - TODO(captainurist): #unicode
  - Same as line 777: hireling slot 0 name vs npcData->name; replace ascii::noCaseEquals with `==` to match the exact comparison already used at line 794 in the same block.
- [ ] **801** - TODO(captainurist): #unicode
  - Same as line 799 for hireling slot 1; both branches can be replaced with plain `==` in one edit.

### `src/GUI/UI/UICharacter.cpp`

- [ ] **1001** - TODO(captainurist): need to also z-draw arms and wrists.
  - In the underwater-suit branch only the body is registered in equipmentHitMap (line 1004) while the arm texture paperdoll_dlads is drawn at 1011 without a hit-map entry and the two-handed-grip variant paperdoll_dlaus is not drawn at all; mirror lines 1080-1088 and add `equipmentHitMap.add(...)` with `player->inventory.entry(ITEM_SLOT_ARMOUR).index()` for those quads, guarded by `!bRingsShownInCharScreen`.
- [ ] **1312** - TODO(pskelton): #time check tickcount usage here
  - The blend phase uses wall-clock `platform->tickCount() / 10` while the animation's lifetime right above is driven by ItemEnchantmentTimer (game time via pEventTimer->dt()); derive the phase from ItemEnchantmentTimer.ticks() instead so the effect is consistent and deterministic under trace replay.

### `src/GUI/UI/UICredits.cpp`

- [ ] **45** - TODO(captainurist): #time gotta be dt-based.
  - GUICredits::Update() advances _moveY by a fixed 0.25 per frame, so the credits scroll faster at higher frame rates; store the previous platform->tickCount() (game timers are irrelevant in CreditsState) and scale the increment by the elapsed milliseconds.

### `src/GUI/UI/UIDialogue.cpp`

- [ ] **361** - TODO(captainurist): #unicode this is not ascii
  - Same pattern as NPCTopics.cpp:799 — the loop just above at line 358 already compares the very same names with plain `==`, so replace ascii::noCaseEquals(pParty->pHirelings[0].name, speakingNPC->name) with `==`.
- [ ] **363** - TODO(captainurist): #unicode this is not ascii
  - The hireling-slot-1 half of the same comparison; fixed by the same one-line change to `==`.

### `src/GUI/UI/UIGame.cpp`

- [ ] **250** - TODO(pskelton): #time check tickcount usage here
  - GameMenuUI_GetKeyBindingColor blinks using wall-clock `platform->tickCount() % 1000`, which makes the keybindings menu render non-deterministically across trace replays/screenshot comparisons; either swap it for an engine-side frame/Duration counter or confirm real time is acceptable here and drop the TODO.
- [ ] **992** - TODO(captainurist): get rid of this std::to_underlying cast.
  - An ItemId is funnelled through the int `pickedObjectID` only so the legacy z-buffer checks at 1002-1003 (`== 0 || == -65536 || >= 5000`) can run, and those are meaningless for an item id; rewrite this branch to test the InventoryEntry directly (`if (pItemGen) { setPermanent(pItemGen->GetDisplayName()); ... }`) and drop the cast.
- [ ] **1372** - TODO(pskelton): actually check for party movement
  - `partymoved` is hardcoded true so the outdoor minimap texture is rebuilt and reuploaded every frame and the else branch is dead; cache the party position, zoom and the viewparams->location_minimap pointer in statics next to `minimaptemp` and only regenerate when one of them changes (the minimap pointer check also covers map reloads).

### `src/GUI/UI/UIPartyCreation.cpp`

- [ ] **748** - TODO(pskelton): why just CHARACTER_BUFF_RESIST_WATER?
  - This runs once after party creation finishes on freshly constructed Character objects that have no active buffs, so the single Reset() is a no-op decompilation artifact - either drop the line or reset all pCharacterBuffs entries in a loop; no observable behavior change either way.

### `src/GUI/UI/UIPopup.cpp`

- [ ] **1198** - TODO(captainurist): fmt can throw
  - fmt::sprintf is called with a runtime format string from game data (LSTR_FMT_RECOVERY_TIME_D), which throws fmt::format_error on a malformed %-spec; route it through localization->format(LSTR_FMT_RECOVERY_TIME_D, ticks) and make Localization::format (Localization.h:27, which carries the same TODO) catch format_error and fall back to the raw string.
- [ ] **1214** - TODO(captainurist): fmt can throw
  - Identical to line 1198 - the fmt::sprintf on line 1219 with the game-data format string LSTR_FMT_RECOVERY_TIME_D; same fix (use localization->format plus a non-throwing fallback there).
- [ ] **1662** - TODO(captainurist): do a 2nd rewrite here
  - Cleanup-only: GameUI_CharacterQuickRecord_Draw still carries decompiler locals (v13, v36, uFramesetIDa, the declared-far-from-use spellName) and builds its text as four concatenated fmt::format calls; merge into one format call with named locals, keeping the emitted string byte-identical.
- [ ] **1717** - TODO(captainurist): we need a saner check for baby dragon, comparing pointers here is questionable.
  - NPCData has no id field, but the same magic index is already hardcoded in PartyHasDragon() (NPC.cpp:50) - add a named constant (e.g. NPC_ID_BABY_DRAGON = 57) plus a small `isBabyDragon(const NPCData *)` helper next to it and use it in both places.
- [ ] **1768** - TODO(captainurist): use pchestoffsets
  - The hardcoded (42, 34) origin is the old pChestOffsets; the modern table is chestTable[vChests[id].chestTypeId].inventoryOffset - replace the two divisions with mapToInventoryGrid(mousePos, chestTable[...].inventoryOffset) exactly as Chest.cpp:163 does.

### `src/GUI/UI/UISaveLoad.cpp`

- [ ] **183** - TODO(captainurist): #unicode might not be ascii
  - The on-disk name is always written as "saves/autosave.mm7" by autoSave() (SaveLoad.cpp:230), so compare pFileList[i] against that shared constant instead of the localized LSTR_AUTOSAVE_MM7 string, which removes the encoding question entirely (LSTR_AUTOSAVE stays for the displayed name).

### `src/GUI/UI/UITransition.cpp`

- [ ] **40** - TODO(Nik-RE-dev): Use location enums here.
  - Replace the 11 .blv strings with a std::array<MapId, 11> (MAP_DRAGON_CAVES, MAP_LORD_MARKHAMS_MANOR, MAP_BANDIT_CAVES, ...) and compare pMapStats->pInfos[map].fileName against the argument, keeping the same 1-based return index; don't call MapStats::GetMapInfo directly since it asserts on unknown names.

### `src/Io/KeyboardActionMapping.h`

- [ ] **18** - #enum
  - TextInputType's values None/Text/Number violate HACKING.md (SNAKE_CASE_ALL_CAPS prefixed with the type name plus a `using enum`); rename to TEXT_INPUT_TYPE_NONE/_TEXT/_NUMBER, add `using enum TextInputType;`, and update the ~15 mechanical call sites in KeyboardInputHandler and the GUI/Application files - optionally also move the enum out of KeyboardActionMapping.h, where it does not belong.

### `src/Io/Mouse.cpp`

- [ ] **252** - actual shape is oval, this check is bugged.
  - Confirmed bug: the click test uses sqrt(dx*dx+dy*dy) < rect.w (a circle of radius = width) while the hover test in UIGame.cpp:1041-1051 uses the correct ellipse test dx*dx/w*w + dy*dy/h*h < 1.0; replace the check with the ellipse form, ideally extracting it into a GUIButton helper shared by all three sites.
- [ ] **341** - Move this to keyboard
  - UI_OnKeyDown (lines 342-452) is already declared in KeyboardInputHandler.h yet defined in Mouse.cpp; cut it into KeyboardInputHandler.cpp, add includes for GUI/GUIButton.h, Media/Audio/AudioPlayer.h and Engine/EngineIocContainer.h, and drop the local `extern bool UI_OnKeyDown` re-declaration at GUIWindow.cpp:556.

### `src/Library/FileSystem/Mounting/MountingFileSystem.cpp`

- [ ] **148** - this is not symmetric with that's done in read / openForReading / openForWriting.
  - Mirror the read path: `auto [node, mount, tail] = walk(path); return mount ? mount->displayPath(tail) : join(_displayName, "://", path.string());` - MountingFileSystem has no users outside its own unit test and no displayPath assertions there, so the risk is limited to picking what virtual (node-only) directories should report.

### `src/Media/Audio/OpenALSoundProvider.h`

- [ ] **43** - contain?
  - setSourceDefaults()/checkOpenALError() are free functions used only inside media_audio (OpenALSample16.cpp, OpenALTrack16.cpp, OpenALAudioDataSource.cpp); make them static members of OpenALSoundProvider (or move them to an internal header in an anonymous/detail namespace) and mechanically update the ~30 call sites.

### `src/Media/Audio/SoundList.h`

- [ ] **9** - should be const
  - soundInfo() can't be const only because AudioPlayer::loadSoundDataSource lazily fills si->dataSource; mark `mutable PAudioDataSource dataSource;` in SoundInfo, make soundInfo() const returning `const SoundInfo *`, and change loadSoundDataSource to take `const SoundInfo *` (two call sites, AudioPlayer.cpp:198 and :427).

### `src/Scripting/GameBindings.cpp`

- [ ] **30** - This static variable must be moved elsewhere
  - Drop the `static` from GameBindings::_characterInfoQueryTable, declare it `mutable std::unique_ptr<...>` in the header (createBindingTable is const), delete the out-of-line definition at GameBindings.cpp:26 and the destructor body; ScriptingSystem owns the bindings and destroys them before _solState, so lifetime is still safe.
- [ ] **37** - This check is also a hack to avoid initializing the static characterInfoTable. multiple times, Needs to be moved to another place where the ScriptingSystem is providing binding helpers
  - Same fix as the static above - once _characterInfoQueryTable is a per-instance mutable member, the `if (!_characterInfoQueryTable)` guard stops being a hack and becomes a plain lazy init; the larger "ScriptingSystem provides binding helpers" idea can be dropped from the comment.
- [ ] **63** - We shouldn't have a misc table but it will disappear soon
  - The table holds only two entries: goToScreen, which no Lua script calls (only declared in resources/scripts/_definitions/def_common.lua:13) and can be deleted, and canClassLearn, whose single caller is skills_command.lua:58 - move it onto an existing table (e.g. Game.party/Game.items style) and update def_common.lua/def_game.lua.
- [ ] **146** - #enum need proper toDisplayString.
  - Mastery has no serialization functions anywhere in the tree; add MM_DECLARE/MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS(Mastery) next to CharacterEnums.h/CharacterEnumFunctions and replace `static_cast<int>(*mastery)` with `toString(*mastery)` in the Exception message.

### `src/Scripting/GameBindings.h`

- [ ] **24** - This variable becoming a static is just a temporary hack Needs to be moved to another place where the ScriptingSystem is providing binding helpers instead
  - Duplicate of the GameBindings.cpp:30 item - making _characterInfoQueryTable a `mutable` non-static member removes the hack and lets both comments be deleted.

## Stale (7)

_Already done or obsolete - the comment itself is the thing to delete._

### `src/Engine/Evt/EvtInstruction.cpp`

- [ ] **844** - TODO(yoctozepto): zeroing-out the struct to prevent values from previous events from lingering; this makes it slightly easier to spot uninitialised members but, since the 0s may have a proper meaning, not always;
  - The zero-initialization it describes is already implemented on the next line (EvtInstruction ir = {}); the comment is an explanatory note about a tradeoff, not an open task, and can be demoted to a plain comment.

### `src/Engine/Graphics/Collisions.cpp`

- [ ] **894** - TODO(captainurist): why there is no call to _46ED8A_collide_against_sprite_objects? See ProcessPartyCollisionsODM.
  - The question is answered by pskelton's follow-up comment on the next line (essentially no sprite objects exist in BLV maps), and the original binary's BLV path matches; downgrade the TODO to a NOTE or delete it.

### `src/Engine/Graphics/Renderer/OpenGLRenderer.cpp`

- [ ] **758** - TODO (pskelton): should force drawing if buffer is full
  - Obsolete since commit 6124bcf1e4e (#2309, 'Use vectors for vertex buffer data') replaced the fixed store + forceperstorecnt with std::vector<ForcePerVertex> _forcePerVertices that grows unbounded and is uploaded wholesale in DrawForcePerVerts; there is no 'full' state anymore, delete the comment.
- [ ] **2750** - might have to pass a texture width through for the waterr flow textures to size right and get the correct water speed
  - glbspshader.frag now computes `ivec3 texsize = textureSize(textureArray0, 0)` and divides all flow/water deltas by it, so no width needs to be passed through from C++; the comment can be deleted.
- [ ] **3029** - TODO(captainurist): adding in IDs below?
  - The question came from the old field names `pVertexUIDs`/`pTextureUIDs` (introduced in d2dc72f44d6), which read as if vertex IDs were being added to UVs; the fields are now `textureUs`/`textureVs` (plain UV coords) and faceExtras indirection is gone, so the comment can be deleted.
- [ ] **3044** - TODO(captainurist): adding in IDs???
  - Same as the TODO 15 lines above — an artifact of the old `pVertexUIDs`/`pVertexVIDs` naming; the code now adds `face->textureUs[z+i] + face->textureDeltaU`, which is plainly UV math, so delete the comment.
- [ ] **3513** - TODO: invalidate all previously loaded textures and then load them again as they can be no longer alive on GPU (issue #199).
  - The two lines below it already do exactly this — `assets->releaseAllTextures()` drops every render id (AssetsManager.cpp:42-56) so textures are re-uploaded lazily, plus ReleaseTerrain/ReleaseBSP; the comment can be deleted (the remaining open point is the platform-testing TODO on the next line).

## Medium (184)

_Clear goal, but touches several files or needs care and tests._

### `src/Application/Game.cpp`

- [ ] **814** - TODO(Nik-RE-dev): need separate message type
  - Requires adding a new UIMSG enum value and making the sender side (the character-portrait buttons created in UISpell.cpp / handled through the enchant path in UICharacter.cpp and Character.cpp, all gated on the IsEnchantingInProgress global) post it instead of overloading UIMSG_CastSpell_TargetCharacter — touches several GUI files and the spell-targeting flow needs retesting.
- [ ] **1107** - TODO: sometimes it is called twice, prevent that for now and investigate why later
  - The TODO is the investigation itself: trace why UIMSG_Rest8Hour is sometimes enqueued twice (double posting from the rest UI button vs. keyboard path) before removing the defensive clear(); no obvious repro, so it needs message-queue debugging rather than a known fix.

### `src/Application/GameConfig.h`

- [ ] **31** - TODO(captainurist): apply codestyle here.
  - Renaming the UpperCamelCase config entry members (NoSound, TurboSpeed, ...) to the project's member naming convention is purely mechanical but ripples through hundreds of `config->section.Entry` use sites across the whole codebase, so it is a large, review-heavy rename rather than an easy fix.
- [ ] **124** - TODO(captainurist): move all Trace* options into a separate section.
  - Mechanical but multi-file: add a new `Trace : ConfigSection` class, move the ~5 Trace* entries, and update all `config->debug.Trace*` references in the trace/test components; dev-only options so the ini section rename is harmless, but the change spans several files.
- [ ] **467** - TODO(captainurist): #enum, will need to support cycleIncrement for enums entries.
  - Converting RenderFilter from Int to a proper enum ConfigEntry needs a new enum type with serialization plus extending the config-entry cycling machinery (used by the in-game options UI) to enumerate enum values — a small feature in the config framework, not a local edit.
- [ ] **611** - TODO(captainurist): move to [audio]?
  - The move itself is mechanical, but music_level is a real user setting persisted in [settings] and wired to the in-game sliders; GameStarter explicitly migrates nothing but saves, so moving the key silently resets users' volume unless a config-migration mechanism is decided on first.
- [ ] **626** - TODO(captainurist): move to [audio]?
  - Same situation as MusicLevel at line 611: mechanically trivial move of walk_sound to [audio], but it is a persisted user setting and there is no config migration, so it needs the same decision about resetting existing users' configs.

### `src/Application/GameMenu.cpp`

- [ ] **43** - TODO(Nik-RE-dev): drop variable and load game only on double click
  - The click-then-click-again load flow via this static flag is shared between GameMenu.cpp and GUIWindow_Load::slotSelected in UISaveLoad.cpp (which also has an unused shadow member isLoadSlotClicked); switching to real double-click needs plumbing the double-click event into the save-slot list and retesting both the main-menu and in-game load screens.

### `src/Application/GameWindowHandler.cpp`

- [ ] **226** - TODO(captainurist): `UI_OnMouseRightClick` is a mixed-concern dispatcher — it both draws popups and runs actions (potion mixing, identify/repair, monster-id speech). It should be split into two functions: one for popup rendering (called from `GUI_UpdateWindows`) and one for press-time actions (called from here). When that split happens, this `tryUseItemOnPortrait` call should be folded into the actions function so all "right-click mutates something" paths sit alongside each other in the press handler.
  - The target design is fully spelled out, but executing it means teasing apart a ~250-line per-screen switch in UIPopup.cpp:1745 into draw-every-frame vs run-once-at-press halves — behavior-sensitive (actions must not re-run each frame) and needs game-test coverage across many screens.
- [ ] **291** - TODO: many of hardcoded keys below should be moved out of there and made configurable
  - Requires auditing the key dispatch in OnKey, defining new InputAction values for the currently hardcoded keys, and wiring them through KeyboardActionMapping (and potentially the key-binding options UI) — clear goal but spread over the input-mapping files.

### `src/Application/Startup/GameStarter.cpp`

- [ ] **172** - TODO(captainurist): Initialize should throw?
  - Mechanical but multi-file: change Renderer::Initialize from bool to void-throwing across Renderer.h, BaseRenderer, NullRenderer and OpenGLRenderer (whose internals currently return false in several failure paths that would each need descriptive exceptions), plus this caller — aligns with the codebase's exception style but spans the renderer hierarchy.

### `src/Application/Startup/PathResolver.cpp`

- [ ] **97** - TODO(captainurist): need a mechanism to show user-visible errors. Commenting out for now.
  - Partially superseded — GameStarter::failOnInvalidPath now shows a platform message box for bad data paths — but it runs after platform creation while this Android no-storage case fires earlier (and an empty candidate list would hit candidates.back() on an empty vector in resolveDataPath), so surfacing this specific error needs startup reordering or a deferred-error mechanism.

### `src/Arcomage/Arcomage.h`

- [ ] **132** - TODO(pskelton): make vector ?
  - cards_at_hand uses fixed slots with -1 sentinel holes plus a parallel card_shift[10] array, and slot indices drive card positions/animations across ~28 use sites in Arcomage.cpp; a vector changes slot semantics (holes would shift), so this is a careful rework, not a type swap.

### `src/Engine/Components/Control/EngineControlComponent.h`

- [ ] **17** - TODO(captainurist): Engine- in component names kinda doesn't make sense. Drop?
  - Mechanical but wide rename: EngineControlComponent/EngineController/EngineControlState/EngineTrace* classes, their files, CMakeLists and every include/test that uses them; also needs the maintainer's yes before doing it.

### `src/Engine/Components/Trace/EngineTracePlayer.cpp`

- [ ] **104** - TODO(captainurist): this really should be just a JSON diff.
  - Replacing ~60 lines of field-by-field checkState comparisons requires a generic JSON diff helper that still produces readable per-field error messages, plus wiring EventTraceGameState through its JSON snapshot form; clear goal but new utility code and error-reporting rework.

### `src/Engine/Components/Trace/EngineTraceStateAccessor.cpp`

- [ ] **57** - TODO(captainurist): Right now setting keybindings here doesn't work
  - Needs investigation of why keybinding config entries applied by patch.apply() during playback don't take effect (the keyboard action mapping is presumably read from config only at startup) and then adding a resync/notification path between config and the input mapping.

### `src/Engine/Data/PortraitFrameData.h`

- [ ] **3** - TODO(captainurist): Data -> Objects dependency, we don't want that.
  - PortraitId lives in Engine/Objects/CharacterEnums.h; breaking the layering means splitting/moving that enum header below Engine/Data and updating many includes, and the destination is itself undecided (see the Engine/Data/CMakeLists.txt TODO).

### `src/Engine/Data/SpecialEnchantmentData.h`

- [ ] **5** - TODO(captainurist): Data -> Objects dependency, we don't want that.
  - The struct needs ItemType and the ITEM_TYPE_*_SPECIAL_ENCHANTABLE range from Engine/Objects/ItemEnums.h; splitting ItemEnums out of Engine/Objects is a wide include-graph refactor whose target location is still an open question.

### `src/Engine/Data/StandardEnchantmentData.h`

- [ ] **7** - TODO(captainurist): Data -> Objects dependency, we don't want that.
  - Same layering problem as SpecialEnchantmentData.h: requires relocating ItemType enums out of Engine/Objects/ItemEnums.h, touching many includes, with the destination undecided.

### `src/Engine/Engine.cpp`

- [ ] **315** - TODO(captainurist): this math makes no sense
  - The flicker term (ran - RAND_MAX*.4)/torchLightFlicker scales with the platform-dependent RAND_MAX macro (32767 on MSVC vs 2^31-1 on glibc) and is biased upward, so behavior differs per platform; fixing needs a deliberately designed flicker model plus visual verification (vrng, so traces are unaffected).
- [ ] **615** - TODO(captainurist): on error in `open` we had this: Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY)); however, at this point localization isn't initialized yet, so this was a guaranteed crash. Implement proper user-facing error reporting!
  - Needs an error-reporting mechanism (platform message box with hardcoded English text) that works before localization and game data are loaded, plus wiring it into the LOD/resource open failure paths in MM7_LoadLods and startup.

### `src/Engine/EngineGlobals.h`

- [ ] **17** - TODO(captainurist): drop all of these, they are accessible through PlatformApplication
  - Replacing the platform/window/eventHandler/eventLoop/openGLContext globals with accessors through the application global is mechanical but touches roughly 58 use sites across ~27 files.

### `src/Engine/EngineIocContainer.h`

- [ ] **14** - TODO(captainurist): this is a legacy class, drop.
  - Engine already holds all six objects (decal_builder, mouse, vis, etc.), so the goal is clear: move construction out of the container and replace ~57 Resolve() calls in 22 files, but init order needs care since RendererFactory and GameStarter use it before Engine is fully up.

### `src/Engine/Evt/EvtEnums.h`

- [ ] **7** - TODO(captainurist): rename the enum values properly
  - Renaming ~150 EVENT_*/VAR_* mixed-case values (e.g. EVENT_SpeakInHouse) to codebase-style UPPER_SNAKE names is compiler-checked and mechanical, but spans ~360 use sites in the event interpreter and needs a naming scheme decided first.

### `src/Engine/Evt/EvtInstruction.cpp`

- [ ] **859** - TODO(captainurist): verify enum ranges here.
  - parse() does a dozen raw static_casts to enums (HouseId, SoundId, ItemId, ChestFlag, SpeechId, Season, ...) without validation; fixing needs per-enum valid-range checks plus a consistent failure policy matching requireSize's exceptions, and tests against real evt data.

### `src/Engine/Evt/EvtInterpreter.cpp`

- [ ] **178** - TODO(captainurist): ir.data.sound_descr.x, ir.data.sound_descr.y used to be passed in.
  - The parsed x/y coordinates are dropped because AudioPlayer::playSound only supports Pid-based positioning, not raw coordinates; restoring this needs a coordinate-positioned playback path in the audio subsystem plus checking how the original spatializes scripted sounds.

### `src/Engine/Evt/Processor.cpp`

- [ ] **90** - TODO(Nik-RE-dev): using time of last visit will help timers only slightly because each map leaving resets it. To support fair timers they need to be saved directly.
  - Requires persisting per-map MapTimer state into savegames: new snapshot structs and serialization plumbing in Engine/Snapshots plus rework of the alarm-recomputation logic in registerTimerTriggers, with save-compat handling.

### `src/Engine/Graphics/BSPModel.h`

- [ ] **12** - visibility flag TODO(pskelton): use for map tooltip checking or remove
  - Either path is multi-file: removal touches the writers in OpenGLRenderer.cpp (2284, 2473) and the snapshot copy in CompositeSnapshots.cpp:433/EntitySnapshots.h (save-format care), while using it for map tooltips is new feature work in the outdoor tooltip code.

### `src/Engine/Graphics/BspRenderer.cpp`

- [ ] **9** - TODO(yoctozepto): we should not see it here
  - Asks to eliminate the file-scope global pBspRenderer instance; de-globalizing means deciding an owner (Engine vs indoor renderer) and updating 26 references across 8 files including Vis, Outdoor and both renderers.
- [ ] **21** - TODO(yoctozepto): might be nice to check if the vertices actually form a plane and not a line; this could be done when loading the location and filtering out such broken faces
  - Moving degenerate-face filtering to indoor level load needs a collinearity check plus verification that dropped faces are not still needed by collision/event/portal code, and regression testing across all indoor maps.
- [ ] **43** - TODO(yoctozepto): original vertices could have been just Vec3f
  - Switching originalFaceVertices from RenderVertexSoft to Vec3f requires Camera3D::ClipFaceToFrustum (and the CalcPortalShapePoly pipeline consuming its output) to accept Vec3f, an API change with other callers such as OpenGLRenderer.cpp:683.

### `src/Engine/Graphics/BspRenderer.h`

- [ ] **33** - TODO(yoctozepto): hide these
  - Purely mechanical encapsulation (private members plus span/count accessors) but the fields are read in ~20 places across 7 files (Engine.cpp, Indoor, Outdoor, BaseRenderer, OpenGLRenderer, Vis), so it is a multi-file sweep rather than a localized edit.

### `src/Engine/Graphics/Camera.cpp`

- [ ] **56** - TODO(pskelton): swap components to match expectation - eg x is depth make it z
  - Changing the view-space convention means updating every consumer of ViewTransform/vWorldViewPosition in lockstep (~40 references across SpellFxRenderer, Vis, OpenGLRenderer, Camera projection/clip code); mechanical mapping but any missed site silently breaks rendering or picking, so it needs screenshot-diff verification.

### `src/Engine/Graphics/Collisions.cpp`

- [ ] **135** - TODO(pskelton): should probably tweak EPS when finished moving to floats
  - The float migration is done, so the task is now live, but choosing a replacement for the 65536.0f near-parallel cutoff (and COLLISIONS_EPS interplay) has no derivable correct value; it needs empirical tuning validated by the collision-sensitive game tests.
- [ ] **141** - TODO(captainurist): move into face->Contains.
  - BLVFace::Contains has ~10 other call sites, several of which deliberately pass off-plane points (FACE_XY_PLANE overrides with z=0 in Indoor.cpp/Outdoor.cpp), so the assert must be made conditional on override_plane/slack and every caller audited to confirm the on-plane precondition.
- [ ] **443** - TODO(pskelton): Modify game data face attribs to ethereal eventually - hack so that secret tunnel under prison bed can be accessed
  - OpenEnroth cannot edit the user's MM7 data, so this needs a load-time face-attribute fixup mechanism (none exists today) setting FACE_ETHEREAL on the listed face ids at map load, plus verification that etherealizing them does not change vis/picking or other Ethereal() consumers.

### `src/Engine/Graphics/Image.cpp`

- [ ] **95** - TODO(captainurist): _initialized == false happens, investigate
  - ImageLoader::Load failures are silently swallowed here with no logging, so finding which resources fail needs a run against real game data with logging added, and then a decision between a hard failure and a placeholder texture.

### `src/Engine/Graphics/Indoor.cpp`

- [ ] **443** - TODO(captainurist): code looks very similar to Camera3D::GetFacetOrientation
  - Camera3D::GetFacetOrientation (Camera.cpp:62) dispatches on the normal while _get_normals dispatches on polygonType, and their U/V outputs are swapped and differ in sign (outV.z = -1 vs +1), plus _get_normals applies FACE_FlipNormalU/V; unifying them (also with the third copy at PortalFunctions.cpp:169) needs sign reconciliation and visual verification.
- [ ] **1250** - TODO(pskelton): Need to add check against terrain
  - Outdoor line-of-sight currently only tests bmodel faces; adding a ray march against pOutdoor->pTerrain's heightmap is new code that silently changes AI aggro and spell targeting on every outdoor map, so it needs its own tests and trace re-verification.
- [ ] **1699** - TODO(Nik-RE-dev): use calculated velocity of party and walk/run flags instead of delta
  - Walk/run sound selection currently derives speed from the post-collision position delta (`integer_sqrt((oldPos - pParty->pos).lengthSqr())`) with hand-tuned thresholds 2/4; switching to pParty->velocity requires re-deriving all thresholds and mirroring the identical change in ODM_ProcessPartyActions (Outdoor.cpp:1467), and correctness is only judgeable by ear.
- [ ] **1759** - TODO(pskelton): investigate why this happens
  - faceId can come back -1 because ProcessPartyCollisionsBLV re-runs GetIndoorFloorZ (Collisions.cpp:913), which sets *pFaceID = -1 when pIndoor->GetSector() returns 0 for the adjusted position; fixing it means deciding what the party stands on when it lands outside any sector, and note the lava check is guarded while the footstep-sound reads `pIndoor->faces[faceId]` at lines 1716/1727 are not, so there is a latent OOB read to fix too.

### `src/Engine/Graphics/Outdoor.cpp`

- [ ] **190** - TODO(pskelton): consider order of drawing / lighting
  - The mobile/stationary light stacks are reset and the torch light re-stacked *after* DrawOutdoorSky/Terrain/Buildings, so terrain and buildings are lit from the previous frame's stack, whereas PrepareDrawLists_BLV (Indoor.cpp:146-148) resets first; swapping the order is a few lines but changes outdoor lighting every frame and needs pixel-level verification of terrain, buildings and billboards.
- [ ] **928** - TODO(pskelton): Magic numbers
  - ODM_ProcessPartyActions spans lines 933-1515 and is dense with unexplained physics constants (32, 512, gravity multipliers -2/8, the 20*overlayId+119 index, speed divisors); naming them individually requires understanding each against the original 0x473893 code and re-testing party movement.
- [ ] **1467** - TODO(Nik-RE-dev): use calculated velocity of party and walk/run flags instead of delta
  - Outdoor twin of Indoor.cpp:1699 — same position-delta heuristic with 2/4 thresholds feeding walkSoundForTileset; converting to velocity must be done in both files at once and re-tuned, since the thresholds already deviate from the original 8/16.
- [ ] **1676** - TODO(pskelton): common gravity code extract
  - There are 9 GetGravityStrength() call sites (Indoor.cpp:878/1654, Outdoor.cpp:1284/1294/1676/1685, SpriteObject.cpp:163/167/354/487) with inconsistent multipliers (-16, -8, -2, 1, 8) and differing float/int casts, so a shared helper first requires deciding whether those multipliers are intentional physics or porting artefacts.

### `src/Engine/Graphics/ParticleEngine.h`

- [ ] **28** - TODO(pskelton): eliminate this one
  - Particle_sw is the input-only prefix of Particle with differently named fields (shiftX vs shift_x, uDiffuse vs uParticleColor); collapsing it means touching its 21 use sites across SpellFxRenderer.cpp, SpriteObject.cpp, Indoor.cpp and BaseRenderer.cpp, including the memset-then-fill idiom.

### `src/Engine/Graphics/PortalFunctions.cpp`

- [ ] **169** - TODO(captainurist): code looks very similar to Camera3D::GetFacetOrientation
  - Deduplicating is not a straight swap: this switch classifies by pFace->polygonType and also produces the var_28/var_24 selector pair, whereas Camera3D::GetFacetOrientation classifies by normal magnitudes and has an extra 'other' branch - equivalence must be proven for all faces, and the same TODO also sits at Indoor.cpp:443.

### `src/Engine/Graphics/Renderer/BaseRenderer.cpp`

- [ ] **469** - TODO(pskelton): could draw in 3d rather than convert to billboard for ogl
  - SpellFX_Billboard vertices arrive already screen-projected from SpellFxRenderer::SpellFXProject, so a 3D path means changing what SpellFxRenderer produces plus a new GL submission path and re-checking transparency sort order against the rest of the billboard list.

### `src/Engine/Graphics/Renderer/NullRenderer.cpp`

- [ ] **34** - TODO(captainurist): doesn't belong here.
  - Both backends duplicate this reset (OpenGLRenderer.cpp:297) and Renderer.h:173 notes the counter dangles when BeginScene3D isn't called, so fixing it means introducing a shared frame-begin hook while respecting the constraint noted in the GL comment that the list must stay alive until mouse picking is done.

### `src/Engine/Graphics/Renderer/OpenGLRenderer.cpp`

- [ ] **65** - TODO(pskelton): Combine and contain
  - pBillboardRenderList/uNumBillboardsToDraw/uNumDecorationsDrawnThisFrame/uNumSpritesDrawnThisFrame are mutable globals defined in the GL backend but read/written from 8-9 files (Indoor, Outdoor, Engine, SpellFx, GUI), so containing them in a render-state object is a wide mechanical refactor with lifetime/reset ordering to get right.
- [ ] **221** - TODO(Gerark) Should we bind to the framebuffer before clearing?
  - With render scaling (outputRender != outputPresent) ClearTarget clears the default framebuffer while all subsequent 2D drawing goes to the offscreen FBO bound in flushAndScale, so answering it means auditing the two call sites (Game.cpp:115 frame start, MediaPlayer.cpp:848) under a non-1 render scale and checking for stale-content artifacts.
- [ ] **404** - TODO(pskelton): do these need batching?
  - DrawForcePerVerts is forced per projectile because DrawProjectile wraps it in its own GL state (additive blend, depth mask off, cull off); batching requires carrying that state into the vertex records or splitting force-per-vertex buckets by state.
- [ ] **562** - TODO(pskelton): sort this - forcing the draw is slow
  - BlendTextures rebuilds an RgbaImage pixel-by-pixel on the CPU, creates a throwaway GraphicsImage and calls DrawTwodVerts() every frame per enchanted item; removing the forced flush means caching/animating the blend texture and letting it ride the normal 2D batch.
- [ ] **634** - TODO(pskelton): renderbase
  - Hoisting DrawIndoorSky into BaseRenderer means splitting the camera/sky math from the GL-only DrawIndoorSkyPolygon submission behind a virtual, and the math would then also run under NullRenderer (used by the headless game tests), so it needs care plus test verification.
- [ ] **646** - TODO(pskelton): repeated maths could be saved when calculating sky planes
  - DrawIndoorSky runs once per sky face (called from the BSP loop at line 2963) and recomputes ~10 camera-only trig constants (rot_to_rads, horizon offset, far-clip projections, pitch rotation vector) that DrawOutdoorSky also duplicates; hoisting them into a per-frame struct filled in BeginScene3D touches both sky paths and is a perf-only change that needs pixel verification.
- [ ] **993** - TODO(pskelton): move this to map loading
  - The ~130-line terrain VBO/texture-array build is lazily triggered by the first DrawOutdoorTerrain (guarded by !_terrainBuffer) and is also re-run after ReleaseTerrain on season change; moving it to map load needs a renderer hook called at load time with a live GL context plus keeping the season-change rebuild path working.
- [ ] **1173** - TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
  - glPolygonMode has no GLES equivalent, so debug terrain wireframe needs a shader-side implementation (barycentric edge factor in the terrain shaders) or line-index drawing, plus a GLES device/emulator to validate - not a local edit.
- [ ] **1247** - TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
  - The cap of 20 is baked into std::array<PointLightUniform,20> in OpenGLShaderParams.h and into the GLSL uniform array size, so 'configurable' means shader recompilation with a define; the sorting half additionally needs both light stacks merged and distance-sorted per frame.
- [ ] **1265** - TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
  - Same TODO for the mobile-light loop of the terrain pass; fixing it properly means one shared distance-sorted light selection feeding the fixed-size shader array, so it cannot be done at this one call site.
- [ ] **1294** - TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
  - The closing half of the terrain wireframe debug block; it disappears only together with a GLES-capable wireframe implementation in the shaders, same work item as line 1173.
- [ ] **1299** - TODO(pskelton): clean up and move to seperate function in decal builder
  - The ~80-line bloodsplat-to-terrain stacking loop reads the renderer-private _terrainVertices array to rebuild per-tile triangles and bboxes, so moving it into DecalBuilder first requires exposing terrain triangle geometry (from OutdoorTerrain rather than the GL vertex buffer).
- [ ] **1338** - TODO(pskelton): terrain and boxes should be saved for easier retrieval
  - Wants per-tile triangle vertices and BBoxf cached instead of reconstructed from _terrainVertices and GetPolygonMinZ/MaxZ per bloodsplat; the cache belongs in OutdoorTerrain, must be built at load and invalidated on season change, and is coupled to the decal-builder move above.
- [ ] **1388** - TODO(pskelton): renderbase
  - DrawOutdoorSky's math is API-agnostic and could move to BaseRenderer, but only after the GL-specific tail (DrawOutdoorSkyPolygon feeding _forcePerVertices) is put behind a virtual; same treatment is needed for DrawIndoorSky at line 634.
- [ ] **2288** - TODO(pskelton): Same as indoors. When ODM and BLV face is combined - seperate out function
  - The precondition now holds (ODMFace was dropped in aadb5eaea6c/#2513 and outdoor models iterate BLVFace), so the remaining work is extracting the ~90-line texture unit/layer atlas-registration loop shared with the indoor bsptexmap version at lines 2820-2880, which requires parameterising over the two texmap/size arrays.
- [ ] **2500** - TODO(pskelton): set to water for now - fountains in walls of mist
  - This outdoor-building fallback fires whenever an animated face's current frame is missing from the atlas map - note the lookup one line above uses bsptexmap (the indoor map, filled only by DrawIndoorFaces) instead of outbuildtexmap, which looks like a copy-paste bug; fixing means auditing frame registration on both paths and verifying in-game rather than editing this branch.
- [ ] **2568** - TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
  - Third copy of the glPolygonMode debug guard, this time around the outdoor buildings pass; needs the same shader-based wireframe implementation and GLES testing as line 1173.
- [ ] **2623** - TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
  - Outbuild-pass copy of the 20-light cap; configurability is blocked on the fixed-size uniform arrays in OpenGLShaderParams.h and the GLSL sources, so it must be solved once for terrain/outbuild/BSP together.
- [ ] **2641** - TODO(pskelton): make this configurable - also lights should be sorted by distance so nearest are used first
  - Mobile-light half of the outbuild copy; same shared fix (distance-sorted light selection plus a shader-side configurable array size) rather than a local edit.
- [ ] **2683** - TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
  - Closing guard of the outbuild wireframe debug block; removable only once wireframe is implemented in a GLES-compatible way (barycentric edges in the shaders or line drawing).
- [ ] **2687** - TODO(pskelton): clean up / need to stack decals
  - The bloodsplat pass is bolted onto the tail of DrawOutdoorBuildings (with a dead `return;` and "shader end" marker after it) and duplicates the indoor version at line 3252; pulling it out into its own decal pass means deciding where it belongs in the render pipeline and re-verifying decal rendering.
- [ ] **2810** - TODO(pskelton): Same as outdoors. When ODM and BLV face is combined - seperate out function
  - The precondition is already met (ODMFace was dropped, outdoor models use BLVFace), so this is now extracting the ~80-line texture-array registration block shared by DrawIndoorFaces (2810-2889) and DrawOutdoorBuildings (2320-2392), which differ only in which member arrays/maps they touch — mechanical but rendering-visible, so it needs screenshot verification.
- [ ] **2879** - TODO(pskelton): any instances where animTime is not consistent would need checking
  - The loop advances with `animationFrameLength(face->animationId)` — always the *first* frame's length — so animations with unequal per-frame lengths get frames skipped in the atlas (and later fall back to the water texture); a robust fix means adding a frame-enumeration API to TextureFrameTable and using it at both the indoor and outdoor registration sites.
- [ ] **3016** - TODO(pskelton): set to water for now - fountains in walls of mist
  - BLVFace::SetTexture (Indoor.cpp:224) invalidates texlayer at runtime, and if the new texture was never registered the face renders as water; fixing it means inserting textures into an already-allocated GL_TEXTURE_2D_ARRAY (allocated with an exact layer count via glTexImage3D) or rebuilding the BSP atlas on demand.
- [ ] **3068** - TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
  - Making the debug wireframe work on GLES needs an alternative implementation (barycentric-coordinate attribute plus edge factor in glbspshader/glterrain/gloutbuild, or a separate GL_LINES pass), touching several shader files and all four `glPolygonMode` sites — not a local edit.
- [ ] **3189** - TODO(pskelton): nearest lights should be prioritised
  - The 40-light uniform budget is filled first-come across three separate loops (torchlight, stationary, mobile), so prioritising by distance requires collecting all candidates into one scored list before filling `uniforms.pointLights`, and the change is directly visible in indoor lighting so it needs screenshot checks.
- [ ] **3248** - TODO: OpenGL ES doesn't provide wireframe functionality so enable it only for classic OpenGL for now
  - Duplicate of the same limitation at line 3068 (indoor pass end); fixing it is the same shader-level wireframe work across the terrain/bsp/outbuild shaders.

### `src/Engine/Graphics/Renderer/Renderer.h`

- [ ] **106** - TODO(captainurist): DrawImage uses palette, should be refactored to use a separate palette shader.
  - DrawImage duplicates DrawQuad2D but tags every vertex with a paletteid, so gltwodshader.frag does a paltex2D texelFetch/branch for all 2D geometry; splitting it means a new shader plus vertex type, batching by palette in DrawTwodVerts, and updating the palette call sites (monster portraits, BlendTextures, MediaPlayer).
- [ ] **173** - TODO(captainurist): this is not properly cleared if BeginScene3D is not called, resulting in dangling textures in pBillboardRenderListD3D.
  - Reset currently happens only in OpenGLRenderer::BeginScene3D (line 297, deliberately late because Vis mouse picking reads the list afterwards) and as a patch in Engine::ResetCursor_Palettes_LODs_Level_Audio_SFT_Windows; a real fix needs a defined frame-lifetime point for the billboard list that still satisfies the three Vis picking loops.

### `src/Engine/Graphics/SpriteEnums.h`

- [ ] **17** - TODO(captainurist): Should be split in two, see FrameFlags. Also, I'm not sure the value naming is correct, some of the flags aren't even used.
  - SPRITE_FRAME_HAS_MORE/FIRST duplicate Engine/Data/FrameEnums.h's FrameFlag; splitting means SpriteFrame carries two flag fields, and updating EntitySnapshots.cpp:269 deserialization, Sprites.cpp iteration and SpriteEnumFunctions — plus the naming half needs data research.

### `src/Engine/Graphics/Sprites.h`

- [ ] **27** - TODO(captainurist) : move to Engine/Data and Engine/Tables
  - Following the existing TextureFrameData/TextureFrameTable split means creating Engine/Data/SpriteFrameData.h plus Engine/Tables/SpriteFrameTable.{h,cpp}, separating the runtime `Sprite*` cache from the deserialized data, and fixing 19 including files and two CMakeLists.

### `src/Engine/Graphics/Vis.cpp`

- [ ] **774** - TODO(captainurist): Flags.
  - Turning the `1 << aiState` mask into a proper flags enum is blocked by Vis_SelectionFilter reusing the same `at_ai_state`/`no_at_ai_state` ints as a FaceAttributes mask for face filters (vis_door_filter passes FACE_HAS_HINT, and isFacePartOfSelection casts them to FaceAttributes), so the struct has to be split per object type first.

### `src/Engine/Localization.h`

- [ ] **30** - TODO(captainurist): there was also a call to sprintfex_internal after a call to vsprintf.
  - sprintfex_internal still exists in mm7text_ru.cpp:659 but is dead code called from nowhere; re-enabling the Russian ^I/^L/^R/^P declension post-processing means porting that legacy char*/strcpy/assert/goto implementation (with its 8KB stack buffer and gender tables) to std::string and deciding where in format()/str() it hooks in, plus tests.

### `src/Engine/MapInfo.h`

- [ ] **49** - TODO(captainurist): EAX audio reverb preset (0-26); set per-map - wire up to the audio backend.
  - uEAXEnv is parsed at MapInfo.cpp:90 and read nowhere; OpenALSoundProvider has no EFX code at all, so this needs ALC_EXT_EFX detection, an aux effect slot with a reverb effect, a table of the 27 EAX presets, routing every sound source through the slot, and switching the preset on map load.

### `src/Engine/Objects/Actor.cpp`

- [ ] **830** - TODO(pskelton): Should this be moved somewhere else - not actor related
  - The Vec3f/Vec3f overload is pure geometry, but AIDirection lives in Actor.h and is referenced from ~30 places in Actor.cpp, TurnEngine.cpp and EvtInterpreter.cpp, so moving it means creating a new header/TU for AIDirection plus include churn and a decision on which module owns it.
- [ ] **2066** - TODO(pskelton): change to PID_INVALID and sort logic in calling funcs
  - There is no PID_INVALID today - Pid() is OBJECT_None/id 0 and doubles as the falsy no-target sentinel; adding a real invalid value means extending the Pid API and auditing the ~50 references to ai_near_actors_targets_pid across Actor.cpp and TurnEngine.cpp that compare or test it.
- [ ] **2410** - TODO(captainurist): drop the cast here, store the data properly.
  - field_3C_some_special_attack is an int16_t that holds a MonsterId for summon abilities and a DamageType otherwise (see the matching 'Split.' TODO at Monsters.h:75); splitting it touches MonsterInfo layout, the parser in Monsters.cpp:430-441, the save (de)serializers in EntitySnapshots.cpp:1370/1464 and the reads in Character.cpp:5851/6021.
- [ ] **4335** - TODO(pskelton): We calculate a new position for the monster, but we never use it.
  - In SpawnEncounter, newPos (spawn point + random offset at 64/128 units) is computed and only used for the indoor sector/floor sanity check; pMonster->pos was already set to spawn->position at line 4327 and never updated, so all monsters of an encounter stack on one point. The one-line fix (pMonster->pos = newPos) changes every encounter spawn layout, interacts with the removal logic below and issue #2074, and needs game-test verification.
- [ ] **4446** - using absolute Z here is BS, it's used as speed in ItemDamageFromActor
  - attVF is built as Vec3f(distanceVec.x, distanceVec.y, pActors[actorID].pos.z) - a world Z of several thousand mixed with relative X/Y, then normalized and used as the knockback direction in ItemDamageFromActor (velocity = 50 * knockback * pVelocity) and in DamageMonsterFromParty/ActorDamageFromMonster. Correct value is almost certainly distanceVec.z, but this is faithful-to-vanilla nonsense that changes AoE knockback for every monster, so it needs tape/game-test verification.

### `src/Engine/Objects/Actor.h`

- [ ] **267** - TODO(captainurist): wrap pActors and this variable into an ActorList class.
  - pActors appears ~660 times across 33 files, but almost all uses are operator[]/size()/range-for; only 4 sites mutate it (Engine.cpp:832/868 clear, Actor.cpp:4540 emplace_back, GameTests_2000.cpp:556). The friction is that snapshot(pActors,...)/reconstruct(...,&pActors) in CompositeSnapshots.cpp serialize the raw std::deque, so the wrapper needs to expose the container or get its own serializer, plus AllocateActor and the three nextActorReuseScanStart resets move into the class.

### `src/Engine/Objects/ActorEnums.h`

- [ ] **55** - TODO(captainurist): #enum need to do some renamings here
  - AIState members are bare CamelCase (Standing, Removed, Dead, AttackingRanged1..4) exported globally via `using enum`, unlike every neighbouring enum which uses a prefix (ACTOR_BUFF_*, ANIM_*). Renaming to a prefixed convention is mechanical but touches ~47 AIState references plus every bare-name use across Actor.cpp, Outdoor.cpp, Indoor.cpp, Collisions.cpp, Engine.cpp, Party.cpp and the renderers, and some names (Tethered, Interacting, AttackingRanged1/2/3/4) still need better meanings picked.

### `src/Engine/Objects/Character.cpp`

- [ ] **1654** - TODO(captainurist): I don't like this logic. We first take the weapon with larger recovery time, then apply recovery bonuses. Should be the other way around.
  - GetAttackRecoveryTime picks the dual-wield weapon by raw base recovery, and that choice then also drives the sword/axe/bow expert reduction and the Swift/Darkness/Puck enchantment reduction. Computing final recovery per weapon and taking the max means restructuring the whole function and changes recovery numbers for dual-wielders, which will shift RNG/timing in existing game-test tapes.
- [ ] **2155** - TODO(_) would be nice to move these into separate functions
  - The switch is the body of Character::GetItemsBonus (lines 2067-2317), a ~250-line function whose cases share locals (v5, v25, v26, v56, getOnlyMainHandDmg) and a preceding skill-availability guard; splitting it is mostly mechanical but needs care to preserve the shared prologue and the fallthrough/early-return structure.
- [ ] **3336** - TODO(Nik-RE-dev): spell scroll is removed before actual casting and will be consumed even if casting is canceled.
  - Both branches call pParty->takeHoldingItem() before pushScrollSpell/UIMSG_SpellScrollUse; deferring consumption means carrying the scroll item through CastSpellInfo (which already has ON_CAST_CastViaScroll) and giving it back on every cancel path in CastSpellInfo.cpp and the targeted-spell UIs.
- [ ] **6392** - TODO(captainurist): target_pid is ignored here - the arrow re-resolves its target in castSpell() with a different fallback, so it can fly at a different actor than the one checked above.
  - Fixing it means plumbing an explicit target Pid through pushSpellOrRangedAttack/CastSpellInfo (castSpell re-picks via mouse pointing then FindClosestActor with different args at CastSpellInfo.cpp:154-172, while the caller used FindClosestActor(5120, 0, 0)), which changes combat targeting and needs game-test/trace validation.

### `src/Engine/Objects/Character.h`

- [ ] **268** - TODO(Nik-RE-dev): use getCharacterIdInParty directly where this function is called.
  - Mechanical but broad: ~110 call sites of getCharacterIndex() across Engine/GUI would have to become pParty->getCharacterIdInParty(&character), and it is debatable whether removing the member accessor is even an improvement.

### `src/Engine/Objects/Chest.cpp`

- [ ] **209** - TODO(captainurist): this is a weird place to call postGenerate, and we won't call it for items that end up buried. But it's here b/c we don't want to break the traces, at least now. Redo this properly and write a small test.
  - postGenerate consumes grng (potion power, wand charges), so moving it from PlaceItems into GenerateItemsInChest shifts the RNG stream and invalidates the recorded game-test traces, which have to be regenerated/validated alongside the new test the comment asks for.

### `src/Engine/Objects/DecorationList.h`

- [ ] **50** - TODO(captainurist): IndexedArray.
  - MonsterList (TableSerialization.cpp:48-61) shows the recipe, but DecorationId has no FIRST/LAST bounds yet and CodeGen.cpp:444 regenerates that very enum by iterating this vector, so a fixed-size IndexedArray would hard-fail on data whose decoration count differs from the current enum.

### `src/Engine/Objects/Item.cpp`

- [ ] **751** - TODO(captainurist): change to 2d4+2.
  - grng->randomDice(2, 4) draws two numbers where `2 * grng->random(4)` draws one, so the change shifts the whole RNG stream (breaking every recorded trace) and also changes the value range from {2,4,6,8} to 4..10 for monster-dropped potions.

### `src/Engine/Objects/Item.h`

- [ ] **139** - TODO(captainurist): introduce ATTRIBUTE_NULL?
  - Replacing std::optional<Attribute> standardEnchantment with a sentinel touches ~40 uses including save (de)serialization in EntitySnapshots.cpp:451-487 and allEnchantableAttributes() validation, and ATTRIBUTE_NULL must be placed outside the ATTRIBUTE_FIRST_STAT..ATTRIBUTE_LAST_STAT ranges that IndexedArray keys on.

### `src/Engine/Objects/ItemEnums.h`

- [ ] **995** - TODO(captainurist): ITEM_POTION_BOTTLE equip type is ITEM_TYPE_POTION, but we don't have an empty bottle in the range below. Not good.
  - ITEM_POTION_BOTTLE (220) sits just below ITEM_FIRST_POTION (221), so free isPotion(ItemId) excludes it while type-based Item::isPotion() includes it; aligning the two changes save (de)serialization, which keys the potionPower/enchantment union off isPotion (EntitySnapshots.cpp:451/474), plus the several `itemId != ITEM_POTION_BOTTLE` special cases.

### `src/Engine/Objects/MonsterEnumFunctions.cpp`

- [ ] **305** - TODO(captainurist): This needs some reworking it seems. Water elemental supertype is about water walking, treant supertype is about being a tree that can't move. The rest are about "of X slaying".
  - MonsterSupertype is a single-valued field that conflates item 'slaying' categories with per-monster behaviour flags; splitting it into a slaying category plus predicates means touching ~20 call sites (Outdoor.cpp:1603 water walking, 8 treant checks in Actor.cpp, the undead/kreegan/dragon/elf/titan checks in Character.cpp/CastSpellInfo.cpp).

### `src/Engine/Objects/Monsters.cpp`

- [ ] **105** - TODO(captainurist): "Ener" should map to DAMAGE_ENERGY but the original parser only inspected the first letter, so it collided with "Earth". Preserving the buggy mapping so existing trace tests still pass; fix-and-retrace separately.
  - Flipping the map entry is one line, but DAMAGE_ENERGY is currently only used for display strings - the resistance switches (Actor::_43B3E0_CalcDamage, Actor::DoesDmgTypeDoDamage, Character::receiveDamage) would silently take their default branch for 11 monsters (droids, Archmage, Chaos Hydra, Mega-Dragon), so it needs a decision on energy resistance plus a full retrace.
- [ ] **252** - TODO(captainurist): move to MonsterTable?
  - Other .txt loaders live in Engine/Tables/*Table.cpp, but MonsterInfo must stay next to Actor (Actor.h embeds it), so the move means splitting the data struct from MonsterStats/MonsterList + the loader, creating Engine/Tables/MonsterTable.{h,cpp}, and updating CMakeLists plus the 4 includers.
- [ ] **431** - TODO(captainurist): do a honest parse of air/ground.
  - The air/ground flag is smuggled into specialAbilityDamageDiceSides, which is read back in Outdoor.cpp:743 (`!= 1`) and round-tripped through the MonsterInfo_MM7 savegame snapshot, so an honest parse means adding a dedicated enum field and keeping the snapshot serialization writing the old byte.

### `src/Engine/Objects/Monsters.h`

- [ ] **75** - TODO(captainurist): MonsterId for MONSTER_SPECIAL_ABILITY_EXPLODE, DamageType for MONSTER_SPECIAL_ABILITY_EXPLODE. Split.
  - (Comment itself has a typo - the first case is SUMMON.) Splitting into summonedMonsterId + explosionDamageType touches the two writers in Monsters.cpp:430/441, the four readers (Actor.cpp:2339/2411, Character.cpp:5851/6021) and both directions of the MonsterInfo_MM7 snapshot, which must keep packing them back into one int16_t for savegame compatibility.

### `src/Engine/Objects/SpriteObject.cpp`

- [ ] **51** - TODO(pskelton): refactor this so check isnt needed
  - Create() copies *this into the global pSpriteObjects, so the assert loop guards against being called on an element of that vector; removing the need means turning the ~30 `SpriteObject tmp; ...; tmp.Create(...)` call sites in CastSpellInfo.cpp/Actor.cpp/SpriteObject.cpp/Party.cpp/Engine.cpp into a free spawn function taking a prototype.
- [ ] **214** - TODO(pskelton): move to collisions
  - The 60-line outdoor sweep loop here is the sprite twin of ProcessActorCollisionsODM/ProcessPartyCollisionsODM in Collisions.cpp, but it is interleaved with sprite-specific logic (processSpellImpact, water splash, trail particles), so extraction is not a straight cut-and-paste and any drift shows up as trace desyncs.
- [ ] **355** - TODO(Nik-RE-dev): get rid of goto here
  - LABEL_25 is entered from two places - the forward jump at line 349 for OBJECT_DESC_NO_GRAVITY sprites and the backward jump at line 500 that turns the whole block into an implicit outer loop; untangling it means extracting the collision block into a function and rebuilding that outer loop while keeping iteration counts (and hence RNG/trace state) identical.
- [ ] **356** - TODO(pskelton): move to Collisions
  - Indoor counterpart of the line 214 TODO - the loop mixes collision stepping with sprite bounce/impact/event-trigger logic, so moving it into Collisions.cpp next to ProcessActorCollisionsBLV requires factoring the sprite-specific parts out first.

### `src/Engine/OurMath.h`

- [ ] **8** - TODO(captainurist): drop this header
  - The header is included by ~25 translation units and still exports live symbols - `pi`/`pi_double` (~34 uses), int_get_vector_length (~15), integer_sqrt (~8), bankersRounding (~5) - so dropping it means relocating each to a proper Library/Utility math header, deleting the dead round_to_int and the large commented-out `fixed` block, and fixing every include.

### `src/Engine/Party.h`

- [ ] **302** - #time replace with GetCurrentCivilTime().
  - The seven uCurrentYear/Month/.../Second fields are redundant state derived from playing_time but are read in ~30 places (Character.cpp, EvtInterpreter, Transport/Temple UI, Decoration, CastSpellInfo) and are written into the vanilla-format Party_MM7 snapshot, so removing them means adding a Party::GetCurrentCivilTime() accessor, converting every reader, and recomputing the fields in snapshot().
- [ ] **386** - make 0-based with -1 for none??
  - _activeCharacter is private behind activeCharacterIndex()/setActiveCharacterIndex()/hasActiveCharacter(), but the 1-based convention leaks into ~109 call sites (UI code, turn engine `pQueue[0].uPackedID.id() + 1`, Pid character ids, savegame snapshot), so the switch is a wide, off-by-one-prone sweep.

### `src/Engine/Pid.h`

- [ ] **48** - Should be <= 3, but 4-5 are used for hirelings in UIMSG_OnCastTownPortal.
  - Pid::character is being abused as a caster id where ids 4-5 mean hirelings (NPC.cpp:188 packs `pCharacters.size() + id`, TownPortalBook.cpp:177-202 unpacks it); tightening the assert requires introducing a proper caster-identity type and threading it through NPC.cpp, CastSpellInfo.cpp, Game.cpp, TownPortalBook and LloydsBook.

### `src/Engine/SaveLoad.cpp`

- [ ] **52** - remained from Party::Reset, doesn't really belong here (or in Party::Reset).
  - The duplicated UI/turn-engine reset (current_character_screen_window, pTurnEngine->End) exists in both loadGame() and Party::Reset() (Party.cpp:540); consolidating it needs a new "reset transient game/UI state" hook and agreement on where it lives (GUI vs Engine), plus care that new-game and load-game paths keep identical ordering.

### `src/Engine/Snapshots/CompositeSnapshots.cpp`

- [ ] **109** - just use Newell's method for everything & retrace.
  - Replacing the first-non-degenerate-edge normal with pure Newell is a small edit in repairFaceNormal, but it perturbs face planes for every map, so it needs a full `OpenEnroth retrace --check-canonical` pass over test/Data plus a visual/collision sanity check.

### `src/Engine/Snapshots/EntitySnapshots.cpp`

- [ ] **369** - can drop?
  - The "Dummy" marker is load-bearing: NPCStats::setNPCNamesOnLoad (NPCTable.cpp:63-66) uses `!pHirelings[i].name.empty()` to detect an occupied hireling slot, so dropping it requires switching that check to another occupancy signal (profession/portraitId) and verifying against existing saves.
- [ ] **481** - Do this properly for every single enum in this file.
  - There are ~60 raw static_cast<Enum> reconstructions in this 1900-line file; validating each needs a generic "is this value legal" helper plus a per-enum valid set and a per-enum fallback for corrupt/vanilla-out-of-range saves - straightforward but broad and easy to regress save loading.

### `src/Engine/Spells/CastSpellInfo.cpp`

- [ ] **117** - TODO(pskelton): caster index not supplied to buffs ".Apply"
  - Roughly 45 SpellBuff::Apply calls in this file pass caster=0 while party buffs pass casterCharacterIndex+1, and consumers index `pCharacters[caster - 1]` (Engine.cpp:1141, Outdoor.cpp:940/1024), so filling in the caster must be done per-buff with knowledge of which buffs actually read it, and the field is save-serialized.
- [ ] **2800** - TODO(captainurist): reimplement this in a saner way.
  - Dark Sacrifice resolves its victim by rebuilding a FlatHirelings list and indexing it with pParty->hirelingScrollPosition + targetCharacterIndex, so a sane version means storing the resolved hireling identity in CastSpellInfo, touching TargetedSpellUI_Hirelings, spellTargetPicked and the FlatHirelings API.
- [ ] **3215** - TODO(Nik-RE-dev): need to get rid of uPlayerID_2 and use pid with OBJECT_Character and something else for hirelings.
  - uPlayerID_2 is now CastSpellInfo::targetCharacterIndex, which doubles as a party-member index and (for Dark Sacrifice) a hireling index; replacing it with a Pid plus a hireling addressing scheme touches all TargetedSpellUI subclasses, pushCastSpellInfo and every targetCharacterIndex use in castSpell.

### `src/Engine/Spells/CastSpellInfo.h`

- [ ] **67** - TODO(captainurist): also pParty->hirelingScrollPosition-based hireling index for dark sacrifice.
  - Documents that targetCharacterIndex is overloaded with a scroll-relative hireling index; splitting it into two properly typed fields is the same refactor as CastSpellInfo.cpp:2800/3215 and touches the targeted-spell UI classes plus the Dark Sacrifice handler.
- [ ] **74** - TODO(captainurist): doesn't look like sound id. Maybe flags? Bits 0-2 for caster (1-based), bit 3 for blaster.
  - The hypothesis is confirmed by Character.cpp:6369/6374 passing activeCharacterIndex()+8 and SpriteObject.cpp:606 testing `item->uSoundID & 8`; renaming/splitting the field means renaming SpriteObject::uSoundID too, which is mirrored in the save-game snapshot structs.

### `src/Engine/Spells/SpellBuff.h`

- [ ] **11** - TODO(pskelton): check for inconsistent use of caster
  - 86 Apply call sites, most passing caster=0, while readers do `pCharacters[caster - 1]` (Engine.cpp:1141, Outdoor.cpp:940/1024, Engine.cpp:1299) - auditing which buffs must carry a real caster and fixing the 0/-1 indexing needs per-buff reasoning plus save-compat thought.

### `src/Engine/Spells/SpellEnumFunctions.h`

- [ ] **58** - TODO(captainurist): I think we can drop most usages of this function.
  - The four callers (UISpellbook.cpp:115/178/219, EngineController.cpp:367) use the index into per-school 12-element arrays (pIconPos, pSpellbookSpellIndices, SBPageSSpellsTextureList) and into the "SpellBook_Spell{index}" button names, so dropping it means re-keying those tables by SpellId and changing button ids that the game-test controller depends on.

### `src/Engine/Spells/Spells.cpp`

- [ ] **332** - TODO: use SoundID not uint16_t
  - SoundId deliberately names only sounds used in code, while this table holds ~99 base ids and AudioPlayer.cpp:479 does `SpellSoundIds[spell] + is_impact`, so typing it means either ~200 new enum entries or keeping casts plus reworking the +1 impact arithmetic.
- [ ] **333** - TODO(captainurist): Originally the array was two elements shorter, last two zeros are my addition. Can we drop elements for non-regular spells?
  - The answer is "not without a guard": arrow and blaster sprites reach playSpellSound with SPELL_BOW_ARROW/SPELL_LASER_PROJECTILE (SpriteObject.cpp:744/797), so shrinking the IndexedArray range requires playSpellSound to bail out for non-regular spells and a check that no audible sound is lost.

### `src/Engine/Tables/ItemTable.cpp`

- [ ] **175** - TODO(captainurist): #unicode this is not ascii
  - Both operands are localized strings (items.txt vs stditems.txt), and the repo has no Unicode case-folding utility at all (Utility/String/Unicode.h only has isSpace), so this needs a new casefold/noCaseEquals primitive plus tests before the one-line call swap; it is one of 10 sites tagged #unicode.
- [ ] **182** - TODO(captainurist): #unicode this is not ascii
  - Identical to the standard-enchantment comparison two lines above: blocked on adding a Unicode-aware case-insensitive compare to Utility/String, since ascii::noCaseEquals folds only ASCII and the special-enchantment suffixes are translated.

### `src/Engine/Tables/NPCTable.h`

- [ ] **20** - TODO(Nik-RE-dev): It seems that two greet flags are used purely because it's modification is performed before greeting string is constructed. It is also ensures that NPC in multi-NPC houses always greet you with first line until you leave the house. Ideally there should be only one flag.
  - Collapsing NPC_GREETED_FIRST/SECOND means restructuring the greeting-selection order in UIDialogue.cpp:51-56/231/329 and UIHouses.cpp:412-417/738 plus EvtInterpreter.cpp:507, and the flags word is written verbatim to save files (EntitySnapshots.cpp:349/371), so save compatibility needs a translation step.

### `src/Engine/Time/Time.h`

- [ ] **76** - TODO(captainurist): #time doesn't belong to GameTime.
  - The -1 sentinel is only consumed through SpellBuff (Spells.cpp:453 SetExpired, SpellBuff::Expired and ~10 call sites), so moving the expired state into SpellBuff is doable, but that sentinel is what gets serialized as the buff expire time in EntitySnapshots.cpp:430/439, so save round-tripping must be preserved.

### `src/Engine/Time/Timer.h`

- [ ] **21** - TODO(captainurist): isTurnBased is just a synonym for 'really-totally-paused'. Doesn't belong here, move out.
  - _paused and _turnBased have identical effect in tick(), so merging them needs a pause-reason/counter mechanism (callers set them independently from Game.cpp, SaveLoad.cpp and TurnEngine.cpp), and both bits are written to the save via Timer_MM7 (EntitySnapshots.cpp:322-342).

### `src/Engine/mm7_data.h`

- [ ] **97** - TODO(captainurist): #enum
  - Converting the 14 GAME_SETTINGS_* macros into an `enum class` + MM_DECLARE_FLAGS and retyping dword_6BE364_game_settings_1 is mechanical but spans 16 files (~54 macro uses, 39 variable uses), and several bits (0800, 1000, 4000, NO_INTRO...) are near-dead and need naming decisions.

### `src/GUI/GUIWindow.cpp`

- [ ] **203** - TODO(pskelton): this function may modify frameRect - extract out
  - DrawMessageBox clamps the caller's Recti& to screen and callers depend on the mutation afterwards (e.g. UIMessageScroll.cpp:33-42 lays out text using the adjusted rect), so splitting into a pure "fit rect to screen" helper plus a by-value draw means auditing all ~21 call sites in UIPopup.cpp/UIGameOver/UIPartyCreation/LoadStep2State.
- [ ] **1025** - TODO(captainurist): Unload() un-pauses the event timer, which is not always the right thing to do. So we hack. Find a better way.
  - The real fix is dropping `pEventTimer->setPaused(false)` from MPlayer::Unload (src/Media/MediaPlayer.cpp:933) and making the ~13 Unload call sites in Game.cpp/EvtInterpreter.cpp/Tavern.cpp manage the timer themselves, which requires checking movie/event timing in each case.

### `src/GUI/UI/Books/LloydsBook.cpp`

- [ ] **194** - TODO(Nik-RE-dev): need separate function for teleportation to other maps
  - The autoSave/onMapLeave/SKIP_WORLD_UPDATE/GAME_STATE_CHANGE_LOCATION/_transitionMapId/setTeleportTarget sequence is duplicated here, in TownPortalBook.cpp:160, Transport.cpp:162 and several Game.cpp sites, and the copies differ (TownPortal also calls Actor::InitializeActors, autoSave placement varies), so unifying them requires reconciling those differences and retesting map transitions.

### `src/GUI/UI/Books/MapBook.cpp`

- [ ] **191** - TODO(pskelton): stretch texture dont scale it
  - The nearest-neighbour loop does not only rescale - it also bakes per-pixel fog of war (IsMapCellFullyRevealed/PartiallyRevealed dithering), so replacing it with a stretched quad requires generating the reveal mask as a separate overlay plus renderer support for a scaled sub-rect draw (same issue as the twin TODO at UIGame.cpp:1390).

### `src/GUI/UI/Books/TownPortalBook.cpp`

- [ ] **160** - TODO(Nik-RE-dev): need separate function for teleportation to other maps
  - Same duplicated map-change sequence as LloydsBook.cpp:194; this copy additionally calls Actor::InitializeActors() and saves before the branch, so extracting one shared helper means deciding which variant is correct and retesting town portal plus beacon recall.

### `src/GUI/UI/Houses/MagicGuild.cpp`

- [ ] **150** - TODO(pskelton): Extract common item picking code
  - Part of the same shop-grid refactor as the Shops.cpp TODOs: the hit-test here (32 + 70*x, rows at y=90/250) is a fifth hardcoded layout variant, so a shared "item slot layout + pick + draw merchant phrase" helper has to cover weapon/armor/alchemy/guild-book grids without shifting any click boxes.

### `src/GUI/UI/Houses/Shops.cpp`

- [ ] **277** - TODO(pskelton): extract common code around shop item picking
  - sellDialogue/identifyDialogue/repairDialogue share an identical inventory-grid pick + BuildDialogueString + centered-text block that differs only in phrase table and screen id; factoring it out is straightforward but must keep the per-dialogue special cases (identify's already-identified branch, repair's ITEM_BROKEN filter) and spans several functions plus MagicGuild.cpp.
- [ ] **304** - TODO(pskelton): extract common code around shop item picking
  - Same extraction as line 277 (identifyDialogue); its extra branch picks the "%24" phrase for already-identified items, so the shared helper needs a hook for per-screen phrase selection.
- [ ] **338** - TODO(pskelton): extract common code around shop item picking
  - Same extraction as line 277 (repairDialogue); it additionally filters on `entry->flags & ITEM_BROKEN`, so the common helper must allow an item predicate.
- [ ] **370** - TODO(pskelton): extract common code around shop item picking
  - Weapon shop uses its own 6-slot layout (60 - w/2 + 70*i, weaponYPos[i] + 30); unifying it with armor/alchemy/guild grids requires a data-driven slot-rect table so hit boxes stay byte-identical to the current hardcoded math.
- [ ] **437** - TODO(pskelton): extract common code around shop item picking
  - Armor shop is the 2x4 layout (105px pitch, top row bottom-aligned at y=98, bottom row at y=126, second row offset by -420); note its item-count loop only scans 6 of the 8 slots, so the extraction should also settle that inconsistency.
- [ ] **524** - TODO(pskelton): extract common code around shop item picking
  - Magic/alchemy shop is the 2x6 layout with 75px pitch plus edge clamping for slots 0/5/6/11; the clamping means slot rectangles must be computed once and reused by both the draw and the pick path in the extracted helper.
- [ ] **878** - TODO(pskelton): extract common code around shop item picking
  - This is the click side (houseScreenClick) of the same duplication - it repeats the `pt.x <= 13 || pt.x >= 462` + mapToInventoryGrid pick for sell/identify/repair that the draw functions do, so the extracted picker must serve both hover-text and click handling.

### `src/GUI/UI/NPCTopics.cpp`

- [ ] **273** - TODO(pskelton): This doesnt work properly and we dont want draw calls here
  - prepareArenaFight() runs a full BeginScene3D/Draw/Present cycle inline just to flash the "please wait while I summon the monsters" panel; removing it means restructuring arena setup so the message is shown by the normal render loop and the monster spawn happens on a later frame (a loading/deferred state), touching the dialogue-option handling path.
- [ ] **403** - TODO(Nik-RE-dev): this code is walking only through inventory, but item was added to hand, so it will not bind new item if it was acquired; rather this code will bind jars that already present in inventory to liches that currently do not have binded jars
  - Confirmed: AddVariable(VAR_PlayerItemInHands) ends in Party::setHoldingItem (Character.cpp:4689-4697), so the freshly granted jar is the picked item and the inventory scan at 410-417 cannot see it; fixing needs a decision on when a jar gets bound (at grant time on the picked item, or when it lands in an inventory) plus checking vanilla behaviour for already-owned unbound jars.
- [ ] **628** - TODO(Nik-RE-dev): place NPC events in array
  - Mechanical but multi-file: NPCData::dialogue_1..6_evt_id (NPCTable.h:51-56) would become a std::array, updating the ADD_NPC_SCRIPTED_DIALOGUE macro plus three switch/if chains here (577-587, 654-663, 738-759), the snapshot/reconstruct pairs in EntitySnapshots.cpp:357-362/379-384, the txt parser in NPCTable.cpp:84, and EvtInterpreter.cpp:426.

### `src/GUI/UI/UICharacter.cpp`

- [ ] **1308** - TODO(captainurist): doesn't belong here, and doesn't belong in Item.
  - The enchant-animation timer is ticked and expired inside the draw function CharacterUI_DrawItem, so the animation only advances while the item is visible; moving the ItemEnchantmentTimer decrement plus the flag reset and ptr_50C9A4_ItemToEnchant clearing into the per-frame game update means finding an owner for that global state (it is also touched from Game.cpp:527-531, CastSpellInfo.cpp and UIPopup.cpp).
- [ ] **1316** - TODO(captainurist): after my changes id==0 is a valid item id
  - The `id` parameter is now conflated: paperdoll callers pass InventoryEntry::index() (0-based, -1 when invalid) while CharacterUI_InventoryTab_Draw at line 1267 passes the `Cover_Strip` bool, so `|| id` is meaningless in both cases; fixing needs untangling what that fourth argument was meant to signal before the unidentified-item tint condition can be corrected.

### `src/GUI/UI/UIDialogue.cpp`

- [ ] **136** - TODO(Nik-RE-dev): this is for compatability. Previously when NPC can use ability, dialogue allocated 4 buttons unconditionally. Without it many test will fail because of changed buttons positions.
  - Removing the padding DIALOGUE_NULL entry shifts every dialogue button's Y position, which invalidates the recorded game-test traces that click those buttons, so it needs the traces re-recorded and re-verified with game data rather than a code-only change.

### `src/GUI/UI/UIGame.cpp`

- [ ] **1134** - TODO(pskelton): this looks duplicated extract
  - The two button-hover loops (1024-1080 and 1135-1195) are not actually equivalent — the BUTTON_TYPE_NORMAL branch differs (shouldMirror(pWindow) label vs the books `uData == 0` case) and BUTTON_TYPE_SKILLS is implemented in one and `assert(false)` in the other — so extracting a shared helper requires unifying those semantics and re-testing hint behaviour.
- [ ] **1390** - TODO(pskelton): could stretch texture rather than rescale
  - The outdoor minimap is resampled pixel-by-pixel on the CPU into a fresh GraphicsImage every frame; Renderer already has DrawQuad2D(texture, srcRect, dstRect), so the loop plus the static minimaptemp and the rect.w==137 assert could go, but converting the 16.16 fixpoint source window into an integer srcRect changes sampling/filtering, so it needs pixel-level visual verification.

### `src/GUI/UI/UIHouses.cpp`

- [ ] **441** - TODO(Nik-RE-dev): untested until houses NPC can join the party
  - NPCHireableDialogPrepare is only reachable via DIALOGUE_13_hiring_related for a house NPC with a profession; verifying it needs a save/game-test scenario that reaches a hireable house NPC and exercises the hire flow, not a code change.
- [ ] **522** - TODO(Nik-RE-dev): can use GUIWindow_Transition
  - updateHouseNPCTopics hand-builds the same 4 buttons that GUIWindow_Transition::createButtons makes, but with different coordinates (566/486 vs 556/476) and UIMSG_HouseTransitionConfirmation, and the GUIWindow_Transition ctor has side effects (pauses pEventTimer, swaps current_screen_type, allocates the dialogue background) that pDialogueWindow must not have - reusing it means reworking window ownership.

### `src/GUI/UI/UIPartyCreation.cpp`

- [ ] **349** - TODO(captainurist): #unicode this won't work with a Russian localization.
  - Localization strings are raw bytes in the game's legacy codepage (Localization does no encoding conversion), so per-char toupper() only handles ASCII; a correct fix needs the data encoding to be tracked and an encoding-aware upcaser (txt::encodedToUtf32 + case mapping), which is the wider #unicode workstream.
- [ ] **454** - TODO(captainurist): #unicode this won't work for Russian localization.
  - Same issue as line 349 for LSTR_CLASS; both need an encoding-aware toUpper rather than the byte-wise toupper(), which depends on knowing the localization's codepage (cp1251/cp1252).

### `src/GUI/UI/UIPopup.cpp`

- [ ] **582** - TODO(captainurist): check how other durations are formatted, this is not the only place that creates a CivilDuration. Unify the code?
  - This block prints a hardcoded English "Duration:" with "{}:yr/mo/dy/hr/mn" tokens while buff timers go through MakeDateTimeString (GUIWindow.cpp) with localized day/hour/minute words; unifying changes user-visible text and needs a decision plus localization strings for years/months.
- [ ] **845** - TODO(captainurist): Display ranged attack as "Ranged attack". This currently doesn't fit in the table, we used to just do attackStr + " R" but that's cryptic. Redo properly with dynamic alignment.
  - The monster popup uses fixed X_RIGHT_COLUMN/X_RIGHT_DATA columns with only a partial measure pass (the `pWindow == nullptr` measureWidth path); making a longer label fit means turning that into real dynamic column layout and re-checking every localization's string widths.
- [ ] **1465** - TODO(pskelton): Extract common item picking code
  - The per-house-type mouse hit-testing in ShowPopupShopItem duplicates the same arithmetic used in Houses/Shops.cpp:404 and :1012 (which carries its own "extract common code" TODO at Shops.cpp:437); factoring out a shared "shop item under cursor" helper spans several shop window classes with different grid layouts.

### `src/GUI/UI/UISaveLoad.cpp`

- [ ] **85** - TODO(captainurist): lazy-load.
  - All 45 slot thumbnails are pcx-decoded and uploaded when the Save window opens; deferring means storing the thumbnail Blob in SavegameList and decoding on draw, but line 97 derives pSavegameUsedSlots from a successful decode, so the used-slot semantics have to be reworked too.
- [ ] **195** - TODO(captainurist): lazy-load.
  - Same eager pcx decode in the Load window loop; here used-slots don't depend on the decode, but it still needs SavegameList to hold the thumbnail Blobs and a decode-on-first-draw path with correct release semantics (SaveLoad.cpp:120/246 release them).

### `src/Io/InputEnumFunctions.h`

- [ ] **25** - find a better place for this code
  - PlatformKey lives in Library/Platform/Interface but its serialization lives in Io, which is why Application/GameConfig.h and Library/Trace/EventTrace.cpp have to include Io/InputEnumFunctions.h (EventTrace even carries its own "doesn't belong here" TODO); moving the table into a new Library/Platform/Interface/PlatformEnumSerialization.{h,cpp} is mechanical but touches 4 includes, 2 CMakeLists, and adds a library_platform_interface -> library_serialization dependency edge.

### `src/Library/Environment/Win/WinEnvironment.cpp`

- [ ] **15** - revisit this code once I'm on a win machine.
  - OS_GetAppStringRecursive copies into fixed wchar_t[256]/[4096] buffers with unchecked wcsncpy/wcscpy (overflow for long registry paths) and only handles a single split; rewriting it over std::wstring/std::wstring_view is mechanical, but the file only compiles and can only be verified on Windows.

### `src/Library/FileSystem/Interface/FileSystem.h`

- [ ] **52** - _ls(vector*) should append, not overwrite.
  - Beyond dropping entries->clear() in FileSystem::ls and updating all ten _ls overrides, MountingFileSystem::_ls sorts the whole vector and MaskingFileSystem::_ls runs erase_if over it, so both must be reworked to operate only on the newly appended tail; each FS has its own unit test to update.

### `src/Library/FileSystem/Merging/MergingFileSystem.cpp`

- [ ] **74** - This is not ideal, we might want to know ALL merged paths, e.g. see ScriptingSystem::_initPackageTable. But the API that we have here doesn't allow that.
  - Needs a new FileSystem interface method (e.g. virtual _displayPaths(path, std::vector<std::string>*) defaulting to _displayPath) implemented across the FS hierarchy before ScriptingSystem.cpp:84 can list every candidate path the way Lua's searchpath does.

### `src/Library/Platform/Interface/PlatformEnums.h`

- [ ] **76** - this doesn't belong here
  - KEY_CHAR is a pseudo-key used only to funnel text characters through KeyboardInputHandler::ProcessTextInput(PlatformKey, int); removing it means splitting that method into a key handler and a character handler, updating GameWindowHandler.cpp:175 and the keybinding-capture branch, and moving KEY_FIRST off KEY_CHAR.

### `src/Library/Platform/Interface/PlatformEvents.h`

- [ ] **64** - PlatformGamepadKey
  - Splitting the KEY_GAMEPAD_* values into their own enum ripples through SdlEnumTranslation, the shared PlatformKey serialization table, GameConfig's whole gamepad section (KeyConfigEntry is typed on PlatformKey), KeyboardActionMapping::gamepadKeyFor/isBound and GameWindowHandler dispatch - about 160 KEY_GAMEPAD references across 6 files.
- [ ] **69** - PlatformGamepadAxis
  - Same split for axes, which today are smuggled as PlatformKey values (KEY_GAMEPAD_LEFTSTICK_*): needs a new enum, SDL translation changes, and a decision on how axis-as-key bindings in the gamepad config section keep working.

### `src/Library/Platform/Interface/PlatformGamepad.h`

- [ ] **10** - add rumble methods here!
  - Plumbing is small (virtual rumble() on PlatformGamepad, SDL_RumbleGamepad in SdlGamepad, forwarding in ProxyGamepad), but the API shape - intensity model, duration, separate trigger rumble - has to be chosen, and nothing in the engine yet decides when to rumble.

### `src/Library/Trace/EventTrace.cpp`

- [ ] **12** - #include "Io/InputEnumFunctions.h" // doesn't belong here
  - library_trace (a Library/ target) pulls in a header from src/Io just to get MM_DECLARE_SERIALIZATION_FUNCTIONS(PlatformKey); the fix is to move the PlatformKey serialization table (src/Io/InputEnumFunctions.cpp:174ff) plus its declaration into a new Library/Platform/Interface/PlatformKeySerialization.{h,cpp}, add library_serialization to library_platform_interface's link libs, and re-include it from Io/InputEnumFunctions.h so the other four consumers keep compiling.

### `src/Library/Trace/EventTrace.h`

- [ ] **52** - std::string saveFileChecksum;
  - Adding the field means picking a hash, computing it in EngineTraceRecorder::startRecording next to `_trace->header.saveFileSize = _savedGame.size()`, validating it in EngineTracePlayer alongside checkSaveFileSize, and handling the ~100 existing test/Data/*.json traces that have no checksum (either via EventTraceMigrations or by regenerating them).

### `src/Media/Audio/CMakeLists.txt`

- [ ] **31** - should be private
  - OpenAL cannot be PRIVATE while media_audio's public headers expose AL types - OpenALSoundProvider.h (ALenum/ALCdevice/ALCcontext/ALuint) is included from src/Media/MediaPlayer.cpp in a different target, and OpenALAudioDataSource.h/OpenALSample16.h/OpenALTrack16.h include <al.h> too; the fix is to replace AL typedefs with int/unsigned in the headers and push the al.h includes into the .cpp files.

### `src/Media/MediaPlayer.cpp`

- [ ] **538** - no need to copy here.
  - video.last_frame is a shared Blob (Blob::share at MediaPlayer.cpp:265) while RgbaImage owns a malloc'd buffer, so eliminating the memcpy means having AVVideoStream::decode_frame sws_scale straight into an RgbaImage and reworking the shared-ownership of last_frame (which is also re-rendered on pause) across all four copy sites plus IMovie::GetFrame's Blob return type.
- [ ] **690** - no need to copy here.
  - Same root cause as the PlayBink site: _renderTexture takes a `const Blob &` that came from GetFrame()/Blob::share, and GraphicsImage::Create demands an owned RgbaImage, so avoiding the copy requires changing IMovie::GetFrame to hand over an owned frame image.
- [ ] **777** - no need to copy here.
  - HouseMovieLoop copies pMovie_Track->GetFrame()'s Blob into an RgbaImage every frame; removing the copy depends on the same IMovie::GetFrame ownership change (Blob -> RgbaImage) as the other three sites.
- [ ] **862** - no need to copy here.
  - PlayFullscreenMovie's frame loop has the same Blob-to-RgbaImage copy; fixing it in isolation is not possible because GetFrame returns a shared Blob that the movie keeps as last_frame.

### `src/Scripting/GameBindings.cpp`

- [ ] **237** - Use serialization tables to automate this.
  - detail::EnumSerializationTable exposes no way to iterate its value/name pairs and the per-enum serializer objects have internal linkage (static in each .cpp), so automating new_enum registration needs a new public iteration API plus accepting that Lua-visible constant names change (e.g. "Poison_weak", "BlackKnight"), which breaks resources/scripts and the def_*.lua stubs.

### `src/Utility/Streams/FileInputStream.h`

- [ ] **9** - just use raw file io, not FILE*
  - The class already disables libc buffering (setvbuf _IONBF) and manages its own buffer, so switching to open/read/lseek/close means rewriting the .cpp including the Windows path (currently UTF-8 paths work only because of UnicodeCrt + fopen, and ftello/fseeko are #defined to _ftelli64/_fseeki64) - straightforward but needs 64-bit offsets and per-platform testing.

### `src/Utility/Streams/FileOutputStream.h`

- [ ] **9** - just use raw file io, not FILE*
  - Mirror of FileInputStream.h - the buffered write path (_overflow/writeBuffer/_flush) has to be reimplemented on raw descriptors with a Windows wide-char open, and the existing Streams/Tests/FileOutputStream_ut.cpp only partially covers the error paths.

### `src/Utility/String/Tests/Encoding_ut.cpp`

- [ ] **471** - it kinda sucks we can't just do toDebugString for enums via magic enum in Utility. Fix this?
  - magic_enum is only linked into library_serialization (test_utility links just testing_unit + utility), and EnumSerialization.h defines MAGIC_ENUM_RANGE_MIN/MAX before including magic_enum.hpp, so making a generic enum-name helper available in Utility means moving the dependency (and those range defines) down a layer without creating an ODR mismatch between translation units.

### `test/Bin/GameTest/GameTestMain.cpp`

- [ ] **52** - TODO(captainurist): we need a separate test that testing framework terminates correctly if the engine throws.
  - Cannot be a normal GAME_TEST since the failure mode is the whole process hanging or aborting: it needs a new out-of-process test (a flag/mode that makes the engine thread throw, plus a CTest-level runner asserting exit code 1 and no deadlock) exercising the exception marshalling in EngineControlComponent.cpp:34-39 that rethrows on the game thread.

### `test/Bin/GameTest/GameTests_0000.cpp`

- [ ] **434** - TODO(captainurist): this one doesn't obey the unlimited FPS when retracing. Implement properly.
  - Arcomage runs its own blocking nested loop with a private FrameLimiter and hardcoded ArcomageGame::_targetFPS = 32 (Arcomage.h:178, ticked at Arcomage.cpp:880 and 1184), completely ignoring config->graphics.FPSLimit that TestController::adjustMaxFps sets; making it honor the config (including the 0 = unlimited case, which would currently divide by zero in FrameLimiter::tick) means reworking the Arcomage loop's timing and re-verifying the issue_388 trace.

### `test/Testing/Game/TestController.cpp`

- [ ] **140** - TODO(captainurist): this should really happen somewhere in the main loop. When new game is started, or a save is loaded.
  - Moving engine->_messageQueue->clear() out of the test harness into the engine's new-game/load-game transitions in src/Application/Game.cpp (which already clears the queue in a dozen ad-hoc places) risks changing real gameplay message flow, so it needs the right hook in the game-state transition plus a full game-test run to confirm no trace regressions.

## Hard (21)

_Significant refactor or deep investigation._

### `src/Application/GameTraceHandler.h`

- [ ] **6** - TODO(captainurist): tbh we just need a hotkey system instead of this monstrosity.
  - Calls for designing a general hotkey subsystem that would replace this bespoke event filter and the hardcoded key dispatch in GameWindowHandler::OnKey — a cross-cutting input-architecture feature, not a local change.

### `src/Application/GameWindowHandler.cpp`

- [ ] **573** - TODO(captainurist): this is temporary, we need separate axis enum and proper axis handling
  - Gamepad axes are currently shoehorned into PlatformKey with sign-flipping hacks; a proper fix introduces an axis enum and analog event handling through the whole platform stack (SDL backend, Platform event types, event filters, KeyboardController/action mapping), an open-ended cross-layer refactor.

### `src/Bin/OpenEnroth/OpenEnroth.cpp`

- [ ] **199** - TODO: on android without this it won't close application properly until it finishes music track?! Something is not closing and preventing proper teardown?
  - Requires on-device Android debugging to find which thread or resource (likely the music/audio player) is not shut down and blocks normal process teardown; exit(0) papers over an unknown lifecycle bug.

### `src/Engine/AssetsManager.h`

- [ ] **18** - TODO(captainurist): These are called back from GraphicsImage::Release, which is a questionable design.
  - GraphicsImage::release() (src/Engine/Graphics/Image.cpp:60) probes three AssetsManager caches by name and then does 'delete this'; fixing means inverting image ownership so the manager owns lifetimes, which touches the raw-pointer caches and dozens of release() call sites across UI/engine code.

### `src/Engine/EngineGlobals.h`

- [ ] **24** - TODO(captainurist): this global should go, together with this header file.
  - Removing the application global requires giving ~22 use sites in legacy engine/GUI code a non-global path to PlatformApplication, i.e. a dependency-plumbing refactor through code that currently has no context objects.

### `src/Engine/Graphics/Image.h`

- [ ] **29** - TODO(captainurist): drop
  - release() does `delete this` plus cache eviction via AssetsManager's raw-pointer name maps, and there are 86 call sites; dropping it means introducing a real ownership model (AssetsManager-owned unique_ptr or refcounted handles) across GUI and Engine.

### `src/Engine/Graphics/Indoor.cpp`

- [ ] **539** - TODO(captainurist): uncomment this
  - The assert cannot hold today: callers such as Indoor.cpp:375 and Outdoor.cpp:1542 deliberately pass Vec3f(x, y, 0) together with a FACE_XY_PLANE override, so the point is never on the face plane; the commented code also still says pos.toFloat() from when pos was integral. Enabling it means reworking the floor-search API so every caller passes a real 3D point.

### `src/Engine/Graphics/Outdoor.cpp`

- [ ] **929** - TODO(pskelton): check pointer maths for updating fly and waterwalk overlays
  - `decorVars[20 * overlayId + 119]` (6 sites: 938, 1267, 1275, 1279, 1393, 1397) indexes a std::array<unsigned char,125>, so any overlayId >= 1 is an out-of-bounds write; the stride 20 equals sizeof(ActiveOverlay_MM7), meaning the original was byte-pointer arithmetic walking past the persist-vars global into pOtherOverlayList->pOverlays[], and fixing it needs reverse-engineering which overlay field bit 0 was meant to be.
- [ ] **930** - TODO(pskelton): Split function up
  - ODM_ProcessPartyActions is ~580 lines (933-1515) of interleaved flight/water-walk/fall-damage/input/collision/sound logic sharing a dozen mutable locals (floorZ, partyNotTouchingFloor, ceilingHeight, partyInputSpeed...), so splitting it safely needs a state struct and careful behaviour-equivalence testing of party physics.

### `src/Engine/Graphics/Renderer/OpenGLRenderer.cpp`

- [ ] **563** - TODO(pskelton): stencil masking with opacity would be a better way to do this
  - Replacing the CPU pixel loop with a stencil/shader technique is a new render path (shader authoring, opacity params that are currently ignored, item-mask handling) plus visual-parity work against the existing crude enchant effect.

### `src/Engine/Objects/ItemEnchantment.h`

- [ ] **5** - TODO(captainurist): This was an attempt at refactoring the various enchantments we had, and it wasn't finished. Enchantment unification makes sense, so finish this!
  - CEnchantment is only used by the two hardcoded maps in Item.cpp (specialBonusMap/artifactBonusMap); unifying it with Item's standardEnchantment/standardEnchantmentStrength/specialEnchantment fields and the item-table enchantment data is a large refactor with no written target design.

### `src/Engine/Objects/SpriteObject.cpp`

- [ ] **245** - TODO(captainurist): projectiles can fly right through small actors - an ice bolt aimed at a peasant ~250 units away missed it and hit a titan further along the flight path.
  - A reported physics bug with no diagnosis yet: it needs reproduction and then digging into the swept-cylinder test in CollideWithCylinder/CollideWithActor plus how the 100-step move loop picks the nearest hit, and any fix changes projectile behaviour game-wide.

### `src/Engine/Spells/Spells.cpp`

- [ ] **866** - TODO(captainurist): See the logic in Outdoor.cpp, right now the force is applied in fixed amounts per frame, while it should be applied in amounts relative to frame time --- basically, armageddon should provide some acceleration, and then this acceleration should be applied to actors over a brief period of time.
  - Requires reworking the per-frame velocity kick at Outdoor.cpp:1689 into a time-integrated acceleration model plus a replacement for the frame-counting armageddonForceCount, which changes actor physics and invalidates recorded traces used by the game tests.

### `src/Engine/Time/Timer.cpp`

- [ ] **19** - TODO(captainurist): This is needed because we roll back time in tests. We're dancing around with 32_ticks to maintain trace compatibility. Think how to do this better.
  - Removing the fudge means changing how trace playback drives platform->tickCount() (recorded PaintEvent tickCounts go backwards on replay) or resetting the timer at trace start, and the exact 32_ticks value is what keeps existing recorded traces reproducible, so every trace would need re-verification.

### `src/GUI/GUIWindow.cpp`

- [ ] **205** - TODO(pskelton): Derived Messagebox types for different kinds of popup boxes
  - Introducing a popup class hierarchy means restructuring the ~15 ad-hoc DrawMessageBox call sites in UIPopup.cpp (item info, spell info, monster info, minimap hint, etc.), each of which hand-computes its own rect and then draws its own content on top - a large UI refactor with pixel-level regression risk.

### `src/GUI/UI/Books/MapBook.cpp`

- [ ] **154** - TODO(captainurist): this needs to be merged with GameUI_DrawMinimap
  - DrawBook_Map_sub (~160 lines) and GameUI_DrawMinimap (UIGame.cpp:1335, ~200 lines) share only outline/arrow/decoration drawing; they differ in fog-of-war handling, zoom source (uMapBookMapZoom vs uMinimapZoom), fixpoint scaling math, red actor dots and wizard-eye logic, so merging is a large rewrite of two legacy rendering paths with pixel-exact expectations.

### `src/Library/FileSystem/Interface/FileSystem.h`

- [ ] **48** - internal functions should NOT throw? Honestly, makes a lot of sense. Just throw in FileSystem impl! Is it OK for the underlying functions to throw? I think no. All exceptions should be `FileSystemException`s. Then I'll also be able to drop the exists() -> ls() paired calls that are inherently race-y.
  - Changing the _exists/_stat/_ls/_read/... contract to non-throwing means reworking error reporting and return types in all ten implementations (Directory, Memory, Merging, Mounting, Lowercase, Masking, Sub, Embedded, Null, Proxy) and rewriting the racy stat+ls composites in MergingFileSystem::_ls and MountingFileSystem::_ls, with every FS test suite affected.

### `src/Library/LodFormats/LodFormats.cpp`

- [ ] **223** - just store blob in GrayscaleImage, no need to copy here.
  - Image<T> stores std::unique_ptr<T, FreeDeleter> (malloc'd, mutable) while Blob exposes only const data, so blob-backed pixels require a new storage policy in detail::ImageBase plus a const/immutable image flavour, and consumers such as TileGenerator that mutate the decoded image must be adapted.

### `src/Library/Platform/Interface/PlatformEventLoop.h`

- [ ] **35** - this should be dropped.
  - waitForMessages exists purely to feed the MessageLoopWithWait macro used by six nested blocking loops (Game.cpp x2, GameMenu, UIPartyCreation, MediaPlayer x2) and is special-cased in EngineControlComponent for the trace/control routine; dropping it means unwinding those nested loops and deciding how the game idles while inactive.

### `src/Media/MediaPlayer.h`

- [ ] **29** - Remove this method once we move all the Video to be played in the Fsm
  - PlayFullscreenMovie is a blocking modal loop called from the middle of Game::gameLoop (Game.cpp:175, :1647) and from EVENT_ShowMovie inside EvtInterpreter.cpp:269; converting those to the FsmState model (as VideoState already does for logo/intro) requires making the script interpreter and death/new-game flows resumable rather than blocking.

### `src/Utility/String/Tests/Encoding_ut.cpp`

- [ ] **530** - TODO(captainurist): extend this to all encodings once ztd.text is fixed - decoding a single (odd) byte in the fixed-width UTF-16 / UTF-32 encodings reads out of bounds, and an unmapped byte in some multi-byte encodings may still hang.
  - The test edit is one line (replace the hand-written singleByteEncodings array with Segment(ENCODING_FIRST, ENCODING_LAST)), but it is gated on fixing decoder bugs inside the vendored thirdparty/ztd_text (OOB read on odd-length input for basic_utf16_*/basic_utf32_*, and a non-advancing decode loop for unmapped bytes in multi-byte encodings) - i.e. debugging a large third-party text library and carrying a local patch or upstream fix.

## Design (282)

_Blocked on a decision, missing data, or original-game research._

### `src/Application/Game.cpp`

- [ ] **590** - TODO(Nik-RE-dev): what is this? Btw, 153 == HOUSE_EARTH_GUILD_STONE_CITY.
  - Answering "what is this" requires researching the original MM7 binary/decompilation to learn why escaping SCREEN_INPUT_BLV special-cases house animation 153 (likely a decompiled magic constant for the members-only guild movie); can't be resolved by just reading this code.
- [ ] **744** - TODO(Nik-RE-dev): rest and heal uncoditionally even if party does not have food?
  - It is a correctness question about original-game behavior: restAndHeal(travelTime) runs unconditionally, then pParty->restAndHeal() runs again only with food — deciding whether that matches vanilla MM7 travel needs comparison against the original binary, not just a code edit.
- [ ] **832** - TODO(captainurist): what's going on here?
  - The code is nonsense as written: v53 is uint16_t so the `v53 < 0` teleport branch is dead, `_quest_bit` is typed as QuestBit while the original signed value encoded a negative teleport index, and the whole UIMSG_HouseTransitionConfirmation case starts with assert(false) — untangling it needs decompilation research into the original table column semantics.

### `src/Application/GameStates/LoadStep2State.cpp`

- [ ] **12** - TODO(Gerark) This specific value GAME_SETTINGS_4000 is checked only in UIPartyCreation. So, this assignment might be removed after the Party Creation becomes part of the FSM
  - Blocked on an upstream refactor: the flag can only be dropped once Party Creation is migrated into the FSM state machine, which is a separate planned piece of work — nothing to code here until that lands.

### `src/Application/GameStates/MainMenuState.cpp`

- [ ] **77** - TODO(Gerark) Remove this GUI_UpdateWindows once we have a proper Retained Mode UI system. Right now we're forced to call this to cause the proper removal of temporary "buttons"
  - Blocked on the planned retained-mode UI rework; until the immediate-mode GUI_UpdateWindows lifecycle is replaced, this call is genuinely required to release temporary buttons, so the TODO cannot be acted on independently.

### `src/Application/GameWindowHandler.cpp`

- [ ] **542** - TODO(captainurist): That's a very convoluted way to exit the game, redo this properly once we have a unified event loop.
  - Explicitly blocked on the future unified event loop; today the control-routine that scripts 'go to main menu, press ExitGame button' is the only clean shutdown path, so this cannot be redone until that architecture work happens.

### `src/Application/Startup/GameStarter.cpp`

- [ ] **102** - TODO(captainurist): actually move datapath to config?
  - Question-marked policy decision: datapath is currently resolved from CLI/env/registry/platform candidates, and moving it into the config file changes precedence and UX for portable installs and first-run detection — needs a decision on how config, CLI and auto-detection interact before any code.

### `src/Arcomage/Arcomage.cpp`

- [ ] **1535** - TODO(captainurist): this was drawn with blendMode = 1 in original binary, did it have special meaning?
  - Answering requires reverse-engineering what blend mode 1 meant in the original software renderer and pixel-comparing against the original game's Arcomage screen; nothing can be coded until that is known.

### `src/Bin/OpenEnroth/OpenEnroth.cpp`

- [ ] **48** - TODO(captainurist): #cpp26 use std::sat_sub
  - Blocked on the toolchain moving to C++26 (the function is std::sub_sat in <numeric>); until then the existing ternary is the correct code, per the project's #cpp26 convention.

### `src/Engine/Components/Trace/EngineTraceStateAccessor.cpp`

- [ ] **101** - TODO(captainurist): drop this call once we have everything in UTF-8.
  - Explicitly blocked on the codebase-wide UTF-8 migration; dropping txt::utf8ToEncoded now would change item-name strings in trace game state and invalidate recorded traces.

### `src/Engine/Data/CMakeLists.txt`

- [ ] **6** - TODO(captainurist): Game-related enums go to Engine/XYZ/....??? Engine/Model?
  - Open architectural question about where game-domain enums belong in the layering (Engine/Data vs a new Engine/Model); it is the same undecided question behind the Data->Objects dependency TODOs, so nothing can be moved until it is answered.

### `src/Engine/Data/HouseEnums.h`

- [ ] **607** - TODO(captainurist): Somehow this is the type of almost all houses in the game. So, not a mercenary guild?
  - Requires research into the original MM6/MM7 2dEvents.txt data and binary to learn what house type 18 actually denoted before the enumerator can be renamed meaningfully.

### `src/Engine/Engine.cpp`

- [ ] **335** - TODO: either add conversion functions, or keep only glm / only Vec3_* classes.
  - An unresolved either/or: conversion helpers between glm and Vec3f are trivial, but the real question is whether to unify on one vector library across Engine/Graphics (Camera, renderers), which is a sweeping refactor - someone must pick the direction first.
- [ ] **541** - TODO(captainurist): need to zero this one out when loading a save, but is this a proper place to do that?
  - The clear works today (DoPrepareWorld runs on both save-load and map transition; attackList is per-frame transient state also cleared in Actor.cpp:4476); resolving the TODO is a placement decision about where save-load state resets belong, not a code fix.
- [ ] **722** - TODO(captainurist): try resurrecting the food / gold animations using resource files from MM6?
  - The commented code references icons (glow03, glow05, torchA, wizeyeA) that are not in MM7 data; blocked on deciding whether/how to source MM6 assets users may not own and how optional cross-game resources would be loaded.
- [ ] **1049** - TODO(pskelton): adding this here for now but behaviour around firespike permanence needs checking
  - Whether fire-spike cast counters/spikes should reset on the daily tick requires checking original MM7 behavior around fire spike permanence; it is a research question, not a code change.
- [ ] **1053** - TODO(pskelton): do water and lava damage need to be more accurate to dt?
  - Open question whether the fixed 128-tick water/lava damage timer should be scaled to frame dt; needs a decision and comparison against the original engine's timing before any change.
- [ ] **1064** - TODO(pskelton): fire damage?
  - Drowning damage is applied as DAMAGE_FIRE even though DAMAGE_WATER exists; changing it alters which resistances apply, so it first needs verification of what damage type the original binary used.
- [ ] **1085** - TODO(captainurist): #time drop once we move to msecs in duration.
  - Explicitly blocked on the repo-wide #time migration of Duration from ticks to milliseconds; until that lands the _roundingDt carry logic for halved recovery cannot be dropped.
- [ ] **1212** - TODO: actually this looks like it never triggers. we get cursed_times, which is a time the character was cursed since the start of the game (a very large number), and compare it with times_triggered, which is a small number
  - Documents why the curse-breaks-flight block below is #if 0'd; reinstating it correctly requires reverse-engineering the original curse logic (tracked in issue #123), not just code edits.
- [ ] **1229** - TODO: cursed_times was a pointer before, and we had cursed_times = 0 here, was this meant to cancel the curse?
  - Inside the same #if 0'd curse block; answering whether the original code cancelled the curse needs decompilation research of the original binary (issue #123).

### `src/Engine/Evt/EvtEnums.h`

- [ ] **260** - TODO(Nik-RE-dev): currently exclusive for MM7, need to be independent from players number
  - EvtTargetCharacter hardcodes CHOOSE_PLAYER1..4; decoupling it from the 4-character party is tied to future MM6/MM8 support and needs a design for how event data addresses variable party sizes.

### `src/Engine/Evt/EvtInstruction.cpp`

- [ ] **784** - TODO
  - toString for EVENT_SetActorGroup cannot be written: the opcode is absent from used MM7 data and its payload is not even parsed (parse case at line 1109 is also TODO), so the wire format must be reverse-engineered first.
- [ ] **797** - TODO
  - toString for EVENT_ChangeGroup is blocked the same way: opcode not present in used MM7 data, payload layout unknown and unparsed (line 1142), so it needs reverse-engineering from the original engine or MM6/MM8 data.
- [ ] **800** - TODO
  - toString for EVENT_ChangeGroupAlly is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1145) is also unimplemented.
- [ ] **813** - TODO
  - toString for EVENT_OnDateTimer is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1170) is also unimplemented.
- [ ] **816** - TODO
  - toString for EVENT_EnableDateTimer is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1173) is also unimplemented.
- [ ] **819** - TODO
  - toString for EVENT_StopAnimation is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1176) is also unimplemented.
- [ ] **822** - TODO
  - toString for EVENT_CheckItemsCount is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1179) is also unimplemented.
- [ ] **825** - TODO
  - toString for EVENT_RemoveItems is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1182) is also unimplemented.
- [ ] **828** - TODO
  - toString for EVENT_SpecialJump is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1185) is also unimplemented.
- [ ] **831** - TODO
  - toString for EVENT_IsTotalBountyHuntingAwardInRange is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1188) is also unimplemented.
- [ ] **834** - TODO
  - toString for EVENT_IsNPCInParty is blocked on unknown payload layout; the opcode is absent from used MM7 data and its parse case (line 1191) is also unimplemented.
- [ ] **860** - TODO(yoctozepto): which are present in global events and which in local?
  - Classifying which opcodes occur in global vs local event scripts requires surveying the actual MM7 (and ideally MM6/MM8) game data; it is a data-research task whose result would then inform validation.
- [ ] **861** - TODO(yoctozepto): some types (marked with further TODOs) are not present in used MM7 data - their parsing might thus be wrong
  - Confirming the parsing of opcodes unseen in MM7 data requires checking the original engine's decompilation or MM6/MM8 data files; nothing can be coded until that research is done.
- [ ] **866** - TODO(yoctozepto): always 0 in MM7 data, check MM6&8
  - Answering requires MM6/MM8 event data (not shipped with this MM7-focused repo) or original-binary reverse engineering to learn whether the skipped byte in EVENT_Exit ever carries meaning; nothing to code until that is known.
- [ ] **883** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_LocationName never occurs in shipped MM7 event data, so the assumed empty payload cannot be verified without MM6/MM8 data or original-engine disassembly.
- [ ] **894** - TODO(captainurist): Is this correct? Houses can have ids > 255.
  - Whether EVENT_MoveToMap really stores house_id as a single byte (making ids > 255 unencodable) versus this being a parsing bug can only be settled by checking the original MM7 engine's event-format handling; the fixed offsets before the trailing string make it plausible but unproven.
- [ ] **902** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_ShowFace never occurs in shipped MM7 event data, so its assumed who+portrait layout is unverified; needs MM6/MM8 data or original-binary research.
- [ ] **913** - TODO(yoctozepto): not present in used MM7 data; likely present in MM6
  - EVENT_SetSnow's payload layout can only be verified against MM6 event data, which the project does not yet support or ship.
- [ ] **925** - TODO(yoctozepto): always 1 in MM7 data, check MM6&8
  - The meaning of the skipped byte in EVENT_ShowMovie is unknowable from MM7 data alone (it is constant there); requires MM6/MM8 data or disassembly research.
- [ ] **967** - TODO(yoctozepto): why add 1? it is not done with Event_CheckSkill
  - Explaining why CastSpell's mastery field is 0-based while CheckSkill's is 1-based requires reading the original MM7 engine's two event handlers in disassembly; the +1 matches observed data so changing code blind risks breaking scripted spells.
- [ ] **986** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_ToggleActorFlag never occurs in shipped MM7 event data, so its assumed id/attr/is_set layout is unverified; needs MM6/MM8 data or original-binary research.
- [ ] **1008** - TODO(yoctozepto): not present in used MM7 data; likely present in MM8 (e.g., Escaton's riddles)
  - EVENT_InputString's layout can only be confirmed against MM8 event data (e.g. Escaton's riddles), which the project does not yet ship or support.
- [ ] **1030** - TODO(yoctozepto): always 0 in MM7 data, check MM6&8
  - The skipped trailing uint16 of EVENT_OnTimer/OnLongTimer is constant in MM7 data; learning its meaning needs MM6/MM8 data or original-engine research.
- [ ] **1037** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_PressAnyKey never occurs in shipped MM7 data and the body is literally a guess ('Nothing?'); confirming it has no payload needs MM6/MM8 data or disassembly.
- [ ] **1040** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_SummonItem never occurs in shipped MM7 event data, so its assumed 7-field layout is unverified; needs MM6/MM8 data or original-binary research.
- [ ] **1060** - TODO(yoctozepto): always 0 in MM7 data, check MM6&8
  - The skipped byte in EVENT_OnMapReload is constant in MM7 data; its meaning can only be established from MM6/MM8 data or original-engine research.
- [ ] **1098** - TODO(yoctozepto): always 0 in MM7 data, check MM6&8
  - The skipped byte in EVENT_EndCanShowDialogItem is constant in MM7 data; its meaning can only be established from MM6/MM8 data or original-engine research.
- [ ] **1109** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_SetActorGroup never occurs in shipped MM7 event data and its payload is entirely unparsed; the wire format must first be recovered from MM6/MM8 data or original binaries.
- [ ] **1110** - TODO
  - Placeholder for the unimplemented EVENT_SetActorGroup payload parsing; blocked on the same missing-format research as the comment on the case label above it.
- [ ] **1131** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_CanShowTopic_IsActorKilled never occurs in shipped MM7 data; its parsing was copied from EVENT_IsActorKilled and cannot be verified without MM6/MM8 data or disassembly.
- [ ] **1140** - TODO(yoctozepto): always 0 in MM7 data, check MM6&8
  - The skipped byte in EVENT_OnMapLeave is constant in MM7 data; its meaning can only be established from MM6/MM8 data or original-engine research.
- [ ] **1142** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_ChangeGroup never occurs in shipped MM7 event data and its payload is entirely unparsed; the wire format must first be recovered from MM6/MM8 data or original binaries.
- [ ] **1143** - TODO
  - Placeholder for the unimplemented EVENT_ChangeGroup payload parsing; blocked on the same missing-format research as the comment on the case label above it.
- [ ] **1145** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_ChangeGroupAlly never occurs in shipped MM7 event data and its payload is entirely unparsed; the wire format must first be recovered from MM6/MM8 data or original binaries.
- [ ] **1146** - TODO
  - Placeholder for the unimplemented EVENT_ChangeGroupAlly payload parsing; blocked on the same missing-format research as the comment on the case label above it.
- [ ] **1170** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_OnDateTimer never occurs in shipped MM7 event data and its payload is entirely unparsed; the wire format must first be recovered from MM6/MM8 data or original binaries.
- [ ] **1171** - TODO
  - Placeholder for the unimplemented EVENT_OnDateTimer payload parsing; blocked on the same missing-format research as the comment on the case label above it.
- [ ] **1173** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_EnableDateTimer never occurs in shipped MM7 event data and its payload is entirely unparsed; the wire format must first be recovered from MM6/MM8 data or original binaries.
- [ ] **1174** - TODO
  - Placeholder for the unimplemented EVENT_EnableDateTimer payload parsing; blocked on the same missing-format research as the comment on the case label above it.
- [ ] **1176** - TODO(yoctozepto): not present in used MM7 data
  - EVENT_StopAnimation never occurs in shipped MM7 event data and its payload is entirely unparsed; the wire format must first be recovered from MM6/MM8 data or original binaries.
- [ ] **1177** - TODO
  - Placeholder body for EVENT_StopAnimation parsing; the opcode never occurs in used MM7 data, so its binary layout must be researched from MM6/MM8 or GrayFace docs before parsing can be written (the case also skips requireSize, so encountering it would trip the assert after the switch).
- [ ] **1179** - TODO(yoctozepto): not present in used MM7 data
  - Annotation on the EVENT_CheckItemsCount case whose parsing is unimplemented; resolving it means reverse-engineering the opcode's wire format from MM6/MM8/GrayFace sources since no MM7 data exercises it.
- [ ] **1180** - TODO
  - Placeholder body for EVENT_CheckItemsCount parsing; blocked on the same missing-format research as the case annotation above, and the matching interpreter case at EvtInterpreter.cpp:573 is also an assert(false) stub.
- [ ] **1182** - TODO(yoctozepto): not present in used MM7 data
  - Annotation on EVENT_RemoveItems whose parsing is unimplemented; needs opcode-format research from MM6/MM8/GrayFace documentation because used MM7 data never contains it.
- [ ] **1183** - TODO
  - Placeholder body for EVENT_RemoveItems parsing; same blocker — the binary layout is unknown and cannot be verified against any MM7 data.
- [ ] **1185** - TODO(yoctozepto): not present in used MM7 data
  - Annotation on EVENT_SpecialJump whose parsing is unimplemented; blocked on researching the opcode format outside MM7 data.
- [ ] **1186** - TODO
  - Placeholder body for EVENT_SpecialJump parsing; same missing-format blocker, and the interpreter side (EvtInterpreter.cpp:581) is an assert(false) stub too.
- [ ] **1188** - TODO(yoctozepto): not present in used MM7 data
  - Annotation on EVENT_IsTotalBountyHuntingAwardInRange whose parsing is unimplemented; needs MM6/MM8/GrayFace format research since MM7 data never uses it.
- [ ] **1189** - TODO
  - Placeholder body for EVENT_IsTotalBountyHuntingAwardInRange parsing; blocked on the same unknown wire format.
- [ ] **1191** - TODO(yoctozepto): not present in used MM7 data
  - Annotation on EVENT_IsNPCInParty whose parsing is unimplemented; blocked on researching the opcode format from non-MM7 sources.
- [ ] **1192** - TODO
  - Placeholder body for EVENT_IsNPCInParty parsing; same missing-format blocker as the annotation above.

### `src/Engine/Evt/EvtInterpreter.cpp`

- [ ] **150** - TODO: enconunter and process
  - The #if 0 code for EVENT_CanShowTopic_IsActorKilled in NPC mode mirrors the working EVENT_IsActorKilled handler, but the opcode never occurs in used MM7 data, so there is no way to test-enable it without first finding data (MM6/MM8/mods) that exercises it.
- [ ] **190** - TODO(pskelton): Fix #1890 this should be a data mod
  - The hardcoded z-coordinate patch for d20.blv event 501 should move into game data, but the engine has no data-mod/patch mechanism for evt scripts yet, so this is blocked on that infrastructure decision (issue #1890).
- [ ] **202** - TODO(pskelton): Fix #2117 this should be a data mod - stop it overwriting the teleport point
  - Hardcoded d25.blv/event-451 teleport-point workaround that should be expressed as a data fix; blocked on the same nonexistent data-mod mechanism (issue #2117).
- [ ] **208** - TODO(pskelton): Fix #2117 this should be a data mod
  - Second half of the same d25.blv/event-451 workaround (rewriting ir.str to out06.odm); also blocked on data-mod infrastructure (issue #2117).
- [ ] **344** - TODO(pskeltonm): Fix #2223 stop tutorial message spam - should be data mod
  - The Emerald Island event-200..218 dedup via _OE_transientVariables is an engine-side hack for a data problem; moving it out is blocked on a data-mod mechanism (issue #2223).
- [ ] **369** - TODO(Nik-RE-dev): this event is not used in MM7. In GrayFace's data it's called "Question" and must have additional arguments that control where events executions must be continued on correct/incorrect input.
  - Implementing EVENT_InputString as GrayFace's Question opcode requires the extended argument format from GrayFace's documentation plus new correct/incorrect-branch dialogue flow, and it cannot be exercised by MM7 data.
- [ ] **446** - TODO(Nik-RE-dev): Looks like it's artifact of MM6
  - The #if 0 EVENT_MoveNPC special case references HOUSE_BODY_GUILD_MASTER_ERATHIA, which is an MM7 house, so the 'MM6 artifact' guess needs verification against the original MM7 disassembly before the block can be deleted or re-enabled.
- [ ] **483** - TODO(Nik-RE-dev): original code for this option is dubious
  - EVENT_CheckSkill asserts out the CHOOSE_PARTY target because the original code's semantics for a party-wide skill check are unclear; replacing the assert requires researching the original MM7 behavior to define what it should do.
- [ ] **494** - TODO: enconunter and process
  - EVENT_SetActorGroup is an assert(false) stub whose #if 0 body is undecipherable decompiler pointer arithmetic; implementing it needs reverse-engineering the original opcode semantics, and no used MM7 data triggers it to test against.
- [ ] **520** - TODO: enconunter and process
  - EVENT_ChangeGroup has plausible #if 0 code (retarget actor uGroup), but the opcode's parsing is also unimplemented (EvtInstruction.cpp:1142-1144, 'not present in used MM7 data'), so enabling it is blocked on format research and data that exercises it.
- [ ] **532** - TODO: enconunter and process
  - Same situation as EVENT_ChangeGroup: EVENT_ChangeGroupAlly's #if 0 body looks sensible but its parser case is an empty TODO and the opcode never appears in used MM7 data, so it cannot be implemented and verified without external format research.
- [ ] **562** - TODO: seems unused
  - EVENT_OnDateTimer is an assert(false) stub; the parser side already establishes it is not present in used MM7 data, so resolving the TODO means deciding whether MM6/MM8/mod support warrants implementing it (needs external format/behavior research) or keeping the assert.
- [ ] **566** - TODO: seems unused
  - Same as EVENT_OnDateTimer: EVENT_EnableDateTimer is an assert(false) stub for an opcode absent from used MM7 data; implementing it requires MM6/MM8/GrayFace research, otherwise only the comment wording can be firmed up.
- [ ] **570** - TODO: seems unused
  - EVENT_StopAnimation interpreter stub matching the unimplemented parser case at EvtInstruction.cpp:1176-1178; blocked on the same missing opcode-format/behavior research.
- [ ] **574** - TODO: seems unused
  - EVENT_CheckItemsCount interpreter stub matching the unimplemented parser case at EvtInstruction.cpp:1179-1181; blocked on researching the opcode's format and semantics outside MM7 data.
- [ ] **578** - TODO: seems unused
  - EVENT_RemoveItems interpreter stub matching the unimplemented parser case at EvtInstruction.cpp:1182-1184; blocked on the same missing-format research since no used MM7 data contains the opcode.

### `src/Engine/Graphics/BspRenderer.cpp`

- [ ] **34** - TODO(yoctozepto): are face vertices consecutive? are face vertices shared/overlapping?
  - A research question about invariants of original MM7 level geometry; nothing to code until someone surveys the game data (or original engine) to answer it.
- [ ] **60** - TODO(yoctozepto): does the below happen?
  - Answering whether untextured non-portal faces actually occur requires surveying all indoor map data (or instrumenting and soaking every map); it is a data-research question, not a code change.
- [ ] **61** - TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also OpenGLRenderer)
  - Depends on the unanswered question above and on deciding what the original game rendered for untextured faces; needs coordinated changes in BspRenderer and OpenGLRenderer once that behavior is decided.

### `src/Engine/Graphics/Camera.cpp`

- [ ] **28** - TODO(pskelton): do we want to be overriding the config value here?
  - Blocked on a policy decision: whether per-map far-clip overrides (Wromthrax's Cave hardcode) should exist at all, live in map metadata, or respect the user's ClipFarDistance config.

### `src/Engine/Graphics/Camera.h`

- [ ] **81** - TODO(captainurist): up is negative? wtf???
  - The inverted pitch sign is the original game's convention inherited via party state and input handling; resolving it requires deciding whether to flip the convention engine-wide (rippling through input, view-matrix construction and possibly save data) or just document it.

### `src/Engine/Graphics/Collisions.cpp`

- [ ] **500** - TODO(pskelton): disable this for time being - this appears to be a obselete legacy collision remnant. Was meant to handle portal crossing to update sector id during movement. Causes issue where portal "collision" overrides actual wall collision
  - Blocked on deciding whether portal-crossing sector updates are fully handled elsewhere (making the dead body and the caller's now-pointless 100-iteration loop at line 891 deletable) or whether a correct reimplementation is needed; "for time being" marks that unresolved call.
- [ ] **707** - TODO(pskelton): Do actors need same exclusions as party?
  - An open gameplay question - whether the party path's per-map face exclusions and slope special-cases should also apply to actor collisions; needs investigation/repro of actor behavior on the affected geometry, possibly against the original binary.
- [ ] **709** - TODO(pskelton): This 'catch all' is probably unsafe - would be better as above
  - Replacing the generic invisible/POLYGON_InBetweenFloorAndWall exemption with explicit per-map face lists first requires surveying which faces across all maps actually rely on the catch-all - map-data investigation before any code change.
- [ ] **881** - TODO(pskelton): check this
  - Verifying the head-sphere placement (pos + height - radius_lo, identical in the ODM path at line 1038 but different from actors' pos + height at lines 604/765) requires comparison against the original binary's collision code; note radius_hi == radius_lo here so a radius swap alone changes nothing.
- [ ] **961** - TODO(pskelton): Better way to do this? Maybe add a climbable attribute
  - There is no per-map/per-face data-override mechanism in the repo (no map patch infra at all), so a "climbable" attribute needs a decision on where such hand-authored face fixes live plus a sweep of every map for faces relying on the current hardcoded id lists.
- [ ] **983** - TODO(pskelton): This 'catch all' is probably unsafe - would be better as above
  - Replacing the blanket "invisible InBetweenFloorAndWall is walkable" rule with explicit per-map face-id lists requires playtesting every indoor map to discover which faces currently depend on it — it cannot be derived from the code.
- [ ] **1038** - TODO(pskelton): check this
  - radius_lo and radius_hi are both pParty->radius here so the expression is self-consistent; verifying that the upper collision sphere really belongs at `height - radius` is a question about original MM7 collision geometry that needs RE/behaviour comparison, not a code change.

### `src/Engine/Graphics/FaceEnums.h`

- [ ] **44** - TODO: MMExt: HasData, are we talking about BLVFaceExtra?
  - All 32 bits of FaceAttribute are already named, so answering means cross-referencing GrayFace's MMExtension bit tables (external source) to find which bit it calls HasData and whether it corresponds to BLVFace::faceExtraId — research, not code.

### `src/Engine/Graphics/ImageLoader.cpp`

- [ ] **26** - TODO(captainurist): #jsonify & move to compiled-in game data
  - No compiled-in game-data pipeline exists yet (Library/Json is only used by the trace recorder), so this is blocked on designing where such data files live and how they get embedded in the binary.

### `src/Engine/Graphics/Indoor.cpp`

- [ ] **59** - TODO(captainurist): move to SoundEnums.h?
  - Media/Audio/SoundEnums.h currently has no Engine dependencies, and this table is indexed by Engine's MapId, so moving it there inverts the layering — the real question (where map→sound data belongs) has to be decided before anything is moved.
- [ ] **817** - TODO(pskelton): asserts trips on test 416 with Dragons OOB - consider running actor check on file load to correct positions so this assert can be reinstated.
  - Reinstating the assert requires inventing a load-time fixup for actors that ship out-of-bounds in the original map data (what counts as a valid position, where to snap them), and that relocation would change test 416's recorded trace.
- [ ] **876** - TODO(pskelton): why 8? doesnt match party
  - The 8x multiplier is an original-game constant (party uses -2 at Indoor.cpp:1654, outdoor actors use -1 at Outdoor.cpp:1685); answering "why" needs MM7 reverse-engineering, and changing it would alter every indoor actor's fall speed.
- [ ] **1100** - TODO: Does POLYGON_Ceiling really belong here? Returned z is then used like this in UpdateActors_BLV: if (actor.z <= z) { actor.z = z + 1; And if this z is ceiling z, then this will place the actor above the ceiling.
  - Whether ceiling faces may act as floors is original-game behaviour that has to be verified per map (some indoor sectors do use ceiling-typed faces as walkable surfaces); note the cited UpdateActors_BLV snippet no longer exists in that form after the actor-collision rewrite, so the justification part of the comment is out of date.
- [ ] **1552** - TODO(captainurist): #time think about a better way to write this formula.
  - The formula `dt().ticks() * _yawRotationSpeed * TrigLUT.uIntegerPi / 180 / Duration::TICKS_PER_REALTIME_SECOND` mixes tick counts with binary-angle units; writing it "better" means designing a Duration API (e.g. a float realtimeSeconds() plus an angle-per-second type) and accepting changed integer truncation, which shifts turn rates and would move replay/game-test traces.
- [ ] **1719** - TODO(Nik-RE-dev): need to probe surface
  - BLV faces carry no surface-material data beyond FACE_INDOOR_CARPET/FACE_IsFluid/FACE_IsLava, so everything non-carpet falls back to SOUND_RunWood; picking stone/metal variants (SoundEnums has unidentified candidates like SOUND_Run60_99 "indoor stone walk") needs research into how the original mapped face textures to footstep sounds, plus a new texture-name-to-material table.
- [ ] **1730** - TODO(Nik-RE-dev): need to probe surface
  - Same TODO as line 1719 but on the walking (not running) branch; blocked on the same missing per-face material data and would be fixed by the same table.
- [ ] **1916** - TODO(captainurist): this needs checking
  - Item::GenerateArtifact() already calls Reset() and then only sets itemId, so this Reset() is behaviourally the original's `uItemID = 0` — i.e. the artifact spawn point drops a sprite whose contained item is empty; deciding whether to keep that vanilla quirk or actually spawn the artifact requires checking original game behaviour and how pIsArtifactFound is meant to be set.

### `src/Engine/Graphics/Indoor.h`

- [ ] **84** - TODO(captainurist): why is this one unused?
  - BLVFaceExtra::additionalBitmapId is write-only: CompositeSnapshots.cpp:181-183 hard-codes it to -1 with the loadTexture call commented out, and nothing ever reads it; resolving means researching what the original engine did with a face's second bitmap before either implementing it or deleting the field and its snapshot plumbing.
- [ ] **176** - TODO(captainurist): why is this one unused?
  - Same field copied onto BLVFace at CompositeSnapshots.cpp:190 and never read; blocked on the same reverse-engineering question as the BLVFaceExtra copy at line 84.

### `src/Engine/Graphics/LocationFunctions.cpp`

- [ ] **14** - TODO(captainurist): indoor & outdoor messed up, is this a bug?
  - GetAlertStatus() reads pOutdoor->ddm.alertStatus when indoors and pIndoor->dlv.alertStatus when outdoors — clearly swapped, but it faithfully reproduces the decompiled 0x450DA3, so fixing it changes monster-alert gameplay and needs a decision on whether to preserve the vanilla bug (currentLocationInfo() right below already does it the sane way).

### `src/Engine/Graphics/Outdoor.cpp`

- [ ] **789** - TODO(pskelton): drop tint color anyway?
  - sTintColor feeds the config->graphics.Tinting path in BaseRenderer.cpp:304 (documented as "vanilla's monster coloring method from hardware mode", default off), so removing it deletes an intentionally preserved vanilla rendering mode — a maintainer decision, not a coding task.
- [ ] **1013** - TODO(captainurist): #time think about a better way to write this formula.
  - Exact duplicate of Indoor.cpp:1552 (same dturn expression); resolving both means agreeing on a Duration/angular-speed API and accepting the rounding change, so it is a library-design decision rather than a local edit.
- [ ] **1698** - TODO(pskelton): this cancels out the above - is this intended
  - The armageddon panic block adds random x/y velocity in [-50,50), and the very next check zeroes horizontal velocity whenever lengthSqr < 400 (|v| < 20) on non-steep ground, so most panic impulses are discarded; deciding whether to reorder or gate the clamp needs a check of the original 0x47B0C9 behaviour and how visible actor panic should be.
- [ ] **1778** - TODO(Nik-RE-dev): why there's logic for loading maps that are not listed in info?
  - The MAP_INVALID branch feeds an empty filename into OutdoorLocation::Initialize, which then returns false without loading anything (and OutdoorLocation::CreateDebugLocation is itself dead code); MapStats::GetMapInfo really can return MAP_INVALID for an unknown save/transition map name, so removing the branch requires deciding what unknown-map error handling should be (assert vs. fallback).

### `src/Engine/Graphics/OutdoorTerrain.cpp`

- [ ] **54** - TODO(captainurist): this function had some code that would push the party -60 units down when on a water tile AND not water-walking, but this isn't enabled in the game. I tried it, and it actually looks good, as if the party is actually a bit submerged and swimming. The only problem is that party would be jerked up upon coming ashore, and this just looks ugly. Find a way to reimplement this properly.
  - The TODO itself states the blocker: a submerge offset applied inside heightByPos causes a visual pop on leaving water, so it needs a designed solution (smoothed party-z offset outside the terrain query) rather than restoring the old code.

### `src/Engine/Graphics/Overlays.h`

- [ ] **10** - TODO(pskelton): Overlays in MM7/ MM8 are blank so most of this isnt used. MM6 does have overlays - investigate if needed
  - Requires MM6 data/original-game research to decide whether ActiveOverlayList/OverlayList should be kept and wired up or deleted; nothing to code until that is answered.

### `src/Engine/Graphics/ParticleEngine.cpp`

- [ ] **125** - TODO(captainurist): checking pMiscTimer->isPaused(), then using pEventTimer->uTimeElapsed?
  - The line mixes two timers (pause gate from the animation timer, delta from the event timer) and picking either one changes when particles freeze in dialogues/turn-based mode - it needs a decision about intended vanilla semantics, not just an edit.

### `src/Engine/Graphics/Renderer/BaseRenderer.cpp`

- [ ] **204** - OpenEnroth do not support mods and data patches right now. TODO(pskelton): data patch So the changes are in C++ code. But it better be placed in lua scripts or binary data patches.
  - Blocked on there being a mod/data-patch (or Lua) mechanism at all - the hardcoded lightEmittingDecorations set can only move once that subsystem and its data format are designed.

### `src/Engine/Graphics/Renderer/OpenGLRenderer.cpp`

- [ ] **639** - TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
  - Both here and BspRenderer.cpp:62 silently drop textureless faces that the BSP pass still counted as visible; deciding the right behaviour needs map-data investigation into which faces these are and what the original engine drew for them.
- [ ] **763** - TODO(pskelton): should this call drawworld instead??
  - MakeViewportScreenshot hand-duplicates Engine::drawWorld (Engine.cpp:132-166) minus the bloodsplat pass and PauseGameDrawing check, and there is no drawworld entry point on the renderer; unifying them requires deciding whether the renderer may call engine-level world drawing and whether the thumbnail should include decals.
- [ ] **933** - TODO(pskelton): to camera?
  - Moving _set_3d_projection_matrix into Camera3D means the camera (which today exposes only a glm::mat3x3 ViewMatrix and CreateViewMatrixAndProjectionScale) would own the GL-style glm::mat4 projmat that the renderer feeds to every shader uniform block - an ownership decision, not a mechanical move.
- [ ] **942** - TODO(pskelton): to camera?
  - Same ownership question for _set_3d_modelview_matrix: it builds a left-handed glm::lookAtLH viewmat from camera yaw/pitch, which overlaps Camera3D::CreateViewMatrixAndProjectionScale; merging the two matrix conventions into the camera is a design call about who owns render matrices.
- [ ] **958** - TODO(pskelton): to camera?
  - _set_ortho_projection also issues glViewport, so it cannot move into Camera3D as-is; deciding what (if anything) belongs there requires splitting GL state from matrix math first.
- [ ] **969** - TODO(pskelton): to camera?
  - _set_ortho_modelview is a one-line identity assignment to the renderer's viewmat with nothing camera-specific in it; the TODO is a speculative question that should most likely just be dropped along with the sibling ones once matrix ownership is decided.
- [ ] **1691** - TODO(pskelton): move ?
  - SetFogParametersGL mixes config/weather/outdoor state into the renderer's shared FogUniforms and is called once from BeginScene3D; the TODO names no target, so someone has to decide whether fog state belongs in BaseRenderer, in the level/weather code, or stays here.
- [ ] **2248** - TODO(pskelton): might have to pass a texture width through for the waterr flow textures to size right and get the correct water speed
  - Speculative ('might have to') - the outbuild shader scrolls flow textures using flowtimer without knowing the per-face texture size, so confirming whether flow speed/scale is actually wrong needs in-game observation of non-standard-sized flowing textures before any code change.
- [ ] **2290** - TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
  - Faces with no texture are silently skipped here while BSPRenderer still treats them as visible; deciding what they should look like (invisible, placeholder, or original-engine behaviour) needs research into the original renderer plus a matching change in BSPRenderer.
- [ ] **2382** - TODO(pskelton): any instances where animTime is not consistent would need checking
  - The atlas pre-registration walks an animation by repeatedly adding animationFrameLength(animationId), assuming every frame of a texture animation has the same duration; validating that assumption requires auditing the texture frame table data, not a code change.
- [ ] **2806** - TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
  - Requires determining whether textureless non-portal faces actually occur in game data and what vanilla draws for them; BSPRenderer.cpp:60-62 has the same skip plus a "does the below happen?" question, so it is research before code.
- [ ] **3262** - TODO(yoctozepto, pskelton): we should probably try to handle these faces as they are otherwise marked as visible (see also BSPRenderer)
  - Duplicate of line 2806 in the indoor decal loop; same open question about whether untextured faces exist and what vanilla renders for them, so it can't be coded until that is researched.
- [ ] **3514** - TODO(pskelton): Needs testings on other platforms
  - Pure verification work: whether the window.reload_tex path (default true) is actually needed on Windows/macOS/Linux/Android after issue #199 stopped reproducing on Windows — needs running on real hardware, not a code change.

### `src/Engine/Graphics/Renderer/OpenGLRenderer.h`

- [ ] **171** - TODO(captainurist): reserve the buffers here, then adjust the flush checks to use reserved size?
  - Speculative: the vertex containers are now plain std::vectors and there are no size-based flush checks left in OpenGLRenderer.cpp to "adjust", so what the second half should become (per-frame capacity budget? forced flush at capacity?) is an open call rather than a defined change.

### `src/Engine/Graphics/SpriteEnums.h`

- [ ] **5** - TODO(captainurist): somehow most flags aren't used. Figure out why.
  - BILLBOARD_TRANSPARENT/GLOWING/STONED/0X200 are written (SpriteEnumFunctions.h, Outdoor.cpp:776/792) but never read by any renderer path, so this is an investigation into how vanilla used them and whether the effects are handled elsewhere before anything can be coded or deleted.

### `src/Engine/Graphics/Sprites.cpp`

- [ ] **188** - TODO(pskelton): investigate and fix properly - dragon breath is missing last two frames??
  - The `while (v4->sprites[0] == NULL) --v4;` band-aid hides a mismatch between animationLength/frameLength stepping and the frames InitializeSprite actually filled; identifying the real cause requires inspecting sprites.lod/sft data for that animation, not just editing this loop.

### `src/Engine/Graphics/Viewport.cpp`

- [ ] **168** - TODO: WTF? 184 / 185 qbits are associated with Tatalia's Mercenery Guild Harmondale raids. Are these about castle's tapestries ?
  - The code faithfully mirrors vanilla (sets QBIT_SPLITTER_FOUND=184 / QBIT_REMOVE_FEAR_FOUND=185 on item pickup) while the quest table text for those bits says something else; resolving it needs quests.txt/original-binary research, and the enum names may just need correcting.

### `src/Engine/Localization.cpp`

- [ ] **41** - TODO(captainurist): should be moved to localization files eventually
  - All strings today come from game data (global.txt via engine->resources()); these 11 OE-authored strings need a new resource format, load path and per-language selection mechanism that doesn't exist yet, so the format decision comes first.
- [ ] **218** - TODO(captainurist): why not LSTR_ACOLYTE?
  - Profession 42 (Acolyte) maps to LSTR_CHAPLAIN while profession 53 (Acolyte2) takes LSTR_ACOLYTE; UIDialogue.cpp:133 carries the same doubt ("or Chaplain? mb discrepancy between game versions?"), so it needs checking the original binary/localized data before touching.
- [ ] **299** - TODO(captainurist): Not currently used anywhere
  - Explains a commented-out block giving SKILL_CLUB a name/description; enabling it is blocked on the same missing OE-string infrastructure as line 41 (LSTR_CLUB does not exist in MM7 global.txt) plus a decision about exposing the hidden club skill — note the dead code still uses the old CHARACTER_SKILL_CLUB name.

### `src/Engine/Objects/Actor.cpp`

- [ ] **258** - TODO(captainurist): why do we ignore passed skill mastery?
  - spell_skill feeds CalcSpellDamage, shrink duration and the GM-only ice-blast/shrinking-ray branches in SpriteObject.cpp, so passing uSkillMastery.mastery() instead of MASTERY_NONE changes how hard every monster spell hits the party - needs vanilla verification that the original really zeroed it.
- [ ] **277** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster-cast Haste gives 1h+points at novice/expert but only 40+2*points minutes at master (shorter), and none of it matches the party-side formula in CastSpellInfo.cpp:812; picking correct numbers requires vanilla disassembly/spell-table research.
- [ ] **338** - TODO(captainurist): why do we ignore passed skill mastery?
  - Same question for monster Meteor Shower sprites: the mastery is dropped so CalcSpellDamage always sees MASTERY_NONE; changing it is a combat-balance change that needs original-game confirmation.
- [ ] **387** - TODO(captainurist): why do we ignore passed skill mastery?
  - Same question for monster Sparks sprites; the fix is one line but it alters party damage taken and must be checked against vanilla behavior first.
- [ ] **414** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Air Shield jumps from ~1h at novice/expert/master to 64+points HOURS at GM, an inconsistency that can only be resolved by researching the original monster-AI duration table.
- [ ] **437** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Stoneskin repeats the same 1h vs 64+points-hours GM discontinuity as Air Shield; the intended values are unknown without vanilla research.
- [ ] **460** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Bless mixes fromHours(1) with a 4+N*points minute term unlike the party-side formula; deciding the right constants is an original-game behavior question.
- [ ] **507** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Heroism duplicates the odd Bless formula (1h + 4 + N*points minutes); no way to fix without knowing the vanilla numbers.
- [ ] **530** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Hammerhands lasts fromHours(realPoints) with no mastery switch at all, unlike every neighbouring spell; the correct formula needs vanilla research.
- [ ] **547** - TODO(pskelton): This is a vanilla bug - monsters with instant targeting spells can't actually use them - #1246
  - Documents a known vanilla bug (issue #1246) where Paralyze falls through to Dispel Magic; fixing it means implementing instant-target monster casting and deciding whether to deviate from the original game.
- [ ] **569** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Day of Protection uses 64+N*points minutes where the party version uses hours; the intended values require vanilla disassembly.
- [ ] **596** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Hour of Power repeats the 64+N*points-minutes pattern; same unresolved question about the original formula.
- [ ] **644** - TODO(captainurist): why do we ignore passed skill mastery?
  - Same question for monster Sharpmetal sprites; forcing MASTERY_NONE changes damage via CalcSpellDamage and needs vanilla confirmation before touching.
- [ ] **671** - TODO(Nik-RE-dev): calculation of duration is strange
  - Monster Pain Reflection mixes fromMinutes(64) with fromSeconds(5*30*points), an obviously transcribed-from-asm expression whose intended meaning needs vanilla research.
- [ ] **1405** - TODO(pskelton): Consider adding potshots if no LOS
  - Proposes new AI behavior (ranged attacks while pursuing without line of sight) that does not exist in vanilla; it is a gameplay design decision, not a code fix.
- [ ] **1757** - TODO(pskelton): vanilla behaviour but does it make sense to drop all carried treasure
  - Whether a resurrected (turned friendly) monster should keep its gold/treasure rolls is an exploit-vs-fidelity gameplay decision, not a mechanical fix.
- [ ] **1814** - TODO(pskelton): looks incomplete - sounds meant to change depending on actor size
  - All four shrink-power branches call playSound identically, so implementing the intent requires knowing what vanilla varied (pitch/sample) and likely per-source pitch support in AudioPlayer; only the trivial alternative (collapse the dead switch) is mechanical.
- [ ] **2627** - TODO(captainurist): this check makes no sense, it fails only for monsters that are: stunned && non-friendly && recovering && far from target && don't have missile attack. Seriously?
  - The trailing `|| uAIState != Stunned` makes the whole condition almost always true, so the block is effectively unguarded; recovering the intended condition requires vanilla disassembly and any change gates every monster attack in the game.
- [ ] **3045** - TODO: should be changed to GetActual* equivalents?
  - Swapping getSkillValue(SKILL_BLASTER) for getActualSkillValue at lines 3047-3048 is one line but changes blaster to-hit rolls by including item/buff skill bonuses; whether vanilla used the base or effective skill has to be verified first.
- [ ] **3596** - TODO(pskelton): Only cure below half health?
  - Currently monsters cast Power Cure at any hp below max; restricting it to below 50% is an AI-tuning decision that needs original-game verification.
- [ ] **4363** - with above position should we either retry or fallback to spawn pos. Cant just remove actors as they appear killed see #2074
  - Asks what to do when the randomized spawn position lands in another sector or too far off the floor - retry N times, clamp to spawn->position, or something else; the obvious answer (remove the actor) is already ruled out by issue #2074, so a placement policy has to be decided first.

### `src/Engine/Objects/ActorEnums.h`

- [ ] **31** - TODO(captainurist): medusas should stone other actors?
  - ACTOR_BUFF_STONED is an MM6 leftover that nothing in MM7 applies; whether medusa gaze should petrify other actors is a gameplay/original-game-behavior question, not a coding task.
- [ ] **76** - TODO(captainurist): what is this? Document properly.
  - AIState::Disabled is set in OutdoorLocation::InitalizeActors based on ACTOR_UNKNOW7/ACTOR_UNKNOW11 and GetAlertStatus(), and treated as 'skip entirely' by render/collision/AI; documenting it properly means first reverse-engineering the vanilla alert-status spawn mechanic and the two still-unnamed actor attribute flags.

### `src/Engine/Objects/Character.cpp`

- [ ] **583** - TODO(Nik-RE-dev): is check for boots correct?
  - CanRepair gives the Alchemist hireling `item.type() >= ITEM_TYPE_BOOTS`, which overlaps the Armorer's isArmor() range (ARMOUR..BOOTS) and also covers wands/reagents/potions/scrolls/books/gold/gems; the likely intent is >= ITEM_TYPE_RING, but confirming needs the vanilla MM7 hireling-repair rules.
- [ ] **1062** - TODO(captainurist): these are some weird casts to CharacterAttributeType
  - (Attribute)dmg_type compares DamageType values (max 12) against ATTRIBUTE_RESIST_MIND=14 / BODY=15 / SPIRIT=33, so the lich-immunity branch is unreachable dead code - a faithful port of the same vanilla bug. Cleaning the casts up means deciding whether to enable lich immunity to mind/body/spirit damage (a real gameplay change) or to just document the vanilla bug.
- [ ] **1066** - TODO(_): determine if spirit resistance should be handled by body res. modifier
  - Requires researching how vanilla MM7 resolved spirit damage resistance (ATTRIBUTE_RESIST_SPIRIT=33 exists but the switch below maps DAMAGE_SPIRIT to it directly); no code change is determinable without that answer.
- [ ] **1199** - TODO(captainurist): returns not used - should luck attribute affect?
  - The StealResult return is indeed ignored at the only call site (Actor.cpp:1251) - dropping it is trivial - but the substantive half, whether luck should factor into monster stealing (it currently affects neither StealFromActor nor StealFromShop), is an original-game balance question.
- [ ] **1392** - TODO(captainurist): can't break wands b/c they are not regular items. Makes little in-game sense IMO.
  - SPECIAL_ATTACK_BREAK_ANY filters on isRegular(itemId), which excludes wands (and quest items/artifacts); widening it is a gameplay change that needs a call on whether to keep vanilla item-breaking rules.
- [ ] **1407** - TODO(captainurist): This can break a wetsuit, and this looks like vanilla behavior. But the code in SPECIAL_ATTACK_BREAK_ANY can't break a wetsuit. Huh.
  - Documents an inconsistency between the type-based BREAK_ARMOR filter (ITEM_TYPE_ARMOUR/SHIELD, which includes ITEM_QUEST_WETSUIT) and the isRegular-based BREAK_ANY filter; resolving it requires deciding which vanilla behavior is canonical rather than writing code.
- [ ] **1422** - TODO(captainurist): why doesn't this affect wands?
  - Same class of question as the BREAK_ANY TODO: SPECIAL_ATTACK_BREAK_WEAPON only matches BOW/SINGLE_HANDED/TWO_HANDED, and whether equipped wands should be breakable is an original-game behavior decision. (Unrelated: the `if (!itemstobreaklist.empty()) return 0;` at line 1427 is an inverted guard, but that is not what this TODO is about.)
- [ ] **2559** - TODO(_): move the individual implementations to attribute classes once possible ?? check
  - Depends on an 'attribute class' abstraction that does not exist in the codebase - Attribute is a flat enum and GetSkillBonus/GetItemsBonus switch over it; someone has to design that abstraction before any of this can be moved.
- [ ] **3079** - TODO(Nik-RE-dev): no CanAct check?
  - The scroll and book branches gate on playerAffected->CanAct() while the potion branch does not; whether potions should be drinkable by an unconscious/paralyzed target is deliberate vanilla behavior in MM7 and needs confirming before adding a guard.
- [ ] **5699** - TODO(pskelton): check - should this be able to forget a skill '0' or min of '1'
  - SubtractSkillByEvent clamps to level 1 via std::max(1, ...); whether an event-driven skill subtraction may un-learn a skill entirely is a vanilla-behavior question that has to be researched in the original game/decompile first.
- [ ] **5700** - TODO(pskelton): check - should this modify mastery as well
  - The mastery half decoded by CombinedSkillValue::fromJoinedUnchecked(subSkillValue) is currently discarded; deciding whether events should also demote mastery needs original-game research since it changes gameplay.
- [ ] **5961** - TODO(captainurist): should this include other projectile attacks? Vanilla only checks arrows.
  - Whether the GM-unarmed 1%-per-point evade should apply to all monster projectiles instead of only SPRITE_PROJECTILE_ARROW is a gameplay/vanilla-fidelity decision, not a coding problem.
- [ ] **5962** - TODO(captainurist): should we check that the character is actually unarmed? Vanilla doesn't check, apparently.
  - Adding an "is actually unarmed" precondition to the unarmed evade would deviate from vanilla; needs a decision plus original-game verification before it can be coded.
- [ ] **6087** - TODO(captainurist): another weird default.
  - Unlike the twin at 5947 this branch is live (the final else means the caster can be OBJECT_Character, i.e. a party arrow hitting a party member), so picking the right DamageType (likely DAMAGE_PHYSICAL) changes resistance math and needs vanilla verification.
- [ ] **6369** - TODO(captainurist): +8???
  - The value lands in CastSpellInfo::overrideSoundId and then in SpriteObject::uSoundID, which is savegame-serialized and read only as `uSoundID & 8` (SpriteObject.cpp:605) to detect laser/wand projectiles; establishing the field's real vanilla semantics (flag bit + character index vs sound id) is reverse-engineering work before any rename/split is safe.
- [ ] **6753** - TODO(pskelton): firespike meant to remain permanantly??
  - Whether the per-character fire spike cast counter is supposed to persist (a twin TODO sits at Engine.cpp:1049, and Party.cpp:788 also zeroes it) can only be settled by checking original-game behavior for fire spike permanence.
- [ ] **6774** - TODO(captainurist): isn't it weird that promotions aren't included in comparisons here?
  - matchesAttackPreference compares raw classType, so promoted characters (e.g. Cavalier vs Knight) never match monster attack preferences; changing that alters monster targeting and requires vanilla research to know what is correct.

### `src/Engine/Objects/Character.h`

- [ ] **63** - TODO(pskelton): maybe expand so we can handle different strength enchantments
  - Every regen source in Engine.cpp:1330-1375 currently contributes exactly 1 and the non-stacking mode caps regen at one tick's worth regardless of count; supporting per-enchantment strengths needs a decision on the strength values (MM7 has no such data) and on how non-stacking should compose them.
- [ ] **279** - TODO(captainurist): check all usages, most should be using getActualSkillValue.
  - getSkillValue returns the raw pActiveSkills entry; auditing the ~20 non-test call sites (Actor.cpp, NPCTopics, UICharacter, UIPopup, EvtInterpreter) means deciding per site whether vanilla used base or bonus-inclusive skill, each change being a gameplay change.

### `src/Engine/Objects/ItemEnumFunctions.h`

- [ ] **310** - TODO(captainurist): Values 1-19 are supposedly mapped to ItemType, but are not used in MM7. Figure out if this is an MM6 remnant, if not then just drop.
  - treasureType comes from game data (Monsters.cpp:497 parses it from the monster table), so deciding whether the 1-19 fallback can be dropped requires checking MM6/MM7 data files for such values - research, not coding.

### `src/Engine/Objects/MonsterEnumFunctions.cpp`

- [ ] **19** - TODO(captainurist): a bit weird that all the monsters belong to RACE_HUMAN.
  - Race only has HUMAN/ELF/GOBLIN/DWARF (it is the character race enum), so dragons/oozes/elementals have no honest value; fixing needs a decision on a RACE_NONE sentinel plus what NPCTable.cpp:189 (generating NPC data from a monster type) and Actor.cpp:704 (peasant-vs-peasant hostility) should do with it.
- [ ] **158** - TODO(captainurist): Tbh the table still makes little sense. Why are Angels bounty-huntable in Celeste?
  - The bountyHuntableMask table was autogenerated from since-deleted codegen and then hand-edited; deciding the 'correct' contents requires researching the original MM7 town-hall bounty lists, not a code change.

### `src/Engine/Objects/SpriteObject.cpp`

- [ ] **130** - TODO(captainurist): TBH this makes no sense, investigate
  - Color(rand(0x100), rand(0x100), 0, 0) gives a random red/green line particle with zero blue and zero alpha (alpha is then overwritten at ParticleEngine.cpp:184); working out what the original binary encoded in that packed diffuse word is reverse-engineering work, not a code fix.
- [ ] **137** - TODO(captainurist): TBH this makes no sense, investigate
  - Identical random-diffuse expression as line 130, on the OBJECT_DESC_TRAIL_PARTICLE branch; same original-binary research is needed, and both should be resolved together.
- [ ] **499** - TODO(Nik-RE-dev): is this correct?
  - Asks whether jumping back into the flying/collision block for a sprite that has already been placed on the floor and damped matches the original; answering requires comparing against the original binary's 0x47257F loop, not just reading the C++.
- [ ] **605** - TODO(captainurist): item->uSoundID & 8 checks for laser projectiles, wtf...
  - SpriteObject::uSoundID is only ever written as 0 by OE (Party.cpp:915, Engine.cpp:1301) and otherwise comes from a savegame, so bit 3 is an original-binary encoding; replacing the check with an explicit spriteId test needs reverse-engineering what the original packed into that field.

### `src/Engine/Party.cpp`

- [ ] **242** - check if this condition really should be here. it is equal to the original source but still seems kind of weird
  - In switchToNextActiveCharacter the `i > 0` guard makes character slot 0 fall through to the speed-based selection instead of being picked round-robin; deciding whether to keep vanilla's asymmetry or "fix" it is a gameplay/vanilla-parity call that also changes trace-recorded behavior.
- [ ] **668** - We overwrite the random timing from the PORTRAIT_NORMAL branch here. Doesn't seem intentional!
  - Moving the `animationDuration()` override into the else-branch is two lines, but whether it even changes anything depends on the portrait frame table data (animationDuration returns 0 for index 0), and changing portrait timing is a deliberate divergence from vanilla that needs a decision plus in-game verification.

### `src/Engine/PartyEnums.h`

- [ ] **121** - is there other flag values? Maybe just drop this enum.
  - Only PARTY_FLAGS_2_RUNNING is known and it is used in 8 places, but uFlags2 is round-tripped as a full int32 through the vanilla save (EntitySnapshots.cpp:589/704), so replacing the enum with a bool would silently drop unknown vanilla bits - it needs research into what MM7 actually stored in flags2.

### `src/Engine/PriceCalculator.cpp`

- [ ] **91** - blaster price is 0 and we can sell it for 1 gold because of the code below, but this is probably not how it should work
  - What a shop should pay for a zero-value item is an original-game behavior question (does vanilla refuse the sale, or pay 1 gold?), so it can't be coded without deciding/verifying the intended rule.

### `src/Engine/Resources/ResourceManager.cpp`

- [ ] **13** - on exception: Error(localization->str(LSTR_MIGHT_AND_MAGIC_VII_IS_HAVING_TROUBLE), localization->str(LSTR_REINSTALL_NECESSARY)); but we can't use localization object here cause it's not yet initialized.
  - Reporting a missing/corrupt events.lod with a localized message is blocked on a startup-ordering decision - either bootstrap localization before resources, or accept an untranslated early-failure path - which is an engine init architecture call, not a local edit.

### `src/Engine/SaveLoad.cpp`

- [ ] **131** - disable flashing for all books until we save state to savegame file
  - bFlashQuestBook/Autonotes/History are plain globals with no home in the vanilla-compatible SaveGame_MM7 layout, so persisting them requires first deciding how OpenEnroth extends the vanilla save format (or which unused vanilla field to reuse).

### `src/Engine/Snapshots/CompositeSnapshots.cpp`

- [ ] **183** - unused for some reason.
  - Both branches assign additionalBitmapId = -1 and the field is never read anywhere (Indoor.h:84/176 carry the same "why is this unused?" TODO), so answering means researching what vanilla did with BLV face-extra additional bitmaps before deciding to implement it or delete the field.
- [ ] **304** - vanilla MM7 only allocated memory for 500 actors, 1000 sprite objects, and 20 chests at runtime. We should either cap these or fail gracefully on save to maintain compatibility.
  - The TODO itself offers two mutually exclusive policies (cap the containers at load/spawn time vs. refuse to save an over-full level); picking one is a compatibility/gameplay decision that also determines where the check must live.
- [ ] **595** - vanilla MM7 only allocated memory for 500 actors, 1000 sprite objects, and 20 chests at runtime. We should either cap these or fail gracefully on save to maintain compatibility.
  - Same open policy question as the indoor snapshot at line 304, duplicated for the outdoor delta; both sites must be resolved together with whatever cap/failure mechanism is chosen.

### `src/Engine/Snapshots/EntitySnapshots.cpp`

- [ ] **171** - do we need to check for overflows here?
  - Vec3i -> Vec3s narrowing on the save path: needs a project-wide policy for save-time narrowing failures (assert, throw, or clamp) and knowledge of the reachable coordinate ranges before any code can be written; there is no existing narrowing helper in the codebase.
- [ ] **184** - do we need to check for overflows here?
  - Same open question for Vec3f -> Vec3s (float truncation plus int16 range); the answer must be the same policy chosen for the Vec3i overload at line 171.
- [ ] **197** - do we need to check for overflows here?
  - Vec3f -> Vec3i: only huge/NaN float coordinates overflow here, so deciding whether a check is worth it (and what it should do when saving) is the same unresolved policy question as lines 171/184.
- [ ] **210** - do we need to check for overflows here?
  - BBoxi -> BBoxs_MM7 narrowing, six components; blocked on the same save-time narrowing policy decision as the Vec overloads above.
- [ ] **229** - do we need to check for overflows here?
  - BBoxf -> BBoxs_MM7 narrowing; same unresolved policy question, and float bounding boxes make the "what counts as overflow" definition (rounding vs truncation) part of the decision.
- [ ] **347** - Can drop? Need to check that vanilla doesn't choke here.
  - dst->name is a Pointer_MM7 (a raw pointer field in the vanilla save); writing 0/1 instead of a real pointer can only be validated by loading an OpenEnroth save in the original MM7 binary, which is exactly the research the TODO asks for.

### `src/Engine/Snapshots/EntitySnapshots.h`

- [ ] **141** - 2 bytes in MM6?
  - SpriteFrame_MM6 is only used as the base of SpriteFrame_MM7 (no MM6 sprite frame table is loaded today), so confirming the MM6 field width requires MM6 data files or an external struct reference - it cannot be settled from this repo.
- [ ] **866** - I feel this was supposed to be named numConvexFaces?
  - The field is always 0 and unused, so the rename itself is free, but choosing between numCylinderFaces and numConvexFaces (the name used for BSPModelData_MM7 at line 1214) needs an authoritative source such as MMExtension's BLV sector struct.
- [ ] **924** - TODO(captainurist): What is this? Could be minimum shade?
  - Asks what ODMFace_MM7::shadeType (values 0..24 in MM7 data, currently unused by the engine) means; answering needs reverse-engineering of the original renderer or map data analysis, there is no code change to make.

### `src/Engine/Snapshots/TableSerialization.cpp`

- [ ] **94** - TODO(captainurist): there are duplicate ids in the sounds array, look into it.
  - Deciding whether last-wins (current behaviour of _mapSounds[sound.soundId] = sound) or first-wins is correct requires inspecting the actual MM7 SOUNDS.LOD table; the neighbouring TileTable code chose first-wins for the same situation, so the two need reconciling based on data.

### `src/Engine/Spells/CastSpellInfo.cpp`

- [ ] **232** - TODO(Nik-RE-dev): does scrolls must fail?
  - The curse-based 50% failure check is skipped whenever overrideSkillValue is set, which lumps scrolls together with temple/NPC casts; whether vanilla MM7 lets a cursed character fail a scroll cast is an original-behaviour question, not a coding one.
- [ ] **279** - TODO(pskelton): was pParty->uPartyHeight / 2
  - Records a deliberate deviation - projectiles now spawn at pParty->height/3 (64 units) instead of vanilla's /2 (96); picking the right value needs in-game comparison of blaster/spell projectile origin against the original, not a code fix.
- [ ] **507** - TODO(pskelton): was pParty->uPartyHeight / 2
  - Same deviation as line 279, here for acid burst / blades / flying fist / toxic cloud; resolving all three occurrences (279, 507, 2708) together requires verifying the projectile spawn height against vanilla.
- [ ] **2038** - TODO: is this correct? Spell description says that spell effect is infinite.
  - GM Berserk is given a hard-coded 1 hour while GM Charm at line 644 fakes "infinite" with Duration::fromYears(1); choosing between matching the spell description, vanilla's actual behaviour, or the Charm hack is a gameplay decision needing original-game research.
- [ ] **2708** - TODO(pskelton): was pParty->uPartyHeight / 2
  - Third copy of the party-height deviation (Dark Sharpmetal spawn point); same vanilla-comparison research needed as lines 279 and 507.
- [ ] **3153** - TODO: if no more place for spells in queue then spell is just ignored? Need assert?
  - pushCastSpellInfo returns -1 when all 10 CAST_SPELL_QUEUE_SIZE slots are busy and the caller silently drops the spell; choosing between assert (crash risk in release-tested paths), a log warning, or keeping silent needs a call on whether overflow is reachable in normal play.
- [ ] **3217** - TODO(Nik-RE-dev): why recovery time is set here?
  - spellTargetPicked unconditionally slaps 300 ticks of recovery on the caster before the spell is even processed; whether this is vanilla anti-double-cast behaviour or a decompilation artifact needs original-game research, and removing it would change combat pacing.

### `src/Engine/Spells/Spells.cpp`

- [ ] **147** - TODO(captainurist): Looks like this is a flaming arrow spell, should map to SPRITE_PROJECTILE_FLAMING_ARROW?
  - SPELL_101 is asserted never to be cast (CastSpellInfo.cpp:242) and nothing assigns it to a sprite, so the mapping cannot be validated in-game; deciding it needs research into what spell 101 was in MM6/MM7 data.
- [ ] **522** - TODO(captainurist): move these flag patches into the patched data tables instead of hardcoding them here.
  - There is no data-patching layer at all today (ResourceManager only reads events.lod verbatim; grep for "patched" finds nothing but these two TODOs), so this is blocked on designing and building an overridable data-table mechanism first.
- [ ] **539** - TODO(captainurist): these strings are translatable; move this fix into the patched data files.
  - Same missing patched-data-file infrastructure as the flag patch above, and additionally requires deciding how per-localization text overrides are shipped (the current replaceAll only fixes the English wording).
- [ ] **854** - TODO(captainurist): wtf? Looks like a quick hack for some bug.
  - The `armageddon_timer > 417_ticks -> reset to 0` early-out is copied from vanilla; deciding whether it can go requires reverse-engineering why vanilla capped the timer (armageddon is started at 256_ticks in CastSpellInfo.cpp:2904, so the branch looks unreachable unless a save carries a larger value).
- [ ] **865** - TODO(pskelton): ignore if pEventTimer->uTimeElapsed is zero?
  - Whether armageddonForceCount should stop decrementing on zero-dt frames is a behavior question tied to the same frame-rate dependence as the TODO below, and any change alters recorded game-test traces.

### `src/Engine/Tables/AutonoteTable.cpp`

- [ ] **32** - TODO(captainurist): We have "0" in autonote texts, and it gets shown. Find out what it was supposed to be.
  - The code already works around it ("0" is mapped to an empty string); what remains is data archaeology on vanilla autonote.txt to learn what those placeholder rows meant, which cannot be coded without that answer.

### `src/Engine/Tables/HouseTable.cpp`

- [ ] **70** - TODO(captainurist): Is this right and not Merc Guild (18)?
  - Needs MM6 research: whether MM6's "Mercenary Guild" houses behave as a town hall or as HOUSE_TYPE_MERCENARY_GUILD (=18, itself annotated as "not a mercenary guild" in HouseEnums.h:607) can only be settled against MM6 data/behavior.
- [ ] **77** - TODO(captainurist): We don't check if int is in range. A better way would be to deal away with enums entirely, and just use typed ids. Do this once we iron out the details of how #mm6 enums will be handled by the engine. Also apply to other table parsers.
  - Explicitly deferred until the #mm6 enum strategy is decided; replacing HouseId/MapId/QuestBit enums with typed ids would ripple through every table parser and all consumers of those enums.

### `src/Engine/Tables/NPCTable.cpp`

- [ ] **295** - TODO(captainurist): Caching in kinda wrong? Revisit when working on #mm6. See "O Ho! %13! Er, %13. I think. Whatever..."
  - The static cache keys only on the first letter and ignores gender (a hit can index the other gender's name vector), but the function is unreachable in MM7 (%13 is MM6-only per the header doc), so the correct caching semantics can only be decided while doing #mm6.

### `src/Engine/Time/Time.h`

- [ ] **80** - TODO(captainurist): #time This is something to look at, we have comparisons with GameTime() in the code, they are not the same as Valid().
  - Requires deciding the meaning of a zero/negative Time (unset vs valid vs expired) across mixed idioms still in the tree — operator bool at UIHouses.cpp:339, isValid() at Processor.cpp:238, and direct `<= GetPlayingTime()` comparisons — before any of the call sites can be normalized.

### `src/Engine/TurnEngine/TurnEngine.cpp`

- [ ] **882** - TODO(captainurist): why is this not handled?
  - HOSTILITY_LONG is the top of the escalation ladder the other cases promote into, so plausibly nothing is missing, but confirming that vanilla didn't intend a distinct long-range case here needs comparison against the original turn-based AI code.

### `src/Engine/mm7_data.cpp`

- [ ] **367** - TODO(captainurist): Something's off here. PORTRAIT_TALK is a special animated portrait that's played back for the length of the speech sound.
  - Deciding whether PORTRAIT_TALK belongs in the SPEECH_ID_ITEM_WEAK variant list (mixed with static mouth-shape portraits) requires establishing how vanilla played the talking animation versus picking a random variant — a behavior question, not a code change.

### `src/GUI/GUIEnums.h`

- [ ] **243** - TODO(captainurist): check against this: https://github.com/GrayFace/MMExtension/blob/4d6600f164315f38157591d7f0307a86594c22ef/Scripts/Core/ConstAndBits.lua#L875
  - Requires pulling the external MMExtension constant list and reconciling it with the ScreenType values, including the ~10 unnamed ones (SCREEN_6, SCREEN_20, SCREEN_63/64/67...); the naming/verification is research, and renames then ripple through the UI code.

### `src/GUI/GUIFont.cpp`

- [ ] **314** - this return is very sus.
  - WrapText's `return_on_carriage` is never passed true by any caller, so any string containing a '\r' right-justify tag silently skips wrapping entirely and is returned verbatim; deciding what wrapping should mean for right-justified segments requires checking vanilla FitTextInAWindow semantics, not just a code change.

### `src/GUI/GUIWindow.cpp`

- [ ] **437** - TODO(pskelton): check tickcount usage here
  - The blink uses raw wall-clock `platform->tickCount() % 1000 > 500`; whether it should instead follow a game/misc timer (and thus freeze when the game is paused) is a behavior question to settle against vanilla, there is no obviously-correct code change.
- [ ] **805** - TODO(captainurist): ^ and what about night?
  - Vanilla only ships morning/day/evening greeting strings (LSTR_MORNING/DAY_LOWERCASE/EVENING = 395-397) and folds night into "evening"; adding a night greeting would need a new string and a deliberate deviation from original behavior.

### `src/GUI/UI/Houses/MercenaryGuild.cpp`

- [ ] **58** - TODO(captainurist): #mm6 this is MM6 legacy, and this decompiled code doesn't look sane. Reimplement properly once we get to MM6.
  - The block sits behind `if (false)` after two `assert(false)`s and dereferences a null `short *v6`; the TODO explicitly defers it until MM6 support exists, so it is blocked on upstream MM6 data/behavior research rather than on coding.

### `src/GUI/UI/Houses/Tavern.cpp`

- [ ] **144** - TODO(pskelton): check this behaviour
  - Asks whether silently doing `setActiveToFirstCanAct()` when no character is active (an anti-null-deref hack added by the port, also at Temple.cpp:128, Training.cpp:123, NPCTopics.cpp:441) matches vanilla, which likely just refused the dialogue - resolving it needs original-game behavior research.

### `src/GUI/UI/Houses/Temple.cpp`

- [ ] **126** - TODO(pskelton): check this behaviour
  - Identical question to Tavern.cpp:144 - the `setActiveToFirstCanAct()` fallback when the whole party is incapacitated is a port-added guard, and whether temple heal/donate should instead be unavailable is a vanilla-behavior decision.

### `src/GUI/UI/Houses/TownHall.cpp`

- [ ] **75** - TODO(Nik-RE-dev): game resources does not contain such sounds for town halls
  - playHouseSound() derives the SoundId arithmetically from the room's uRoomSoundId, and for town-hall rooms the resulting "not enough gold" sample simply is not in the game data, so the call is a silent no-op; resolving it means deciding whether to substitute another sound or drop the call, which needs vanilla-behaviour research.
- [ ] **186** - TODO(captainurist): what do we do with exceptions inside fmt?
  - fmt::sprintf is fed a format string that comes from game data (pNPCTopics[352].pText), so a malformed/translated string throws fmt::format_error out of the draw path; this is one instance of a project-wide policy question (also at Localization.h:28, NPCTopics.cpp:396, UICharacter.cpp:855, UIPopup.cpp:1198/1214) that needs a decision on a non-throwing sprintf wrapper before any site can be fixed.

### `src/GUI/UI/Houses/Training.cpp`

- [ ] **121** - TODO(pskelton): check this behaviour
  - The `if (!hasActiveCharacter()) setActiveToFirstCanAct()` "avoid nzi" hack is repeated in Tavern.cpp:145, Temple.cpp:127 and NPCTopics.cpp:439; deciding what should happen when nobody in the party can act requires comparing against the original game rather than just coding.

### `src/GUI/UI/NPCTopics.cpp`

- [ ] **396** - TODO(captainurist): what if fmt throws?
  - Same fmt-exception policy question as TownHall.cpp:186 — the format string is pNPCTopics[666].pText from game data; blocked on choosing a non-throwing sprintf strategy for all data-driven format strings.
- [ ] **439** - TODO(pskelton): check this behaviour
  - Same forced-active-character workaround as Training.cpp:121 — joinGuildOptionString() dereferences activeCharacter() unconditionally afterwards, so what a guild should say when no character can act has to be researched against the original game.

### `src/GUI/UI/UIBranchlessDialogue.cpp`

- [ ] **41** - TODO(Nik-RE-dev): this code related to text input in MM6/MM8, revisit this functionality when it's time to support it.
  - The block is `#if 0`-ed out and explicitly parked until MM6/MM8 support lands; it cannot be actioned before that engine work is scheduled.

### `src/GUI/UI/UICharacter.cpp`

- [ ] **855** - TODO(captainurist): fmt can throw
  - Ten fmt::sprintf calls formatting pAwards[...].pText from game data; same unresolved policy question as TownHall.cpp:186 — once a non-throwing sprintf helper exists this becomes a mechanical replace, but the decision is the blocker.

### `src/GUI/UI/UIDialogue.cpp`

- [ ] **72** - TODO(Nik-RE-dev): this looks like checks for NPC that only talk if party has enough fame which is a thing only for MM8 if I remember correctly
  - The `#if 0` block is decompiled fame/contact-count logic whose meaning is guessed at; deciding whether to reimplement or delete it requires MM8 behaviour research.

### `src/GUI/UI/UIGame.cpp`

- [ ] **1099** - TODO(_) fix these and move them up before the window check loop.
  - The code it refers to is a large commented-out block that the surrounding comment itself describes as unreachable and bugged even in the original; reviving it means deciding from scratch what hover hints should do on shop/house inventory screens, not restoring the old pointer arithmetic.

### `src/GUI/UI/UIHouses.cpp`

- [ ] **371** - TODO(pskelton): check this behaviour
  - Entering a magic guild with no active character silently calls pParty->setActiveToFirstCanAct() and then tests that character's guild-membership award, i.e. it mutates party state and picks an arbitrary character; deciding what is right needs original-game research into whose membership the guild greeting checks.
- [ ] **709** - TODO(captainurist): this is a weird access into pTransitionStrings, investigate & add docs
  - pTransitionStrings (465 entries, loaded from trans.txt) is indexed by MapId here but by HouseId in UITransition.cpp:144 and by a 1..11 "special message" index at UITransition.cpp:170, so working out what the table is actually keyed by requires reading trans.txt/original-game behaviour before anything can be documented or fixed.
- [ ] **815** - TODO(Nik-RE-dev): maybe need to unify selectColor for all dialogue
  - Call sites pass colorTable.Sunflower (shops, training, generic NPC dialogue) vs colorTable.PaleCanary (bank, temple, tavern, town hall, transport, magic guild); picking one changes visible UI colors and needs verification against the original game to know which is correct.

### `src/GUI/UI/UIPopup.cpp`

- [ ] **1199** - TODO(captainurist): write it out in game seconds?
  - Whether recovery time should be shown in engine ticks (as the original game does, via the localized "Recovery time: %d" string) or converted to game seconds is a UX/fidelity decision, not a coding problem.

### `src/GUI/UI/UISpell.cpp`

- [ ] **53** - TODO(pskelton): why is position / size different to normal character buttons
  - Both coordinate sets come from the original binary ({52,422} r=35 here vs {61,424} 31x40 in GUIWindow::CreateCharacterButtons) and BUTTON_TYPE_CHARACTER is hit-tested as a circle whose own check is flagged as bugged (Mouse.cpp:252), so aligning them requires deciding the true portrait hit area against the original game.

### `src/GUI/UI/UITransition.cpp`

- [ ] **144** - TODO(Nik-RE-dev): is this correct?
  - _transitionStringId is a raw HouseId used to index pTransitionStrings, whose 465 entries are indexed as MapId in UIHouses.cpp:709 and as a 1..11 special-message index below - and HouseId reaches 601, so this can even read out of bounds; resolving it needs the trans.txt keying to be established from the original game first.
- [ ] **152** - TODO(Nik-RE-dev): if message is special then no video when entering indoor?
  - Asks whether the original game really skips OpenHouseMovie for the 11 special-message dungeons; answering requires checking original-game behaviour/disassembly, not a local code change.
- [ ] **195** - TODO(captainurist): mm7 map names never starts with ' ', what is this check?
  - The `getTeleportMap().starts_with(' ')` guard is presumably an MM6-era or event-script marker convention; deciding whether it is dead requires auditing every setTeleportMap caller (events, savegames, MM6 data) rather than just reading this function.

### `src/Io/InputEnumFunctions.cpp`

- [ ] **9** - these should be localizable.
  - The same MM_DEFINE_ENUM_SERIALIZATION_FUNCTIONS table is both the keybindings config-file format and the Controls-menu label source (GetDisplayName at line 84, used by UIGame.cpp:353), so display names must first be split from the stable serialization names, and there is no localized source for them - LocalizationEnums.h has no LSTR for "Forward"/"Always Run"/etc., so a whole new i18n mechanism for engine-owned strings has to be decided on.
- [ ] **173** - these should be localizable?
  - Same coupling as the InputAction table plus a genuinely open question (note the '?') - these strings are round-tripped through TryParseDisplayName/config deserialization, and it is unclear whether key names like "F1"/"LSTICK RIGHT" should be translated at all.

### `src/Io/KeyboardActionMapping.cpp`

- [ ] **36** - maybe we need to split InputActions to sets by WindowType so guarantee of only one InputAction per key is restored.
  - Requires designing a context/window scoping model for input actions (which action set is active per screen) and reworking the config layout and conflict detection accordingly; the TODO itself is still phrased as "maybe".

### `src/Io/KeyboardInputHandler.cpp`

- [ ] **116** - why next frame?
  - Answering why UIMSG_CycleCharacters is posted via addMessageNextFrame on the paused path (instead of addMessageCurrentFrame like every neighbouring action) needs archaeology of the original message-pump ordering plus in-game verification that switching does not double-trigger; no code change is defined by the comment.
- [ ] **269** - why next frame?
  - Same open question for UIMSG_CastQuickSpell - it is deferred a frame while the sibling UIMSG_Attack in the same if/else is posted for the current frame; deciding whether the delay is load-bearing requires reproducing the original quick-cast timing behaviour.
- [ ] **302** - why next frame?
  - Third instance of the same question, on the gameplay-path UIMSG_CycleCharacters; resolving all three is one investigation into the frame/message-queue semantics (GUIMessageQueue::swapFrames) plus behaviour testing, not a codeable task as written.

### `src/Io/Mouse.cpp`

- [ ] **123** - consider this in future - hide cursor when holding item
  - The code is a single commented-out platform->setCursorShown(false), but DrawPickedItem() draws the item at pickedItemOffset from the hotspot, so hiding the OS cursor loses the precise pointer - this is a UX call about matching the original game, not a coding problem.

### `src/Library/Binary/CommonSerialization.h`

- [ ] **142** - can we do this better?
  - The guard only knows that each element needs at least one byte, so tightening it requires either a per-type serialized-size trait (impossible for variable-size elements) or switching to incremental deserialization without an upfront resize - an API-level decision for the whole binary serialization layer.

### `src/Library/FileSystem/Interface/FileSystem.h`

- [ ] **44** - I still think most of FSs should inherit from ProxyFS.
  - An unresolved architectural opinion about the class hierarchy (which of Lowercase/Masking/Sub/Merging/Mounting should be ProxyFileSystem subclasses) with no stated criterion - it needs a decision before any code can be written.
- [ ] **46** - Masking is for the portable mode. Mask out non-relevant parts from the corresponding FSs.
  - MaskingFileSystem currently has no users outside its own tests; wiring it into FileSystemStarter for portable mode first requires deciding exactly which paths must be hidden when the data dir and the user dir are the same folder.

### `src/Library/FileSystem/Merging/MergingFileSystem.h`

- [ ] **55** - think about smth like a displayPriority for _displayPath? Basically a FS that you want to forward displayPath calls to if there are conflicts (no files exist / multiple files exist).
  - An explicit "think about" API-shape question - whether priority belongs on the merging FS constructor, per-base, or is subsumed by the multi-path API from the sibling TODO, has to be settled first.

### `src/Library/Json/Tests/Json_ut.cpp`

- [ ] **138** - nlohmann json just chokes on this! fix upstream & uncomment.
  - Blocked on an upstream nlohmann/json fix for serializing a vector of a type whose to_json throws; the local work is only re-enabling one EXPECT_ANY_THROW line once (and if) upstream changes.

### `src/Library/Platform/Sdl/SdlEventLoop.cpp`

- [ ] **171** - mouse events are floats in SDL3
  - Preserving the float precision means carrying Pointf through PlatformMouseEvent into Mouse/GUIWindow hit-testing, which is integer-based end to end; the alternative is to accept the truncation, so this needs a decision about the UI coordinate system rather than a code change.
- [ ] **192** - We're on SDL3 now and this needs to be retested.
  - The "every 2nd click is a double click" hack and its inverted Android variant can only be resolved by empirically re-checking SDL3's clicks counter on desktop and on an Android device - it is a testing task, not a code change.

### `src/Library/Platform/Sdl/SdlPlatform.cpp`

- [ ] **53** - SDL orientation code turned out to be buggy and works only before window creation, hardcode only landscape modes there for now.
  - Replacing the Android landscape-only hint hack depends on SDL's orientation handling actually working after window creation, which needs verification against current SDL3 on a real device (and possibly an upstream fix) before any code can be written.

### `src/Library/Platform/Sdl/SdlPlatformSharedState.cpp`

- [ ] **105** - The assert below triggers with @pskelton's xbox controller, and it shouldn't trigger. Figure out why it happens and either fix the bug in our logic, or add a comment here describing what's happening. // assert(!_gamepadById.contains(id)); // if (SdlGamepad *result = _gamepadById[id].get()) // return result;
  - Requires reproducing a duplicate SDL_JoystickID on specific hardware (an Xbox pad) to decide whether SDL legitimately re-reports the same instance id or our add/remove bookkeeping is broken; note the current fallthrough `_gamepadById.emplace(id, make_unique<SdlGamepad>(..., gamepad.release(), ...))` silently constructs and then destroys a second SdlGamepad when the id already exists, so the "fix" depends on which behavior is correct.

### `src/Library/Trace/EventTrace.cpp`

- [ ] **222** - well, nlohmann json is retarded in that it chokes if we throw exceptions inside to_json calls for individual elements. Fix upstream? Note: there is an example in tests to reproduce.
  - The repro is the commented-out `EXPECT_ANY_THROW(to_json(json, v))` in src/Library/Json/Tests/Json_ut.cpp:138-141; fixing it means either an upstream nlohmann patch or deciding on a local workaround (pre-validating elements before serializing containers), which is a decision, not a code change.

### `src/Library/Trace/EventTrace.h`

- [ ] **14** - this should go to Core/, not Library/,
  - There is no src/Core layer in the tree at all - EventTrace mixes generic platform-event serialization with game-specific state (hp/mp/equipment), so acting on this means inventing and populating a whole new layer between Library/ and Engine/.

### `src/Media/Audio/AudioPlayer.cpp`

- [ ] **193** - do we need to reinstate this optimisation? dropped to allow better sound tracing
  - The commented-out `engine->config->settings.SoundLevel.value() < 1` early return was removed on purpose so traces record sound calls at zero volume; restoring it is a tradeoff decision between tracing fidelity and skipping work, not a mechanical change.
- [ ] **272** - Vanilla sounds like it does unique id but as exclusives
  - Current code deliberately uses playUniqueSoundId (skip if already playing) for actor sounds with a comment explaining the anti-cacophony rationale; switching to exclusive (stopSoundId then replay, as for SOUND_MODE_EXCLUSIVE) needs original-game behavior research plus a call on whether to keep the deliberate deviation.

### `src/Media/Audio/OpenALAudioDataSource.h`

- [ ] **10** - this middleware class is temporary because Media API is not fully ready to properly support current use cases
  - Removing the wrapper means redesigning IAudioDataSource (src/Media/AudioDataSource.h) so that GPU/AL-side buffer ownership and the linkSource step fit the base interface - there is no stated target design to code against.

### `src/Media/Audio/SoundEnums.h`

- [ ] **225** - played back at speech volume?
  - The SOUND_MODE_HOUSE_SPEECH branch in AudioPlayer::playSound (AudioPlayer.cpp:235-238) is the only mode that never calls SetVolume(uVoiceVolume); whether it should needs checking what the original game does with house NPC greetings.

### `src/Scripting/GameBindings.cpp`

- [ ] **35** - exposing the info/stats of a character this way might suggest we should expose the Character class directly to lua. The idea is to wait till we'll talk about serious modding/scripting and not taking a direction upfront
  - Explicitly parked until the project decides on a modding/scripting API direction - choosing between the LuaItemQueryTable string-key model and sol usertype bindings for Character is the whole task.

### `src/Utility/Streams/FileInputStream.cpp`

- [ ] **93** - !canThrow => log OR attach
  - When _close is called from the destructor (canThrow == false) the fclose error is dropped, but the `utility` target cannot log - library_logger links against utility, so there is a dependency cycle; resolving this needs a decision on an error-reporting mechanism for the bottom layer (injected sink, or attaching to the in-flight exception).

### `src/Utility/Streams/FileOutputStream.cpp`

- [ ] **71** - !canThrow => log OR attach
  - Same blocked layering problem as FileInputStream: the fclose failure during a non-throwing close has nowhere to go because Utility sits below library_logger.

### `src/Utility/String/Encoding.h`

- [ ] **6** - #cpp26 can we just use std::text_encoding?
  - Blocked on the toolchain (project is on C++23) and on the fact that std::text_encoding only *identifies* encodings - it does no transcoding - so the ztd::text-based conversion functions stay regardless, and ENCODING_BYTES has no std counterpart; the answer to the question is most likely "only for the enum".

### `src/Utility/String/TransparentFunctors.h`

- [ ] **8** - TODO(captainurist): This is not needed in C++26 as we'll have transparent operator[].
  - Not actionable on the current toolchain: the repo sets CMAKE_CXX_STANDARD 23, so the TransparentString workaround is still required; deleting it (mechanical, ~10 files: Config.h, ConfigSection.h, FileSystemTrie.h, FsmTypes.h, EnumSerializer.h, OverlaySystem.h, TileGenerator.h, CodeGenMap.h plus the unit test) can only happen after the project moves to C++26 and all supported compilers ship transparent operator[].

### `test/Bin/GameTest/GameTests_1500.cpp`

- [ ] **261** - TODO(pskelton): fixing cylinder collisions means we can no longer get into this hole
  - The whole body of GAME_TEST(Issues, Issue1665) is now commented out so the test asserts nothing; resolving it requires deciding whether the secret room under the bed in Castle Harmondale should still be reachable with cylinder collisions (original-game behavior research) and then either fixing collisions or re-recording a new trace with fresh expectations.

### `test/Testing/Game/CommonTapeRecorder.h`

- [ ] **108** - TODO(pskelton): Tape will be spammy as it is recording everything.
  - A caveat with no stated action: allGUIWindowsText records every string passed to GUIWindow::DrawText/DrawTitleText (GUIWindow.cpp:322,361) on every frame, and all 9 call sites immediately flatten+filter it; a fix means deciding on an API (e.g. a predicate parameter threaded through TestController::recordFunctionTape/TestCallObserver::record, or per-frame dedup) rather than just editing this file.

## Unclear (1)

_Meaning could not be determined from the code._

### `src/Engine/Objects/Character.cpp`

- [ ] **2747** - TODO(pskelton): ?? needs changing - check behavious
  - GetMultiplierForSkillLevel just maps mastery to one of four caller-supplied multipliers and returns 0 for MASTERY_NONE, which looks correct; the comment gives no hint what behaviour is suspect, and the function was reformatted since (commit 3879ee820b4) without the concern being spelled out.

