#pragma once

#include <cassert>
#include <cstdint>

#include "Engine/Spells/SpellEnums.h"

#include "Utility/Workaround/ToUnderlying.h"
#include "Utility/Segment.h"

enum class Condition : uint32_t {
    CONDITION_CURSED = 0,
    CONDITION_WEAK = 1,
    CONDITION_SLEEP = 2,
    CONDITION_FEAR = 3,
    CONDITION_DRUNK = 4,
    CONDITION_INSANE = 5,
    CONDITION_POISON_WEAK = 6,
    CONDITION_DISEASE_WEAK = 7,
    CONDITION_POISON_MEDIUM = 8,
    CONDITION_DISEASE_MEDIUM = 9,
    CONDITION_POISON_SEVERE = 10,
    CONDITION_DISEASE_SEVERE = 11,
    CONDITION_PARALYZED = 12,
    CONDITION_UNCONSCIOUS = 13,
    CONDITION_DEAD = 14,
    CONDITION_PETRIFIED = 15,
    CONDITION_ERADICATED = 16,
    CONDITION_ZOMBIE = 17,
    CONDITION_GOOD = 18,
};
using enum Condition;

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterSkillMastery {
    CHARACTER_SKILL_MASTERY_NONE = 0,
    CHARACTER_SKILL_MASTERY_NOVICE = 1,
    CHARACTER_SKILL_MASTERY_EXPERT = 2,
    CHARACTER_SKILL_MASTERY_MASTER = 3,
    CHARACTER_SKILL_MASTERY_GRANDMASTER = 4,

    CHARACTER_SKILL_MASTERY_FIRST = CHARACTER_SKILL_MASTERY_NOVICE,
    CHARACTER_SKILL_MASTERY_LAST = CHARACTER_SKILL_MASTERY_GRANDMASTER
};
using enum CharacterSkillMastery;

inline Segment<CharacterSkillMastery> SkillMasteries() {
    return Segment(CHARACTER_SKILL_MASTERY_FIRST, CHARACTER_SKILL_MASTERY_LAST);
}

enum class CharacterBuff {
    CHARACTER_BUFF_RESIST_AIR = 0,
    CHARACTER_BUFF_BLESS = 1,
    CHARACTER_BUFF_RESIST_BODY = 2,
    CHARACTER_BUFF_RESIST_EARTH = 3,
    CHARACTER_BUFF_FATE = 4,
    CHARACTER_BUFF_RESIST_FIRE = 5,
    CHARACTER_BUFF_HAMMERHANDS = 6,
    CHARACTER_BUFF_HASTE = 7,
    CHARACTER_BUFF_HEROISM = 8,
    CHARACTER_BUFF_RESIST_MIND = 9,
    CHARACTER_BUFF_PAIN_REFLECTION = 10,
    CHARACTER_BUFF_PRESERVATION = 11,
    CHARACTER_BUFF_REGENERATION = 12,
    CHARACTER_BUFF_SHIELD = 13,
    CHARACTER_BUFF_STONESKIN = 14,
    CHARACTER_BUFF_ACCURACY = 15,
    CHARACTER_BUFF_ENDURANCE = 16,
    CHARACTER_BUFF_INTELLIGENCE = 17,
    CHARACTER_BUFF_LUCK = 18,
    CHARACTER_BUFF_STRENGTH = 19,
    CHARACTER_BUFF_PERSONALITY = 20,
    CHARACTER_BUFF_SPEED = 21,
    CHARACTER_BUFF_RESIST_WATER = 22,
    CHARACTER_BUFF_WATER_WALK = 23,

    CHARACTER_BUFF_FIRST = CHARACTER_BUFF_RESIST_AIR,
    CHARACTER_BUFF_LAST = CHARACTER_BUFF_WATER_WALK
};
using enum CharacterBuff;

/*  301 */
enum class CharacterSpeech {
    SPEECH_NONE = 0,
    SPEECH_KILL_WEAK_ENEMY = 1,
    SPEECH_KILL_STRONG_ENEMY = 2,
    SPEECH_STORE_CLOSED = 3,
    SPEECH_TRAP_DISARMED = 4,
    SPEECH_TRAP_EXPLODED = 5,
    SPEECH_AVOID_DAMAGE = 6,
    SPEECH_ID_ITEM_WEAK = 7,
    SPEECH_ID_ITEM_STRONG = 8,
    SPEECH_ID_ITEM_FAIL = 9,
    SPEECH_REPAIR_SUCCESS = 10,
    SPEECH_REPAIR_FAIL = 11,
    SPEECH_SET_QUICK_SPELL = 12,
    SPEECH_CANT_REST_HERE = 13,
    SPEECH_SKILL_INCREASE = 14,
    SPEECH_NO_ROOM = 15,
    SPEECH_POTION_SUCCESS = 16,
    SPEECH_POTION_EXPLODE = 17,
    SPEECH_DOOR_LOCKED = 18,
    SPEECH_WONT_BUDGE = 19,
    SPEECH_CANT_LEARN_SPELL = 20,
    SPEECH_LEARN_SPELL = 21,
    SPEECH_GOOD_DAY = 22,
    SPEECH_GOOD_EVENING = 23,
    SPEECH_DAMAGED = 24,
    SPEECH_WEAK = 25,
    SPEECH_FEAR = 26,
    SPEECH_POISONED = 27,
    SPEECH_DISEASED = 28,
    SPEECH_INSANE = 29,
    SPEECH_CURSED = 30,
    SPEECH_DRUNK = 31,
    SPEECH_UNCONSCIOUS = 32,
    SPEECH_DEAD = 33,
    SPEECH_PETRIFIED = 34,
    SPEECH_ERADICATED = 35,
    SPEECH_DRINK_POTION = 36,
    SPEECH_READ_SCROLL = 37,
    SPEECH_NOT_ENOUGH_GOLD = 38,
    SPEECH_CANT_EQUIP = 39,
    SPEECH_ITEM_BROKEN = 40,
    SPEECH_SP_DRAINED = 41,
    SPEECH_AGING = 42,
    SPEECH_SPELL_FAILED = 43,
    SPEECH_DAMAGED_PARTY = 44,
    SPEECH_TIRED = 45,
    SPEECH_ENTER_DUNGEON = 46,
    SPEECH_LEAVE_DUNGEON = 47,
    SPEECH_BADLY_HURT = 48,
    SPEECH_CAST_SPELL = 49,
    SPEECH_SHOOT = 50,
    SPEECH_ATTACK_HIT = 51,
    SPEECH_ATTACK_MISS = 52,
    SPEECH_BEG = 53,
    SPEECH_BEG_FAIL = 54,
    SPEECH_THREAT = 55,
    SPEECH_THREAT_FAIL = 56,
    SPEECH_BRIBE = 57,
    SPEECH_BRIBE_FAIL = 58,
    SPEECH_NPC_DONT_TALK = 59,
    SPEECH_FOUND_ITEM = 60,
    SPEECH_HIRE_NPC = 61,
    SPEECH_62 = 62,   // unknown
    SPEECH_LOOK_UP = 63,
    SPEECH_LOOK_DOWN = 64,
    SPEECH_YELL = 65,
    SPEECH_FALLING = 66,
    SPEECH_PACKS_FULL = 67,
    SPEECH_TAVERN_DRINK = 68,
    SPEECH_TAVERN_GOT_DRUNK = 69,
    SPEECH_TAVERN_TIP = 70,
    SPEECH_TRAVEL_HORSE = 71,
    SPEECH_TRAVEL_BOAT = 72,
    SPEECH_SHOP_IDENTIFY = 73,
    SPEECH_SHOP_REPAIR = 74,
    SPEECH_ITEM_BUY = 75,
    SPEECH_ALREADY_INDENTIFIED = 76,
    SPEECH_ITEM_SOLD = 77,
    SPEECH_SKILL_LEARNED = 78,
    SPEECH_WRONG_SHOP = 79,
    SPEECH_SHOP_RUDE = 80,
    SPEECH_BANK_DEPOSIT = 81,
    SPEECH_TEMPLE_HEAL = 82,
    SPEECH_TEMPLE_DONATE = 83,
    SPEECH_HELLO_HOUSE = 84,
    SPEECH_SKILL_MASTERY_INC = 85,
    SPEECH_JOINED_GUILD = 86,
    SPEECH_LEVEL_UP = 87,
    SPEECH_88 = 88,  // unknown
    SPEECH_89 = 89,  // unknown
    SPEECH_90 = 90,  // unknown
    SPEECH_STAT_BONUS_INC = 91,
    SPEECH_STAT_BASE_INC = 92,
    SPEECH_QUEST_GOT = 93,
    SPEECH_94 = 94,  // unknown
    SPEECH_95 = 95,  // unknown
    SPEECH_AWARD_GOT = 96,  // award
    SPEECH_97 = 97,  // unknown
    SPEECH_AFRAID_SILENT = 98,
    SPEECH_CHEATED_DEATH = 99,  // zombie/ death groan
    SPEECH_IN_PRISON = 100,
    SPEECH_101 = 101,  // unknown
    SPEECH_PICK_ME = 102,
    SPEECH_AWAKEN = 103,
    SPEECH_ID_MONSTER_WEAK = 104,
    SPEECH_ID_MONSTER_STRONG = 105,
    SPEECH_ID_MONSTER_FAIL = 106,
    SPEECH_LAST_MAN_STANDING = 107,
    SPEECH_NOT_ENOUGH_FOOD = 108,
    SPEECH_DEATH_BLOW = 109,
    SPEECH_110 = 110,  // unknown

    SPEECH_FIRST = SPEECH_NONE,
    SPEECH_LAST = SPEECH_110
};
using enum CharacterSpeech;

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterRace {
    CHARACTER_RACE_HUMAN = 0,
    CHARACTER_RACE_ELF = 1,
    CHARACTER_RACE_GOBLIN = 2,
    CHARACTER_RACE_DWARF = 3,

    CHARACTER_RACE_FIRST = CHARACTER_RACE_HUMAN,
    CHARACTER_RACE_LAST = CHARACTER_RACE_DWARF
};
using enum CharacterRace;

enum class ClassSkillAffinity : uint8_t {
    CLASS_SKILL_DENIED = 0,
    CLASS_SKILL_AVAILABLE = 1,
    CLASS_SKILL_PRIMARY = 2
};
using enum ClassSkillAffinity;

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterSkillType : int8_t {
    CHARACTER_SKILL_INVALID = -1,
    CHARACTER_SKILL_STAFF = 0,
    CHARACTER_SKILL_SWORD = 1,
    CHARACTER_SKILL_DAGGER = 2,
    CHARACTER_SKILL_AXE = 3,
    CHARACTER_SKILL_SPEAR = 4,
    CHARACTER_SKILL_BOW = 5,
    CHARACTER_SKILL_MACE = 6,
    CHARACTER_SKILL_BLASTER = 7,
    CHARACTER_SKILL_SHIELD = 8,
    CHARACTER_SKILL_LEATHER = 9,
    CHARACTER_SKILL_CHAIN = 10,
    CHARACTER_SKILL_PLATE = 11,
    CHARACTER_SKILL_FIRE = 12,
    CHARACTER_SKILL_AIR = 13,
    CHARACTER_SKILL_WATER = 14,
    CHARACTER_SKILL_EARTH = 15,
    CHARACTER_SKILL_SPIRIT = 16,
    CHARACTER_SKILL_MIND = 17,
    CHARACTER_SKILL_BODY = 18,
    CHARACTER_SKILL_LIGHT = 19,
    CHARACTER_SKILL_DARK = 20,
    CHARACTER_SKILL_ITEM_ID = 21,
    CHARACTER_SKILL_MERCHANT = 22,
    CHARACTER_SKILL_REPAIR = 23,
    CHARACTER_SKILL_BODYBUILDING = 24,
    CHARACTER_SKILL_MEDITATION = 25,
    CHARACTER_SKILL_PERCEPTION = 26,
    CHARACTER_SKILL_DIPLOMACY = 27,
    CHARACTER_SKILL_THIEVERY = 28,
    CHARACTER_SKILL_TRAP_DISARM = 29,
    CHARACTER_SKILL_DODGE = 30,
    CHARACTER_SKILL_UNARMED = 31,
    CHARACTER_SKILL_MONSTER_ID = 32,
    CHARACTER_SKILL_ARMSMASTER = 33,
    CHARACTER_SKILL_STEALING = 34,
    CHARACTER_SKILL_ALCHEMY = 35,
    CHARACTER_SKILL_LEARNING = 36,
    CHARACTER_SKILL_CLUB = 37, // In vanilla clubs are using separate hidden & non-upgradable skill.
    CHARACTER_SKILL_MISC = 38, // Hidden skill that's always 1. Used for wetsuits, for example.

    CHARACTER_SKILL_FIRST_VISIBLE = CHARACTER_SKILL_STAFF,
    CHARACTER_SKILL_LAST_VISIBLE = CHARACTER_SKILL_LEARNING,

    CHARACTER_SKILL_FIRST = CHARACTER_SKILL_STAFF,
    CHARACTER_SKILL_LAST = CHARACTER_SKILL_MISC,
};
using enum CharacterSkillType;

inline Segment<CharacterSkillType> allSkills() {
    return Segment(CHARACTER_SKILL_FIRST, CHARACTER_SKILL_LAST);
}

/**
 * @return                              List of skills that are visible to the player and that are stored in a savegame.
 */
inline Segment<CharacterSkillType> allVisibleSkills() {
    return Segment(CHARACTER_SKILL_FIRST_VISIBLE, CHARACTER_SKILL_LAST_VISIBLE);
}

/**
 * @return                              List of skills that are drawn in the "Armor" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allArmorSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_LEATHER, CHARACTER_SKILL_CHAIN, CHARACTER_SKILL_PLATE,
        CHARACTER_SKILL_SHIELD,  CHARACTER_SKILL_DODGE
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Weapons" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allWeaponSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_AXE,   CHARACTER_SKILL_BOW,     CHARACTER_SKILL_DAGGER,
        CHARACTER_SKILL_MACE,  CHARACTER_SKILL_SPEAR,   CHARACTER_SKILL_STAFF,
        CHARACTER_SKILL_SWORD, CHARACTER_SKILL_UNARMED, CHARACTER_SKILL_BLASTER
        // CHARACTER_SKILL_CLUB is not displayed in skills.
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Misc" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allMiscSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_ALCHEMY,      CHARACTER_SKILL_ARMSMASTER,
        CHARACTER_SKILL_BODYBUILDING, CHARACTER_SKILL_ITEM_ID,
        CHARACTER_SKILL_MONSTER_ID,   CHARACTER_SKILL_LEARNING,
        CHARACTER_SKILL_TRAP_DISARM,  CHARACTER_SKILL_MEDITATION,
        CHARACTER_SKILL_MERCHANT,     CHARACTER_SKILL_PERCEPTION,
        CHARACTER_SKILL_REPAIR,       CHARACTER_SKILL_STEALING
    };

    return result;
}

/**
 * @return                              List of skills that are drawn in the "Magic" section of the character
 *                                      screen's skills tab.
 */
inline std::initializer_list<CharacterSkillType> allMagicSkills() {
    static constexpr std::initializer_list<CharacterSkillType> result = {
        CHARACTER_SKILL_FIRE,  CHARACTER_SKILL_AIR,    CHARACTER_SKILL_WATER,
        CHARACTER_SKILL_EARTH, CHARACTER_SKILL_SPIRIT, CHARACTER_SKILL_MIND,
        CHARACTER_SKILL_BODY,  CHARACTER_SKILL_LIGHT,  CHARACTER_SKILL_DARK
    };

    return result;
}

inline CharacterSkillType skillForMagicSchool(MagicSchool school) {
    switch (school) {
    case MAGIC_SCHOOL_FIRE:     return CHARACTER_SKILL_FIRE;
    case MAGIC_SCHOOL_AIR:      return CHARACTER_SKILL_AIR;
    case MAGIC_SCHOOL_WATER:    return CHARACTER_SKILL_WATER;
    case MAGIC_SCHOOL_EARTH:    return CHARACTER_SKILL_EARTH;
    case MAGIC_SCHOOL_SPIRIT:   return CHARACTER_SKILL_SPIRIT;
    case MAGIC_SCHOOL_MIND:     return CHARACTER_SKILL_MIND;
    case MAGIC_SCHOOL_BODY:     return CHARACTER_SKILL_BODY;
    case MAGIC_SCHOOL_LIGHT:    return CHARACTER_SKILL_LIGHT;
    case MAGIC_SCHOOL_DARK:     return CHARACTER_SKILL_DARK;
    default:
        assert(false);
        return CHARACTER_SKILL_INVALID;
    }
}

inline CharacterSkillType skillForSpell(SpellId spell) {
    if (isRegularSpell(spell)) {
        return skillForMagicSchool(magicSchoolForSpell(spell));
    } else if (spell == SPELL_BOW_ARROW) {
        return CHARACTER_SKILL_BOW;
    } else if (spell == SPELL_LASER_PROJECTILE) {
        return CHARACTER_SKILL_BLASTER;
    } else {
        assert(false && "Unknown spell");
        return CHARACTER_SKILL_INVALID;
    }
}

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterClassType : uint8_t {
    CHARACTER_CLASS_KNIGHT = 0,
    CHARACTER_CLASS_CAVALIER = 1,
    CHARACTER_CLASS_CHAMPION = 2,
    CHARACTER_CLASS_BLACK_KNIGHT = 3,
    CHARACTER_CLASS_THIEF = 4,
    CHARACTER_CLASS_ROGUE = 5,
    CHARACTER_CLASS_SPY = 6,
    CHARACTER_CLASS_ASSASSIN = 7,
    CHARACTER_CLASS_MONK = 8,
    CHARACTER_CLASS_INITIATE = 9,
    CHARACTER_CLASS_MASTER = 10,
    CHARACTER_CLASS_NINJA = 11,
    CHARACTER_CLASS_PALADIN = 12,
    CHARACTER_CLASS_CRUSADER = 13,
    CHARACTER_CLASS_HERO = 14,
    CHARACTER_CLASS_VILLIAN = 15,
    CHARACTER_CLASS_ARCHER = 16,
    CHARACTER_CLASS_WARRIOR_MAGE = 17,
    CHARACTER_CLASS_MASTER_ARCHER = 18,
    CHARACTER_CLASS_SNIPER = 19,
    CHARACTER_CLASS_RANGER = 20,
    CHARACTER_CLASS_HUNTER = 21,
    CHARACTER_CLASS_RANGER_LORD = 22,
    CHARACTER_CLASS_BOUNTY_HUNTER = 23,
    CHARACTER_CLASS_CLERIC = 24,
    CHARACTER_CLASS_PRIEST = 25,
    CHARACTER_CLASS_PRIEST_OF_SUN = 26,
    CHARACTER_CLASS_PRIEST_OF_MOON = 27,
    CHARACTER_CLASS_DRUID = 28,
    CHARACTER_CLASS_GREAT_DRUID = 29,
    CHARACTER_CLASS_ARCH_DRUID = 30,
    CHARACTER_CLASS_WARLOCK = 31,
    CHARACTER_CLASS_SORCERER = 32,
    CHARACTER_CLASS_WIZARD = 33,
    CHARACTER_CLASS_ARCHAMGE = 34,
    CHARACTER_CLASS_LICH = 35,

    CHARACTER_CLASS_FIRST = CHARACTER_CLASS_KNIGHT,
    CHARACTER_CLASS_LAST = CHARACTER_CLASS_LICH
};
using enum CharacterClassType;

inline CharacterClassType getTier1Class(CharacterClassType classType) {
    return static_cast<CharacterClassType>(std::to_underlying(classType) & ~3);
}

inline CharacterClassType getTier2Class(CharacterClassType classType) {
    return static_cast<CharacterClassType>((std::to_underlying(classType) & ~3) + 1);
}

inline CharacterClassType getTier3LightClass(CharacterClassType classType) {
    return static_cast<CharacterClassType>((std::to_underlying(classType) & ~3) + 2);
}

inline CharacterClassType getTier3DarkClass(CharacterClassType classType) {
    return static_cast<CharacterClassType>((std::to_underlying(classType) & ~3) + 3);
}

inline int getClassTier(CharacterClassType classType) {
    int index = (std::to_underlying(classType) & 3);
    return index == 3 ? 3 : index + 1;
}

/**
 * Get priomotions of higher tier class relative to given one.
 *
 * Base class is of tier 1.
 * After initial promotion class becomes tier 2.
 * Tier 2 class is promoted through light or dark path to tier 3 class.
 *
 * @param classType     Character class.
 */
inline Segment<CharacterClassType> getClassPromotions(CharacterClassType classType) {
    int tier = getClassTier(classType);

    if (tier == 1) {
        return {getTier2Class(classType), getTier3DarkClass(classType)};
    } else if (tier == 2) {
        return {getTier3LightClass(classType), getTier3DarkClass(classType)};
    } else {
        return {}; // tier 3 max
    }
}

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterExpressionID : uint16_t {
    CHARACTER_EXPRESSION_INVALID = 0,
    CHARACTER_EXPRESSION_NORMAL = 1,
    CHARACTER_EXPRESSION_CURSED = 2,
    CHARACTER_EXPRESSION_WEAK = 3,
    CHARACTER_EXPRESSION_SLEEP = 4,
    CHARACTER_EXPRESSION_FEAR = 5,
    CHARACTER_EXPRESSION_DRUNK = 6,
    CHARACTER_EXPRESSION_INSANE = 7,
    CHARACTER_EXPRESSION_POISONED = 8,
    CHARACTER_EXPRESSION_DISEASED = 9,
    CHARACTER_EXPRESSION_PARALYZED = 10,
    CHARACTER_EXPRESSION_UNCONCIOUS = 11,
    CHARACTER_EXPRESSION_PETRIFIED = 12,
    CHARACTER_EXPRESSION_BLINK = 13,
    CHARACTER_EXPRESSION_WINK = 14, // some faces wink, some shrug with eyebrows
    CHARACTER_EXPRESSION_MOUTH_OPEN_RANDOM = 15, // used for random expression, slightly opens mouth
    CHARACTER_EXPRESSION_PURSE_LIPS_RANDOM = 16, // used for random expression
    CHARACTER_EXPRESSION_LOOK_UP = 17,
    CHARACTER_EXPRESSION_LOOK_RIGHT = 18,
    CHARACTER_EXPRESSION_LOOK_LEFT = 19,
    CHARACTER_EXPRESSION_LOOK_DOWN = 20,
    CHARACTER_EXPRESSION_TALK = 21,
    CHARACTER_EXPRESSION_MOUTH_OPEN_WIDE = 22,
    CHARACTER_EXPRESSION_MOUTH_OPEN_A = 23,
    CHARACTER_EXPRESSION_MOUTH_OPEN_O = 24,
    CHARACTER_EXPRESSION_NO = 25,
    CHARACTER_EXPRESSION_26 = 26,
    CHARACTER_EXPRESSION_YES = 27,
    CHARACTER_EXPRESSION_28 = 28,
    CHARACTER_EXPRESSION_PURSE_LIPS_1 = 29, // these 3 seems to produce same expression
    CHARACTER_EXPRESSION_PURSE_LIPS_2 = 30,
    CHARACTER_EXPRESSION_PURSE_LIPS_3 = 31,
    CHARACTER_EXPRESSION_32 = 32,
    CHARACTER_EXPRESSION_AVOID_DAMAGE = 33,
    CHARACTER_EXPRESSION_DMGRECVD_MINOR = 34,
    CHARACTER_EXPRESSION_DMGRECVD_MODERATE = 35,
    CHARACTER_EXPRESSION_DMGRECVD_MAJOR = 36,
    CHARACTER_EXPRESSION_SMILE = 37, // not drowning
    CHARACTER_EXPRESSION_WIDE_SMILE = 38,
    CHARACTER_EXPRESSION_SAD = 39,
    CHARACTER_EXPRESSION_CAST_SPELL = 40,

    // ?

    CHARACTER_EXPRESSION_SCARED = 46,  // like falling

    CHARACTER_EXPRESSION_54 = 54,
    CHARACTER_EXPRESSION_55 = 55,
    CHARACTER_EXPRESSION_56 = 56,
    CHARACTER_EXPRESSION_57 = 57,
    CHARACTER_EXPRESSION_FALLING = 58,

    // ?

    CHARACTER_EXPRESSION_DEAD = 98,
    CHARACTER_EXPRESSION_ERADICATED = 99,
};
using enum CharacterExpressionID;

enum class CharacterSex : uint8_t {
    SEX_MALE = 0,
    SEX_FEMALE = 1,

    SEX_FIRST = SEX_MALE,
    SEX_LAST = SEX_FEMALE,
};
using enum CharacterSex;

// TODO(pskelton): drop CHARACTER_ at start?
enum class CharacterAttributeType {
    CHARACTER_ATTRIBUTE_MIGHT = 0,
    CHARACTER_ATTRIBUTE_INTELLIGENCE = 1,
    CHARACTER_ATTRIBUTE_PERSONALITY = 2,
    CHARACTER_ATTRIBUTE_ENDURANCE = 3,
    CHARACTER_ATTRIBUTE_ACCURACY = 4,
    CHARACTER_ATTRIBUTE_SPEED = 5,
    CHARACTER_ATTRIBUTE_LUCK = 6,
    CHARACTER_ATTRIBUTE_HEALTH = 7,
    CHARACTER_ATTRIBUTE_MANA = 8,
    CHARACTER_ATTRIBUTE_AC_BONUS = 9,

    CHARACTER_ATTRIBUTE_RESIST_FIRE = 10,
    CHARACTER_ATTRIBUTE_RESIST_AIR = 11,
    CHARACTER_ATTRIBUTE_RESIST_WATER = 12,
    CHARACTER_ATTRIBUTE_RESIST_EARTH = 13,
    CHARACTER_ATTRIBUTE_RESIST_MIND = 14,
    CHARACTER_ATTRIBUTE_RESIST_BODY = 15,

    CHARACTER_ATTRIBUTE_SKILL_ALCHEMY = 16,
    CHARACTER_ATTRIBUTE_SKILL_STEALING = 17,
    CHARACTER_ATTRIBUTE_SKILL_TRAP_DISARM = 18,
    CHARACTER_ATTRIBUTE_SKILL_ITEM_ID = 19,
    CHARACTER_ATTRIBUTE_SKILL_MONSTER_ID = 20,
    CHARACTER_ATTRIBUTE_SKILL_ARMSMASTER = 21,
    CHARACTER_ATTRIBUTE_SKILL_DODGE = 22,
    CHARACTER_ATTRIBUTE_SKILL_UNARMED = 23,

    CHARACTER_ATTRIBUTE_LEVEL = 24,
    CHARACTER_ATTRIBUTE_ATTACK = 25,
    CHARACTER_ATTRIBUTE_MELEE_DMG_BONUS = 26,
    CHARACTER_ATTRIBUTE_MELEE_DMG_MIN = 27,
    CHARACTER_ATTRIBUTE_MELEE_DMG_MAX = 28,
    CHARACTER_ATTRIBUTE_RANGED_ATTACK = 29,
    CHARACTER_ATTRIBUTE_RANGED_DMG_BONUS = 30,
    CHARACTER_ATTRIBUTE_RANGED_DMG_MIN = 31,
    CHARACTER_ATTRIBUTE_RANGED_DMG_MAX = 32,
    CHARACTER_ATTRIBUTE_RESIST_SPIRIT = 33,

    CHARACTER_ATTRIBUTE_SKILL_FIRE = 34,
    CHARACTER_ATTRIBUTE_SKILL_AIR = 35,
    CHARACTER_ATTRIBUTE_SKILL_WATER = 36,
    CHARACTER_ATTRIBUTE_SKILL_EARTH = 37,
    CHARACTER_ATTRIBUTE_SKILL_SPIRIT = 38,
    CHARACTER_ATTRIBUTE_SKILL_MIND = 39,
    CHARACTER_ATTRIBUTE_SKILL_BODY = 40,
    CHARACTER_ATTRIBUTE_SKILL_LIGHT = 41,
    CHARACTER_ATTRIBUTE_SKILL_DARK = 42,
    CHARACTER_ATTRIBUTE_SKILL_MEDITATION = 43,
    CHARACTER_ATTRIBUTE_SKILL_BOW = 44,
    CHARACTER_ATTRIBUTE_SKILL_SHIELD = 45,
    CHARACTER_ATTRIBUTE_SKILL_LEARNING = 46,

    CHARACTER_ATTRIBUTE_FIRST_STAT = CHARACTER_ATTRIBUTE_MIGHT,
    CHARACTER_ATTRIBUTE_LAST_STAT = CHARACTER_ATTRIBUTE_LUCK,

    CHARACTER_ATTRIBUTE_FIRST_ENCHANTABLE = CHARACTER_ATTRIBUTE_MIGHT,
    CHARACTER_ATTRIBUTE_LAST_ENCHANTABLE = CHARACTER_ATTRIBUTE_SKILL_UNARMED
};
using enum CharacterAttributeType;

inline Segment<CharacterAttributeType> enchantableAttributes() {
    return Segment(CHARACTER_ATTRIBUTE_FIRST_ENCHANTABLE, CHARACTER_ATTRIBUTE_LAST_ENCHANTABLE);
}

inline Segment<CharacterAttributeType> statAttributes() {
    return Segment(CHARACTER_ATTRIBUTE_FIRST_STAT, CHARACTER_ATTRIBUTE_LAST_STAT);
}
