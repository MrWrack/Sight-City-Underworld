# Milestone 08 — NPCs, Traffic, Vehicle Lights & Camera

This milestone turns the streamed Sight City prototype into a more active world while keeping the 120 km x 120 km world architecture Vita-friendly.

## Added
- Regional traffic density: Downtown/Urban spawn more vehicles, countryside/coast far fewer.
- Traffic vehicle classes: cars, taxis, buses, trucks and motorcycles, with hooks for police/ambulance.
- Lightweight traffic lights with red/yellow/green phases.
- NPC traffic automatically uses left/right indicators for turning decisions.
- NPC vehicles use low beams by default and have a shared light/indicator state.
- Player vehicle: left indicator, right indicator, hazards, lights Off/Low/High.
- Vehicle settings: indicators On/Off and vehicle lights On/Off.
- Camera settings: first person master On/Off, separate on-foot/in-vehicle toggles, stored FOV setting.
- Camera modes: Third Near, Third Far, First Person.
- Pedestrian states: walking, waiting, crossing and fleeing from nearby danger.
- Vita renderer draws streamed NPC traffic and pedestrians around the player only.

## Vita prototype controls
### On foot
- D-pad Down: cycle camera
- Existing movement/combat mappings remain.

### In vehicle
- D-pad Left: left indicator
- D-pad Right: right indicator
- D-pad Down: hazard lights
- L: cycle vehicle lights Off -> Low -> High -> Off
- Square: cycle camera
- R: accelerate
- X: brake
- Triangle: enter/exit

These are prototype bindings. The planned Settings > Controls remapping UI can replace them later.

## Performance model
The systems keep only a small local population active. Downtown targets around 24 traffic vehicles and 32 pedestrians in this prototype; lower-density regions use fewer. The full 120x120 km world is never simulated at full fidelity at once.
