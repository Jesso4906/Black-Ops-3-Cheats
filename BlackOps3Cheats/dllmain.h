#include "mathStructs.h"
#include "memoryTools.h"
#include "blackOps3Types.h"
#include "directx11.h"

void Esp(ImDrawList* drawList, ImGuiIO io);

ViewAngles GetFov();

ViewAngles CalculateViewAnglesDiff(PlayerList* playerList, int targetPlayerIndex, float* outDistance);

int GetClosestPlayerToCrosshair();

void MoveMouse(float deltaX, float deltaY);

bool IsCursorInWindow();