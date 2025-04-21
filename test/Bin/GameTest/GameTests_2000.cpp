#include "Testing/Game/GameTest.h"

#include "Engine/MapEnums.h"
#include "Engine/Spells/Spells.h"

GAME_TEST(Issues, Issue2018a) {
    // We've deleted assertions checked mana requirements and recovery times in the PR that fixed #2018.
    // They are here as tests now.
    for (const SpellData &data : {pSpellDatas[SPELL_WATER_LLOYDS_BEACON], pSpellDatas[SPELL_WATER_TOWN_PORTAL]}) {
        EXPECT_EQ(data.mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE], data.mana_per_skill[CHARACTER_SKILL_MASTERY_EXPERT]);
        EXPECT_EQ(data.mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE], data.mana_per_skill[CHARACTER_SKILL_MASTERY_MASTER]);
        EXPECT_EQ(data.mana_per_skill[CHARACTER_SKILL_MASTERY_NOVICE], data.mana_per_skill[CHARACTER_SKILL_MASTERY_GRANDMASTER]);
        EXPECT_EQ(data.recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE], data.recovery_per_skill[CHARACTER_SKILL_MASTERY_EXPERT]);
        EXPECT_EQ(data.recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE], data.recovery_per_skill[CHARACTER_SKILL_MASTERY_MASTER]);
        EXPECT_EQ(data.recovery_per_skill[CHARACTER_SKILL_MASTERY_NOVICE], data.recovery_per_skill[CHARACTER_SKILL_MASTERY_GRANDMASTER]);
    }
}

GAME_TEST(Issues, Issue2018b) {
    // Scrolls of Town Portal and Lloyd's Beacon did consume mana or assert when cast by a character with insufficient mana.
    // - Game tweaked to have 6 LB scrolls and 4 TP scrolls on char 4.
    // - Party is on Emerald Isle but has unlocked TP destination Erathia via console.
    // - char 4 tweaked to Archmage with 10GM in Water Magic (5 LB slots, no fail), but SP is insufficient to cast normally.
    // - char 2 is a Thief w/o SP.
    // - char 4 casts LB set from scroll, then char 2 casts TP to Erathia, then char 4 casts LB to get back.
    auto mapTape = tapes.map();
    auto manaTape = charTapes.mps();
    auto scrollsLBTape = tapes.totalItemCount(ITEM_SCROLL_LLOYDS_BEACON);
    auto scrollsTPTape = tapes.totalItemCount(ITEM_SCROLL_TOWN_PORTAL);
    auto statusTape = tapes.statusBar();
    test.playTraceFromTestData("issue_2018.mm7", "issue_2018.json");
    EXPECT_EQ(mapTape, tape(MAP_EMERALD_ISLAND, MAP_ERATHIA, MAP_EMERALD_ISLAND));
    EXPECT_EQ(scrollsLBTape.frontBack(), tape(6, 4)); // Used 2 Lloyd's out of 6, ignore intervening steps from pickup and r-click.
    EXPECT_EQ(scrollsTPTape.frontBack(), tape(4, 2)); // Also used 2 Town Portal because it fails once on the Thief.
    EXPECT_EQ(manaTape.front(), manaTape.back()); // No mana was spent.
    EXPECT_CONTAINS(statusTape, "Spell failed"); // TP failed once - might break on retrace.
}
