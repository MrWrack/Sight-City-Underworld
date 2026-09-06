# Milestone 32 — Unified Emergency Response

Story-first milestone. Online remains intentionally paused until Story has been tested on real PS Vita hardware.

## Added
- Unified incident dispatcher for Police, Ambulance and Fire Department.
- Incident severity determines which services are dispatched.
- Minor crash, major crash, fire, explosion, shooting, medical emergency and crime incident types.
- Emergency lights and siren state while responding/transporting.
- Police secure serious scenes and can hand off into a prototype pursuit state when crime is suspected.
- Ambulance paramedics locate nearby casualties, stabilize them and transport them to a hospital anchor.
- Fire engines suppress active FireExplosionSystem fire patches.
- Civilian NPCs flee serious incidents.
- Civilian traffic yields/slows near emergency vehicles using sirens.
- Vita service-unit budgets are deliberately smaller than desktop budgets.

## Still prototype-level
- Real road-route integration for emergency vehicles.
- Full pedestrian medic animations and stretchers.
- Police tape / physical roadblock props.
- Hospital and fire/police station interiors.
- WantedSystem handoff for suspects.
- Real VitaSDK/VPK build and hardware performance test.

## LiveArea title
The Vita package already exposes the game title through `VITA_APP_NAME` in both CMake build files. The current working title remains **City Limits** until a final public title is chosen. The final selected title should be used consistently for LiveArea metadata and the icon artwork.
