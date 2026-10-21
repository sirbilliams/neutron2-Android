/*  Texture animations (0x4a2270..0x4a2930) and the unused material animations (0x490300..0x4907a0).

    A shape's material whose texture chunk is named "a-<digit>..." is animated: the canvases with the
    same name and digits 1..9 in place of the third character (up to the first one missing) are
    loaded and shown in turn, each for the material's ambient red (+0x6c) seconds (0.5 when 0),
    scaled by the game's time scale (0x540594). One animation per texture name: a second material
    with the same texture keeps its first frame (quirk). Materials named "m-<digit>..." build a
    similar list of materials that the shipped game never plays nor frees.                     */
#pragma once

class OMediaDataBase;
class OMedia3DShape;

namespace neg {
void AddShapeTextureAnims(OMediaDataBase *db, OMedia3DShape *shape);	// 0x4a23e0 ("a-" textures)
void AddShapeMaterialAnims(OMediaDataBase *db, OMedia3DShape *shape);	// 0x490300 ("m-" materials)
void UpdateTextureAnims(float dt);										// 0x4a28c0
void ResetTextureAnims(void);											// 0x4a2270
}
