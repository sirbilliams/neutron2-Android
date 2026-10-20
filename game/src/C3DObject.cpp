/*  C3DObject - see C3DObject.h. */
#include "C3DObject.h"
#include "Game.h"
#include "Bridge.h"
#include "Todo.h"
#include "Physics.h"
#include "OMediaWorld.h"
#include "OMediaLayer.h"

using namespace neg;


C3DObject::C3DObject()										// 0x49a920
{
	SetName((char *)"C3DOBJECT");
	Log("C3DObject()");
	OMediaElement::set_id('3OBJ');							// element +0x78
	m_object_id = '3OBJ';									// CGameObject +0x404
	f_588 = 0;
	f_589 = 1;
}

C3DObject::~C3DObject()										// 0x49aa50
{
	Log("~C3DObject()");
}

void C3DObject::set_shape(OMedia3DShape *shape)				// 0x49ab10
{
	OMedia3DShapeElement::set_shape(shape);
	if (shape) FitToShape();								// element vtable +200
}

void C3DObject::SetObjectID(int id)							// 0x49ab00
{
	OMediaElement::set_id(id);
	m_object_id = id;
}

void C3DObject::FitToShape(void)							// 0x49ab50: radius and extents from the shape
{
	f_414 = get_shape();
	NEG_TODO(0x49ab50, "C3DObject::FitToShape (radius v67, extents v185/v183 from find_cube)");
}

void C3DObject::ev51(void) { NEG_TODO(0x49afd0, "C3DObject ev51 (shape materials flag on)"); }
void C3DObject::ev52(void) { NEG_TODO(0x49b030, "C3DObject ev52 (shape materials flag off)"); }
void C3DObject::ev53(void) { NEG_TODO(0x49b090, "C3DObject ev53 (scale from extents)"); }

void C3DObject::ShowAgain(void)								// 0x49b100
{
	f_589 = 1;
	force_show();											// hide_count = 1; hide(false)
}

void C3DObject::HideAgain(void)								// 0x49b120
{
	if (f_589)
	{
		f_589 = 0;
		hide(true);
	}
}

void C3DObject::InitObject(void)							// 0x49ac20
{
	Log("InitObject()");
	CLocalGameObject::InitObject();
	link(g_world);											// element +0x2c
	link_layer(g_layer_world);								// element +0x34 (0x56afc8)
}

void C3DObject::UnInitObject(void)							// 0x49ac70
{
	Log("UnInitObject()");
	CLocalGameObject::UnInitObject();
	set_shape(NULL);										// element +0xac
	f_414 = NULL;
}

void C3DObject::InitPhysics(void)							// 0x49acb0
{
	Log("InitPhysics()");
	CLocalGameObject::InitPhysics();
	v90(x, y, z);											// position from the element (+0x44..)
	const float k = 360.0f / 16384.0f;						// 0x4cb1f8: OMT angle units -> degrees
	v92((float)anglex * k, (float)angley * k, (float)anglez * k);
	PhysicsAddObject(this);									// 0x4b4ae0 Add3DObject
}

void C3DObject::UnInitPhysics(void)							// 0x49ad60
{
	Log("UnInitPhysics()");
	CGameObject::UnInitPhysics();
}

void C3DObject::Process(float dt)							// 0x49ae00
{
	NEG_TODO(0x49ae00, "C3DObject::Process (attached-to-object following)");
	CLocalGameObject::Process(dt);
}

void C3DObject::v243(float a) { (void)a; NEG_TODO(0x49af30, "C3DObject v243 (camera follow)"); }

void C3DObject::SyncElementPosition(void)					// 0x49ad90
{
	float p[4];
	v195(p);												// CGameObject +0x30c: current position
	x = p[0]; y = p[1]; z = p[2];
}

void C3DObject::v248(void) { NEG_TODO(0x49add0, "C3DObject v248 (random rotation)"); }

void C3DObject::SetElementAngles(short ax, short ay, short az)	// 0x49ade0
{
	anglex = ax; angley = ay; anglez = az;
}
