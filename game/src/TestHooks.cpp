/*  port: test-only hooks (desktop builds only; not in the original).

    NEG_TEST_PLAY=<level.gam>  When that level has loaded, its start trigger (the opening cut
                               scene) is not fired, Jimmy gets the camera and the game mode is 5
                               (Playing): his controls can be tried in a level whose story
                               sequence cannot run yet (level1d's logo intro, level5's museum
                               cut scene whose classes do not exist yet).                     */
#include "TestHooks.h"
#include "C3DJimmy.h"
#include "C2DInGameMenu.h"
#include "CGameType.h"
#include "objects/C3DStartPoint.h"
#include "Game.h"
#include <SDL.h>
#include <stdlib.h>
#include <strings.h>

void neg::TestPlayHook(const char *level)
{
#ifndef __ANDROID__
	const char *want = getenv("NEG_TEST_PLAY");
	if (!want || strcasecmp(want, level) != 0) return;
	C3DJimmy *jim = Jimmy();
	if (!jim || !g_gametype) return;
	for (std::list<CGameObject *>::iterator i = g_objects.begin(); i != g_objects.end(); ++i)
		if (C3DStartPoint *s = dynamic_cast<C3DStartPoint *>(*i)) s->m_trigger_armed = 0;
	g_gametype->SetCamera(jim);
	SetGameMode(5);
	SDL_Log("NEG_TEST_PLAY: %s without its start trigger, Jimmy playing", level);
#else
	(void)level;
#endif
}
