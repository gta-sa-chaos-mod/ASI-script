//----BY Вова Петров aka Ma-ZaHaKa
#pragma once
#include "Windows.h";
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <random>
#include <iomanip>

#include "XMemory\XMemory.h"
#include "CustomGta3Classes.h"


#include "CTheScripts.h"
#include "CRunningScript.h"
#include <extensions\ScriptCommands.h>
#include <extensions\scripting\ScriptCommandNames.h> // !!!!!!!!!!!!!!
#include "eScriptCommands.h"
#include "CTimer.h"
#include "CFileMgr.h"
#include "CMessages.h"
#include "CCutsceneMgr.h"
#include "CGarage.h"
#include "CGarages.h"
#include "CCredits.h"

//--additional
#include "CPed.h"
#include "CPlayerPed.h"
#include "CModelInfo.h"
#include "CFont.h"
#include "CPool.h"
#include "CSprite.h"
#include "CEntity.h"
#include "CText.h"
#include "CWorld.h"
#include "CWorld.h"
#include "CStreaming.h"
#include "CPopulation.h"
#include "ePedModel.h"
#include "eVehicleModel.h"
#include "eAnimations.h" // anims enums


//--DBG--CALL--F12
//#include "CTheScripts.cpp"
//#include "CRunningScript.cpp"

#include "plugin.h"
using namespace plugin;
std::vector<HANDLE> g_threads;
HANDLE InitConsole();




static void SetCarImmunities(CVehicle* pVehicle, int BulletP = -1, int FireP = -1, int ExplosionP = -1, int CollisionP = -1, int MeleeP = -1)
{ // COMMAND_SET_CAR_PROOFS 684 // used handle for pool
	//02AC: set_car $CAT1_CHOPPER immunities 1 1 1 1 1 
	if (!pVehicle || ((BulletP == -1) && (FireP == -1) && (ExplosionP == -1) && (CollisionP == -1) && (MeleeP == -1))) { return; }
	pVehicle->m_nFlags.bBulletProof = (BulletP != -1) ? BulletP : pVehicle->m_nFlags.bBulletProof;
	pVehicle->m_nFlags.bFireProof = (FireP != -1) ? FireP : pVehicle->m_nFlags.bFireProof;
	pVehicle->m_nFlags.bExplosionProof = (ExplosionP != -1) ? ExplosionP : pVehicle->m_nFlags.bExplosionProof;
	pVehicle->m_nFlags.bCollisionProof = (CollisionP != -1) ? CollisionP : pVehicle->m_nFlags.bCollisionProof;
	pVehicle->m_nFlags.bMeleeProof = (MeleeP != -1) ? MeleeP : pVehicle->m_nFlags.bMeleeProof;
	//Command<Commands::SET_CAR_PROOFS>(CPools::GetVehicleRef(pVehicle), BulletP > 0, FireP > 0, ExplosionP > 0, CollisionP > 0, MeleeP > 0); // handle + 5 flags (rewrite all)
}

static CVehicle* CreateVehicle(int vehicleID, CVector position, int PrimaryColor = -1, int SecondaryColor = -1, int degree = -1, int BulletP = -1, int FireP = -1, int ExplosionP = -1, int CollisionP = -1, int MeleeP = -1)
{
#define _M_PI       3.14159265358979323846   // pi
#define _PI (float)_M_PI
#define _DEGTORAD(x) ((x) * _PI / 180.0f)

	CVehicle* vehicle = nullptr;

	unsigned char oldFlags = CStreaming::ms_aInfoForModel[vehicleID].m_nFlags;
	CStreaming::RequestModel(vehicleID, 1);
	CStreaming::LoadAllRequestedModels(false);
	if (CStreaming::ms_aInfoForModel[vehicleID].m_nLoadState == LOADSTATE_LOADED)
	{
		if (!(oldFlags & 1))
		{
			CStreaming::SetModelIsDeletable(vehicleID);
			CStreaming::SetModelTxdIsDeletable(vehicleID);
		}

		Command<eScriptCommands::COMMAND_CREATE_CAR>(vehicleID, position.x, position.y, position.z, &vehicle);

		if (vehicle)
		{
			CTheScripts::ClearSpaceForMissionEntity(position, vehicle);
			if (degree >= 0) { vehicle->SetHeading(_DEGTORAD(degree)); }

			if (!((BulletP == -1) && (FireP == -1) && (ExplosionP == -1) && (CollisionP == -1) && (MeleeP == -1)))
			{ // not SetCarImmunities dependensive
				vehicle->m_nFlags.bBulletProof = (BulletP != -1) ? BulletP : vehicle->m_nFlags.bBulletProof;
				vehicle->m_nFlags.bFireProof = (FireP != -1) ? FireP : vehicle->m_nFlags.bFireProof;
				vehicle->m_nFlags.bExplosionProof = (ExplosionP != -1) ? ExplosionP : vehicle->m_nFlags.bExplosionProof;
				vehicle->m_nFlags.bCollisionProof = (CollisionP != -1) ? CollisionP : vehicle->m_nFlags.bCollisionProof;
				vehicle->m_nFlags.bMeleeProof = (MeleeP != -1) ? MeleeP : vehicle->m_nFlags.bMeleeProof;
			}

			if ((PrimaryColor >= 0 && SecondaryColor >= 0) && (!(PrimaryColor >= 256 || SecondaryColor >= 256))) // CHANGE_CAR_COLOUR - Colours must be less than 256
			{
				vehicle->m_nPrimaryColor = PrimaryColor;
				vehicle->m_nSecondaryColor = SecondaryColor;
			}
			//SetCarImmunities(vehicle, BulletP, FireP, ExplosionP, CollisionP, MeleeP);

			//--SPAWN FROM RE3 ON ROAD NODE (work)
			//// CPools::GetObject(handle) CPools::GetVehicleRef(pVehicle) CPools::GetObject(handle)
			//CPlayerPed* player = FindPlayerPed(); // re3 spawn + custom
			//if (!player) { return vehicle; }
			////CVector pos = FindPlayerCoors(); // crash !!!!!!!!!!!!!!!!
			//CVector playerpos = player->GetPosition();
			//int node = ThePaths.FindNodeClosestToCoors(playerpos, 0, 100.0f, false, false);
			//CVector vehpos = ThePaths.m_aPathNodes[node].m_vecPos;
			//vehpos.z = CWorld::FindGroundZForCoord(vehpos.x, vehpos.y);
			//vehpos.z = vehpos.z + 4.0f;
			//vehicle->SetOrientation(0.0f, 0.0f, 3.49f);
			////vehicle->SetHeading(3.49f);
			//vehicle->SetPosition(vehpos.x, vehpos.y, vehpos.z);
			////Command<Commands::SET_OBJECT_COORDINATES>(CPools::GetObjectRef((CObject*)vehicle), vehpos.x, vehpos.y, vehpos.z); // not work
			////vehicle->Teleport(CVector(vehpos.x, vehpos.y, vehpos.z));
			//if (degree >= 0) { vehicle->SetHeading(_DEGTORAD(degree)); }
		}
	}
	return vehicle;
}

static void CreateCar(int vehicleID = eVehicleModel::MODEL_BANSHEE, int PrimaryColor = -1, int SecondaryColor = -1, int degree = -1, int BulletP = -1, int FireP = -1, int ExplosionP = -1, int CollisionP = -1, int MeleeP = -1) // banshee 119 (modified fast handling 4 testing scripts)
{
	CPlayerPed* player = FindPlayerPed();
	if (player)
	{
		CVector position = player->TransformFromObjectSpace(CVector(0.0f, 5.0f, 0.0f));
		//CVehicle* vehicle = CreateVehicle(vehicleID, position, player->m_fRotationCur + 1.5707964f); // Parik27/ASI-script/src/gta3/effects/generic/SpawnVehicle.cpp
		CVehicle* vehicle = CreateVehicle(vehicleID, position, PrimaryColor, SecondaryColor, degree == -2 ? player->m_fRotationCur + 1.5707964f : degree, BulletP, FireP, ExplosionP, CollisionP, MeleeP);
	}
}

//todo
//static void FixCar(void)
//{
//	CVehicle* veh = FindPlayerVehicle();
//	if (!veh) { return; }
//	veh->m_fHealth = 1000.0f;
//	if (!veh->IsCar()) { return; }
//	((CAutomobile*)veh)->Damage.SetEngineStatus(0);
//	((CAutomobile*)veh)->Fix();
//}


void TeleportPlayer(CVector destination, int degree = -1, bool auto_find_z = false)
{
#define _M_PI       3.14159265358979323846   // pi
#define _PI (float)_M_PI
#define _DEGTORAD(x) ((x) * _PI / 180.0f)

	CEntity* entity = FindPlayerEntity();
	if (entity)
	{
		if (auto_find_z) { destination.z = CWorld::FindGroundZForCoord(destination.x, destination.y) + 0.1f; }
		//CStreaming::StreamZoneModels(destination); // can crash?
		entity->Teleport(destination);

		CWorld::Remove(entity);
		CWorld::Add(entity);

		CPed* player = FindPlayerPed();
		CVehicle* pVehicle = FindPlayerVehicle(); // ? FindPlayerVehicle()->IsCar() Garages.cpp 482 bool IsCar(void) { return m_vehType == VEHICLE_TYPE_CAR; }
		if (player) {
			CWorld::Remove(player);
			CWorld::Add(player);
		}
		CStreaming::StreamZoneModels(destination); // orig

		if (degree >= 0)
		{
			while (degree >= 180.0f) { degree -= 2 * 180.0f; } // limit degree
			while (degree < -180.0f) { degree += 2 * 180.0f; }

			if (player && (player->m_bInVehicle) && pVehicle)
			{
				//CObject* pObject = (handle != -1) ? CPools::GetObjectPool()->GetAt(handle) : obj; // code re3 ERROR NOT FOUND BY OBJECT РАЗБЕРИСЬ
				//InitConsole();
				//int handle = CPools::GetObjectPool()->GetIndex((CObject*)entity); // nf
				//std::cout << "han: " << handle << "\n";
				float angle = _DEGTORAD(degree);
				entity->SetHeading(angle);
				//Command<Commands::SET_OBJECT_HEADING>(handle, angle);

			}

		}
	}
}



char* T_constchar2char(const char* constString) { return const_cast<char*>(constString); }
char* T_string2char(std::string constString) { return const_cast<char*>(constString.c_str()); }
std::string T_Pointer2String(void* pointer) { std::stringstream ss; ss << pointer; return "0x" + ss.str(); }




std::string ToUpper(std::string strToConvert)
{
	std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), std::toupper); //::toupper
	return strToConvert;
}
std::string ToLower(std::string strToConvert)
{
	std::transform(strToConvert.begin(), strToConvert.end(), strToConvert.begin(), std::tolower);
	return strToConvert;
}

std::string Trim(std::string str)
{
	// Find the first non-whitespace character from the beginning.
	size_t start = str.find_first_not_of(" \t\n\r\f\v");

	if (start == std::string::npos) {
		// If the string consists only of whitespace, return an empty string.
		return "";
	}

	// Find the last non-whitespace character from the end.
	size_t end = str.find_last_not_of(" \t\n\r\f\v");

	// Calculate the length of the trimmed substring.
	size_t length = end - start + 1;

	// Extract and return the trimmed substring.
	return str.substr(start, length);
}

std::string Replace(std::string input, std::string target, std::string replacement)
{
	std::string result = input;
	size_t startPos = 0;

	while ((startPos = result.find(target, startPos)) != std::string::npos)
	{
		result.replace(startPos, target.length(), replacement);
		startPos += replacement.length();
	}

	return result;
}

void AsciiToUnicode(const char* src, uint16_t* dst) // wchar
{
	while ((*dst++ = (unsigned char)*src++) != '\0');
}


int _RandVKL(int min, int max)
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(gen);
}



//------------------------------------------------------------------
HANDLE InitConsole() // with proto
{
	AllocConsole();

	//SetConsoleOutputCP(866);
	setlocale(LC_ALL, "Russian");
	SetConsoleOutputCP(1251);
	SetConsoleCP(1251);


	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);

	return hConsole;
}

//void LeaveConsole(HANDLE hConsole = nullptr)
void LeaveConsole(HANDLE hConsole) // with proto
{
	if (hConsole != nullptr) { SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); } // Reset to default color
	FreeConsole();
}













// hook processone command before + get this
// make cop setstate


//------------------------SCRIPTS----------------------------------------------------------------------------
//0 main.scm block vars+MAIN sz(131072)
//131072 mission block sz(32768)
//163840 scripts array idle??

void TestObjPool()
{
	for (int i = 0; i < NumberOfUsedObjects; i++)
	{
		std::cout << "0x" << (void*)UsedObjectArray << " i:" << i << " N: " << UsedObjectArray[i].name << "I:" << UsedObjectArray[i].index << "\n";
	}
}


CRunningScript* GetScriptByNum(int index_script)
{
	CRunningScript* scr = CTheScripts::pActiveScripts;
	int cnt = 0;
	for (; scr; scr = scr->m_pNext) { ++cnt; if (cnt >= index_script) { return scr; } }
	return scr;
}

CRunningScript* GetScriptByName(const char* name)
{
	CRunningScript* scr = CTheScripts::pActiveScripts;
	int cnt = 0;
	for (; scr; scr = scr->m_pNext) { ++cnt; if (strcmp(ToLower(scr->m_szName).c_str(), ToLower(name).c_str()) == 0) { return scr; } }
	return nullptr;
}

int GetScriptCounts()
{
	int cnt = 0;
	for (CRunningScript* scr = CTheScripts::pActiveScripts; scr; scr = scr->m_pNext) { ++cnt; }
	return cnt;
}

//bool g_semaphoreOnDraw = true;
//bool g_OnDrawCallFlag = false;

//int8 CRunningScript::ProcessOneCommand()
//bool g_semaphorePOC = true; // allow run сделай через OnDraw. хук ProcessOneCommand будет только для чистой gta3
//bool g_POCCallFlag = false;
//bool g_POCHookedFlag = false;
//void HockedProcessOneCommandBefore() // danger
//{
//	InitConsole(); std::cout << "HockedProcessOneCommandBefore()" << "\n";
//	g_POCCallFlag = true;
//	while (!g_semaphorePOC) { Sleep(5); }
//	g_POCCallFlag = false;
//}
//
//void HookPOCLogic() // sync runscript danger freeze fps
//{
//	// хук в ProcessOneCommand обходит клео, перемещение call в CRunningScript::Process крашит клео
//	//void* hFuncPath = (void*)0x439500; // ++CTheScripts::CommandsExecuted; 7bytes!! ProcessOneCommand
//	void* hFuncPath = (void*)0x439485; // char __thiscall CRunningScript::Process(CRunningScript *this) patched by cleo. error
//	if (ReadPtrDanger<unsigned char>(hFuncPath) != 0xE8) { return; } // can be cleo call
//	g_POCHookedFlag = true;
//
//	//---SETTINGS--NEW--BLOCK
//	int available_sz_patch = 5; // дёрнуть байты из орига (+3 align 10h можно добавить, без них работает)
//	int need_sz_patch = 5; // bytes // доп блок 1 + 4 (5 байт которые переместили из ориг суммируються в функции), только доп байты!!
//	bool FIRST_PRIORITY = true; // сначала патч затем ориг инструкции
//	int offset = FIRST_PRIORITY ? need_sz_patch : 0; // max need_sz_patch(будет в конце перед jmp)  block(200+51+1+4)
//	bool jmp_patch_in_end_region = false; // optimize
//
//
//	//---COLLECT--ORIG--DATA--OR--POINTERS--TO--PATCH--MOVED---BLOCK (mov/jmp/jz/je/jnz/jne/call)!!
//	// push from ptr not need patch. this is not offset
//	void* orig_call_dest_ptr = CalcPointerFromJMPE9Offset(hFuncPath, ReadPtrDanger<uintptr_t>(Transpose(hFuncPath, sizeof(unsigned char)))); // вычисляю указатель который вызывал call
//
//
//	//---OUT
//	void* out_patch_ptr = nullptr;
//	int out_patch_sz = 0; // mbi.region_sz
//	//bool res = SetPatchBlock(orig_ptr_for_patch_reference, available_sz_patch, need_sz_patch, out_patch_ptr, out_patch_sz, true, offset); // jmp in the end region
//	bool res = SetPatchBlock(hFuncPath, available_sz_patch, need_sz_patch, out_patch_ptr, out_patch_sz, jmp_patch_in_end_region, offset); // jmp after need_sz_patch
//	void* moved_block = Transpose(out_patch_ptr, offset); // указатель на скопированный блок
//
//
//	//----PATCH--LOGIC---BLOCK
//	InsertCall(moved_block, orig_call_dest_ptr); // fix call offset ?? ff 15 ??
//
//
//	//---INSERT CALL
//	void* PatchFreeSpacePtr = FIRST_PRIORITY ? out_patch_ptr : Transpose(moved_block, available_sz_patch);
//	WriteDanger<unsigned char>(PatchFreeSpacePtr, 0xE8); // call
//	InsertCall(PatchFreeSpacePtr, HockedProcessOneCommandBefore);
//
//	//void* buff = MkMem((4 * sizeof(void*))); // bytes // usage 4 * 3
//	//InsertJump(jmp, retn_false_label_ptr);
//}


bool GetPlayerOnMission()
{
	return CTheScripts::IsPlayerOnAMission();
}

bool GetIsCutsceneRunning()
{
	return CCutsceneMgr::ms_running;
}

void SetPlayerFrozen(bool flag)
{
	//01B4: set_player $PLAYER_CHAR frozen_state 0
	//436 COMMAND_SET_PLAYER_CONTROL
	CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(flag); // stop car and no move
}

void PlayerExitVehicle()
{
	// FindPlayerVehicle() can never be NULL here because IsStaticPlayerCarEntirelyInside() is true, and there is no IsCar() check
	CPed* pPed = FindPlayerPed();
	CVehicle* pVehicle = FindPlayerVehicle(); // ? FindPlayerVehicle()->IsCar() Garages.cpp 482 bool IsCar(void) { return m_vehType == VEHICLE_TYPE_CAR; }
	if ((!pPed) || (!pPed->m_bInVehicle) || (!pVehicle)) { return; }
	//CPed* pPed = CPools::GetPedPool()->GetAt(ScriptParams[0]);
	SetPlayerFrozen(true);
	//CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(true); // stop car
	////((CPlayerPed*)pPed)->GetPlayerInfoForThisPlayerPed()->MakePlayerSafe(true); // not tested
	pPed->m_nPedFlags.bScriptObjectiveCompleted = false;
	pPed->SetObjective(OBJECTIVE_LEAVE_CAR, pVehicle);
	while (pPed->m_bInVehicle) { Sleep(5); }
	SetPlayerFrozen(false);
	//CWorld::Players[CWorld::PlayerInFocus].MakePlayerSafe(false); // take control
}


void ClearPlayerStatus()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	//Command<eScriptCommands::COMMAND_SKIP_CUTSCENE_END>();
	//Command<eScriptCommands::COMMAND_CLEAR_CHAR_TASKS>(player);
	Command<eScriptCommands::COMMAND_CLEAR_CUTSCENE>(); // my code

	Command<eScriptCommands::COMMAND_SET_CAMERA_BEHIND_PLAYER>();
	Command<eScriptCommands::COMMAND_SWITCH_WIDESCREEN>(0);
	Command<eScriptCommands::COMMAND_SET_PLAYER_CONTROL>(0, 1);
	Command<eScriptCommands::COMMAND_RESTORE_CAMERA_JUMPCUT>();
	Command<eScriptCommands::COMMAND_DETACH_CHAR_FROM_CAR>(player);
}

void PASS(int money, int duration = 5000, bool add_money = true)
{
	//[M_PASS]
	//MISSION PASSED! $~1~

	if (money < 0) { return; }
	Command<Commands::CLEAR_SMALL_PRINTS>();
	if (money > 0) { Command<eScriptCommands::COMMAND_PRINT_WITH_NUMBER_BIG>("M_PASS", money, duration, 1); } // m = 0 passed! $0
	else
	{
		//Command<eScriptCommands::COMMAND_PRINT_BIG>("M_PASS", duration, 1);  // 00BA: text_styled 'M_FAIL' 5000 ms 1 $-1

		//COMMAND_PRINT_BIG 186
		//wchar* key = TheText.Get((char*)&CTheScripts::ScriptSpace[m_nIp]);
		//CMessages::AddBigMessage(key, ScriptParams[0], ScriptParams[1] - 1);


		//COMMAND_PRINT_WITH_NUMBER_BIG 483
		//wchar* text = CTheScripts::GetTextByKeyFromScript(&m_nIp);
		//CMessages::AddBigMessageWithNumber(text, ScriptParams[1], ScriptParams[2] - 1, ScriptParams[0], -1, -1, -1, -1, -1);

		//CMessages::AddBigMessage((char*)("MISSION PASSED!"), duration, 0); // work  get from TheText.Get wchar and remove ( $~1~) 
		std::wstring wstr = TheText.Get("M_PASS");
		wstr.erase(wstr.end() - 5, wstr.end());
		std::string str(wstr.begin(), wstr.end());
		CMessages::AddBigMessage(const_cast<char*>(str.c_str()), duration, 0);
	}
	Command<Commands::PLAY_MISSION_PASSED_TUNE>(1);

	CPlayerPed* player = FindPlayerPed();
	if (!player) { return; }
	if (add_money && (money > 0)) { player->GetPlayerInfoForThisPlayerPed()->m_nMoney += money; }
	player->SetWantedLevel(0);
}


//0x6F0558 // int32 CTheScripts::MultiScriptArray[MAX_NUM_MISSION_SCRIPTS];
//0x75073C // main.scm header offsets
//unsigned int* MultiScriptArray = (unsigned int*)0x6F0558; //[MAX_NUM_MISSION_SCRIPTS] 120
//unsigned int* GetCTheScripts_MultiScriptArray()
//{
//	//unsigned int& CTheScripts::MultiScriptArray = *(unsigned int*)0x6F0558; //[MAX_NUM_MISSION_SCRIPTS] 120
//	unsigned int* MultiScriptArray = (unsigned int*)0x6F0558;
//	return MultiScriptArray;
//}

int DebugScriptIPsSetIP(int script_num, int new_IP, bool add_ip = false)
{
	//CRunningScript* scr = CTheScripts::pActiveScripts;
	//for (int i = start_ip; i < script_num; i++) { if (!scr) { continue; } scr = scr->m_pNext; } // sync with other logic
	CRunningScript* scr = GetScriptByNum(script_num);
	if (!scr) { return -1; }
	if (add_ip) { scr->m_nIp += new_IP; }
	else { scr->m_nIp = new_IP; }
	return  scr->m_nIp;
}

int DebugScriptIPsSetIP(CRunningScript* scr, int new_IP, bool add_ip = false)
{
	//CRunningScript* scr = CTheScripts::pActiveScripts;
	//for (int i = start_ip; i < script_num; i++) { if (!scr) { continue; } scr = scr->m_pNext; } // sync with other logic
	//CRunningScript* scr = GetScriptByNum(script_num);
	if (!scr) { return -1; }
	if (add_ip) { scr->m_nIp += new_IP; }
	else { scr->m_nIp = new_IP; }
	return scr->m_nIp;
}

void DebugScriptProcessByCount(CRunningScript* scr, int count_instructions, int new_IP = -1)
{
	if (!scr) { return; }
	if (new_IP != -1) { scr->m_nIp = new_IP; }
	for (int i = 0; i < count_instructions; i++) { scr->ProcessOneCommand(); }
}

bool DebugScriptIPsSetIPLabelOffset(CRunningScript* scr, int label_offset) // не путать с DebugScriptIPsSetIP, там просто setip
{ // прыжок на :label_bytescount(label_offset)
	// CTheScripts::ScriptSpace[SIZE_MAIN_SCRIPT]
	if (!scr) { return false; }
	if (label_offset > SIZE_SCRIPT_SPACE) { return false; }
	//DEFINE MISSION 14 AT @TAXI   // Taxi Driver   //!!!!!!
	//0x6F0558 // int32 CTheScripts::MultiScriptArray[MAX_NUM_MISSION_SCRIPTS];
	// ofsets from file. для загрузки будет 0 + (vars+main) = SIZE_MAIN_SCRIPT
	//std::cout << "" << GetCTheScripts_MultiScriptArray()[1] << "\n";
	//CFileMgr::Read(handle, (const char*)&CTheScripts::ScriptSpace[SIZE_MAIN_SCRIPT], SIZE_MISSION_SCRIPT);
	//int base_mission = GetCTheScripts_MultiScriptArray()[19]; // from sanny builder DEFINE MISSIONS 80 num mission
	int target_ip = 0 + SIZE_MAIN_SCRIPT + label_offset;
	//int old_IP = scr->m_nIp;
	scr->m_nIp = target_ip;
	return true;
}

bool DebugScriptIPsSetIPLabelOffsetByPattern(CRunningScript* scr, std::string pattern) // не путать с DebugScriptIPsSetIP, там просто setip
{ // прыжок на :label_bytescount(label_offset)
	// CTheScripts::ScriptSpace[SIZE_MAIN_SCRIPT]
	if (!scr) { return false; }
	void* res_pointer = InlineSearchPointerByPattern(((void*)((uintptr_t)(CTheScripts::ScriptSpace)+SIZE_MAIN_SCRIPT)), SIZE_SCRIPT_SPACE, pattern);
	if (!res_pointer) { return false; }

	int target_ip = ((uintptr_t)res_pointer) - ((uintptr_t)CTheScripts::ScriptSpace);
	//int old_IP = scr->m_nIp;
	scr->m_nIp = target_ip;
	return true;
}

void TestDebugScriptIPsSetIPLabelOffset()
{
	//DebugScriptIPsSetIPLabelOffsetByPattern((CRunningScript*)i, "D6 00 04 00 18 81 02 2C 00 4D"); //(10) 306C3 :EIGHT_6968
	//DebugScriptProcessByCount((CRunningScript*)i, 15);
	//int32 CTheScripts::MultiScriptArray[MAX_NUM_MISSION_SCRIPTS];
	//0x75073C // main.scm header offsets file
	int label_offset = 0; // 118129 2 field
	const char* mission = "eight";
	const char* mission2 = "taxi";
	label_offset = 6933;  // :TAXI_6933
	label_offset = 15331; // :EIGHT_15331
	//std::cout << "" << GetCTheScripts_MultiScriptArray()[1] << "\n";
	CRunningScript* scr = GetScriptByName(mission);
	//std::cout << "0x" << scr << "\n";
	DebugScriptIPsSetIPLabelOffset(scr, label_offset);
	//DebugScriptIPsSetIPLabelOffsetByPattern
}




////-------------Mission 20---------------
//// Originally: Don't Spank My Bitch Up
//
//:LUIGI2
//thread 'LUIGI2'
//gosub @LUIGI2_46
//if
//wasted_or_busted
//else_jump @LUIGI2_37
//gosub @LUIGI2_3489
//
//	:LUIGI2_37
//	gosub @LUIGI2_3572  FIND :LUIGI2_3572 UPPER PASS!!!!!!!!!!!!!!!!
//	end_thread

// приерный pass
// $n = 1; // флаг что миссия пройдена

// if jump // могут быть метки с проверками, при сдвиге каретки ip проверка пустых хендлов педов крашнет
// например
//:ASUKA1_18964
//00D6_if
//8118 : not actor $ASUKA1_MAFIA8 dead
//004D_else_jump @ASUKA1_18988
//01CC: actor $ASUKA1_MAFIA8 kill_player $PLAYER_CHAR

// затем 
//01E3: text_1number_styled 'M_PASS' 25000 5000 ms 1
//0109 : player $PLAYER_CHAR money += 25000
//0110 : clear_player $PLAYER_CHAR wanted_level
//0318 : set_latest_mission_passed 'AM1'
//0394 : play_music 1
//030C : set_mission_points += 1

int g_miss = 19; // dbg

// base script space: 0x74B248
// bridge barrier :LOVE3_4011
std::map<std::string, int> offsetMap =
{																					// flags passed mission vars 
	//{"INTRO", 0},			// 0  Intro Movie										$
	//{"HEALTH", 0},		// 1  Hospital Info Scene								$
	//{"WANTED", 0},		// 2  Police Station Info Scene							$
	{"RC1", 991},			// 3  RC Diablo Destruction								$409+ RC_REWARD
	{"RC2", 991},			// 4  RC Mafia Massacre									$410+ RC_REWARD
	{"RC3", 979},			// 5  RC Rumpo Rampage									$411+ RC_REWARD
	{"RC4", 983},			// 6  RC Casino Calamity								$412+ RC_REWARD
	{"T4X4_1", 3803},		// 7  Patriot Playground								$378+
	{"T4X4_2", 3274},		// 8  A Ride In The Park								$379+
	{"T4X4_3", 4685},		// 9  Gripped!											$380+
	{"MAYHEM", 4661},		// 10 Multistorey Mayhem								$381+
	//{"AMBULAN", 0},		// 11 Paramedic											$
	//{"FIRETRU", 0},		// 12 Firefighter										$
	//{"COPCAR", 0},		// 13 Vigilante											$
	//{"TAXI", 0},			// 14 Taxi Driver										$

	{"MEAT1", 4382},		// 15 The Crook (4382, 4448?) +							$289+
	{"MEAT2", 4704},		// 16 The Thieves (4704) +								$290+
	{"MEAT3", 3528},		// 17 The Wife (3528) +									$291+
	{"MEAT4", 3719},		// 18 Her Lover (3719) +								$292+

	{"EIGHT", 0},			// 19 Give Me Liberty and Luigi's Girls (15437) +		$248+ $245=1
	{"LUIGI2", 3506},		// 20 Don't Spank My Bitch Up (3506) +					$249+
	{"LUIGI3", 5700},		// 21 Drive Misty For Me (5700) +						$250+
	{"LUIGI4", 1988},		// 22 Pump-Action Pimp (1988) +							$251+
	{"LUIGI5", 12927},		// 23 The Fuzz Ball (12927) +							$252+ $253=1

	{"JOEY1", 5245},		// 24 Mike Lips Last Lunch (5245, 5307?) +				$254+
	{"JOEY2", 4673},		// 25 Farewell 'Chunky' Lee Chong (4673) +				$255+
	{"JOEY3", 2782},		// 26 Van Heist (2782) +								$256+
	{"JOEY4", 7810},		// 27 Cipriani's Chauffeur (7810, 7578) +				$257+
	{"JOEY5", 3047},		// 28 Dead Skunk In The Trunk (3047) +					$258+
	{"JOEY6", 9976},		// 29 The Getaway (9976) +								$259+ (:JOEY6_9887) $260 Not used

	{"TONI1", 2028},		// 30 Taking Out The Laundry (2028) +					$261+
	{"TONI2", 2227},		// 31 The Pick-Up (2227) +								$262+ (:TONI2_2584)
	{"TONI3", 8479},		// 32 Salvatore's Called A Meeting (8479) +				$263+
	{"TONI4", 3892},		// 33 Triads And Tribulations (3892) +					$264+ NAMED PASSED_TRIADS_AND_TRIBULATIONS
	{"TONI5", 3206},		// 34 Blow Fish (cut2161, oldgarage3206, 2161 + 387) +	$265+ NAMED PASSED_BLOW_FISH $266 Not used

	{"FRANK1", 16772},		// 35 Chaperone (15485, 16772) +						$267+
	{"FRANK2", 8206},		// 36 Cutting The Grass (8206) +						$268+
	//{"FRANK21", 912},		// 37 Bomb Da Base: Act I +?							$269+
	{"FRANK3", 12278},		// 38 Bomb Da Base: Act II (12278) +					$270+
	{"FRANK4", 4260},		// 39 Last Requests (4260, 4610) +						$271+

	{"DIABLO1", 10326},		// 40 Turismo (10305, nc10326) +						$278+
	{"DIABLO2", 3435},		// 41 I Scream, You Scream (3435) +						$279+
	{"DIABLO3", 1787},		// 42 Trial By Fire (1787) +							$280+
	{"DIABLO4", 2776},		// 43 Big'N'Veiny (2776) +								$281+ NAMED PASSED_BIG_N_VEINY

	{"ASUKA1", 18988},		// 44 Sayonara Salvatore (18988) +						$316+ FIXED
	{"ASUKA2", 3543},		// 45 Under Surveillance (3543) +						$317+
	{"ASUKA3", 7181},		// 46 Paparazzi Purge (7181) +							$318+
	{"ASUKA4", 3113},		// 47 Payday For Ray (3113) +							$319+
	{"ASUKA5", 2586},		// 48 Two-Faced Tanner (2586) +							$320+

	{"KENJI1", 4083},		// 49 Kanbu Bust-Out (4083) +							$329+
	{"KENJI2", 3478},		// 50 Grand Theft Auto (3478) +							$330+
	{"KENJI3", 6565},		// 51 Deal Steal (6565) +								$331+
	{"KENJI4", 4055},		// 52 Shima (4055) +									$332+
	{"KENJI5", 12315},		// 53 Smack Down (12315) +								$333+

	{"RAY1", 3038},			// 54 Silence The Sneak (3038) +						$322+
	{"RAY2", 6285},			// 55 Arms Shortage (6285) +							$323+
	{"RAY3", 4641},			// 56 Evidence Dash (4641) +							$324+
	{"RAY4", 9684},			// 57 Gone Fishing (9684) +								$325+
	{"RAY5", 3403},			// 58 Plaster Blaster (3403) +							$326+
	{"RAY6", 9079},			// 59 Marked Man (9079) +								$327+

	{"LOVE1", 5867},		// 60 Liberator (5867) +								$335+
	{"LOVE2", 4385},		// 61 Waka-Gashira Wipeout! (4385) +					$336+
	{"LOVE3", 3932},		// 62 A Drop In The Ocean (3932) +						$337+ $338 Not used

	{"YARD1", 5680},		// 63 Bling-Bling Scramble (5680) +						$339+
	{"YARD2", 4791},		// 64 Uzi Rider (4791) +								$340+ FIXED
	{"YARD3", 1598},		// 65 Gangcar Round-Up (1598) +							$341+
	{"YARD4", 3406},		// 66 Kingdom Come (3406) +								$342+ NAMED PASSED_KINGDOM_COME

	{"LOVE4", 9999},		// 67 Grand Theft Aero (9999) +							$349+
	{"LOVE5", 11393},		// 68 Escort Service (11393) +							$350+
	{"LOVE6", 2965},		// 69 Decoy (2965) +									$351+
	//{"LOVE7", 347},		// 70 Love's Disappearance (347) +?						$352+ $353 Not used

	{"ASUSB1", 5145},		// 71 Bait (5145) +										$354+
	{"ASUSB2", 7362},		// 72 Espresso-2-Go! (7362) +							$355+
	{"ASUSB3", 8201},		// 73 S.A.M. (8201) +									$356+

	{"HOOD1", 1568},		// 74 Uzi Money (1568) +								$359+
	{"HOOD2", 1978},		// 75 Toyminator (1978) +								$360+
	{"HOOD3", 2160},		// 76 Rigged To Blow (2160) +							$361+
	{"HOOD4", 1364},		// 77 Bullion Run (1364) +								$362+
	{"HOOD5", 5087},		// 78 Rumble (5087) +									$363+

	{"CAT1", 4719 + 11},	// 79 The Exchange (cred+splashbug!!6766, needtp+cred+1000000 6818, 4719!!!) +	$368+
};





//uint16_t& a = GetGlobalVariableByIndex<uint16_t>(vars_scr_map["LUIGI5_NUM_DROPOFFS"].index);
//a = 8;
template <typename T> static T&
GetGlobalVariableByIndex(uint32_t index) // $0 is start file [02 00 01 64], $1 is 0 + 4 [43 00 00 00], $2 is 0 + 4 + 4 [0.0625] thread 'MAIN' $2 = 0.0625
{
	return *reinterpret_cast<T*> (CTheScripts::ScriptSpace + (sizeof(uint32_t) * index));
}
template <typename T> static T&
GetGlobalVariableByOffset(uint32_t offset)
{ // float check
	return *reinterpret_cast<T*> (CTheScripts::ScriptSpace + offset);
}

#define KEY_LENGTH_IN_SCRIPT (8)

static int32_t Read4BytesFromScript(uint32_t* pIp) {
	int32_t retval = CTheScripts::ScriptSpace[*pIp + 3] << 24 | CTheScripts::ScriptSpace[*pIp + 2] << 16 |
		CTheScripts::ScriptSpace[*pIp + 1] << 8 | CTheScripts::ScriptSpace[*pIp];
	*pIp += 4;
	return retval;
}
static int16_t Read2BytesFromScript(uint32_t* pIp) {
	int16_t retval = CTheScripts::ScriptSpace[*pIp + 1] << 8 | CTheScripts::ScriptSpace[*pIp];
	*pIp += 2;
	return retval;
}
static int8_t Read1ByteFromScript(uint32_t* pIp) {
	int8_t retval = CTheScripts::ScriptSpace[*pIp];
	*pIp += 1;
	return retval;
}
static float ReadFloatFromScript(uint32_t* pIp) {
	return Read2BytesFromScript(pIp) / 16.0f;
}
static void ReadTextLabelFromScript(uint32_t* pIp, char* buf) {
	strncpy(buf, (const char*)&CTheScripts::ScriptSpace[*pIp], KEY_LENGTH_IN_SCRIPT);
}



void StartMission(int mission)
{
	for (CCustomRunningScript* pScript = (CCustomRunningScript*)CTheScripts::pActiveScripts; pScript != nullptr; pScript = (CCustomRunningScript*)(pScript->next)) {
		if (!pScript->m_bIsMissionScript || !pScript->m_bDeatharrestEnabled) { // check??
			continue;
		}
		while (pScript->m_nStackPointer > 0)
			--pScript->m_nStackPointer;

		pScript->m_nIp = pScript->m_anStack[pScript->m_nStackPointer];
		*(int32_t*)&CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag] = 0;
		//pScript->m_nWakeTime = 0;
		//pScript->m_bDeatharrestExecuted = true;

		while (!((CRunningScript*)pScript)->ProcessOneCommand());

		CMessages::ClearMessages();
	}

	CTimer::Suspend();
	int offset = MultiScriptArray[mission];

	CFileMgr::ChangeDir("\\");
	int handle = CFileMgr::OpenFile("data\\main.scm", "rb");

	CFileMgr::Seek(handle, offset, 0);
	CFileMgr::Read(handle, (char*)&CTheScripts::ScriptSpace[SIZE_MAIN_SCRIPT], SIZE_MISSION_SCRIPT);
	CFileMgr::CloseFile(handle);
	//CRunningScript* pMissionScript = CTheScripts::StartNewScript(SIZE_MAIN_SCRIPT);
	//CCustomRunningScript* pMissionScript = (CCustomRunningScript*)StartNewScript(SIZE_MAIN_SCRIPT);
	CCustomRunningScript* pMissionScript = nullptr;
	pMissionScript = (CCustomRunningScript*)StartNewScript(SIZE_MAIN_SCRIPT);

	// CTheScripts::StartNewScript----реализация-тут----------------
	// v1 work
	//CCustomRunningScript* pNew = (CCustomRunningScript*)CTheScripts::pIdleScripts;
	////pNew->RemoveScriptFromList(&CTheScripts::pIdleScripts); // modify sdk 
	//RemoveScriptFromList(pNew, &CTheScripts::pIdleScripts);
	//((CRunningScript*)pNew)->Init();
	//pNew->m_nIp = SIZE_MAIN_SCRIPT;
	////pNew->AddScriptToList(&CTheScripts::pActiveScripts); // modify sdk
	//AddScriptToList(pNew, &CTheScripts::pActiveScripts);

	// v2 work
	//pMissionScript = (CCustomRunningScript*)CTheScripts::pIdleScripts;
	////pNew->RemoveScriptFromList(&CTheScripts::pIdleScripts); // modify sdk 
	//RemoveScriptFromList(pMissionScript, &CTheScripts::pIdleScripts);
	//((CRunningScript*)pMissionScript)->Init();
	//pMissionScript->m_nIp = SIZE_MAIN_SCRIPT;
	////pNew->AddScriptToList(&CTheScripts::pActiveScripts); // modify sdk
	//AddScriptToList(pMissionScript, &CTheScripts::pActiveScripts);
	//------------------------------------------------


	CTimer::Resume();
	pMissionScript->m_bIsMissionScript = true;
	pMissionScript->m_bMissionFlag = true;
	CTheScripts::bAlreadyRunningAMissionScript = true;
}


void _StartMission(int mission) // brocken crunning flags ??
{
	for (CRunningScript* pScript = CTheScripts::pActiveScripts; pScript != nullptr; pScript = pScript->m_pNext) {
		if (!pScript->m_bIsMission || !pScript->m_bWastedOrBusted) { // check??
			continue;
		}
		while (pScript->m_nSP > 0)
			--pScript->m_nSP;

		pScript->m_nIp = pScript->m_anStack[pScript->m_nSP];
		*(int32_t*)&CTheScripts::ScriptSpace[CTheScripts::OnAMissionFlag] = 0;
		//pScript->m_nWakeTime = 0;
		//pScript->m_bDeatharrestExecuted = true;

		while (!pScript->ProcessOneCommand());

		CMessages::ClearMessages();
	}

	CTimer::Suspend();
	int offset = MultiScriptArray[mission];

	CFileMgr::ChangeDir("\\");
	int handle = CFileMgr::OpenFile("data\\main.scm", "rb");

	CFileMgr::Seek(handle, offset, 0);
	CFileMgr::Read(handle, (char*)&CTheScripts::ScriptSpace[SIZE_MAIN_SCRIPT], SIZE_MISSION_SCRIPT);
	CFileMgr::CloseFile(handle);
	//CRunningScript* pMissionScript = CTheScripts::StartNewScript(SIZE_MAIN_SCRIPT);

	// CTheScripts::StartNewScript--------------------
	CRunningScript* pNew = CTheScripts::pIdleScripts;
	//pNew->RemoveScriptFromList(&CTheScripts::pIdleScripts); // modify sdk 
	RemoveScriptFromList(pNew, &CTheScripts::pIdleScripts);
	pNew->Init();
	pNew->m_nIp = SIZE_MAIN_SCRIPT;
	//pNew->AddScriptToList(&CTheScripts::pActiveScripts); // modify sdk
	AddScriptToList(pNew, &CTheScripts::pActiveScripts);
	//------------------------------------------------

	CTimer::Resume();
	//pNew->m_bIsActive = true; // ?
	//pNew->m_bIsMission = true;
	CTheScripts::bAlreadyRunningAMissionScript = true;
}




// ChangePedModel(FindPlayerPed(), 1);
void ChangePedModel(CPed* ped, int modelId)
{ // MI_SPECIAL01 = 26
	//https://wiki.gtaconnected.com/Resources/GTA3/PedSkins
	if ((modelId >= MI_SPECIAL01 && modelId <= MI_SPECIAL04 || modelId == MODEL_TAXI_D)) { return; }
	if (!CModelInfo::ms_modelInfoPtrs[modelId]) { return; }


	//if (FindPlayerPed()->IsPedInControl() && CModelInfo::GetModelInfo("player", nullptr)) // CModelInfo::GetModelInfo("player", nullptr) RUIN expression
	//if (FindPlayerPed())
	if (ped)
	{
		//CPlayerPed* ped = FindPlayerPed();
		int AnimGrp = ped->m_nAnimGroupId; // eAnimGroup ??

		uint8_t flags = CStreaming::ms_aInfoForModel[modelId].m_nFlags;
		ped->DeleteRwObject();
		CStreaming::RequestModel(modelId, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_DONT_REMOVE);
		CStreaming::LoadAllRequestedModels(false);
		ped->m_nModelIndex = -1;
		ped->SetModelIndex(modelId);
		ped->m_nAnimGroupId = AnimGrp;
		if (modelId != MODEL_NULL) {
			if (!(flags & STREAMFLAGS_DONT_REMOVE))
				CStreaming::SetModelIsDeletable(modelId);
		}
	}
}

// СreateCharSync(PEDTYPE_CIVMALE, MODEL_COP, CVector(866.0f, -307.8f, 8.3f));
static CPed* СreateChar(ePedType ped_Type, int ped_model, CVector ped_pos) {
	if (CStreaming::ms_aInfoForModel[ped_model].m_nLoadState == LOADSTATE_LOADED) {
		CPed* ped_2 = CPopulation::AddPed(ped_Type, ped_model, ped_pos);
		if (ped_2) {
			ped_2->m_nCharCreatedBy = 2;
			return ped_2;
		}
		else {
			return nullptr;
		}
	}
	else {
		CStreaming::RequestModel(ped_model, GAME_REQUIRED);
		CStreaming::LoadAllRequestedModels(false);
		return nullptr;
	}
}

// СreateCharSync(PEDTYPE_CIVMALE, MODEL_COP, CVector(866.0f, -307.8f, 8.3f));
static CPed* СreateCharSync(ePedType ped_Type, int ped_model, CVector ped_pos) // not tested
{
	if (CStreaming::ms_aInfoForModel[ped_model].m_nLoadState != LOADSTATE_LOADED)
	{
		CStreaming::RequestModel(ped_model, GAME_REQUIRED);
		CStreaming::LoadAllRequestedModels(false);
		while (CStreaming::ms_aInfoForModel[ped_model].m_nLoadState != LOADSTATE_LOADED) { Sleep(10); } // remake this code
	}

	if (CStreaming::ms_aInfoForModel[ped_model].m_nLoadState == LOADSTATE_LOADED) {
		CPed* ped_2 = CPopulation::AddPed(ped_Type, ped_model, ped_pos);
		if (ped_2) {
			ped_2->m_nCharCreatedBy = 2;
			return ped_2;
		}
		else {
			return nullptr;
		}
	}
	else // optional
	{
		CStreaming::RequestModel(ped_model, GAME_REQUIRED);
		CStreaming::LoadAllRequestedModels(false);
		return nullptr;
	}
}

void ReDressPed(CPed* ped, const char* skin) // can crash script logic
{
	//?? COMMAND_UNDRESS_CHAR 850, wait load, COMMAND_DRESS_CHAR 851?
	//0352: set_actor $PLAYER_ACTOR skin_to 'PLAYER' :EIGHT_6968
	if (!ped) { return; }
	//if (ped->m_bInVehicle) { PlayerExitVehicle(); } // broken anim sit
	PlayerExitVehicle(); // broken anim sit in veh
	SetPlayerFrozen(true); // in move cant redress

	char name[KEY_LENGTH_IN_SCRIPT];
	for (int i = 0; i < KEY_LENGTH_IN_SCRIPT; i++) { name[i] = tolower(skin[i]); }
	int mi = ped->m_nModelIndex;

	//ped->DeleteRwObject(); //---------------------------    -------TESTING IF CRASH GAME OR SCRIPTS
	if (ped->IsPlayer()) { mi = 0; }

	//CStreaming::RequestModel(mi, GAME_REQUIRED); //-cus
	CStreaming::RequestSpecialModel(mi, name, STREAMFLAGS_DEPENDENCY | STREAMFLAGS_SCRIPTOWNED);
	CStreaming::LoadAllRequestedModels(false); //--cus
	//CWorld::Remove(ped); // re3. crash CTheScripts? ----------------------- TESTING!!

	while (CStreaming::ms_aInfoForModel[mi].m_nLoadState != LOADSTATE_LOADED) { Sleep(10); } // remake this code
	//if (CStreaming::ms_aInfoForModel[mi].m_nLoadState != LOADSTATE_LOADED) { InitConsole(); } // test

	mi = ped->m_nModelIndex;
	ped->m_nModelIndex = -1;
	ped->SetModelIndex(mi);
	//CWorld::Add(ped); // re3. crash CTheScripts? -------------------------  TESTING!!
	SetPlayerFrozen(false);
}

// int h55_test = CPools::GetObjectRef(CPools::GetObject(v55)); // handle-object convert example
void SetObjectHeading(int degree, CObject* obj, int handle = -1) // heading. z pos
{
	if (!obj && (handle == -1)) { return; }
	//Command<Commands::SET_OBJECT_HEADING>(handle, angle);
	//return;
	// НИЖЕ ТОЖЕ РАБОТАЕТ

	// COMMAND_SET_OBJECT_HEADING 375
	//CEntity* entity = FindPlayerEntity();
	//CObject* pObject = (handle != -1) ? CPools::GetObjectPool()->GetAt(handle) : obj; // code re3 ERROR NOT FOUND BY OBJECT РАЗБЕРИСЬ
	//CPools::GetVehicleRef(pVehicle) get handle from pointer
	CObject* pObject = (handle != -1) ? CPools::GetObject(handle) : obj; // CPools::GetObjectPool()->GetAt(obj_id) НЕ НАХОДИТ КАК В RE
	//pObject = (CObject*)FindPlayerEntity(); // dbg
	//if (!pObject) { InitConsole(); return; } // dbg
	if (!pObject) { return; }
	CWorld::Remove(pObject);
	pObject->SetHeading(DEGTORAD(degree));
	pObject->m_matrix.UpdateRW();
	pObject->UpdateRwFrame();
	CWorld::Add(pObject);
}





//COMMAND_SET_PLAYER_HEADING
//COMMAND_SET_CHAR_HEADING
//COMMAND_SET_CAR_HEADING
//COMMAND_SET_OBJECT_HEADING
CVector GetObjectPos(CObject* obj, int handle = -1)
{
	if (!obj && (handle == -1)) { return CVector(0, 0, 0); }
	float x = 0.0f, y = 0.0f, z = 0.0f; // CPools::GetObject(handle)
	Command<Commands::GET_OBJECT_COORDINATES>(handle, &x, &y, &z);
	return CVector(x, y, z);

	//CObject* pObject = (handle != -1) ? CPools::GetObjectPool()->GetAt(handle) : obj; // code re3 ERROR NOT FOUND BY OBJECT РАЗБЕРИСЬ
	//CPools::GetVehicleRef(pVehicle) get handle from pointer
	CObject* pObject = (handle != -1) ? CPools::GetObject(handle) : obj; // CPools::GetObjectPool()->GetAt(obj_id) НЕ НАХОДИТ КАК В RE
	if (!pObject) { return CVector(0, 0, 0); }
	return pObject->GetPosition();
}

void SetObjectPos(CVector pos, CObject* obj, int handle = -1) //jd1 3073, jd2 3329
{
	if (!obj && (handle == -1)) { return; }
	//SET_OBJECT_HEADING
	//SET_OBJECT_COORDINATES (re3 COMMAND_SET_OBJECT_COORDINATES)
	//SET_OBJECT_VELOCITY
	//SET_OBJECT_COLLISION
	//SET_OBJECT_DYNAMIC // CPools::GetObject(handle)
	Command<Commands::SET_OBJECT_COORDINATES>(handle, pos.x, pos.y, pos.z);
	return;
	// НИЖЕ ТОЖЕ РАБОТАЕТ

	//CPool_7CObject_13CCutsceneHead_::GetAt(LODWORD(ScriptParams)); 449 CRunningScript::ProcessCommands400To499
	//CPools::GetObject(obj_id)

	// COMMAND_CREATE_OBJECT x
	// COMMAND_GET_OBJECT_COORDINATES 443
	// COMMAND_CREATE_OBJECT_NO_OFFSET 667
	//CEntity* entity = FindPlayerEntity();
	//CObject* pObject = (handle != -1) ? CPools::GetObjectPool()->GetAt(handle) : obj; // code re3 ERROR NOT FOUND BY OBJECT РАЗБЕРИСЬ
	CObject* pObject = (handle != -1) ? CPools::GetObject(handle) : obj; // CPools::GetObjectPool()->GetAt(obj_id) НЕ НАХОДИТ КАК В RE
	//pObject = (CObject*)FindPlayerEntity(); // dbg
	//if (!pObject) { InitConsole(); return; } // dbg
	if (!pObject) { return; }
	//*(CVector*)&ScriptParams[0] = pObject->GetPosition();
	if (pos.z <= MAP_Z_LOW_LIMIT) { pos.z = CWorld::FindGroundZForCoord(pos.x, pos.y); }
	pObject->Teleport(pos);
	CTheScripts::ClearSpaceForMissionEntity(pos, pObject);
}



void PassCurrentMission()
{
	//if (CCutsceneMgr::ms_running) { return; }
	//std::cout << "PassCurrenMission()" << "\n";
	//for (CRunningScript* i = CTheScripts::pActiveScripts; i; i = i->m_pNext)
	for (CCustomRunningScript* i = (CCustomRunningScript*)CTheScripts::pActiveScripts; i; i = (CCustomRunningScript*)i->next)
	{
		//if (i && i->m_bIsMission && i->m_bIsActive && !i->m_bIsExternal) // ext taxi copcar firetru etc...
		//03E6: remove_text_box ??
		//00BE: text_clear_all
		if (i && i->m_bMissionFlag && i->m_bIsMissionScript)
		{
			std::string missionName = ToUpper(std::string(i->m_abScriptName));
			if (offsetMap.contains(missionName))
			{
				//std::cout << "PassCurrenMission() SCR_NAME: " << missionName << "\n";
				if (offsetMap[missionName] != -1)
				{ //!!!!!SANNY OPCODE 030D: => [0D 03]    BASE 65536
					bool jmp_flag = true;
					//CMessages::ClearMessages();
					Command<Commands::CLEAR_PRINTS>(); // equal

					if ((missionName == "RC1") || (missionName == "RC2") || (missionName == "RC3") || (missionName == "RC4"))
					{
						//jmp_flag = false; // need set ip
						//uint32_t& RC1_RECORD = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RC1_RECORD"].index);
						//uint32_t& RC2_RECORD = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RC2_RECORD"].index);
						//uint32_t& RC3_RECORD = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RC3_RECORD"].index);
						//uint32_t& RC4_RECORD = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RC4_RECORD"].index);
						//uint32_t& RC_BUGGY_KILLS = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RC_BUGGY_KILLS"].index);
						// RC_REWARD = kills; RC_REWARD - record; RC_REWARD * 1000; RC2_RECORD = RC_BUGGY_KILLS;
						uint32_t& RC_REWARD = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RC_REWARD"].index);
						RC_REWARD = 2 * 1000;
					}
					else if (missionName == "EIGHT") // fix skin name fbase 191371  0x2EB8B  ------------------------------------------
					{ // START GAME (already jump to final. no text pass)
						jmp_flag = false;
						// add check when eball sti on car to active cargen
						// 1000 2205 2528 
						//if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 1000))/* && CCutsceneMgr::ms_running*/) { return; } // crash on start init? (997)
						if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 2205))/* && CCutsceneMgr::ms_running*/) { return; } // 8ball set in $EIGHT_CAR (cargen init)

						//if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 8621))) // bridge block + oblomk'i :EIGHT_8621 29779
						//{
						//	//uint32_t& v1118 = GetGlobalVariableByIndex<uint32_t>(1118); // $1118 // not tested
						//}

						//-EIGHT_15271 on misty off (LUIGI+NEXTCRASH) fade
						//-EIGHT_15693 init cargens (!LUIGI+CARGEB)
						//-EIGHT_15514 onmission 0 (!LUIGI)
						//-EIGHT_15437 luigi label (LUIGI+NEXTCRASH)
						//-EIGHT_15095 misty go mpass (CRASH+MONEY?)

						// hook procesonecmd?
						// ch model player 0352: set_actor $PLAYER_ACTOR skin_to 'PLAYER' 
						// generate cars :EIGHT_15693 [4C 01 02 5C 09 04 65 + opcode return(51 00)] sz(147(cargen) * 7(bytes) + 2(ret)) | LUIGI2 0x32CDF
						// add blockroad orange + teleport final mission ?
						// remove bridge obj's? :EIGHT_8621
						// go to pass 15437
						// print pass


						//DebugScriptIPsSetIPLabelOffsetByPattern((CRunningScript*)i, "D6 00 04 00 18 81 02 2C 00 4D"); //(10) 306C3 :EIGHT_6968
						//DebugScriptProcessByCount((CRunningScript*)i, 15);

						//int old_ip = ((CRunningScript*)i)->m_nIp;
						////DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, 6968 + 16);
						////((CRunningScript*)i)->ProcessOneCommand();
						//DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, 15693); // crash. Its switcher. enable when eball sit down in EIGHT_CAR
						//DebugScriptProcessByCount((CRunningScript*)i, 1); // cargen shoud dosent work if active in cutscene eight. active when check ip
						//((CRunningScript*)i)->m_nIp = old_ip;

						// :EIGHT_8621 remove broken parts bridge (non conflict cleo)
						// mb recode to Command<Commands::CLEAR_CUTSCENE>(); // remove_objects_from_cube if dont hook ondraw
						//bridgefukb - разбитые тачки после взрыва на мосту
						//bridgefuka - хлам после взрыва на мосту
						//DEFINE OBJECT INDHELIX_BARRIER         // Object number -25

						// WORK!!
						//g_semaphoreOnDraw = false; // critical section IP if cleo, hook ProcessOneCommand crash in cleo region
						//while (!g_OnDrawCallFlag) { Sleep(1); } // wait logic[prior:after] + script process (cleo allow) [end fps => new call time]
						//int old_ip = ((CRunningScript*)i)->m_nIp;
						//DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, 8621 + 18); // 03AE :EIGHT_8621 0x30D4A
						////((CRunningScript*)i)->ProcessOneCommand();
						//DebugScriptProcessByCount((CRunningScript*)i, 6);
						//((CRunningScript*)i)->m_nIp = old_ip;
						//g_semaphoreOnDraw = true;

						//03AE: remove_objects_from_cube 804.0 - 948.0 30.0 765.125 - 924.3125 50.0
						//018E : stop_sound $1586
						//0108 : destroy_object $1118
						//0108 : destroy_object $1119
						//03B6 : replace_model_at 1027.25 - 933.75 15.0 radius 50.0 from #LOD_LAND014 to #INDHELIX_BARRIER
						//0363: toggle_model_render_at 1027.25 - 933.75 15.0 radius 50.0 object #INDHELIX_BARRIER 1
						// cmd base 65536
						Command<Commands::REMOVE_PARTICLE_EFFECTS_IN_AREA>(804.0f, -948.0f, 30.0f, 765.125f, -924.3125f, 50.0);			  // 66478	942 (6)
						Command<Commands::REMOVE_SOUND>(GetGlobalVariableByIndex<uint32_t>(1586)); // $1586								  // 65934	398 (1)
						Command<Commands::DELETE_OBJECT>(GetGlobalVariableByIndex<uint32_t>(1118)); // $1118							  // 65800	264 (1)
						Command<Commands::DELETE_OBJECT>(GetGlobalVariableByIndex<uint32_t>(1119));	// $1119							  // 65800	264 (1)
						// orange bridge lock
						Command<Commands::SWAP_NEAREST_BUILDING_MODEL>(1027.25f, -933.75f, 15.0f, 50.0f, -26, -25);	 // [04 E6 04 E7]	  // 66486	950 (6)
						Command<Commands::SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE>(1027.25f, -933.75f, 15.0f, 50.0f, -25, 1); // [04 E7] // 66403	867 (6)



						ReDressPed(FindPlayerPed(), "PLAYER"); // PLAYERP - orange, PLAYER - normal set_actor $PLAYER_ACTOR skin_to 'PLAYER' // crash check
						DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, 15437); // :EIGHT_15437
						PASS(1500);
						// if use onDraw mode add redress flag and find this mission flag
						// TEST !! WHILE !END ON CRASH REDRESS
						//while (GetScriptByName(missionName.c_str())) { Sleep(5); } // recode to async wait end mission (redress ped sometimes crash game logic? when remove)
						//ReDressPed(FindPlayerPed(), "PLAYER"); // PLAYERP - orange, PLAYER - normal set_actor $PLAYER_ACTOR skin_to 'PLAYER'

						// template. do not double pass
						while (GetScriptByName(missionName.c_str())) { Sleep(1); } // recode to async wait end mission
					}
					else if (missionName == "LUIGI5") // fix money pass 235682 0x398A2 (13057 + 222625)
					{
						//jmp_flag = false; // need set ip
						//uint32_t& LUIGI5_NUM_DROPOFFS = GetGlobalVariableByOffset<uint32_t>(vars_scr_map["LUIGI5_NUM_DROPOFFS"].start_offset); // index sc :LUIGI5_12927 $1715 (srch bytes mission+label 04 00 02 3C 03)
						uint32_t& LUIGI5_NUM_DROPOFFS = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["LUIGI5_NUM_DROPOFFS"].index);
						LUIGI5_NUM_DROPOFFS = 8;
					}
					else if (missionName == "JOEY4")
					{
						// teleport to norm pos
						// close jdoors + visible render 
						// pass ip
						// exit maybe broken veh sentinel nomove

						//if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 2355))) {} // joey door check (2356 closed)
						//if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 2966))) { PlayerExitVehicle(); } // cutscene. broken sentinel

						// 0x576F  9b 02 ? ? ? ? ? ? ? ? ? ? ? ? ? ? c7 01 ? ? ? 77 01   !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
						// MISTY_DOOR2, LAUNDRY_DOOR2, JOEY_DOOR1
						uint32_t& JOEY_DOOR1 = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["JOEY_DOOR1"].index);
						uint32_t& JOEY_DOOR2 = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["JOEY_DOOR2"].index);
						CVector pos = GetObjectPos(nullptr, JOEY_DOOR1);
						if (CCutsceneMgr::ms_running) // COMMAND_SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE Script4.cpp
						{ // not important. can remove it
							//Command<Commands::CLEAR_CUTSCENE>();
							////CObject* pObjDoor1 = CPools::GetObject(JOEY_DOOR1);
							////CObject* pObjDoor2 = CPools::GetObject(JOEY_DOOR2);
							////AddToInvisibilitySwapArray(pObjDoor1, 1);
							////AddToInvisibilitySwapArray(pObjDoor2, 1);
							// НУЖЕН ID ОБЪЕКТА А НЕ HANDLE
							Command<Commands::SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE>(1192.188f, -867.25f, 14.0625f, 6.0f, -5, 1); // [04 E7] // 66403	867 (6) JD1
							Command<Commands::SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE>(1192.188f, -867.25f, 14.0625f, 6.0f, -6, 1); // [04 E7] // 66403	867 (6) JD2
						}

						//if (!((pos.x == 1192.188f) && (pos.y == -867.25f) && (pos.z == 14.0625f)))
						if (pos.z != 14.0625f) // joey door not closed
						{
							SetObjectPos(CVector(1192.188f, -867.25f, 14.0625f), nullptr, JOEY_DOOR1); // thread 'MAIN' init coords obj
							SetObjectPos(CVector(1190.063f, -869.3125f, 14.1875), nullptr, JOEY_DOOR2); // thread 'MAIN' init coords obj
							TeleportPlayer(CVector(1197.7, -870.8, 14.7), 225);  // under joey garage
							PlayerExitVehicle();
							// z_angle 45
						}

						// z_angle 45?
						PASS(3000, 7000); // ms_running not play pass music
					}
					else if (missionName == "JOEY6") // fix money pass :JOEY6_9918 $1835 1ped 10k spawn 3ped + disable marker
					{
						//jmp_flag = false; // need set ip
						//uint32_t& LUIGI5_NUM_DROPOFFS = GetGlobalVariableByOffset<uint32_t>(vars_scr_map["LUIGI5_NUM_DROPOFFS"].start_offset); // index sc :LUIGI5_12927 $1715 (srch bytes mission+label 04 00 02 3C 03)
						uint32_t& v1835 = GetGlobalVariableByIndex<uint32_t>(1835); // $1835
						uint32_t& v259 = GetGlobalVariableByIndex<uint32_t>(259); // $259 :JOEY6_9887 :NONAME_26_32 flag passed this mission
						v1835 = 3 * 10000;
						v259 = 1; // disable blip handler

						//int old_ip = ((CRunningScript*)i)->m_nIp;
						//DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, 10037);
						////((CRunningScript*)i)->ProcessOneCommand();
						//DebugScriptProcessByCount((CRunningScript*)i, 14);
						//((CRunningScript*)i)->m_nIp = old_ip;
					}
					else if (missionName == "TONI3") // joey door + garage cutscene
					{ // 1176 - 2243 (2032) joey
						jmp_flag = false; // need set ip
						//:TONI3_1131 01F5: $PLAYER_ACTOR = create_emulated_actor_from_player $PLAYER_CHAR 
						//if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 2205))) { return; }

						if ((((CRunningScript*)i)->m_nIp >= (SIZE_MAIN_SCRIPT + 1176)) && (((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 2242)))
						{ // in joey garage
							TeleportPlayer(CVector(1197.7, -870.8, 14.7), 225); // under joey garage
						}
						PlayerExitVehicle(); // remove vehicle. if no leave ped die
						DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, offsetMap[missionName]); // ip now. stop moving jd12

						// close joey door
						uint32_t& JOEY_DOOR1 = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["JOEY_DOOR1"].index);
						uint32_t& JOEY_DOOR2 = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["JOEY_DOOR2"].index);
						CVector pos = GetObjectPos(nullptr, JOEY_DOOR1);
						if (pos.z != 14.0625f) // joey door not closed
						{
							SetObjectPos(CVector(1192.188f, -867.25f, 14.0625f), nullptr, JOEY_DOOR1); // thread 'MAIN' init coords obj
							SetObjectPos(CVector(1190.063f, -869.3125f, 14.1875), nullptr, JOEY_DOOR2); // thread 'MAIN' init coords obj
							Command<Commands::SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE>(1192.188f, -867.25f, 14.0625f, 6.0f, -5, 1); // [04 E7] // 66403	867 (6) JD1
							Command<Commands::SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE>(1192.188f, -867.25f, 14.0625f, 6.0f, -6, 1); // [04 E7] // 66403	867 (6) JD2
							// z_angle 45
						}
					}
					else if (missionName == "FRANK1") // fix remove maria 0x4DB5B + teleport + garage lock
					{
						//jmp_flag = false; // need set ip
						// can lock garage door. ist ex obj

						// tp if ip
						// try close
						// pass + wait
						// onmissions 0 + sal on mission 0 def offset
						// disable marker 1999 :FRANK1_17158
						// retn garage


						uint32_t& MARIA = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["MARIA"].index);
						uint32_t& MARIAS_STRETCH = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["MARIAS_STRETCH"].index);
						uint32_t& SALVATORES_GARAGE = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["SALVATORES_GARAGE"].index);

						uint32_t& ONMISSION = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["ONMISSION"].index);
						uint32_t& ON_MISSION_FOR_SALVATORE = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["ON_MISSION_FOR_SALVATORE"].index);
						uint32_t& TONI_MISSION_MARKER = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["TONI_MISSION_MARKER"].index);

						if ((((CRunningScript*)i)->m_nIp <= (SIZE_MAIN_SCRIPT + 2815))) // fix garage in anim(CLOSE_GARAGE NOT WORK) + teleport
						{ // not stack in sal garage
							TeleportPlayer(CVector(1416.063, -189.4375, 50.6), 115); // under joey garage
							CGarage& SAL_GARAGE_OBJ = CGarages::aGarages[SALVATORES_GARAGE];
							CObject& SAL_GARAGE_DOOR_OBJ = *(SAL_GARAGE_OBJ.m_pDoorOne); // m_pDoorTwo null
							CVector& position = SAL_GARAGE_DOOR_OBJ.m_matrix.pos;
							//CVector Cposition = SAL_GARAGE_DOOR_OBJ.m_matrix.pos; // copy
							SAL_GARAGE_DOOR_OBJ.m_matrix.SetRotate(0, 0, 0); // DEGTORAD(0)
							// NEED UPDATE ANGLE DOOR1/2 float fields CGarage???????
							//position = Cposition;
							position = CVector(1428.37f, -183.262f, 51.6481f);
							SAL_GARAGE_DOOR_OBJ.m_matrix.UpdateRW(); // upd frame block end
							SAL_GARAGE_DOOR_OBJ.UpdateRwFrame();

							//InitConsole();
							//std::cout << "pos: " << position.x << " " << position.y << " " << position.z << "\n";

							//Command<Commands::CLOSE_GARAGE>(SALVATORES_GARAGE); // do not close
							Command<Commands::CHANGE_GARAGE_TYPE>(SALVATORES_GARAGE, 14);
							Command<Commands::SET_TARGET_CAR_FOR_MISSION_GARAGE>(SALVATORES_GARAGE, -1);
						}
						//DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, offsetMap[missionName]); // ip now. stop check maria health
						//while (GetScriptByName(missionName.c_str())) { Sleep(1); } // recode to async wait end mission maria lock

						//PASS(1500);
						// allow tonny
						//Command<Commands::REGISTER_MISSION_PASSED>("FM1"); // строковый норм вызываеться
						//Command<Commands::PLAYER_MADE_PROGRESS>(1);
						//Command<Commands::ADD_SPRITE_BLIP_FOR_CONTACT_POINT>(1219.563f, -321.0f, 26.375f, 19, TONI_MISSION_MARKER);
						// on :FRANK1_17108 (offset)
						//ONMISSION = 0;
						//ON_MISSION_FOR_SALVATORE = 0;
						//0318: set_latest_mission_passed 'FM1'	792
						//030C : set_mission_points += 1	780
						//02A7 : $TONI_MISSION_MARKER = create_icon_marker_and_sphere 19 at 1219.563 - 321.0 26.375   679


						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(24);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(99);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(10);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(95);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(105);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(100);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(117);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(2);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(41);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(34);
						//Command<Commands::MARK_MODEL_AS_NO_LONGER_NEEDED>(35);

						//Command<Commands::REMOVE_BLIP>(GetGlobalVariableByIndex<uint32_t>(1999)); // $1999
						//Command<Commands::CHANGE_GARAGE_TYPE>(SALVATORES_GARAGE, 14);
						//Command<Commands::SET_TARGET_CAR_FOR_MISSION_GARAGE>(SALVATORES_GARAGE, -1);
						//Command<Commands::UNLOAD_SPECIAL_CHARACTER>(1);
						//Command<Commands::UNLOAD_SPECIAL_CHARACTER>(2);
						//Command<Commands::MISSION_HAS_FINISHED>();

						Command<Commands::DELETE_CHAR>(MARIA);
					}
					//else if (missionName == "FRANK3") // check 1st cutscene (not important)
					//{
					//	//jmp_flag = false; // need set ip
					//	uint32_t& v275 = GetGlobalVariableByIndex<uint32_t>(275); // $275 flag 1th part mission (1 after start)
					//	uint32_t& v6 = GetGlobalVariableByIndex<uint32_t>(6); // $6 flag 2th is part mission
					//	if ((v275 != 1) || (v6 != 1)) { jmp_flag = false; }
					//}
					else if (missionName == "DIABLO1") // fix race time
					{
						//jmp_flag = false; // need set ip
						uint32_t& DIABLO1_RACE_TIME = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["DIABLO1_RACE_TIME"].index);
						DIABLO1_RACE_TIME = 228; // sec
					}
					else if (missionName == "ASUKA1")
					{
						//jmp_flag = false; // need set ip

						//CMessages::ClearMessages(); // upper
						//Command<Commands::CLEAR_PRINTS>(); // equal

						uint32_t& v316 = GetGlobalVariableByIndex<uint32_t>(316); // $316 flag passed this mission
						v316 = 1;

						//Command<Commands::FAIL_CURRENT_MISSION>();
						//PASS(25000);
						////CMessages::ClearMessages();
						//Command<Commands::CLEAR_PRINTS>(); // equal
						////ClearPlayerStatus(); // already in the end
						////DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, offsetMap[missionName]);

						////:ASUKA1_18988
						//Command<Commands::REGISTER_MISSION_PASSED>("AM1"); // строковый норм вызываеться
						//Command<Commands::PLAYER_MADE_PROGRESS>(1);

						//Command<Commands::START_NEW_SCRIPT>(50022); // 0x5EED5 main
						//Command<Commands::START_NEW_SCRIPT>(61388); // 0x5EED5 main
						//Command<Commands::START_NEW_SCRIPT>(63065); // 0x5EED5 main

						//:ASUKA1_18988
						//03F1 : pedtype 7 add_threat 1
						//0237 : set_gang 0 primary_weapon_to 2 secondary_weapon_to 4
						//0318 : set_latest_mission_passed 'AM1'
						//030C : set_mission_points += 1
						//004F_create_thread @NONAME_42 // -15514 50022
						//004F_create_thread @JOE_BUG   // -4148  61388
						//004F_create_thread @YARD_PH   // -2471  63065
					}
					else if (missionName == "KENJI5") // fix money
					{
						//jmp_flag = false; // need set ip

						//CMessages::ClearMessages(); // upper
						//Command<Commands::CLEAR_PRINTS>(); // equal

						//02FD: text_2numbers_lowpriority 'KM5_5' $2844 $2846 3000 ms 1  :KENJI5_12391
						//~g~Congratulations you killed ~1~ Yardies. BONUS $~1~  [KM5_5]
						uint32_t& v2844 = GetGlobalVariableByIndex<uint32_t>(2844); // $2844 killed cnt 'KILLS'
						uint32_t& v2845 = GetGlobalVariableByIndex<uint32_t>(2845); // $2845 (8) need cnt
						//uint32_t& v2846 = GetGlobalVariableByIndex<uint32_t>(2846); // $2846 bonus
						v2844 = v2845;
					}
					else if (missionName == "RAY6") // fix kill ray + 
					{
						//jmp_flag = false; // need set ip
						uint32_t& RAY = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["RAY"].index);
						Command<Commands::DELETE_CHAR>(RAY);
						// spawn bulletproof patr.?
						CreateCar(96, -1, -1, -2, 1, 0, 0, 0, 0); // :RAY6_8770 $3310 //-2 rotation from player. -1 no rotate. 0 rotation degree
					}
					else if (missionName == "YARD1") // fix num packets
					{
						//jmp_flag = false; // need set ip
						uint32_t& v3503 = GetGlobalVariableByIndex<uint32_t>(3503); // $3503 num packets
						v3503 = 15;
					}
					else if (missionName == "YARD2") // fix mission pass (not important)
					{
						//jmp_flag = false; // need set ip
						uint32_t& v340 = GetGlobalVariableByIndex<uint32_t>(340); // $340 mission flag done?
						v340 = 1; // :YARD2_4631
					}
					else if (missionName == "LOVE4") // replace zabor model
					{
						//:LOVE4_8782
						//00D6_if
						//03CA:   object $54 exists
						//0108: destroy_object $54
						//00D6_if
						//03CA : object $55 exists
						//0108: destroy_object $55
						//029B : $54 = init_object #BROKEN_INSIDE at 362.8125 - 341.3125 17.375
						//01C7: remove_object_from_mission_cleanup_list $54
						//029B : $55 = init_object #BROKEN_OUTSIDE at 360.8125 - 390.875 22.5625
						//01C7: remove_object_from_mission_cleanup_list $55

						//jmp_flag = false; // need set ip
						uint32_t& v54 = GetGlobalVariableByIndex<uint32_t>(54); // $54 // #BROKEN_INSIDE -177
						bool v54Ex = !!CPools::GetObject(v54);
						uint32_t& v55 = GetGlobalVariableByIndex<uint32_t>(55); // $55 // #BROKEN_OUTSIDE -178
						bool v55Ex = !!CPools::GetObject(v55);
						//int h55_test = CPools::GetObjectRef(CPools::GetObject(v55));
						if (v54Ex) { Command<Commands::DELETE_OBJECT>(v54); } // 264
						if (v55Ex) { Command<Commands::DELETE_OBJECT>(v55); } // 264
						Command<Commands::CREATE_OBJECT_NO_OFFSET>(-177, 362.8125f, -341.3125f, 17.375f, &v54); // 667
						Command<Commands::DONT_REMOVE_OBJECT>(v54); // 455
						Command<Commands::CREATE_OBJECT_NO_OFFSET>(-178, 360.8125f, -390.875f, 22.5625, &v55); // 667
						Command<Commands::DONT_REMOVE_OBJECT>(v55); // 455

						// orange bridge lock EXAMPLE SWITCH MODEL
						//Command<Commands::SWAP_NEAREST_BUILDING_MODEL>(1027.25f, -933.75f, 15.0f, 50.0f, -26, -25);	 // [04 E6 04 E7]	  // 66486	950 (6)
						//Command<Commands::SET_VISIBILITY_OF_CLOSEST_OBJECT_OF_TYPE>(1027.25f, -933.75f, 15.0f, 50.0f, -25, 1); // [04 E7] // 66403	867 (6)
						//COMMAND_DOES_OBJECT_EXIST 970
					}
					else if (missionName == "CAT1")
					{ // 5cart + 1dcart $4264 $4266 $4267 $4275 $4276	dcart?($4265)
						//0169: set_fade_color 255 255 255
						//jmp_flag = false; // need set ip
						//bool cred_flag = false;
						//Command<Commands::ARE_CREDITS_FINISHED>(); // update flag
						//CCredits::Start();
						//CCredits::Stop();
						//cred_flag = CCredits::AreCreditsDone(); //nw
						//cred_flag = CCredits::bCreditsGoing;
						//if (cred_flag) { jmp_flag = false; }
						//TeleportPlayer(CVector(-1201.563f, 338.5625f, -100.0f)); // cat heli roof -100 in opcode auto find Z
						CVector destination = CVector(-1201.563f, 339.0f, 31.1f); // cat heli roof
						PlayerExitVehicle();
						//CStreaming::StreamZoneModels(destination);
						TeleportPlayer(destination); // cat heli roof

						uint32_t& v4264 = GetGlobalVariableByIndex<uint32_t>(4264);
						uint32_t& v4266 = GetGlobalVariableByIndex<uint32_t>(4266);
						uint32_t& v4267 = GetGlobalVariableByIndex<uint32_t>(4267);
						uint32_t& v4275 = GetGlobalVariableByIndex<uint32_t>(4275);
						uint32_t& v4276 = GetGlobalVariableByIndex<uint32_t>(4276);
						//uint32_t& v4265 = GetGlobalVariableByIndex<uint32_t>(4265); // dcart

						Command<Commands::DELETE_CHAR>(v4264);
						Command<Commands::DELETE_CHAR>(v4266);
						Command<Commands::DELETE_CHAR>(v4267);
						Command<Commands::DELETE_CHAR>(v4275);
						Command<Commands::DELETE_CHAR>(v4276);
						//Command<Commands::DELETE_CHAR>(v4265);
						//TeleportPlayer(CVector(-1192.0, 346.3, 31.1)); // cat heli roof // reput in script :CAT1_7291
						//if ((((CRunningScript*)i)->m_nIp >= (SIZE_MAIN_SCRIPT + 6766))) { jmp_flag = false; } // already ended game 5437

					}


					//else if (missionName == "EMPTY")
					//{
					//	//jmp_flag = false; // need set ip
					//	//uint32_t& LUIGI5_NUM_DROPOFFS = GetGlobalVariableByOffset<uint32_t>(vars_scr_map["LUIGI5_NUM_DROPOFFS"].start_offset); // index sc :LUIGI5_12927 $1715 (srch bytes mission+label 04 00 02 3C 03)
					//	uint32_t& LUIGI5_NUM_DROPOFFS = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["LUIGI5_NUM_DROPOFFS"].index);
					//	LUIGI5_NUM_DROPOFFS = 8;
					//	//DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, offsetMap[missionName]);
					//}


					//i->m_nIp = SIZE_MAIN_SCRIPT + offsetMap[missionName]; // work ??
					if (jmp_flag) { DebugScriptIPsSetIPLabelOffset((CRunningScript*)i, offsetMap[missionName]); }
					// set cop sensive true?
					//if (CCutsceneMgr::ms_running) { Command<Commands::CLEAR_CUTSCENE>(); } // in clear status
					SetPlayerFrozen(false);
					ClearPlayerStatus();
				}
				else { std::cout << "PassCurrenMission() NOT FOUND IN offsetMap MISSION: " << missionName << "\n"; }
				//ClearPlayerStatus();
				//Teleportation::TeleportOutOfMission(missionName);

				break;
			}
			else { std::cout << "PassCurrenMission() NOT FOUND MISSION: " << missionName << "\n"; } // err
		}
	}

	// dbg block
	//uint32_t& a = GetGlobalVariableByOffset<uint32_t>(vars_scr_map["ONMISSION"].start_offset); a = a == 1 ? 0 : 1; // test
	//InitConsole();
	//std::cout << "BASE: " << (void*)CTheScripts::ScriptSpace << "\n";
	//uint32_t& v275 = GetGlobalVariableByIndex<uint32_t>(275);
	//uint32_t& v6 = GetGlobalVariableByIndex<uint32_t>(6);
	//std::cout << "v275: " << v275 << " v6: " << v6 << "\n";
	//CreateCar(119, 180, 1, 1, 1, 1, 1);
}


void FailCurrenMission()
{
	//std::cout << "FailCurrenMission()" << "\n";
	//if (CTheScripts::IsPlayerOnAMission())
	if (GetPlayerOnMission())
	{
		Command<Commands::CLEAR_SMALL_PRINTS>();
		Command<Commands::PRINT_BIG_Q>("M_FAIL", 5000, 1);
	}
	Command<Commands::FAIL_CURRENT_MISSION>();
	ClearPlayerStatus();
}

void FakePassCurrenMission()
{
	//std::cout << "FakePassCurrenMission()" << "\n";
	int randomMoney = 1000 * _RandVKL(5, 25);

	Command<Commands::CLEAR_SMALL_PRINTS>();
	Command<eScriptCommands::COMMAND_PRINT_WITH_NUMBER_BIG>("M_PASS", randomMoney, 5000, 1);
	Command<Commands::PLAY_MISSION_PASSED_TUNE>(1);
	//PASS(randomMoney); // ?

	CPlayerPed* player = FindPlayerPed();
	if (player)
	{
		player->SetWantedLevel(0);
		player->GetPlayerInfoForThisPlayerPed()->m_nMoney += randomMoney;
	}

	Command<Commands::FAIL_CURRENT_MISSION>();
	ClearPlayerStatus();
}





bool tmp1 = false;
int x = 0, y = 0, z = 0; // get curr pos
void TMPTEST(int mode)
{
	int off = 15;
	if (mode == 0) { x += off; }
	else if (mode == 1) { x -= off; }
	else if (mode == 2) { y += off; }
	else if (mode == 3) { y -= off; }
	else if (mode == 4) { z += off; }
	else if (mode == 5) { z -= off; }

	InitConsole();
	uint32_t& JOEY_DOOR1 = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["JOEY_DOOR1"].index); // UsedObjectArray

	CObject* pObject = CPools::GetObject(JOEY_DOOR1);
	CObject* pObject2 = (CObject*)FindPlayerVehicle();
	CObject* pOut = pObject2;


	//std::cout << "0x" << (void*)&(pObject->m_matrix) << "\n";

	//SetObjectAngle(CVector(x, y, z), nullptr, JOEY_DOOR1);
	//CVector res = GetMatrixAngels(&(pObject->m_matrix));
	//std::cout << "x: " << res.x << " y: " << res.y << " z: " << res.z << "\n";
	//DumpMatrix(&(pObject->m_matrix), CVector(x, y, z));

	//SetObjectAngle(CVector(x, y, z), pOut, -1);
	//SetObjectScale(1.5f, nullptr, JOEY_DOOR1);

	//uint32_t& JOEY_DOOR2 = GetGlobalVariableByIndex<uint32_t>(vars_scr_map["JOEY_DOOR2"].index);
	//SetObjectPos(CVector(1192.188f, -867.25f, 14.0625f), nullptr, JOEY_DOOR1); // thread 'MAIN' init coords obj 5,6
	//SetObjectPos(CVector(1192.188f, -865.25f, 14.0625f), nullptr, JOEY_DOOR1); // thread 'MAIN' init coords obj 5,6
	//ped->SetOrientation(0.0f, 0.0f, 0.0f);
	//SetObjectPos(CVector(1190.063f, -869.3125f, 15.1875f), nullptr, JOEY_DOOR2); // thread 'MAIN' init coords obj
	//SetObjectHeading(90, nullptr, JOEY_DOOR1);
	//SetObjectHeading(90, nullptr, JOEY_DOOR2);

	//SetObjectHeading(tmp1 ? 90 : 45, nullptr, JOEY_DOOR1);
	//tmp1 = !tmp1;
	//return;
	//CVector vec = GetObjectAngle(nullptr, JOEY_DOOR1);
	//std::cout << "x: " << vec.x << " y: " << vec.y << " z: " << vec.z << "\n";
}




int glob_bpress_delay = 50;
void KillVehiclePoolHandler()
{
	char action = 'T';

	while (true)
	{

		if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState('T') & 0x8000)) { glob_bpress_delay = 150; PassCurrentMission(); }
		else if ((GetAsyncKeyState('T') & 0x8000)) { glob_bpress_delay = 150; PassCurrentMission(); }

		//if ((GetAsyncKeyState('Y') & 0x8000)) { TeleportPlayer(CVector(866.0f, -307.8f, 8.3f)); } // eight logo

		//if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState('E') & 0x8000)) { TMPTEST(1); }
		//else if ((GetAsyncKeyState('E') & 0x8000)) { TMPTEST(0); }

		//if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState('R') & 0x8000)) { TMPTEST(3); }
		//else if ((GetAsyncKeyState('R') & 0x8000)) { TMPTEST(2); }

		//if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && (GetAsyncKeyState('T') & 0x8000)) { TMPTEST(5); }
		//else if ((GetAsyncKeyState('T') & 0x8000)) { TMPTEST(4); }

		Sleep(glob_bpress_delay);
	}
}

DWORD CALLBACK DebugScriptEntry(LPVOID)
{
	//InitConsole();
	KillVehiclePoolHandler();

	return TRUE;
}




void StartDebugScriptThread()
{
	HANDLE threadHandle = CreateThread(NULL, 0, DebugScriptEntry, NULL, 0, NULL);
	g_threads.push_back(threadHandle);
}

void StopAllThreads()
{
	for (HANDLE handle : g_threads)
	{
		TerminateThread(handle, 0);
		CloseHandle(handle);
	}
}



void OnStart()
{
	//InitConsole();
	//HookPOCLogic(); // control ip logic cleo conflict
	StartDebugScriptThread();
}

void shutdownRwEvent() // CGame::ShutdownRenderWare(void)
{
	//std::cout << "shutdownRwEvent()" << "\n";
	StopAllThreads();
}







bool initRwEventFIX = false;
class GTA3_OFFSETS_TEST {
public:
	GTA3_OFFSETS_TEST() {

		Events::initGameEvent += [] { // only 1 call adapter
			if (initRwEventFIX) { return; } // adapter to initRwEvent
			else { initRwEventFIX = true; }
			//InitConsole();
			OnStart();
			//Patch1();
			//PatchProtectMem();
		};

		Events::drawHudEvent += []
		{
			CPlayerPed* player = FindPlayerPed();
			if (!player) { return; }

			//g_OnDrawCallFlag = true; // 4 control IP Scripts
			//while (!g_semaphoreOnDraw) { Sleep(1); }
			//g_OnDrawCallFlag = false;

			//PatchProtectMem();
			//Test();
			//std::cout << "test: " << test << "\n";
		};

		//Events::shutdownRwEvent += [] {};            
	}


	~GTA3_OFFSETS_TEST() { shutdownRwEvent(); } // аля shutdownRwEvent, вызов идёт asi когда dll уже нет        

} GTA3_OFFSETS_TEST;



void InsertEventPatch(void* ptr, void* handler, bool FIRST_PRIORITY = true) // FIRST_PRIORITY true insert my call above orig call
{
	if (ReadPtrDanger<unsigned char>(ptr) != 0xE8) { InitConsole(); std::cout << "ERROR CHECK InsertEventPatch(0xE8)!! PatchCalls() 0x" << ptr << "\n"; return; }

	//---SETTINGS--NEW--BLOCK
	int available_sz_patch = 5; // дёрнуть байты из орига (+3 align 10h можно добавить, без них работает)
	int need_sz_patch = 5; // bytes // доп блок 1 + 4 (5 байт которые переместили из ориг суммируються в функции), только доп байты!!
	int offset = FIRST_PRIORITY ? need_sz_patch : 0; // max need_sz_patch(будет в конце перед jmp)  block(200+51+1+4)
	bool jmp_patch_in_end_region = false; // optimize


	//---COLLECT--ORIG--DATA--OR--POINTERS--TO--PATCH--MOVED---BLOCK (mov/jmp/jz/je/jnz/jne/call)!!
	// push from ptr not need patch. this is not offset
	void* orig_call_dest_ptr = CalcPointerFromJMPE9Offset(ptr, ReadPtrDanger<uintptr_t>(Transpose(ptr, sizeof(unsigned char)))); // вычисляю указатель который вызывал call


	//---OUT
	void* out_patch_ptr = nullptr;
	int out_patch_sz = 0; // mbi.region_sz
	//bool res = SetPatchBlock(orig_ptr_for_patch_reference, available_sz_patch, need_sz_patch, out_patch_ptr, out_patch_sz, true, offset); // jmp in the end region
	bool res = SetPatchBlock(ptr, available_sz_patch, need_sz_patch, out_patch_ptr, out_patch_sz, jmp_patch_in_end_region, offset); // jmp after need_sz_patch
	void* moved_block = Transpose(out_patch_ptr, offset); // указатель на скопированный блок


	//----PATCH--LOGIC---BLOCK
	InsertCall(moved_block, orig_call_dest_ptr); // fix call offset ?? ff 15 ??


	//---INSERT CALL
	void* PatchFreeSpacePtr = FIRST_PRIORITY ? out_patch_ptr : Transpose(moved_block, available_sz_patch);
	WriteDanger<unsigned char>(PatchFreeSpacePtr, 0xE8); // call
	InsertCall(PatchFreeSpacePtr, handler);

	//void* buff = MkMem((4 * sizeof(void*))); // bytes // usage 4 * 3
	//InsertJump(jmp, retn_false_label_ptr);
}