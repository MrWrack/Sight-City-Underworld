# Vita Milestone 18 — Playable AI Core + Gun Store

This milestone moves City Limits / Sight City toward a playable systemic sandbox. All player-facing game text is English.

## Added
- Story protagonist identity updated to Dash.
- Character AI roster for Dash, Rhys, Mayja, Elias Kane, Adrian Vossler and crew members.
- Ally follow / hold / plan / take-cover / fight intent hooks.
- World AI coordinator for pedestrians, story characters and dynamic NPC crime events.
- NPC crime hooks for theft, robbery, armed robbery, reckless driving and speeding.
- Police-response state on witnessed NPC crimes.
- Weapon inventory with ammo, body armor, sights and suppressors.
- Gun Store with handguns, SMGs, shotguns, rifles and marksman rifles.
- Gun Store purchases deduct player money and persist in the runtime inventory model.
- Ammunition, armor and compatible attachments can be purchased separately.

## Vita direction
The systems are intentionally lightweight and data-driven so nearby AI can be simulated while distant world cells remain streamed out. Rendering, navigation meshes, animation, combat ballistics, audio, save integration and full Vita UI remain separate integration work.
