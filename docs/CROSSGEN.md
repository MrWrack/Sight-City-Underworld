# Cross-generation target plan

## Shared core
C++ gameplay: player, vehicles, missions, wanted level, NPC logic, streaming, save format.

## PS Vita profile
- 960x544 target
- 30 fps
- small streaming radius
- low-poly vehicles/buildings
- baked lighting
- 1 shadowed hero object max or blob shadows
- reduced traffic / pedestrians

## PS3 / Xbox 360 profile
- 720p target
- 30 fps
- medium streaming radius
- medium-poly assets
- more traffic/NPCs than Vita

## PS4 profile
- 1080p target
- 30/60 fps depending scene complexity
- larger draw distance and density
- higher-resolution assets and effects

## Important
Official PS3, Xbox 360 and PS4 builds require the respective licensed platform SDKs.
The repository keeps those backends separate so platform code can be added without rewriting gameplay.
