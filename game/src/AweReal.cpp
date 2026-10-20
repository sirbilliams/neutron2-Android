/*  CAweReal, omt_main and start-up helpers.  Reconstructed from Neutron2.exe.
    0x4a3c00 omt_main   0x4a3380 CAweReal::listen_to_message   0x4b3260 CInput::CInput
    0x4b7db0 InitSoundEngine   0x496110 SetWindowTitle   0x496200 CheckCD   0x4b3ac0 OpenDebugLog */

#include "Game.h"
#include "CGameType.h"
#include "CDefaultGame.h"
#include "CViewPort.h"
#include "Todo.h"
#include "gen/globals.h"
#include "OMediaWindow.h"
#include "OMediaEventManager.h"
#include "OMediaEngineFactory.h"
#include "OMediaSoundEngine.h"
#include "OMediaInputEngine.h"
#include "OMediaDefMessages.h"
#include "OMediaError.h"
#include <SDL.h>
#include <string.h>
#include <stdlib.h>
#include "OMediaPeriodical.h"

using namespace neg;

// Granny (RAD) - replaced by the converted model data on this port; see docs/granny-api.md.
static int GrannyOpenVersionStub(void) { return 0; }

// --------------------------------------------------------------------------- CAweReal

CAweReal::CAweReal() : OMediaApplication(omchint_BestInit, true)		// OMediaApplication(2, 1)
{
	SDL_Log("CAweReal()");
	memset(g_keys, 0, sizeof(g_keys));			// 0x56ada0, 0x40 dwords
	g_app = this;								// 0x56aec4
	if (GrannyOpenVersionStub() != 0) SDL_Log("GRANNY FAILED TO INITIALIZE");
	// GrannyAllowTextureFormat(ver, 4)
	g_granny_scale = 100;						// 0x56aea4
}

CAweReal::~CAweReal() {}

// 0x4a3380. The original reacts to raw Windows messages that OMT's event filter forwards
// (WM_KEYDOWN/UP fill g_keys[VK], WM_ACTIVATEAPP restores the display, WM_USER+n = AWEMSG_*).
// On this port the filter carries SDL events, translated here to the same effects.
void CAweReal::listen_to_message(omt_Message msg, void *param)
{
	switch (msg)
	{
		case omsg_neg_DeActivate:
			SDL_Log("omsg_DeActivate");
			if (g_gametype)
			{
				g_53f27c = g_53b5b4;			// remember the game state (0x53b5b4, CJimmyGame)
				if (g_53b5b4 == 5 && !g_56aed0) g_gametype->PauseGame(true);	// vtable +0x110
				g_gametype->OnDeactivate();		// vtable +0x120
			}
			break;

		case omsg_neg_Activate:
			SDL_Log("omsg_Activate");
			if (g_gametype)
			{
				if (g_53f27c == 5) g_gametype->PauseGame(false);	// vtable +0x110
				g_53f27c = 9;
				g_gametype->OnActivate();		// vtable +0x11c
			}
			break;

		case omsg_LowlevelEventFilter:
		{
			OMediaFilterLocalEvent *f = (OMediaFilterLocalEvent*)param;
			SDL_Event *e = f ? (SDL_Event*)f->local_event : NULL;
			if (!e) break;
			if (e->type == SDL_KEYDOWN || e->type == SDL_KEYUP)
			{
				if (!e->key.repeat) SDLKeyToVK(e->key.keysym.scancode, e->type == SDL_KEYDOWN);
				// F4 with the cheat/debug flag set: "EXIT REQUEST, CALLING QUIT (CLEAN EXIT)"
				if (e->type == SDL_KEYDOWN && e->key.keysym.sym == SDLK_F4 && g_cheat_keys) quit();
			}
			else if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				OMediaApplication::listen_to_message(omsg_neg_Activate, NULL);
			else if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				OMediaApplication::listen_to_message(omsg_neg_DeActivate, NULL);
			else if (e->type == SDL_QUIT)
				SDL_Log("EXIT REQUEST, CALLING QUIT (CLEAN EXIT)");
			break;
		}

		case omsg_Event:					// -0x65: game event switch (0x56ad9c / 0x56aea6 flags)
			// TODO(0x4a3380): cases 1..4
			break;
	}
	OMediaApplication::listen_to_message(msg, param);
}

// --------------------------------------------------------------------------- CInput 0x4b3260

CInput::CInput()
{
	SDL_Log("CInput()");
	g_input_engine = OMediaEngineFactory::get_factory()->create_input_engine(ommeic_Best, g_window);
}

CInput::~CInput() { delete g_input_engine; g_input_engine = NULL; }

// --------------------------------------------------------------------------- helpers

void neg::OpenDebugLog(void)			// 0x4b3ac0: "dat\debug.log", "%s UP @ %s on %s"
{
	SDL_Log("Negatron UP");
}

void neg::SetWindowTitle(void)			// 0x496110
{
	if (g_window) g_window->set_title("Jimmy Neutron 2");
}

bool neg::CheckCD(void)					// 0x496200: "Please insert the Jimmy Neutron 2 CDROM"
{
	return true;						// no CD on this port
}

void neg::InitSoundEngine(void)			// 0x4b7db0
{
	SDL_Log("InitSoundEngine()");
	// The original probes DirectSoundCreate first and disables sound if it fails, then prefers an
	// engine with omseaf_NoChannelLimitation and reserves 32 channels.
	OMediaEngineFactory *f = OMediaEngineFactory::get_factory();
	omt_EngineID id = f->sound_engines.empty() ? ommeic_Best : f->sound_engines.front();
	for (omt_EngineList::iterator i = f->sound_engines.begin(); i != f->sound_engines.end(); i++)
		if (f->get_sound_engine_attr(*i) & omseaf_NoChannelLimitation) { id = *i; break; }
	g_sound = f->create_sound_engine(id, g_window);
	if (g_sound) g_sound->set_nchannels(0x20);
	// 0x6f751c / 0x6f7334 / 0x6f73d4: three 32-entry tables cleared (channel bookkeeping)
}

static void PhysicsReset(void)			{ NEG_TODO(0x4b40d0, "CPhysics::Reset"); }

// 0x4ae4a0: every object is un-initialised, then loses its databases, then is deleted (each
// destructor takes itself off g_objects). The running game type is one of them; g_gametype is left
// pointing at it until the next game type is made.
static void DeleteAllObjects(void)
{
	SDL_Log("+DeleteAllObjects()");
	// "+UnInitObjects()"
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		(*i)->UnInitObject();				// vtable +0x20
	// "-UnInitObjects()"  "+UnInitObjectDatabases()"
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); i++)
		(*i)->UnInitDatabases();			// vtable +0x34
	// "-UnInitObjectDatabases()"
	while (g_objects.size() != 0)
		if (g_objects.front()) delete g_objects.front();	// (a NULL entry would loop for ever, as in the original)
	SDL_Log("-DeleteAllObjects()");
}

// Inline in omt_main (0x4a3ea8) and in CAweReal::listen_to_message for AWEMSG_LOAD (0x4a3380).
void neg::LoadNewLevel(unsigned int which)
{
	SDL_Log("+LoadNewLevel(%c%c%c%c)", (which >> 24) & 0xff, (which >> 16) & 0xff, (which >> 8) & 0xff, which & 0xff);
	CGfx::UnInitCaptions();				// 0x4b16a0
	PhysicsReset();						// 0x4b40d0
	DeleteAllObjects();					// 0x4ae4a0
	CGfx::UnInitAlpha();				// 0x4a0ef0 = UnInitAlpha + InitAlpha
	CGfx::InitAlpha();
	ClearWayPoints();					// 0x4b9b60
	CreateGameType(which);				// 0x480230: a CDefaultGame, whatever the id
	ResetFrameTimer();					// 0x4b9600
	SDL_Log("-LoadNewLevel()");
}

// Windows virtual-key codes for the keys the game reads (keys.txt)
void neg::SDLKeyToVK(int sc, bool down)
{
	int vk = 0;
	if (sc >= SDL_SCANCODE_A && sc <= SDL_SCANCODE_Z) vk = 'A' + (sc - SDL_SCANCODE_A);
	else if (sc >= SDL_SCANCODE_1 && sc <= SDL_SCANCODE_9) vk = '1' + (sc - SDL_SCANCODE_1);
	else if (sc == SDL_SCANCODE_0) vk = '0';
	else if (sc >= SDL_SCANCODE_F1 && sc <= SDL_SCANCODE_F12) vk = 0x70 + (sc - SDL_SCANCODE_F1);
	else switch (sc)
	{
		case SDL_SCANCODE_RETURN: case SDL_SCANCODE_KP_ENTER: vk = 0x0d; break;	// VK_RETURN
		case SDL_SCANCODE_ESCAPE: case SDL_SCANCODE_AC_BACK:  vk = 0x1b; break;	// VK_ESCAPE
		case SDL_SCANCODE_SPACE:  vk = 0x20; break;
		case SDL_SCANCODE_LEFT:   vk = 0x25; break;
		case SDL_SCANCODE_UP:     vk = 0x26; break;
		case SDL_SCANCODE_RIGHT:  vk = 0x27; break;
		case SDL_SCANCODE_DOWN:   vk = 0x28; break;
		case SDL_SCANCODE_LSHIFT: case SDL_SCANCODE_RSHIFT: vk = 0x10; break;
		case SDL_SCANCODE_LCTRL:  case SDL_SCANCODE_RCTRL:  vk = 0x11; break;
		case SDL_SCANCODE_LALT:   case SDL_SCANCODE_RALT:   vk = 0x12; break;
		case SDL_SCANCODE_TAB:    vk = 0x09; break;
		case SDL_SCANCODE_BACKSPACE: vk = 0x08; break;
		default: break;
	}
	if (vk) g_keys[vk] = down ? 1 : 0;
}

// --------------------------------------------------------------------------- port test hook
// NEG_TEXT_TEST=1 draws a line in every font each frame (until the menu is reconstructed).
class CTextTest : public OMediaPeriodical
{
	public:
	virtual void spend_time(void)
	{
		ClearText();
		DrawText(-1, 40, 'BIG', "JIMMY NEUTRON");
		DrawText(-1, 110, 'LARG', "VS JIMMY NEGATRON");
		DrawText(40, 200, 0, "DEFAULT 0123456789");
		DrawText(40, 250, 'SMAL', "SMALL: HELLO, RETROVILLE!");
		DrawText(40, 300, 'MED', "MEDIUM 0123456789");
		DrawText(40, 360, 'TIME', "TIME 12345");
		g_text_center = 1;
		DrawText(320, 430, 'SMAL', "CENTRED AT X=320");
		g_text_center = 0;
	}
};

// --------------------------------------------------------------------------- 0x4a3c00 omt_main

void omt_main(void)
{
	OpenDebugLog();
	// (Windows: refuses to start twice - FindWindow("OMT SingleWindow") titled "Jimmy Neutron 2")
	try
	{
		CAweReal *app = new CAweReal;
		CGfx *gfx = new CGfx;					// 0x4b0e80 (creates window, world, layers)
		SetWindowTitle();						// 0x496110
		OMediaEventManager *em = OMediaEventManager::get_event_manager();
		if (em)
		{
			em->get_msg_filter_broadcaster()->addlistener(app);	// event_manager +0x2c
			em->getkeybroadcaster()->addlistener(app);				// event_manager +0x1c
		}
		if (!CheckCD()) { delete gfx; delete app; return; }

		// (**(window vtable + 0x50))(): show
		g_window->show();
		InitSoundEngine();						// 0x4b7db0
		if (g_windowed) { g_screen_w = 400; g_screen_h = 300; }
		CGfx::SetDisplayMode(g_screen_w, g_screen_h, g_screen_depth, g_windowed);	// 0x4b18a0
		CInput *input = new CInput;				// 0x4b3260

		// TODO(0x4a4aa0): g_editor = new CEditor(1) (0x82c bytes). The original loads the level and
		// runs only when it exists; until CEditor is reconstructed this runs without it.
		LoadNewLevel('MAIN');
		SDL_Log("omtmain 1");

		CTextTest *text_test = getenv("NEG_TEXT_TEST") ? new CTextTest : NULL;
		if (g_gametype)
		{
			SDL_Log("omtmain 2: entering main loop");
			ErrorLog("VER: RC.03   %s  %s", "Aug 27 2002", "18:31:08");
			app->start();						// OMediaApplication::start (vtable +0x10)
		}
		SDL_Log("omtmain 3");
		DeleteAllObjects();						// 0x4ae4a0
		CGfx::UnInitAlpha();					// 0x4a0e10

		SDL_Log("EXITING GAME");
		// TODO(0x4a4870, 0x4a4960): object / sound databases;  TODO(0x4a4aa0): delete g_editor
		delete text_test;
		delete input;
		delete gfx;
		delete app;
	}
	catch (OMediaError &e)
	{
		SDL_Log("OMT error %d (data %d) at %s:%d", (int)e.errorcode, (int)e.errordata, e.file.c_str(), e.line);
	}
	SDL_Log("CLEAN EXIT");
}
