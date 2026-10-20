/*  The game's caches of open OMT databases (omt\*.omt), by file name.
    0x4a4180 GetDatabase  0x4a4560 GetPermanentDatabase  0x4a4730 CloseDatabase
    0x4a4870 CloseAllDatabases  0x4a4960 CloseAllPermanentDatabases  0x4a4a40 DatabaseName

    Two lists: the level's databases (0x56aef4, closed by CloseAllDatabases when a level is
    loaded) and the permanent ones (0x56aee4, kept for the whole game: Icons.omt and the
    permanent icons).                                                                         */
#pragma once

class OMediaDataBase;
class OMediaFileStream;

namespace neg {

struct DatabaseEntry									// 0x70 bytes, operator new
{
	OMediaDataBase		*db;							// 0x00
	OMediaFileStream	*fs;							// 0x04
	char				 name[100];						// 0x08  as asked for ("level1.omt")
	int					 open;							// 0x6c
};

OMediaDataBase *GetDatabase(const char *name);			// 0x4a4180
OMediaDataBase *GetPermanentDatabase(const char *name);	// 0x4a4560
void CloseDatabase(const char *name);					// 0x4a4730
void CloseAllDatabases(void);							// 0x4a4870
void CloseAllPermanentDatabases(void);					// 0x4a4960
const char *DatabaseName(short index);					// 0x4a4a40  name of the index-th level database, NULL if closed

}
