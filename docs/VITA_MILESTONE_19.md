# City Limits — Vita Milestone 19

## NPC Vehicle AI / Living Traffic

This milestone turns traffic into AI-driven NPC drivers rather than simple moving props.

- Region-aware traffic population and vehicle mix.
- Existing fictional vehicle brands/models are used for civilian traffic.
- Driver personalities: Calm, Normal, Impatient, Aggressive.
- AI states: Cruising, Following, Stopped At Light, Avoiding/Overtaking, Parking, Parked and Fleeing Police.
- Drivers react to red traffic lights and vehicles ahead.
- Impatient/aggressive drivers can overtake and use indicators.
- Calm drivers can park and later return to traffic.
- Aggressive NPC drivers can become police targets and flee when police activity is high.
- Traffic density is lower on PS Vita to protect the 30 FPS target while preserving the same world simulation rules.
- Regional vehicle weighting supports trucks in Industrial, utility vehicles around Airport, off-road vehicles in Countryside and luxury/taxis in Downtown.
- All player-facing game language remains English.

This is a gameplay/simulation foundation. Full road-graph navigation, authored parking spaces, 3D vehicle meshes, collision avoidance, animation and final police pursuit integration remain later production work.
