#pragma once

#include <string>
#include <optional>

#include "Engine/Objects/SpriteEnums.h"
#include "Engine/Objects/ItemEnums.h"
#include "Engine/Objects/CharacterEnums.h"

#include "Library/Geometry/Point.h"

#include "Utility/IndexedArray.h"

struct ItemData {
    std::string iconName = ""; // Item's icon as shown in character inventory, stored in icons.lod.
    std::string name = ""; // Item's base name, w/o any enchantments.
    std::string unidentifiedName = ""; // Unidentified name.
    std::string description = ""; // Item description that's shown on right click.
    int baseValue = 0; // Item's base value in gold coins.
    SpriteId spriteId = SPRITE_NULL; // Sprite id that's used when item is dropped.
    Pointi paperdollAnchorOffset; // Paperdoll offset for the item sprite when equipped, relative to the item type-specific anchor point.
    ItemType type = ITEM_TYPE_NONE; // Item type.
    Skill skill = SKILL_MISC; // Skill associated with the item. E.g. `CHARACTER_SKILL_SWORD`.
    int damageDice = 0; // Damage dice, base AC for armor.
    int damageRoll = 0; // Always 1 for armor.
    int damageMod = 0; // Also base charges for wands. Additional AC for armor, effective AC = damageDice+damageMod.
    int reagentPower = 0; // Reagent power, only for reagents. Originally was stored in `damageDice`.
    ItemRarity rarity = RARITY_COMMON; // Item rarity.
    ItemEnchantment specialEnchantment = ITEM_ENCHANTMENT_NULL; // Special enchantment, applied only to `RARITY_SPECIAL` items.
    std::optional<Attribute> standardEnchantment; // Standard (attribute) enchantment, applied only to `RARITY_SPECIAL` items.
    int standardEnchantmentStrength = 0; // Strength of the standard enchantment above.
    IndexedArray<int, ITEM_TREASURE_LEVEL_FIRST_RANDOM, ITEM_TREASURE_LEVEL_LAST_RANDOM> uChanceByTreasureLvl = {{}}; // Weights for seeing this item in random loot by treasure level.
    int identifyAndRepairDifficulty = 0; // Value that the id item skill is checked against, 0 means always identified. Also used for item repair checks.
};
