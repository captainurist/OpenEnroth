#include "ItemEnums.h"

namespace detail {

constinit const IndexedArray<SpellId, ITEM_FIRST_WAND, ITEM_LAST_WAND> spellForWand = {
    {ITEM_WAND_OF_FIRE,                SPELL_FIRE_FIRE_BOLT},
    {ITEM_WAND_OF_SPARKS,              SPELL_AIR_SPARKS},
    {ITEM_WAND_OF_POISON,              SPELL_WATER_POISON_SPRAY},
    {ITEM_WAND_OF_STUNNING,            SPELL_EARTH_STUN},
    {ITEM_WAND_OF_HARM,                SPELL_BODY_HARM},

    {ITEM_FAIRY_WAND_OF_LIGHT,         SPELL_LIGHT_LIGHT_BOLT},
    {ITEM_FAIRY_WAND_OF_ICE,           SPELL_WATER_ICE_BOLT},
    {ITEM_FAIRY_WAND_OF_LASHING,       SPELL_SPIRIT_SPIRIT_LASH},
    {ITEM_FAIRY_WAND_OF_MIND,          SPELL_MIND_MIND_BLAST},
    {ITEM_FAIRY_WAND_OF_SWARMS,        SPELL_EARTH_DEADLY_SWARM},

    {ITEM_ALACORN_WAND_OF_FIREBALLS,   SPELL_FIRE_FIREBALL},
    {ITEM_ALACORN_WAND_OF_ACID,        SPELL_WATER_ACID_BURST},
    {ITEM_ALACORN_WAND_OF_LIGHTNING,   SPELL_AIR_LIGHTNING_BOLT},
    {ITEM_ALACORN_WAND_OF_BLADES,      SPELL_EARTH_BLADES},
    {ITEM_ALACORN_WAND_OF_CHARMS,      SPELL_MIND_CHARM},

    {ITEM_ARCANE_WAND_OF_BLASTING,     SPELL_WATER_ICE_BLAST},
    {ITEM_ARCANE_WAND_OF_THE_FIST,     SPELL_BODY_FLYING_FIST},
    {ITEM_ARCANE_WAND_OF_ROCKS,        SPELL_EARTH_ROCK_BLAST},
    {ITEM_ARCANE_WAND_OF_PARALYZING,   SPELL_LIGHT_PARALYZE},
    {ITEM_ARCANE_WAND_OF_CLOUDS,       SPELL_DARK_TOXIC_CLOUD},

    {ITEM_MYSTIC_WAND_OF_IMPLOSION,    SPELL_AIR_IMPLOSION},
    {ITEM_MYSTIC_WAND_OF_DISTORTION,   SPELL_EARTH_MASS_DISTORTION},
    {ITEM_MYSTIC_WAND_OF_SHRAPMETAL,   SPELL_DARK_SHARPMETAL},
    {ITEM_MYSTIC_WAND_OF_SHRINKING,    SPELL_DARK_SHRINKING_RAY},
    {ITEM_MYSTIC_WAND_OF_INCINERATION, SPELL_FIRE_INCINERATE}
};

constinit const IndexedArray<SpellId, ITEM_FIRST_SPELL_SCROLL, ITEM_LAST_SPELL_SCROLL> spellForScroll = {
    {ITEM_SCROLL_TORCH_LIGHT,           SPELL_FIRE_TORCH_LIGHT},
    {ITEM_SCROLL_FIRE_BOLT,             SPELL_FIRE_FIRE_BOLT},
    {ITEM_SCROLL_FIRE_RESISTANCE,       SPELL_FIRE_PROTECTION_FROM_FIRE},
    {ITEM_SCROLL_FIRE_AURA,             SPELL_FIRE_FIRE_AURA},
    {ITEM_SCROLL_HASTE,                 SPELL_FIRE_HASTE},
    {ITEM_SCROLL_FIREBALL,              SPELL_FIRE_FIREBALL},
    {ITEM_SCROLL_FIRE_SPIKE,            SPELL_FIRE_FIRE_SPIKE},
    {ITEM_SCROLL_IMMOLATION,            SPELL_FIRE_IMMOLATION},
    {ITEM_SCROLL_METEOR_SHOWER,         SPELL_FIRE_METEOR_SHOWER},
    {ITEM_SCROLL_INFERNO,               SPELL_FIRE_INFERNO},
    {ITEM_SCROLL_INCINERATE,            SPELL_FIRE_INCINERATE},

    {ITEM_SCROLL_WIZARD_EYE,            SPELL_AIR_WIZARD_EYE},
    {ITEM_SCROLL_FEATHER_FALL,          SPELL_AIR_FEATHER_FALL},
    {ITEM_SCROLL_AIR_RESISTANCE,        SPELL_AIR_PROTECTION_FROM_AIR},
    {ITEM_SCROLL_SPARKS,                SPELL_AIR_SPARKS},
    {ITEM_SCROLL_JUMP,                  SPELL_AIR_JUMP},
    {ITEM_SCROLL_SHIELD,                SPELL_AIR_SHIELD},
    {ITEM_SCROLL_LIGHTNING_BOLT,        SPELL_AIR_LIGHTNING_BOLT},
    {ITEM_SCROLL_INVISIBILITY,          SPELL_AIR_INVISIBILITY},
    {ITEM_SCROLL_IMPLOSION,             SPELL_AIR_IMPLOSION},
    {ITEM_SCROLL_FLY,                   SPELL_AIR_FLY},
    {ITEM_SCROLL_STARBURST,             SPELL_AIR_STARBURST},

    {ITEM_SCROLL_AWAKEN,                SPELL_WATER_AWAKEN},
    {ITEM_SCROLL_POISON_SPRAY,          SPELL_WATER_POISON_SPRAY},
    {ITEM_SCROLL_WATER_RESISTANCE,      SPELL_WATER_PROTECTION_FROM_WATER},
    {ITEM_SCROLL_ICE_BOLT,              SPELL_WATER_ICE_BOLT},
    {ITEM_SCROLL_WATER_WALK,            SPELL_WATER_WATER_WALK},
    {ITEM_SCROLL_RECHARGE_ITEM,         SPELL_WATER_RECHARGE_ITEM},
    {ITEM_SCROLL_ACID_BURST,            SPELL_WATER_ACID_BURST},
    {ITEM_SCROLL_ENCHANT_ITEM,          SPELL_WATER_ENCHANT_ITEM},
    {ITEM_SCROLL_TOWN_PORTAL,           SPELL_WATER_TOWN_PORTAL},
    {ITEM_SCROLL_ICE_BLAST,             SPELL_WATER_ICE_BLAST},
    {ITEM_SCROLL_LLOYDS_BEACON,         SPELL_WATER_LLOYDS_BEACON},

    {ITEM_SCROLL_STUN,                  SPELL_EARTH_STUN},
    {ITEM_SCROLL_SLOW,                  SPELL_EARTH_SLOW},
    {ITEM_SCROLL_EARTH_RESISTANCE,      SPELL_EARTH_PROTECTION_FROM_EARTH},
    {ITEM_SCROLL_DEADLY_SWARM,          SPELL_EARTH_DEADLY_SWARM},
    {ITEM_SCROLL_STONE_SKIN,            SPELL_EARTH_STONESKIN},
    {ITEM_SCROLL_BLADES,                SPELL_EARTH_BLADES},
    {ITEM_SCROLL_STONE_TO_FLESH,        SPELL_EARTH_STONE_TO_FLESH},
    {ITEM_SCROLL_ROCK_BLAST,            SPELL_EARTH_ROCK_BLAST},
    {ITEM_SCROLL_TELEKINESIS,           SPELL_EARTH_TELEKINESIS},
    {ITEM_SCROLL_DEATH_BLOSSOM,         SPELL_EARTH_DEATH_BLOSSOM},
    {ITEM_SCROLL_MASS_DISTORTION,       SPELL_EARTH_MASS_DISTORTION},

    {ITEM_SCROLL_DETECT_LIFE,           SPELL_SPIRIT_DETECT_LIFE},
    {ITEM_SCROLL_BLESS,                 SPELL_SPIRIT_BLESS},
    {ITEM_SCROLL_FATE,                  SPELL_SPIRIT_FATE},
    {ITEM_SCROLL_TURN_UNDEAD,           SPELL_SPIRIT_TURN_UNDEAD},
    {ITEM_SCROLL_REMOVE_CURSE,          SPELL_SPIRIT_REMOVE_CURSE},
    {ITEM_SCROLL_PRESERVATION,          SPELL_SPIRIT_PRESERVATION},
    {ITEM_SCROLL_HEROISM,               SPELL_SPIRIT_HEROISM},
    {ITEM_SCROLL_SPIRIT_LASH,           SPELL_SPIRIT_SPIRIT_LASH},
    {ITEM_SCROLL_RAISE_DEAD,            SPELL_SPIRIT_RAISE_DEAD},
    {ITEM_SCROLL_SHARED_LIFE,           SPELL_SPIRIT_SHARED_LIFE},
    {ITEM_SCROLL_RESURRECTION,          SPELL_SPIRIT_RESSURECTION},

    {ITEM_SCROLL_REMOVE_FEAR,           SPELL_MIND_REMOVE_FEAR},
    {ITEM_SCROLL_MIND_BLAST,            SPELL_MIND_MIND_BLAST},
    {ITEM_SCROLL_MIND_RESISTANCE,       SPELL_MIND_PROTECTION_FROM_MIND},
    {ITEM_SCROLL_TELEPATHY,             SPELL_MIND_TELEPATHY},
    {ITEM_SCROLL_CHARM,                 SPELL_MIND_CHARM},
    {ITEM_SCROLL_CURE_PARALYSIS,        SPELL_MIND_CURE_PARALYSIS},
    {ITEM_SCROLL_BERSERK,               SPELL_MIND_BERSERK},
    {ITEM_SCROLL_MASS_FEAR,             SPELL_MIND_MASS_FEAR},
    {ITEM_SCROLL_CURE_INSANITY,         SPELL_MIND_CURE_INSANITY},
    {ITEM_SCROLL_PSYCHIC_SHOCK,         SPELL_MIND_PSYCHIC_SHOCK},
    {ITEM_SCROLL_ENSLAVE,               SPELL_MIND_ENSLAVE},

    {ITEM_SCROLL_CURE_WEAKNESS,         SPELL_BODY_CURE_WEAKNESS},
    {ITEM_SCROLL_HEAL,                  SPELL_BODY_FIRST_AID},
    {ITEM_SCROLL_BODY_RESISTANCE,       SPELL_BODY_PROTECTION_FROM_BODY},
    {ITEM_SCROLL_HARM,                  SPELL_BODY_HARM},
    {ITEM_SCROLL_REGENERATION,          SPELL_BODY_REGENERATION},
    {ITEM_SCROLL_CURE_POISON,           SPELL_BODY_CURE_POISON},
    {ITEM_SCROLL_HAMMERHANDS,           SPELL_BODY_HAMMERHANDS},
    {ITEM_SCROLL_CURE_DISEASE,          SPELL_BODY_CURE_DISEASE},
    {ITEM_SCROLL_PROTECTION_FROM_MAGIC, SPELL_BODY_PROTECTION_FROM_MAGIC},
    {ITEM_SCROLL_FLYING_FIST,           SPELL_BODY_FLYING_FIST},
    {ITEM_SCROLL_POWER_CURE,            SPELL_BODY_POWER_CURE},

    {ITEM_SCROLL_LIGHT_BOLT,            SPELL_LIGHT_LIGHT_BOLT},
    {ITEM_SCROLL_DESTROY_UNDEAD,        SPELL_LIGHT_DESTROY_UNDEAD},
    {ITEM_SCROLL_DISPEL_MAGIC,          SPELL_LIGHT_DISPEL_MAGIC},
    {ITEM_SCROLL_PARALYZE,              SPELL_LIGHT_PARALYZE},
    {ITEM_SCROLL_SUMMON_ELEMENTAL,      SPELL_LIGHT_SUMMON_ELEMENTAL},
    {ITEM_SCROLL_DAY_OF_THE_GODS,       SPELL_LIGHT_DAY_OF_THE_GODS},
    {ITEM_SCROLL_PRISMATIC_LIGHT,       SPELL_LIGHT_PRISMATIC_LIGHT},
    {ITEM_SCROLL_DAY_OF_PROTECTION,     SPELL_LIGHT_DAY_OF_PROTECTION},
    {ITEM_SCROLL_HOUR_OF_POWER,         SPELL_LIGHT_HOUR_OF_POWER},
    {ITEM_SCROLL_SUNRAY,                SPELL_LIGHT_SUNRAY},
    {ITEM_SCROLL_DIVINE_INTERVENTION,   SPELL_LIGHT_DIVINE_INTERVENTION},

    {ITEM_SCROLL_REANIMATE,             SPELL_DARK_REANIMATE},
    {ITEM_SCROLL_TOXIC_CLOUD,           SPELL_DARK_TOXIC_CLOUD},
    {ITEM_SCROLL_VAMPIRIC_WEAPON,       SPELL_DARK_VAMPIRIC_WEAPON},
    {ITEM_SCROLL_SHRINKING_RAY,         SPELL_DARK_SHRINKING_RAY},
    {ITEM_SCROLL_SHRAPMETAL,            SPELL_DARK_SHARPMETAL},
    {ITEM_SCROLL_CONTROL_UNDEAD,        SPELL_DARK_CONTROL_UNDEAD},
    {ITEM_SCROLL_PAIN_REFLECTION,       SPELL_DARK_PAIN_REFLECTION},
    {ITEM_SCROLL_SACRIFICE,             SPELL_DARK_SACRIFICE},
    {ITEM_SCROLL_DRAGON_BREATH,         SPELL_DARK_DRAGON_BREATH},
    {ITEM_SCROLL_ARMAGEDDON,            SPELL_DARK_ARMAGEDDON},
    {ITEM_SCROLL_SOULDRINKER,           SPELL_DARK_SOULDRINKER}
};

constinit const IndexedArray<SpellId, ITEM_FIRST_SPELLBOOK, ITEM_LAST_SPELLBOOK> spellForSpellbook = {
    {ITEM_SPELLBOOK_TORCH_LIGHT,           SPELL_FIRE_TORCH_LIGHT},
    {ITEM_SPELLBOOK_FIRE_BOLT,             SPELL_FIRE_FIRE_BOLT},
    {ITEM_SPELLBOOK_FIRE_RESISTANCE,       SPELL_FIRE_PROTECTION_FROM_FIRE},
    {ITEM_SPELLBOOK_FIRE_AURA,             SPELL_FIRE_FIRE_AURA},
    {ITEM_SPELLBOOK_HASTE,                 SPELL_FIRE_HASTE},
    {ITEM_SPELLBOOK_FIREBALL,              SPELL_FIRE_FIREBALL},
    {ITEM_SPELLBOOK_FIRE_SPIKE,            SPELL_FIRE_FIRE_SPIKE},
    {ITEM_SPELLBOOK_IMMOLATION,            SPELL_FIRE_IMMOLATION},
    {ITEM_SPELLBOOK_METEOR_SHOWER,         SPELL_FIRE_METEOR_SHOWER},
    {ITEM_SPELLBOOK_INFERNO,               SPELL_FIRE_INFERNO},
    {ITEM_SPELLBOOK_INCINERATE,            SPELL_FIRE_INCINERATE},

    {ITEM_SPELLBOOK_WIZARD_EYE,            SPELL_AIR_WIZARD_EYE},
    {ITEM_SPELLBOOK_FEATHER_FALL,          SPELL_AIR_FEATHER_FALL},
    {ITEM_SPELLBOOK_AIR_RESISTANCE,        SPELL_AIR_PROTECTION_FROM_AIR},
    {ITEM_SPELLBOOK_SPARKS,                SPELL_AIR_SPARKS},
    {ITEM_SPELLBOOK_JUMP,                  SPELL_AIR_JUMP},
    {ITEM_SPELLBOOK_SHIELD,                SPELL_AIR_SHIELD},
    {ITEM_SPELLBOOK_LIGHTNING_BOLT,        SPELL_AIR_LIGHTNING_BOLT},
    {ITEM_SPELLBOOK_INVISIBILITY,          SPELL_AIR_INVISIBILITY},
    {ITEM_SPELLBOOK_IMPLOSION,             SPELL_AIR_IMPLOSION},
    {ITEM_SPELLBOOK_FLY,                   SPELL_AIR_FLY},
    {ITEM_SPELLBOOK_STARBURST,             SPELL_AIR_STARBURST},

    {ITEM_SPELLBOOK_AWAKEN,                SPELL_WATER_AWAKEN},
    {ITEM_SPELLBOOK_POISON_SPRAY,          SPELL_WATER_POISON_SPRAY},
    {ITEM_SPELLBOOK_WATER_RESISTANCE,      SPELL_WATER_PROTECTION_FROM_WATER},
    {ITEM_SPELLBOOK_ICE_BOLT,              SPELL_WATER_ICE_BOLT},
    {ITEM_SPELLBOOK_WATER_WALK,            SPELL_WATER_WATER_WALK},
    {ITEM_SPELLBOOK_RECHARGE_ITEM,         SPELL_WATER_RECHARGE_ITEM},
    {ITEM_SPELLBOOK_ACID_BURST,            SPELL_WATER_ACID_BURST},
    {ITEM_SPELLBOOK_ENCHANT_ITEM,          SPELL_WATER_ENCHANT_ITEM},
    {ITEM_SPELLBOOK_TOWN_PORTAL,           SPELL_WATER_TOWN_PORTAL},
    {ITEM_SPELLBOOK_ICE_BLAST,             SPELL_WATER_ICE_BLAST},
    {ITEM_SPELLBOOK_LLOYDS_BEACON,         SPELL_WATER_LLOYDS_BEACON},

    {ITEM_SPELLBOOK_STUN,                  SPELL_EARTH_STUN},
    {ITEM_SPELLBOOK_SLOW,                  SPELL_EARTH_SLOW},
    {ITEM_SPELLBOOK_EARTH_RESISTANCE,      SPELL_EARTH_PROTECTION_FROM_EARTH},
    {ITEM_SPELLBOOK_DEADLY_SWARM,          SPELL_EARTH_DEADLY_SWARM},
    {ITEM_SPELLBOOK_STONE_SKIN,            SPELL_EARTH_STONESKIN},
    {ITEM_SPELLBOOK_BLADES,                SPELL_EARTH_BLADES},
    {ITEM_SPELLBOOK_STONE_TO_FLESH,        SPELL_EARTH_STONE_TO_FLESH},
    {ITEM_SPELLBOOK_ROCK_BLAST,            SPELL_EARTH_ROCK_BLAST},
    {ITEM_SPELLBOOK_TELEKINESIS,           SPELL_EARTH_TELEKINESIS},
    {ITEM_SPELLBOOK_DEATH_BLOSSOM,         SPELL_EARTH_DEATH_BLOSSOM},
    {ITEM_SPELLBOOK_MASS_DISTORTION,       SPELL_EARTH_MASS_DISTORTION},

    {ITEM_SPELLBOOK_DETECT_LIFE,           SPELL_SPIRIT_DETECT_LIFE},
    {ITEM_SPELLBOOK_BLESS,                 SPELL_SPIRIT_BLESS},
    {ITEM_SPELLBOOK_FATE,                  SPELL_SPIRIT_FATE},
    {ITEM_SPELLBOOK_TURN_UNDEAD,           SPELL_SPIRIT_TURN_UNDEAD},
    {ITEM_SPELLBOOK_REMOVE_CURSE,          SPELL_SPIRIT_REMOVE_CURSE},
    {ITEM_SPELLBOOK_PRESERVATION,          SPELL_SPIRIT_PRESERVATION},
    {ITEM_SPELLBOOK_HEROISM,               SPELL_SPIRIT_HEROISM},
    {ITEM_SPELLBOOK_SPIRIT_LASH,           SPELL_SPIRIT_SPIRIT_LASH},
    {ITEM_SPELLBOOK_RAISE_DEAD,            SPELL_SPIRIT_RAISE_DEAD},
    {ITEM_SPELLBOOK_SHARED_LIFE,           SPELL_SPIRIT_SHARED_LIFE},
    {ITEM_SPELLBOOK_RESURRECTION,          SPELL_SPIRIT_RESSURECTION},

    {ITEM_SPELLBOOK_REMOVE_FEAR,           SPELL_MIND_REMOVE_FEAR},
    {ITEM_SPELLBOOK_MIND_BLAST,            SPELL_MIND_MIND_BLAST},
    {ITEM_SPELLBOOK_MIND_RESISTANCE,       SPELL_MIND_PROTECTION_FROM_MIND},
    {ITEM_SPELLBOOK_TELEPATHY,             SPELL_MIND_TELEPATHY},
    {ITEM_SPELLBOOK_CHARM,                 SPELL_MIND_CHARM},
    {ITEM_SPELLBOOK_CURE_PARALYSIS,        SPELL_MIND_CURE_PARALYSIS},
    {ITEM_SPELLBOOK_BERSERK,               SPELL_MIND_BERSERK},
    {ITEM_SPELLBOOK_MASS_FEAR,             SPELL_MIND_MASS_FEAR},
    {ITEM_SPELLBOOK_CURE_INSANITY,         SPELL_MIND_CURE_INSANITY},
    {ITEM_SPELLBOOK_PSYCHIC_SHOCK,         SPELL_MIND_PSYCHIC_SHOCK},
    {ITEM_SPELLBOOK_ENSLAVE,               SPELL_MIND_ENSLAVE},

    {ITEM_SPELLBOOK_CURE_WEAKNESS,         SPELL_BODY_CURE_WEAKNESS},
    {ITEM_SPELLBOOK_HEAL,                  SPELL_BODY_FIRST_AID},
    {ITEM_SPELLBOOK_BODY_RESISTANCE,       SPELL_BODY_PROTECTION_FROM_BODY},
    {ITEM_SPELLBOOK_HARM,                  SPELL_BODY_HARM},
    {ITEM_SPELLBOOK_REGENERATION,          SPELL_BODY_REGENERATION},
    {ITEM_SPELLBOOK_CURE_POISON,           SPELL_BODY_CURE_POISON},
    {ITEM_SPELLBOOK_HAMMERHANDS,           SPELL_BODY_HAMMERHANDS},
    {ITEM_SPELLBOOK_CURE_DISEASE,          SPELL_BODY_CURE_DISEASE},
    {ITEM_SPELLBOOK_PROTECTION_FROM_MAGIC, SPELL_BODY_PROTECTION_FROM_MAGIC},
    {ITEM_SPELLBOOK_FLYING_FIST,           SPELL_BODY_FLYING_FIST},
    {ITEM_SPELLBOOK_POWER_CURE,            SPELL_BODY_POWER_CURE},

    {ITEM_SPELLBOOK_LIGHT_BOLT,            SPELL_LIGHT_LIGHT_BOLT},
    {ITEM_SPELLBOOK_DESTROY_UNDEAD,        SPELL_LIGHT_DESTROY_UNDEAD},
    {ITEM_SPELLBOOK_DISPEL_MAGIC,          SPELL_LIGHT_DISPEL_MAGIC},
    {ITEM_SPELLBOOK_PARALYZE,              SPELL_LIGHT_PARALYZE},
    {ITEM_SPELLBOOK_SUMMON_ELEMENTAL,      SPELL_LIGHT_SUMMON_ELEMENTAL},
    {ITEM_SPELLBOOK_DAY_OF_THE_GODS,       SPELL_LIGHT_DAY_OF_THE_GODS},
    {ITEM_SPELLBOOK_PRISMATIC_LIGHT,       SPELL_LIGHT_PRISMATIC_LIGHT},
    {ITEM_SPELLBOOK_DAY_OF_PROTECTION,     SPELL_LIGHT_DAY_OF_PROTECTION},
    {ITEM_SPELLBOOK_HOUR_OF_POWER,         SPELL_LIGHT_HOUR_OF_POWER},
    {ITEM_SPELLBOOK_SUNRAY,                SPELL_LIGHT_SUNRAY},
    {ITEM_SPELLBOOK_DIVINE_INTERVENTION,   SPELL_LIGHT_DIVINE_INTERVENTION},

    {ITEM_SPELLBOOK_REANIMATE,             SPELL_DARK_REANIMATE},
    {ITEM_SPELLBOOK_TOXIC_CLOUD,           SPELL_DARK_TOXIC_CLOUD},
    {ITEM_SPELLBOOK_VAMPIRIC_WEAPON,       SPELL_DARK_VAMPIRIC_WEAPON},
    {ITEM_SPELLBOOK_SHRINKING_RAY,         SPELL_DARK_SHRINKING_RAY},
    {ITEM_SPELLBOOK_SHRAPMETAL,            SPELL_DARK_SHARPMETAL},
    {ITEM_SPELLBOOK_CONTROL_UNDEAD,        SPELL_DARK_CONTROL_UNDEAD},
    {ITEM_SPELLBOOK_PAIN_REFLECTION,       SPELL_DARK_PAIN_REFLECTION},
    {ITEM_SPELLBOOK_SACRIFICE,             SPELL_DARK_SACRIFICE},
    {ITEM_SPELLBOOK_DRAGON_BREATH,         SPELL_DARK_DRAGON_BREATH},
    {ITEM_SPELLBOOK_ARMAGEDDON,            SPELL_DARK_ARMAGEDDON},
    {ITEM_SPELLBOOK_SOULDRINKER,           SPELL_DARK_SOULDRINKER}
};

} // namespace detail
