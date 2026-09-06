# M62 — PS Vita LiveArea install fix

Fixes VitaShell promotion/install error `0x8010113D` caused by LiveArea PNG compatibility.

- `icon0.png`: 128x128 indexed PNG
- `bg.png`: 840x500 indexed PNG
- `startup.png`: 280x158 indexed PNG
- all LiveArea PNGs are below 420 KB
- `template.xml` normalized to UTF-8 + CRLF
- package verifier now rejects non-indexed or oversized LiveArea PNGs
- Vita package version bumped to 00.62

Online remains paused.
