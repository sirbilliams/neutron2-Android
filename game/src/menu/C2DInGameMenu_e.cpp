/*  C2DInGameMenu part e: inventing gadgets. A collected pickup of the pickups page (page 2) is
    announced (0x408f80): it is counted and shown in the top-right pop-up, and when it completes a
    set of three parts Jimmy says so and the gadget waits in 0x545e64 until his comment has played
    (CJimmyGame::Process), when 0x409b20 hands it out with the "brain" animation (0x405420).

    Sets (page 2 items -> the gadget item):
      6 Flurpbottle, 10 Shades, 11 PVCPipe ("Pic_Watergun")   -> 1, the water gun (Jimmy +0x4ca0 = 1,
                                                                 gadget 6 in hand, "BOT1GO" fired)
      4 Megaphone, 12 Megapipe, 13 Megalamp ("Pic_MegaBurp")   -> 0x13, the burp gun (gadget 10)
      10/46/23 -> 9, 24/25/26 -> 7, 29/28/27 -> 0x14, 30/38/32 -> 8, 33/34/35 -> 5,
      36/38/37 -> 2, 39/40/41 -> 3, 42/43/44 -> 0x12 (inventory items only).
    Reconstructed from Neutron2.exe 0x405420, 0x408f10, 0x408f80, 0x409b20.                    */

#include "../C2DInGameMenu.h"
#include "../C3DJimmy.h"
#include "../C3DTriggerType.h"
#include "../CGameObject.h"
#include "../InterfaceElement.h"
#include "../pickups/C3DPickupItem.h"
#include "../CJimmyGame.h"
#include "../Game.h"
#include "../Sound.h"
#include "../gen/globals.h"
#include <list>
#include <string>
#include <string.h>
#include <strings.h>

#define g_545d94 (*(int *)&neg::g_data[0x13d94])		// the pop-up's pickup index (not read by the game)

using namespace neg;

namespace {

typedef C2DInGameMenu::MenuPage MenuPage;

// 0x4aefb0: the object tagged `tag`, case-insensitive.
CGameObject *FindObjectByTag(const char *tag)
{
	for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
		if (*it && strcasecmp((*it)->m_tag, tag) == 0) return *it;
	return NULL;
}

// 0x404f20: the type of item `item` of page `page` (0 without one).
short GetItemState(short page, short item)
{
	MenuPage *p = C2DInGameMenu::Page(page);
	if (p && item >= 0 && item < C2DInGameMenu::MENU_ITEMS) return p->items[item].type;
	return 0;
}

// 0x404dd0: set item `item` of page `page`. The pickups 10/21 and 31/38 come in pairs.
void SetItemState(short page, short item, short state)
{
	if (item < 0 || item >= C2DInGameMenu::MENU_ITEMS) return;
	MenuPage *p = C2DInGameMenu::Page(page);
	if (p) p->items[item].type = state;
	if (page != C2DInGameMenu::MENU_PICKUPS) return;
	MenuPage *pk = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
	if (item == 10)		 { if (!pk || !pk->items[21].type) { SetItemState(2, 21, 1); g_pickup_counts[2 * 21]++; } }
	else if (item == 21) { if (!pk || !pk->items[10].type) { SetItemState(2, 10, 1); g_pickup_counts[2 * 10]++; } }
	else if (item == 38) { if (!pk || !pk->items[31].type) { SetItemState(2, 31, 1); g_pickup_counts[2 * 31]++; } }
	else if (item == 31) { if (!pk || !pk->items[38].type) { SetItemState(2, 38, 1); g_pickup_counts[2 * 38]++; } }
}

// The gadget item `item` of page 2 (0x545db0 + 0x24 + item * 0x28) is not owned yet. Without
// the page the original skips the test.
bool NotOwned(short item)
{
	MenuPage *p = C2DInGameMenu::Page(C2DInGameMenu::MENU_PICKUPS);
	return !p || p->items[item].type == 0;
}

// 0x48f310: play voice `name` once channel `after` is free (CJimmyGame::Process), and then hand
// out the gadget waiting in 0x545e64.
void QueueVoice(int after, const char *name)
{
	g_queued_sfx_after = (unsigned int)after;
	g_queued_sfx_pending = 1;
	g_queued_sfx = name;
}

// 0x408f10: the interface element of a gadget part.
const char *PartElement(int pic)
{
	switch (pic)
	{
		case 4:  return "Pic_Megaphone";
		case 6:  return "Pic_Flurpbottle";
		case 10: return "Pic_Shades";
		case 11: return "Pic_PVCPipe";
		case 12: return "Pic_Megapipe";
		case 13: return "Pic_Megalamp";
		default: return NULL;
	}
}

InterfaceElt *FindPart(int pic)
{
	const char *n = PartElement(pic);
	return n ? g_interface.Find(n) : NULL;
}

// Jimmy's element (0x545fd0 - 0xc0).
C3DJimmy *JimmyElt(void)	{ return g_545fd0 ? dynamic_cast<C3DJimmy *>(g_545fd0) : NULL; }

// The inventory sets of the tail of 0x408f80: all three items there and the gadget not owned.
void CheckSet(short a, short b, short c, short gadget)
{
	if (GetItemState(2, a) && GetItemState(2, b) && GetItemState(2, c) && NotOwned(gadget))
	{
		QueueVoice(g_voice_channel, "jimmyself121");
		g_545e64 = gadget;
	}
}

} // namespace

// 0x405420: an interface animation by name; only "ANTIHOLO" (the brain and its formulas, `t`
// scales the timing) exists.
void neg::MenuInterfaceEffect(const char *name, float t)
{
	if (strcasecmp(name, "ANTIHOLO") != 0) return;
	InterfaceElt *brain = g_interface.Find("Brain");
	InterfaceElt *f1 = g_interface.Find("Formula1");
	InterfaceElt *f2 = g_interface.Find("Formula2");
	InterfaceElt *f3 = g_interface.Find("Formula3");
	InterfaceElt *f4 = g_interface.Find("Formula4");
	brain->Reset();
	f1->Reset();
	f2->Reset();
	f3->Reset();
	f4->Reset();
	brain->Hide();
	brain->QueueFade(0, 0.1f, true, true, 0.0f, t * 0.3f);
	brain->QueueMorph(0, t * 1.5f, true, true, true, 1.0f, 1.0f, 320.0f, 220.0f, 320.0f, 240.0f, 180.0f, 10.0f);
	brain->QueueFade(0, t * 0.5f, true, true, 0.3f, 0.7f);
	brain->QueueWait(0, t);
	brain->QueueFade(0, t * 1.5f, false, true, 0.0f, 0.7f);
	f2->QueueWait(0, t);
	f2->QueueMorphFlash(0, t * 1.5f, true, true, true, 100.0f, 10.0f, 200.0f, 100.0f, 120.0f, 240.0f, 330.0f, 240.0f);
	f2->QueueMorphFlash(0, t * 1.5f, true, true, true, 10.0f, 10.0f, 200.0f, 100.0f, 320.0f, 140.0f, 130.0f, 240.0f);
	f3->QueueWait(0, t * 0.5f);
	f3->QueueMorphFlash(0, t + t, true, true, true, 50.0f, 300.0f, 200.0f, 100.0f, 120.0f, 240.0f, 330.0f, 40.0f);
	f3->QueueMorphFlash(0, t + t, true, true, true, 50.0f, 300.0f, 200.0f, 100.0f, 320.0f, 140.0f, 10.0f, 40.0f);
	f4->QueueWait(0, t * 0.8f);
	f4->QueueMorphFlash(0, t * 2.5f, true, true, true, 50.0f, 50.0f, 200.0f, 100.0f, 100.0f, 0.0f, 300.0f, 150.0f);
	f4->QueueMorphFlash(0, t * 2.5f, true, true, true, 50.0f, 50.0f, 200.0f, 100.0f, 240.0f, 320.0f, 400.0f, 0.0f);
	// (Formula1 is only reset)
}

// 0x408f80: pickup `pic` of page 2 was collected (its item is already set). Only while playing.
void neg::AnnouncePickup(int pic, int index)
{
	if (g_game_mode != 5) return;

	// the top-right pop-up: shown for 3 s, or queued behind the one showing
	if (g_545d90 > 0.0f)
	{
		if (g_545d88 != -1) g_545d8c = pic;
	}
	else
	{
		g_545d8c = -1;
		g_545d88 = pic;
		g_545d90 = 3.0f;
		g_545d94 = index;
	}

	bool sets = true;									// on to the inventory sets (0x40953d)
	int group = -1;
	if (pic == 6 || pic == 10 || pic == 11) group = 1;
	else if (pic == 4 || pic == 12 || pic == 13) group = 0x13;
	if (group != -1)
	{
		// the three parts' pictures show which are owned (+0x9c); the order of the three finds
		// differs by case in the original, not the result
		const int parts[2][3] = { { 11, 6, 10 }, { 12, 13, 4 } };
		const int *pp = parts[group == 1 ? 0 : 1];
		InterfaceElt *e[3];
		for (int k = 0; k < 3; k++)
		{
			e[k] = FindPart(pp[k]);
			if (e[k]) e[k]->f_9c = GetItemState(2, (short)pp[k]);
		}
		if (e[0] && e[1] && e[2])
		{
			sets = false;
			if (e[0]->f_9c && e[1]->f_9c && e[2]->f_9c)
			{
				g_interface.Find(group == 1 ? "Pic_Watergun" : "Pic_MegaBurp");	// (result unused)
				if (FindObjectByTag("JIM1"))
				{
					if (group == 1)
					{
						if (NotOwned(1)) { QueueVoice(g_voice_channel, "jimmyself85"); g_545e64 = 1; }
					}
					else if (NotOwned(0x13)) { QueueVoice(g_voice_channel, "jimmyself86"); g_545e64 = 0x13; }
					for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
						if (*it && (*it)->NameContains((char *)"C3DPICKUPITEM"))
							dynamic_cast<C3DPickupItem *>(*it)->SyncWithMenu();	// element +0xec
				}
			}
		}
	}
	if (sets)
	{
		CheckSet(10, 46, 23, 9);
		CheckSet(24, 25, 26, 7);
		CheckSet(29, 28, 27, 0x14);
		CheckSet(30, 38, 32, 8);
		CheckSet(33, 34, 35, 5);
		CheckSet(36, 38, 37, 2);
		CheckSet(39, 40, 41, 3);
		CheckSet(42, 43, 44, 0x12);
	}
	g_pickup_counts[2 * pic]++;
}

// 0x409b20: after Jimmy's comment (CJimmyGame::Process), the gadget waiting in 0x545e64.
void neg::GivePendingInvention(void)
{
	if (g_545e64 == 1)
	{
		MenuInterfaceEffect("ANTIHOLO", 1.0f);
		C3DJimmy *j = JimmyElt();						// port: the original has no NULL test
		if (j) j->f_4ca0 = 1;
		SetItemState(2, 1, 1);
		AnnouncePickup(1, 0);
		if (j)
		{
			j->ev138(g_545e78, g_545e78, 6, 13.0f);		// element +0x228: the water gun in hand
			j->ev58((char *)"INVENT", 0);				// element +0xe8
		}
		CGameObject *t = FindObjectByTag("BOT1GO");
		if (t && t->NameContains((char *)"C3DTRIGGERTYPE"))
			dynamic_cast<C3DTriggerType *>(t)->CallNextTrigger("BOT1GO", 0);	// element +0xdc
		g_545e64 = 0;
		return;
	}
	if (g_545e64 == 0x13)
	{
		MenuInterfaceEffect("ANTIHOLO", 1.0f);
		C3DJimmy *j = JimmyElt();
		if (j) j->f_4ca0 = 2;
		SetItemState(2, 0x13, 1);
		AnnouncePickup(0x13, 0);
		if (j)
		{
			j->ev138(g_545e78, g_545e78, 10, 13.0f);	// the burp gun
			j->ev58((char *)"INVENT", 0);
		}
		g_545e64 = 0;
		return;
	}
	if (g_545e64 != 0)
	{
		MenuInterfaceEffect("ANTIHOLO", 1.0f);
		SetItemState(2, (short)g_545e64, 1);
		AnnouncePickup(g_545e64, 0);
		for (std::list<CGameObject *>::iterator it = g_objects.begin(); it != g_objects.end(); ++it)
			if (*it && (*it)->NameContains((char *)"C3DPICKUPITEM"))
				dynamic_cast<C3DPickupItem *>(*it)->SyncWithMenu();
		g_545e64 = 0;
	}
}
