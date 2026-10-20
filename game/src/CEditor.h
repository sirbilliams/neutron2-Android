/*  CEditor - the level loader/saver and the developer level editor (one instance, g_editor).
    Reconstructed from Neutron2.exe: constructor 0x4a4aa0, destructor 0x4a5460 (deleting thunks
    0x4a5430 via the listener vtable, 0x4aa880 via the streamer vtable), 0x82c bytes.

    vtables: 0x51b03c element (86 slots, game slots from 43), 0x51b028 listener at +0x30,
    0x51b018 streamer (virtual base, at +0x828). No CGameObject base.

    What it does in the shipped game (the editor UI is never switched on there - nothing sets
    m_editing - but every frame goes through it):
      * update_logic (slot 15) runs UpdateGame every frame: on the first frame it shows splash 1,
        reads save\startlevel.txt (normally "autosave": restore the autosave task) and arms
        m_load_countdown; when the countdown or m_load_delay runs out m_load_pending is set and
        CheckLoadLevel deletes the level's objects and calls LoadGame(m_level_name, -1).
      * LoadGame reads gam\<name>: the game type's 4cc, the object count, then per object its 4cc
        and its properties (CGameObject::ReadProperties); objects come from neg::CreateObject.
        Then ResetLevel resets every object.
      * SaveGame writes the same format. RequestLevel / RequestLevelDelayed (slots 68/69) are
        how the rest of the game asks for a level change.
      * OpenEditorDatabase / DrawSplash (slots 70/75, vtable +0x118 / +0x12c) are called by the
        game-type factory 0x480230 after it creates a new game type: open omt\Editor.omt and
        show splash canvas 3.

    Element-family layout: offsets < 0x8c are OMediaElement (use the OMT accessors); own members
    are named after their offset in the 2002 object.                                          */
#pragma once
#include "OMediaElement.h"
#include "Globals.h"

class OMediaStringField;
class OMediaCanvas;
class OMediaCanvasFont;
class OMediaCanvasElement;
class OMediaDataBase;
class OMediaFileStream;
class OMediaPickRequest;
class OMedia3DShape;
class OMedia3DPolygon;
class OMedia3DMaterial;
class CGameObject;

class CEditor : public OMediaElement
{
	public:
	CEditor();																// 0x4a4aa0 (new CEditor(1) in omt_main)
	virtual ~CEditor();														// 0x4a5460

	// ---- OMT element slots
	virtual void update_logic(float millisecs_elapsed);						// SLOT 15 0x4a5640

	// ---- game slots (element vtable 0x51b03c)
	virtual void InitObject(void);											// SLOT 43 0x4a5690
	virtual void UnInitObject(void);										// SLOT 44 0x4a56e0
	virtual void InitEditor(void);											// SLOT 45 0x4a5700
	virtual void UnInitEditor(void);										// SLOT 46 0x4a5af0
	virtual void InitGrid(void);											// SLOT 47 0x4a5c30
	virtual void UnInitGrid(void);											// SLOT 48 0x4a5c50
	virtual void UpdateGame(float dt);										// SLOT 49 0x4a5c70
	virtual void UpdateEditor(float dt);									// SLOT 50 0x4a5ec0
	virtual void ViewportClicked(OMediaPickRequest *req);					// SLOT 51 0x4a74f0 (omsg_ViewPortClicked)
	virtual void ev52(void);												// SLOT 52 0x4a7e30
	virtual void ClickList(int x, int y);									// SLOT 53 0x4a6b20
	virtual void ClickMenu(int x, int y);									// SLOT 54 0x4a8f90
	virtual bool ClickPropertyField(int x, int y);							// SLOT 55 0x4a71b0
	virtual void HighlightSelection(bool on);								// SLOT 56 0x4a78c0
	virtual void SetMenuSelection(const char *entry);						// SLOT 57 0x4a7780
	virtual void HideList(bool hide);										// SLOT 58 0x4a6ac0
	virtual void ApplyPropertyEdit(void);									// SLOT 59 0x4a7e50
	virtual void ShowProperties(bool on);									// SLOT 60 0x4a7b30
	virtual bool ToggleOption(int index);									// SLOT 61 0x4a6f90
	virtual void SaveGame(char *name);										// SLOT 62 0x4a8100
	virtual void LoadGame(char *name, int level_id);						// SLOT 63 0x4a8590
	virtual void CheckLoadLevel(void);										// SLOT 64 0x4a8420
	virtual void MakeBackupName(char *name);								// SLOT 65 0x4a8dc0
	virtual void ListObjects(void);											// SLOT 66 0x4a8c70
	virtual CGameObject *FindObject(int id);								// SLOT 67 0x4a8c40
	virtual void RequestLevel(char *name);									// SLOT 68 0x4a8340
	virtual void RequestLevelDelayed(char *name, float delay);				// SLOT 69 0x4a83b0
	virtual void OpenEditorDatabase(void);									// SLOT 70 0x4aa0b0 (vtable +0x118)
	virtual bool IsSingletonLoaded(int id);									// SLOT 71 0x4a8c00
	virtual void ShowObjectList(void);										// SLOT 72 0x4a9d20
	virtual void ShowCatalogue(void);										// SLOT 73 0x4a9a50
	virtual void ResetLevel(void);											// SLOT 74 0x4a8ad0
	virtual void DrawSplash(int canvas_id);									// SLOT 75 0x4aa180 (vtable +0x12c)
	virtual void PlaceSprite(OMediaCanvasElement *e, float x, float y, int canvas_id, float unused, bool scaled);	// SLOT 76 0x4aa500
	virtual void SetSpriteCanvas(OMediaCanvasElement *e, int canvas_id, bool scaled);	// SLOT 77 0x4aa5c0
	virtual OMediaCanvas *GetCanvas(int canvas_id);							// SLOT 78 0x4aa690
	virtual void PlaceSplash(OMediaCanvasElement *e, int unused1, int unused2, int canvas_id);	// SLOT 79 0x4aa3e0
	virtual void ShowPanel(bool on);										// SLOT 80 0x4aa7e0
	virtual int  CountObjects(void);										// SLOT 81 0x4a9c80
	virtual int  CountCollisionObjects(void);								// SLOT 82 0x4a9ce0
	virtual int  CountSavedObjects(void);									// SLOT 83 0x4a9ca0
	virtual void ResetLevelObjects(void);									// SLOT 84 0x4a8a90
	virtual void PasteObject(CGameObject *src);								// SLOT 85 0x4aa6c0

	// ---- helpers (inline in the original)
	void ClearSelection(void);			// zeroes m_sel_element/object/material/polygon and m_pick

	// ---- members (offsets in the 2002 object)
	unsigned char		m_editing;				// 0x8c  editor UI active (neg::EditorIsEditing)
	float				m_cam_save[4];			// 0x90  viewport position kept over InitEditor (w = 1)
	float				f_a0[4];				// 0xa0  (w = 1; only cleared)
	OMediaElement		*m_sel_element;			// 0xb0  selected object's element
	CGameObject			*m_sel_object;			// 0xb4  selected object
	OMedia3DMaterial	*m_sel_material;		// 0xb8  material taken off the picked polygon
	OMedia3DPolygon		*m_sel_polygon;			// 0xbc  picked polygon ("Select Poly")
	OMedia3DShape		*m_sel_shape;			// 0xc0  shape of the picked element
	OMediaPickRequest	*m_pick;				// 0xc4  pick request of the selection
	int					f_c8[13];				// 0xc8  cleared, unused
	OMediaStringField	*m_menu_fields[100];	// 0xfc  right-hand menu (g_editor_menu entries)
	OMediaStringField	*m_list_fields[100];	// 0x28c list: catalogue, objects, levels, toggles, databases
	OMediaStringField	*m_edit_name;			// 0x41c property being edited (name field)
	OMediaStringField	*m_edit_value;			// 0x420 property being edited (value field)
	OMediaStringField	*m_prop_names[100];		// 0x424 property names of the selected object
	OMediaStringField	*m_prop_values[100];	// 0x5b4 property values
	int					m_mode;					// 0x744 0 select, 1 create, 2 select poly, 4 toggles, 5 object list, 6 load game, 7 databases
	float				m_move_speed;			// 0x748 = 100
	unsigned char		m_load_pending;			// 0x74c set: CheckLoadLevel loads m_level_name (cleared by the new level)
	char				m_level_name[0x53];		// 0x74d level to load next
	int					f_7a0;					// 0x7a0 = -1
	char				m_game_name[0x54];		// 0x7a4 last .gam loaded (Save Game writes it back)
	unsigned char		f_7f8;					// 0x7f8 (set by C3DPlayer / C3DTriggerType)
	unsigned char		f_7f9;					// 0x7f9
	short				m_load_countdown;		// 0x7fa frames until m_load_pending (-1 = off)
	float				m_load_delay;			// 0x7fc seconds until m_load_pending (RequestLevelDelayed)
	int					m_list_offset;			// 0x800 first list entry shown (scroll by 20)
	unsigned char		m_key_latch;			// 0x804
	float				m_start_timer;			// 0x808 = 3, counts up
	int					m_start_state;			// 0x80c 1 = first frame (read startlevel.txt), 3 = running
	unsigned char		f_810;					// 0x810
	unsigned char		f_811;					// 0x811
	unsigned char		m_panel_open;			// 0x812 list panel shown (arrow keys scroll instead of fly)
	int					m_label_mode;			// 0x814 object labels: 0 off, 1 near (1000), 2 all
	int					m_list_filter;			// 0x818 object list: 0 all, 1 3D, 2 cutscene, 3 pickups
	CGameObject			*m_copied;				// 0x81c Ctrl-C
	unsigned char		m_paste_latch;			// 0x820 Ctrl-V
};

// --- globals owned by the editor
namespace neg {
extern OMediaCanvas				*g_editor_font_canvas;	// 0x56af00 png\fontsmall.png
extern OMediaCanvasFont			*g_editor_font;			// 0x56af04
extern unsigned char			 g_new_game;			// 0x56af08 set when level1d.gam (a new game) is loaded
extern OMediaCanvasElement		*g_splash;				// 0x56af10
extern OMediaDataBase			*g_editor_db;			// 0x56af14 omt\Editor.omt
extern OMediaFileStream			*g_editor_stream;		// 0x56af18
extern OMediaCanvasElement		*g_editor_menu_panel;	// 0x56af1c canvas 5
extern OMediaCanvasElement		*g_editor_list_panel;	// 0x56af20 canvas 6
extern OMediaCanvasElement		*g_editor_props_panel;	// 0x56af24 canvas 7
}
