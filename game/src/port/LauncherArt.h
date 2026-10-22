/*  port: the Android launcher's pictures (android LauncherView), taken from the player's own game
    files at run time. Not in the original. See LauncherArt.cpp.                              */
#pragma once

namespace neg
{
	// Once screens.omt is open: saves the full-screen pictures the launcher shows to the folder
	// NEG_LAUNCHER_ART names (once each), and, as levels load, each exit's loading picture as
	// level-<gam>.png. Cheap when there is nothing to do. Called every frame (CompanionTick).
	void LauncherArtTick(void);
}
