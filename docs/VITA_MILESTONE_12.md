# Vita Milestone 12 — Campaign Progression + Online Character Creator v2

This milestone expands the separation between Story Mode and Sight City Online.

## Story Mode
- Fixed main-character identity remains locked.
- Clothing/accessory outfit changes are allowed without replacing the protagonist.
- Six linked campaign missions now form a simple chapter chain.
- Completing a mission unlocks its next prerequisite mission.

## Online
- Full Online-only Character Creator logic with bounded/cycling choices for face, hair, hair colour, skin tone, body build, top, bottoms, shoes and accessories.
- Six linked Online missions form a separate campaign chain with co-op, racing, heists and business progression.
- Online missions cannot be started as Story missions.

## Separate progression/save data
- Character profiles use a versioned save format.
- Story and Online each retain their own money, bank balance, XP, level, stats and appearance data.
- Mission progress can be stored separately by using distinct Story and Online mission save paths.

This is still systems/prototype code. Character meshes, clothing assets and the final Vita Character Creator UI are future rendering/UI work.
