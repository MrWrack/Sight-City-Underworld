# City Limits - PS Vita Milestone 04

Milestone 04 improves the visual foundation of Sight City's first playable Downtown area while keeping the 120 x 120 km sparse world architecture from Milestone 03.

## Downtown visual pass

- Stronger building bases so towers feel attached to the street instead of floating on a flat plane.
- Deterministic facade material palette for concrete, stone, steel and muted painted surfaces.
- Lightweight procedural window bands instead of large image textures.
- Rooftop parapets and occasional HVAC silhouettes.
- Sidewalk pads around dense blocks.
- Darker asphalt with center-lane markings on arterial roads.
- Pocket parks and paved plazas mixed into city blocks.
- Downtown, Urban and Suburb retain different density/height profiles.

## PS Vita budget strategy

The facade system intentionally uses procedural geometry/material colors rather than heavy per-building texture files. This keeps memory usage low and allows deterministic rebuilding of streamed cells. Window detail is capped by building floor count to avoid excessive Vita draw calls.

## Settings fix

The Vita entry point now creates `ux0:data/CityLimits` before loading/saving `settings.cfg`, preventing first-run settings saves from failing just because the folder did not exist yet.

## Next logical milestone

Add lightweight traffic lanes, parked cars, pedestrians/NPC spawn points, collision/navigation data, and authored Downtown landmarks while retaining the same sparse streaming API.
