/*  Marks code that is not reconstructed yet. Each site logs its original address the first time it
    runs, so a play session lists exactly which functions it needed.                          */
#pragma once
#include <SDL.h>

#define NEG_TODO(addr, what) do { static bool _neg_once = false; \
	if (!_neg_once) { _neg_once = true; SDL_Log("TODO %#x %s", (unsigned)(addr), what); } } while (0)
