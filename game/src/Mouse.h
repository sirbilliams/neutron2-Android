/*  The mouse pointer helpers of Neutron2.exe (CGfx 0x4b1d10..0x4b1dc0, CAweReal 0x4a3990), shared.
    Defined in Input.cpp. Several modules still have private static TODO copies (C2DInGameMenu_b
    GetMousePos / SetMousePos / CenterMouse; C3DPlayer, C3DFlyingObject and CEditor GetMouseDelta):
    they can call these instead. On the Thor the right stick moves the pointer (engine/sdl), so
    GetMouseDelta is also the right stick's camera turn.                                          */
#pragma once

namespace neg
{
void GetMousePos(short *x, short *y);		// 0x4b1d10  the pointer, logical window coordinates
void SetMousePos(short x, short y);			// 0x4b1d30
short MouseCentreX(void);					// 0x4b1da0  half the viewport's width
short MouseCentreY(void);					// 0x4b1dc0  half its height
void CenterMouse(void);						// 0x4b1d50  the pointer to the viewport's centre
void GetMouseDelta(short *dx, short *dy);	// 0x4a3990  centre - pointer, then re-centred
}
