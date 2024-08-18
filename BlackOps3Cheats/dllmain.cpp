#include "dllmain.h"

uintptr_t bo3BaseAddress = 0;

bool enableAimbot = true;
float aimbotStrength = 4;

bool enableEsp = true;

bool enableCrosshair = true;

bool targetSameTeam = false;

DWORD WINAPI Thread(LPVOID param)
{
	bo3BaseAddress = (uintptr_t)GetModuleHandle(L"blackops3.exe");
	
	if (bo3BaseAddress == 0 || !HookPresent())
	{
		FreeLibraryAndExitThread((HMODULE)param, 0);
		return 0;
	}

	bool aimbot = false;
	int aimbotTargetPlayerIndex = -1;
	int aimbotTimer = 0;

	while (!GetAsyncKeyState(VK_INSERT)) // exit when ins key is pressed
	{
		if (enableAimbot && GetAsyncKeyState(VK_MBUTTON) & 1)
		{
			aimbot = !aimbot;
			aimbotTargetPlayerIndex = GetClosestPlayerToCrosshair();
			if (aimbotTargetPlayerIndex == -1) { aimbot = false; }
		}

		if (aimbot)
		{
			aimbotTimer++;
			if (aimbotTimer < 1000) { continue; }
			aimbotTimer = 0;

			PlayerList* playerList = *(PlayerList**)(bo3BaseAddress + playerListOffset);
			if (IsBadReadPtr(playerList, sizeof(playerList))) { continue; }
			
			ViewAngles viewAnglesDiff = CalculateViewAnglesDiff(playerList, aimbotTargetPlayerIndex, nullptr);
			if (!IsCursorInWindow() || !playerList->players[aimbotTargetPlayerIndex].isAlive || !playerList->players[0].isAlive) { aimbot = false; continue; }

			float zoom = *(float*)(bo3BaseAddress + zoomOffest);
			MoveMouse(viewAnglesDiff.yaw * (aimbotStrength / zoom), viewAnglesDiff.pitch * (aimbotStrength / zoom));
		}
	}

	UnhookPresent();

	Sleep(100);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	if (mainRenderTargetView) { mainRenderTargetView->Release(); mainRenderTargetView = NULL; }
	if (p_context) { p_context->Release(); p_context = NULL; }
	if (p_device) { p_device->Release(); p_device = NULL; }
	SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)(oWndProc));
	FreeLibraryAndExitThread((HMODULE)param, 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH) 
	{ 
		CreateThread(0, 0, Thread, hModule, 0, 0);
	}
	
	return TRUE;
}

void Draw() // called in DetourPresent()
{
	ImU32 primaryColor = IM_COL32(255, 80, 0, 255);
	ImU32 secondaryColor = IM_COL32(150, 50, 0, 255);
	
	ImGui::PushStyleColor(ImGuiCol_CheckMark, primaryColor);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, primaryColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBgActive, primaryColor);

	ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, secondaryColor);
	ImGui::PushStyleColor(ImGuiCol_TitleBg, secondaryColor);
	ImGui::PushStyleColor(ImGuiCol_TitleBgActive, secondaryColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, secondaryColor);
	ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, secondaryColor);
	
	ImGui::Begin("Black Ops 3 Jesso Cheats", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowPos(ImVec2(0, 0));
	ImGui::SetWindowSize(ImVec2(400, 200), ImGuiCond_Always);

	ImGui::FocusWindow(ImGui::GetCurrentWindow());

	ImGui::Text("Ins - uninject");

	ImGui::Checkbox("Enable aimbot", &enableAimbot);
	ImGui::SliderFloat("Aimbot strength", &aimbotStrength, 0.1, 4);
	ImGui::Checkbox("Enable ESP", &enableEsp);
	ImGui::Checkbox("Enable crosshair", &enableCrosshair);
	ImGui::Checkbox("Target players on same team", &targetSameTeam);

	ImGui::End();

	ImGui::PopStyleColor(8);
	
	if (enableEsp || enableCrosshair)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		ImGui::PushStyleColor(ImGuiCol_WindowBg, { 0.0f, 0.0f, 0.0f, 0.0f });
		ImGui::Begin("invis window", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoInputs);

		ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::SetWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y), ImGuiCond_Always);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		ImDrawList* drawList = window->DrawList;

		if (enableCrosshair) { drawList->AddCircle(ImVec2(io.DisplaySize.x / 2 - 1, io.DisplaySize.y / 2 - 1), 2, IM_COL32(255, 0, 0, 255), 0, 4); }

		if (enableEsp) { Esp(drawList, io); }

		window->DrawList->PushClipRectFullScreen();
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar(2);
	}
}

void Esp(ImDrawList* drawList, ImGuiIO io)
{
	ViewAngles fov = GetFov();
	float zoom = *(float*)(bo3BaseAddress + zoomOffest);

	PlayerList* playerList = *(PlayerList**)(bo3BaseAddress + playerListOffset);
	if (IsBadReadPtr(playerList, sizeof(playerList))) { return; }

	if (!playerList->players[0].isAlive) { return; }
	
	for (int i = 1; i < 64; i++) 
	{
		ListPlayer currentPlayer = playerList->players[i];
		
		if (currentPlayer.index != i) { break; }
		if (!targetSameTeam && currentPlayer.team == playerList->players[0].team) { continue; }
		
		float distance = 0;
		ViewAngles viewAnglesDiff = CalculateViewAnglesDiff(playerList, i, &distance);
		
		float screenY = viewAnglesDiff.pitch / (fov.pitch * zoom);
		screenY = (screenY + 1) / 2;
		screenY *= io.DisplaySize.y;

		float screenX = viewAnglesDiff.yaw / (fov.yaw * zoom);
		screenX = (screenX + 1) / 2;
		screenX *= io.DisplaySize.x;

		float radius = 5000 / distance / zoom;
		ImU32 color = currentPlayer.isAlive ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255);
		drawList->AddCircle(ImVec2(screenX - (radius / 2), screenY - (radius / 2)), radius, color, 0, 2);
	}
}

ViewAngles GetFov() 
{
	ViewAngles result = {};
	result.yaw = *(float*)(bo3BaseAddress + fovOffset);
	result.pitch = (-0.002 * result.yaw * result.yaw) + (1.32 * result.yaw) + 2.8;
	return result;
}

ViewAngles CalculateViewAnglesDiff(PlayerList* playerList, int targetPlayerIndex, float* outDistance)
{
	ViewAngles result = {};

	LocalPlayerInfo* localPlayerInfo = *(LocalPlayerInfo**)(bo3BaseAddress + localPlayerInfoOffset);
	
	ListPlayer localPlayer = playerList->players[0];
	localPlayer.pos.z += localPlayer.eyeHeight - 70;

	ListPlayer targetPlayer = playerList->players[targetPlayerIndex];
	targetPlayer.pos.z += targetPlayer.eyeHeight - 85;

	targetPlayer.pos = targetPlayer.pos - localPlayerInfo->velocity * 0.1;
	
	Vector3 diff = targetPlayer.pos - localPlayer.pos;
	float distance = sqrt((diff.x * diff.x) + (diff.y * diff.y) + (diff.z * diff.z));
	if (distance == 0) { return result; }

	if (outDistance != nullptr) { *outDistance = distance; }

	float targetPitch = -(asin(diff.z / distance) * rToD);
	float targetYaw = (atan2(diff.y, diff.x) * rToD);

	float pitchDiff = targetPitch - localPlayerInfo->viewAngles.pitch;

	float yawDiff = localPlayerInfo->viewAngles.yaw - targetYaw;
	if (yawDiff > 180) { yawDiff = -(360 - yawDiff); }
	if (yawDiff < -180) { yawDiff = (360 + yawDiff); }

	result.pitch = pitchDiff;
	result.yaw = yawDiff;

	return result;
}

int GetClosestPlayerToCrosshair()
{
	float minScreenDistance = 99999;
	int result = -1;

	PlayerList* playerList = *(PlayerList**)(bo3BaseAddress + playerListOffset);
	if (IsBadReadPtr(playerList, sizeof(playerList))) { return -1; }

	ListPlayer localPlayer = playerList->players[0];
	if (!localPlayer.isAlive) { return -1; }

	for (int i = 1; i < 64; i++) 
	{
		ListPlayer currentPlayer = playerList->players[i];
		if (currentPlayer.index != i) { break; }
		if (!currentPlayer.isAlive || (!targetSameTeam && currentPlayer.team == localPlayer.team)) { continue; }
		
		ViewAngles viewAnglesDiff = CalculateViewAnglesDiff(playerList, i, nullptr);

		float screenDistance = sqrt((viewAnglesDiff.yaw * viewAnglesDiff.yaw) + (viewAnglesDiff.pitch * viewAnglesDiff.pitch));

		if (screenDistance < minScreenDistance)
		{
			minScreenDistance = screenDistance;
			result = i;
		}
	}

	return result;
}

void MoveMouse(float deltaX, float deltaY)
{
	INPUT input;
	ZeroMemory(&input, sizeof(input));

	input.type = INPUT_MOUSE;

	MOUSEINPUT mouseInput;
	ZeroMemory(&mouseInput, sizeof(mouseInput));

	mouseInput.dwFlags = MOUSEEVENTF_MOVE;

	if (deltaX > 0 && deltaX < 1) { deltaX = 1; }
	if (deltaX < 0 && deltaX > -1) { deltaX = -1; }

	if (deltaY > 0 && deltaY < 1) { deltaY = 1; }
	if (deltaY < 0 && deltaY > -1) { deltaY = -1; }

	mouseInput.dx = deltaX;
	mouseInput.dy = deltaY;

	input.mi = mouseInput;
	SendInput(1, &input, sizeof(INPUT));
}

bool IsCursorInWindow()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	if (viewport == nullptr) { return false; }

	ImVec2 windowPos = viewport->Pos;
	ImVec2 windowSize = viewport->Size;
	ImVec2 cursorPos = ImGui::GetIO().MousePos;

	if (cursorPos.x < windowPos.x) { return false; }
	if (cursorPos.x > windowPos.x + windowSize.x) { return false; }

	if (cursorPos.y < windowPos.y) { return false; }
	if (cursorPos.y > windowPos.y + windowSize.y) { return false; }

	return true;
}