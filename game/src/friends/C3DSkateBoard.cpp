/*  C3DSkateBoard - see C3DSkateBoard.h. Reconstructed from Neutron2.exe 0x46ff50..0x470250. */
#include "C3DSkateBoard.h"
#include "../Game.h"

using namespace neg;

C3DSkateBoard::C3DSkateBoard()								// 0x46ff50
{
	SetName((char *)"C3DSKATEBOARD");
	Log("C3DSkateBoard()");
	C3DSkateBoard::InitObject();							// 0x470170 inlined: InitObject() called directly
	C3DObject::SetObjectID('3BOA');							// 0x49ab00
	v40(0);													// CGameObject +0xa0
	v42(0);
	C3DObject::InitPhysics();								// 0x49acb0
	v52(0);
	v132(0);												// no collisions
	v107(0);
	// (then a call to the empty 0x463890)
}

C3DSkateBoard::~C3DSkateBoard()								// 0x470120
{
}

void C3DSkateBoard::InitObject(void)						// 0x470170
{
	Log("InitObject()");
	C3DAnimated::InitObject();								// 0x41b0a0
	InitAnim();												// element +0x110
	CreateAnim((char *)"BOARD", (char *)"board.ase");		// +0xe0
	CreateTexture((char *)"skate.png", 0);					// +0xfc
	ApplyTexture(f_590, 0);									// +0x100
	v67(1.0f);												// CGameObject +0x10c: radius
	ev58((char *)"BOARD", 1);								// +0xe8
	// (then a call to the empty 0x463890)
}
