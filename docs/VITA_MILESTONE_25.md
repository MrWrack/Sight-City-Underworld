# City Limits — Milestone 25: Time of Day / Day & Night

Story Mode now has a deterministic game clock and day/night simulation.

- Full game day: 48 real minutes (2 real minutes per game hour).
- Phases: Dawn, Morning, Day, Evening, Dusk, Night.
- Smooth daylight, ambient, sun-height and sky-brightness values for renderer integration.
- Street lights switch on at night; vehicle lights can be recommended by time.
- Rush-hour and late-night traffic multipliers.
- Pedestrian activity multiplier.
- Nightclub activity window and standard shop opening-hours hook.
- 24-hour English-facing clock output.
- Story-first: no Online work added in this milestone.

The Vita renderer can consume the lightweight lighting values without requiring expensive dynamic global illumination.
