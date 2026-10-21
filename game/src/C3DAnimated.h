/*  C3DAnimated - a C3DObject that plays animations, either OMT morph animations built from .ase
    files (f_6d0 = 0) or RAD Granny skeletal animations (.grn, f_6d0 = 1: through
    game/granny/granny_compat.h, see docs/granny-api.md). The base of every character
    (C3DPlayerObject -> C3DPlayer -> C3DJimmy, C3DAI ...) and of C3DGrannyObj.
    Reconstructed from Neutron2.exe: constructor 0x41a690, destructor 0x41a9f0 (deleting thunks
    0x41a9c0 / 0x41f8c0 / 0x41f8d0 / 0x41f8e0). No factory ID of its own.

    Original layout (classes.json): as C3DObject (element vtable 0x4cd99c, 94 slots: C3DObject's
    56 + 56..93; OMediaAnim at +0x90, vtable 0x4cd94c; listener +0x30, vtable 0x4cd988;
    CLocalGameObject at +0xc0, vtable 0x4cd4d0, 286 slots), own members 0x58c..0x924 (C3DObject's
    data ends at 0x58a; its vtordisp moves to the end), vtordisp 0x924, streamer 0x928.
    Derived classes put their members from 0x924 on.

    Granny objects (f_6d0 = 1): InitObject creates an empty OMedia3DShape for the element.
    InitModel(file) opens the model ("grn\<file>", shared between objects through
    neg::g_granny_models) and one instance of it (a "sequence", f_6e8); BindTex turns the model's
    texture into f_918/f_91c, BuildTex fills the shape with the bind pose (positions and normals
    in OMT axes: (-y, z, x)). AddAnimation(file, name) opens an animation and keeps it in f_6d4.
    PlayAnimation (ev86) starts up to four of them by name (into the play slots f_6f4[0..3]);
    ev80, every frame, copies the posed vertices into the shape and notices the end of the main
    animation (f_8a5; ev67 is the end-of-animation hook).

    Morph objects (f_6d0 = 0): InitAnim takes the base shape (chunk 0) of the object's private
    database; CreateAnim(name, file) imports an .ase animation into a new morph definition kept in
    the AnimList chain at f_5a4; ev58(name, loop) switches to one.

    Element-family game slots are `ev<n>` (`// ESLOT n`, n = vtable offset / 4) unless a log
    string names them; CGameObject-family slots keep their CGameObject names (`// SLOT n`).
    Members are f_<offset in the original object> (CGameObject-family code sees them at
    offset - 0xc0).                                                                          */
#pragma once
#include <list>
#include <string>
#include "C3DObject.h"
#include "granny_compat.h"

class OMediaCanvas;
class OMedia3DMaterial;
class OMedia3DMorphAnimDef;
class OMediaMatrix_4x4;

// One morph animation of an OMT-animated object (0x50 bytes, "AnimList" in the logs).
struct AnimList
{
	char					name[64];		// +0x00  g_545f14 + animation name (63 chars max)
	AnimList				*next;			// +0x40
	int						id;				// +0x44  sequence number (f_5a0 when created)
	int						nframes;		// +0x48  frames of the current sequence (SetSequence)
	OMedia3DMorphAnimDef	*def;			// +0x4c  the imported morph definition (database 'A3dm')
};

// One Granny animation of an object (0x64 bytes, "AnimInfo_t" in the logs).
struct AnimInfo_t
{
	granny_uint32			anim[2];		// +0x00  GrannyOpenAnimation
	granny_uint32			control[3];		// +0x08  GrannyAddControlledAnimation (control[2] != 0: playing)
	char					name[80];		// +0x14  name the game plays it by ("STOP", "RUN", ...)
};

// A Granny model shared by all objects that use the same file (0x1c bytes, "ModelInfo_t").
struct ModelInfo_t
{
	granny_uint32			model[2];		// +0x00  GrannyOpenModel
	std::string				name;			// +0x08  file name as given to InitModel (MSVC6 string, 16 bytes)
	int						refs;			// +0x18  objects using it
};

namespace neg {
extern std::list<ModelInfo_t *> g_granny_models;		// 0x545f20 (head 0x545f24, size 0x545f28)
}

class C3DAnimated : public C3DObject
{
	public:
	C3DAnimated();															// 0x41a690
	virtual ~C3DAnimated();													// 0x41a9f0

	// ---- element-family game slots (vtable offset = slot * 4)
	virtual void CreateAnim(char *name, char *file);						// ESLOT 56 0x41b360 "Anim3D_CreateAnim"; granny: AddAnimation
	virtual AnimList *GetSequence(char *name);								// ESLOT 57 0x41b8d0 "Anim3D_GetSequence" (morph only)
	virtual void ev58(char *name, unsigned char loop);						// ESLOT 58 0x41bea0 play animation <name> (morph: sequence, granny: ev86(name, 0, 0, 0, loop, 0.2))
	virtual void SetSequence(int seq, unsigned char loop);					// ESLOT 59 0x41b940 "Anim3D_SetSequence(long)" (morph only)
	virtual void ev60(void) {}												// ESLOT 60 0x41f890 (empty) ev86 found no animation of that name
	virtual char *GetAnim(void);											// ESLOT 61 0x41ba20 "Anim3D_GetAnim": current AnimList (morph) / f_6f4[0].name (granny); "" if none
	virtual int  ev62(void);												// ESLOT 62 0x41ba90 f_5a0: number of morph animations
	virtual void CreateTexture(char *file, short index);					// ESLOT 63 0x41baa0 "Anim3D_CreateTexture": png\<file> into f_5ac/f_5d4[index]
	virtual void ApplyTexture(OMedia3DShape *shape, short index);			// ESLOT 64 0x41be50 "Anim3D_ApplyTexture"
	virtual void ev65(short index, int src, int dst);						// ESLOT 65 0x41bdc0 blend of material [index] (granny: of the whole shape)
	virtual char *ev66(void);												// ESLOT 66 0x41c0f0 name of the current animation
	virtual void ev67(void) {}												// ESLOT 67 0x463890 (empty) the animation ended (ev80 / Process)
	virtual void InitAnim(void);											// ESLOT 68 0x41c790 "InitAnim": morph base shape from the object database
	virtual void ev69(void) {}												// ESLOT 69 0x463890 (empty) after InitAnim / before ev70(true)
	virtual void ev70(unsigned char show);									// ESLOT 70 0x41c920 show/hide with collision (level visibility)
	virtual void ev71(float alpha);											// ESLOT 71 0x41cd00 alpha blending (<= 0: opaque)
	virtual void ev72(float alpha);											// ESLOT 72 0x41ca20 additive blending (<= 0: opaque) - "Lit"
	virtual void ev73(void);												// ESLOT 73 0x41ce30 alpha blending, second pass, as is
	virtual void ev74(float alpha, float r, float g, float b);				// ESLOT 74 0x41cbc0 tint (+ alpha)
	virtual void ev75(float time);											// ESLOT 75 0x41b2e0 fade in over <time> s ("now im fading")
	virtual void ev76(float time);											// ESLOT 76 0x41b320 fade out over <time> s ("now im fading out")
	virtual unsigned char ev77(char *state);								// ESLOT 77 0x41c6b0 state f_8b1 is <state> (_stricmp)
	virtual void ev78(char *state);											// ESLOT 78 0x41c680 set state f_8b1
	virtual void ev79(unsigned char pause);									// ESLOT 79 0x41b010 pause the morph animation
	virtual void ev80(void);												// ESLOT 80 0x41e120 granny: pose -> shape each frame, end detection
	virtual void InitModel(std::string file);								// ESLOT 81 0x41cff0 "InitModel": open the granny model + sequence
	virtual void BuildTex(void);											// ESLOT 82 0x41d7a0 "BuildTex": shape from the bind pose
	virtual void ev83(OMedia3DShape *shape);								// ESLOT 83 0x41dc90 empty the shape's geometry
	virtual void BindTex(void);												// ESLOT 84 0x41ae10 "BindTex": granny texture -> f_918 / f_91c
	virtual void AddAnimation(std::string file, char *name);				// ESLOT 85 0x41df00 "AddAnimation": open grn\<file> as <name>
	virtual void ev86(char *a, char *b, char *c, char *d, unsigned char loop, float blend);	// ESLOT 86 0x41d470 granny: play the animations named a..d (slots 0..3)
	virtual unsigned char ev87(char *file, char *dir, char *ext);			// ESLOT 87 0x41e6a0 <dir>\<file> exists? then file = that path (returns 1)
	virtual void ev88(char *bone, CGameObject *other);						// ESLOT 88 0x41af40 attach other's granny sequence to our <bone>
	virtual void ev89(CGameObject *other);									// ESLOT 89 0x41afd0 detach other's granny sequence
	virtual void ev90(unsigned char detach);								// ESLOT 90 0x439ba0 drop the C3DGrannyObj props attached to us
	virtual void ev91(unsigned char on);									// ESLOT 91 0x41cb60 second-pass flag of all materials
	virtual void ev92(int src, int dst);									// ESLOT 92 0x41c9d0 blend of all materials
	virtual unsigned char ev93(void);										// ESLOT 93 0x41e840 culled: too far, or not in front of the viewport

	// ---- not virtual
	unsigned char IsAnim(char *name);										// 0x41c6d0 the current animation (ev66 / GetAnim) is <name>

	// ---- CGameObject-family overrides
	virtual void InitObject(void);											// SLOT 7   0x41b0a0 properties
	virtual void UnInitObject(void);										// SLOT 8   0x41ab10 (does not call the base's)
	virtual void Process(float dt);											// SLOT 240 0x41c120
	virtual void v242(float t);												// SLOT 242 0x41b070 ShowAgain + ev80
	virtual void v260(void);												// SLOT 260 0x41cec0 properties loaded
	virtual void v267(int level);											// SLOT 267 0x41c880 RequiredLevel/ExactLevel/RemoveLevel -> ev70
	virtual void v268(int state);											// SLOT 268 0x41c710 "NextState": 1 show, 0 hide
	virtual void v270(int a, int msg);										// SLOT 270 0x41e960 msg 5: end the fades (C3DHERMAN only)
	virtual void v277(void);												// SLOT 277 0x41ce80 ev79(1)
	virtual void v278(void);												// SLOT 278 0x41cea0 ev79(0)
	virtual void v280(void);												// SLOT 280 0x41e460 editor: list the animation names
	virtual void v282(OMediaMatrix_4x4 &m, float *w, float *h);				// SLOT 282 0x41cfd0 layer projection matrix

	// ---- data members (object offsets)
	int					f_58c;			// 0x58c  0 (InitAnim)
	OMedia3DShape		*f_590;			// 0x590  morph: base shape (object database '3DSh' chunk 0)
	short				f_594;			// 0x594  -1; 0 (InitAnim)
	float				f_598;			// 0x598  morph: time in the current animation (+= dt)
	int					f_59c;			// 0x59c  morph: current sequence (-1)
	int					f_5a0;			// 0x5a0  morph: number of animations (AnimList ids)
	AnimList			*f_5a4;			// 0x5a4  morph: first animation
	int					f_5a8;			// 0x5a8  0
	OMediaCanvas		*f_5ac[10];		// 0x5ac  morph textures (CreateTexture)
	OMedia3DMaterial	*f_5d4[10];		// 0x5d4  their materials
	char				f_5fc[64];		// 0x5fc  name of the animation last asked for (ev58)
	unsigned char		f_63c;			// 0x63c  InitAnim done
	// 0x640
	AnimList			*f_640;			// 0x640  morph: current animation
	unsigned char		f_648;			// 0x648  1: animations on
	unsigned char		f_649;			// 0x649  1: animations loaded (C3DGrannyObj: 0 until its first ev69)
	int					f_64c;			// 0x64c  "RequiredLevel" (-1)
	int					f_650;			// 0x650  "ExactLevel" (-1)
	int					f_654;			// 0x654  "RemoveLevel" (-1)
	int					f_658;			// 0x658  "HasCollision" (-1: leave as is)
	int					f_65c;			// 0x65c  "InitiallyVisible" (-1)
	int					f_660;			// 0x660  "CanMove" (1; 0: kept at its start position)
	int					f_664;			// 0x664  "SecondPass" (0)
	unsigned char		f_668;			// 0x668  morph animation paused (ev79)
	char				f_669[100];		// 0x669  "PickupLink" ("none"): hidden once that pickup is taken
	unsigned char		f_6cd;			// 0x6cd  PickupLink checked
	int					f_6d0;			// 0x6d0  animation system: 0 OMT morph, 1 Granny
	std::list<AnimInfo_t *> f_6d4;		// 0x6d4  granny animations (AddAnimation)
	granny_uint32		f_6e0[2];		// 0x6e0  granny model
	granny_uint32		f_6e8[2];		// 0x6e8  granny sequence (model instance)
	float				f_6f0;			// 0x6f0  granny shape scale (1.0)
	AnimInfo_t			f_6f4[4];		// 0x6f4  playing animations (ev86's a..d; [0] is the main one)
	int					f_884;			// 0x884  "CanRotate" (1; 0: kept at its start rotation)
	void				*f_888;			// 0x888  morph: current frame (OMediaAnim current_frame)
	int					f_88c;			// 0x88c  "FaceViewport" (0)
	unsigned char		f_890;			// 0x890  fading out
	unsigned char		f_891;			// 0x891  fading in
	float				f_894;			// 0x894  fade duration
	float				f_898;			// 0x898  fade time left
	float				f_89c;			// 0x89c  "Translucent" (alpha, 0 = opaque)
	float				f_8a0;			// 0x8a0  "Lit" (additive alpha)
	unsigned char		f_8a4;			// 0x8a4  never culled; granny: pose every frame
	unsigned char		f_8a5;			// 0x8a5  granny: main animation over (or "STOP")
	unsigned char		f_8a6;			// 0x8a6  granny: main animation just expired -> ev67
	unsigned char		f_8a7;			// 0x8a7  no animation changes (ev58 ignored)
	unsigned char		f_8a8;			// 0x8a8  culled this frame (ev93)
	int					f_8ac;			// 0x8ac  "CanClip" (1)
	unsigned char		f_8b0;			// 0x8b0  fades allowed (1)
	char				f_8b1[100];		// 0x8b1  state name (ev77/ev78, "none")
	OMediaCanvas		*f_918;			// 0x918  granny texture
	OMedia3DMaterial	*f_91c;			// 0x91c  granny material (also the texture cookie)
	unsigned char		f_920;			// 0x920  granny: no blending between animations
};
