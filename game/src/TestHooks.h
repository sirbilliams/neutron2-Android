/*  port: test-only hooks (desktop and Android; inert unless set), see TestHooks.cpp. */
#pragma once

namespace neg {
const char *TestPlayLevel(void);			// CEditor start-up: NEG_TEST_PLAY (or NULL) is the level to start with
void TestPlayHook(const char *level);		// end of CEditor::LoadGame: NEG_TEST_PLAY=<level.gam>
}
