#include "mathStructs.h"

// offset from blackops3.exe
const unsigned int playerListOffset = 0xA3D19F0;
const unsigned int localPlayerInfoOffset = 0x4D18740;
const unsigned int fovOffset = 0x17AC6388;
const unsigned int zoomOffest = 0xF8C3254;

struct ViewAngles 
{
	float pitch, yaw;
};

const unsigned int velocityOffset = 0x3C;
const unsigned int viewAnglesOffset = 0x318;
struct LocalPlayerInfo 
{
	char pad1[velocityOffset];
	Vector3 velocity;
	
	char pad2[viewAnglesOffset - velocityOffset - sizeof(velocity)];
	ViewAngles viewAngles;
};

const unsigned int isAliveOffset = 0x86;
const unsigned int teamOffset = 0x1A4;
const unsigned int eyeHeightOffset = 0x210;
const unsigned int posOffset = 0x230;
const unsigned int listPlayerSize = 0x4F8;
struct ListPlayer 
{
	int index;

	char pad1[isAliveOffset - sizeof(index)];
	bool isAlive;

	char pad2[teamOffset - isAliveOffset - sizeof(isAlive)];
	int team;
	
	char pad3[eyeHeightOffset - teamOffset - sizeof(team)];
	float eyeHeight;

	char pad4[posOffset - eyeHeightOffset - sizeof(eyeHeight)];
	Vector3 pos;

	char pad5[listPlayerSize - posOffset - sizeof(pos)];
};

struct PlayerList
{
	ListPlayer players[64];
};