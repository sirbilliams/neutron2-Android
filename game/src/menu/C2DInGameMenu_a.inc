// C2DInGameMenu part a: slot declarations (inside the class). See docs/decomp-guide.md.
// Constructor 0x401380 and destructor 0x403fa0 are declared in C2DInGameMenu.h
// (slot 0 0x403f80 is the compiler's deleting destructor around 0x403fa0).

	// Closes screens.omt: the base's UnInitObject, then the screens database and its stream.
	virtual void UnInitObject();											// SLOT 8 0x4043d0

	// --- member types (set up by the constructor)
	// TYPE 0x4e0 float
	// TYPE 0x4e8 int
	// TYPE 0x4f0 float
	// TYPE 0x4f4 float
	// TYPE 0x588 int
	// TYPE 0x590 int
	// TYPE 0x594 int
	// TYPE 0x598 float
	// TYPE 0x5a0 float
	// TYPE 0x5ac float
	// TYPE 0x5b0 OMediaCanvasElementWrapper *
