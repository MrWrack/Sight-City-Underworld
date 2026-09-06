from pathlib import Path
import struct
import sys
import xml.etree.ElementTree as ET

root = Path(__file__).resolve().parents[1]

EXPECTED = {
    root / 'sce_sys' / 'icon0.png': (128, 128),
    root / 'sce_sys' / 'livearea' / 'contents' / 'bg.png': (840, 500),
    root / 'sce_sys' / 'livearea' / 'contents' / 'startup.png': (280, 158),
}

def png_size(path: Path):
    if not path.exists():
        sys.exit(f'Missing required Vita asset: {path.relative_to(root)}')
    data = path.read_bytes()
    if data[:8] != b'\x89PNG\r\n\x1a\n':
        sys.exit(f'{path.relative_to(root)} is not a PNG')
    return struct.unpack('>II', data[16:24])

for path, expected in EXPECTED.items():
    actual = png_size(path)
    if actual != expected:
        sys.exit(f'{path.relative_to(root)} must be {expected[0]}x{expected[1]}, got {actual[0]}x{actual[1]}')

xml_path = root / 'sce_sys' / 'livearea' / 'contents' / 'template.xml'
if not xml_path.exists():
    sys.exit('Missing sce_sys/livearea/contents/template.xml')
try:
    tree = ET.parse(xml_path)
except ET.ParseError as exc:
    sys.exit(f'Invalid LiveArea template.xml: {exc}')
if tree.getroot().tag != 'livearea':
    sys.exit('LiveArea template root must be <livearea>')

cmake = (root / 'CMakeLists.txt').read_text(encoding='utf-8')
required_tokens = [
    'set(VITA_APP_NAME "Sight City: Underworld")',
    'set(VITA_TITLEID "SCUW00001")',
    'set(VITA_VERSION "00.61")',
    'sight_city_underworld.vpk',
    'sce_sys/icon0.png',
    'sce_sys/livearea/contents/template.xml',
    'sce_sys/livearea/contents/bg.png',
    'sce_sys/livearea/contents/startup.png',
]
for token in required_tokens:
    if token not in cmake:
        sys.exit(f'CMake Vita packaging is missing: {token}')

print('M61 Vita package assets OK')
print('  icon0.png  : 128x128')
print('  bg.png     : 840x500')
print('  startup.png: 280x158')
print('  template.xml: valid XML')
print('  Title ID   : SCUW00001')
print('  VPK name   : sight_city_underworld.vpk')
