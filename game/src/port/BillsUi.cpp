/*  port: "Bill's UI" - see BillsUi.h. */
#include "BillsUi.h"
#include <SDL.h>
#include <stdlib.h>
#include <strings.h>

namespace neg {

static volatile int s_bills = -1;	// -1: not read yet

bool BillsUi(void)
{
	if (s_bills < 0)
	{
		const char *e = getenv("NEG_UI");
		s_bills = (e && strcasecmp(e, "vanilla") == 0) ? 0 : 1;	// default: Bill's UI
		SDL_Log("In-game UI: %s", s_bills ? "Bill's UI" : "Vanilla");
	}
	return s_bills != 0;
}

void SetBillsUi(bool on)
{
	s_bills = on ? 1 : 0;
}

}

#ifdef __ANDROID__
#include <jni.h>

// Companion.setBillsUi (the in-game port menu's "In-game UI" item)
extern "C" JNIEXPORT void JNICALL
Java_com_sirbilliams_negatron_Companion_nativeSetBillsUi(JNIEnv *, jclass, jboolean on)
{
	neg::SetBillsUi(on != JNI_FALSE);
}
#endif
