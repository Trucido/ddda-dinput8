﻿// taken from CheatEngine table by Cielos
// http://forum.cheatengine.org/viewtopic.php?p=5641841#5641841

#include "Cheats.h"
#include "dinput8.h"
#include "TweakBar.h"

bool runAnimation;
LPBYTE pRunAnimation;
LPVOID oRunAnimation;
void __declspec(naked) HRunAnimation()
{
	__asm	mov		eax, 0x20;
	__asm	jmp		oRunAnimation;
}

float mWeight = 1.0;
LPBYTE pWeight;
LPVOID oWeight;
void __declspec(naked) HWeight()
{
	__asm	movss	xmm0, mWeight;
	__asm	mulss	xmm5, xmm0;
	__asm	jmp		oWeight;
}

bool realTime;
float mTimeInterval;
UINT rTimeInterval;
LPBYTE pTimeInterval;
LPVOID oTimeInterval;
void __declspec(naked) HTimeInterval()
{
	__asm
	{
		cmp		dword ptr[esp + 8], 0;
		je		getBack;
		cmp		realTime, 0;
		je		notRealTime;
		mov		eax, 0x11;
		mov		dword ptr[esp + 8], eax;
		jmp		getBack;

	notRealTime:
		fild	dword ptr[esp + 8];
		fmul	mTimeInterval;
		fistp	rTimeInterval;
		mov		eax, rTimeInterval;
		mov		dword ptr[esp + 8], eax;

	getBack:
		jmp oTimeInterval;
	}
}

void getCheats(void *value, void *clientData)
{
	if (clientData == &runAnimation)
		*(bool*)value = *(bool*)clientData;
	else
		*(float*)value = *(float*)clientData;
}

void setCheats(const void *value, void *clientData)
{
	if (clientData == &runAnimation)
	{
		config.setBool(L"cheats", L"useTownRun", runAnimation = *(bool*)value);
		Hooks::SwitchHook("Cheat (useTownRun)", pRunAnimation, runAnimation);
	}
	else if (clientData == &mWeight)
	{
		bool prevState = mWeight >= 0;
		config.setBool(L"cheats", L"weightMultiplicator", mWeight = *(float*)value);
		bool newState = mWeight >= 0;

		if (prevState != newState)
			Hooks::SwitchHook("Cheat (weight)", pWeight, newState);
	}
	else if (clientData == &mTimeInterval)
	{
		bool prevState = mTimeInterval >= 0;
		config.setBool(L"cheats", L"timeInterval", mTimeInterval = *(float*)value);
		realTime = mTimeInterval == 0;
		bool newState = mTimeInterval >= 0;

		if (prevState != newState)
			Hooks::SwitchHook("Cheat (timeInterval)", pTimeInterval, newState);
	}
}

void Hooks::Cheats()
{
	BYTE sigRun[] = { 0x8B, 0x42, 0x40,			//mov	eax, [edx+40h]
					0x53,						//push	ebx
					0x8B, 0x5C, 0x24, 0x08 };	//mov	ebx, [esp+4+arg_0]
	if (FindSignature("Cheat (useTownRun)", sigRun, &pRunAnimation))
	{
		runAnimation = config.getBool(L"cheats", L"useTownRun", false);
		CreateHook("Cheat (useTownRun)", pRunAnimation += 3, &HRunAnimation, &oRunAnimation, runAnimation);
		TweakBarAddCB("miscRun", TW_TYPE_BOOLCPP, setCheats, getCheats, &runAnimation, "group=Misc label='Use town run'");
	}

	BYTE sigWeight[] = { 0xF3, 0x0F, 0x58, 0xAB, 0x4C, 0x02, 0x00, 0x00,	//addss		xmm5, dword ptr [ebx+24Ch]
							0x45 };											//inc		ebp
	if (FindSignature("Cheat (weight)", sigWeight, &pWeight))
	{
		mWeight = config.getFloat(L"cheats", L"weightMultiplicator", -1);
		CreateHook("Cheat (weight)", pWeight, &HWeight, &oWeight, mWeight >= 0);
		TweakBarAddCB("miscWeight", TW_TYPE_FLOAT, setCheats, getCheats, &mWeight, "group=Misc label='Weight multiplicator' step=0.1 min=-1.0 max=1.0");
	}

	BYTE sigTime[] = { 0x8B, 0x44, 0x24, 0x08, 0x01, 0x86, 0x68, 0x87, 0x0B, 0x00 };
	if (FindSignature("Cheat (timeInterval)", sigTime, &pTimeInterval))
	{
		mTimeInterval = config.getFloat(L"cheats", L"timeInterval", -1);
		realTime = mTimeInterval == 0;
		CreateHook("Cheat (timeInterval)", pTimeInterval, &HTimeInterval, &oTimeInterval, mTimeInterval >= 0);
		TweakBarAddCB("miscTime", TW_TYPE_FLOAT, setCheats, getCheats, &mTimeInterval, "group=Misc label='Time speed' step=0.1 min=-1.0");
	}

	BYTE *pOffset;
	if (config.getBool(L"cheats", L"shareWeaponSkills", false))
	{
		BYTE sig1[] = { 0x0F,0x84,0x97,0x00,0x00,0x00,0x8B,0xCC,0xCC,0xCC,0x8B,0xCC,0x8B };
		BYTE sig2[] = { 0x75,0x06,0xC7,0x02,0xFF,0xFF,0xFF,0xFF,0x8B,0xCC,0xCC,0x83 };
		BYTE sig3[] = { 0x75,0x18,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x70,0x02 };
		BYTE sig4[] = { 0x75,0x18,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x74,0x02 };
		BYTE sig5[] = { 0x75,0x18,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x78,0x02 };
		BYTE sig6[] = { 0x75,0x18,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x7C,0x02 };
		BYTE sig7[] = { 0x75,0x18,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x80,0x02 };
		BYTE sig8[] = { 0x75,0x18,0x8B,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x33,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0xCC,0x84,0x02 };

		if (FindSignature("Cheat (shareWeaponSkills1)", sig1, &pOffset))
			Set<BYTE>(pOffset, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 });

		if (FindSignature("Cheat (shareWeaponSkills2)", sig2, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
		if (FindSignature("Cheat (shareWeaponSkills3)", sig3, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
		if (FindSignature("Cheat (shareWeaponSkills4)", sig4, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
		if (FindSignature("Cheat (shareWeaponSkills5)", sig5, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
		if (FindSignature("Cheat (shareWeaponSkills6)", sig6, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
		if (FindSignature("Cheat (shareWeaponSkills7)", sig7, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
		if (FindSignature("Cheat (shareWeaponSkills8)", sig8, &pOffset)) Set<BYTE>(pOffset, { 0xEB });
	}
	else
		logFile << "Cheat (shareWeaponSkills): disabled" << std::endl;

	if (config.getBool(L"cheats", L"ignoreEquipVocation", false))
	{
		BYTE sig1[] = { 0x0F, 0x94, 0xC0, 0xC2, 0x0C, 0x00 };
		BYTE sig2[] = { 0x74, 0x0F, 0x3B, 0xC2, 0x74, 0x0E, 0x8B, 0x41, 0x04, 0x83 };
		BYTE sig3[] = { 0x3B, 0xC2, 0x75, 0x15, 0x8B, 0x4D, 0x10 };
		BYTE sig4[] = { 0x0F, 0x94, 0xC0, 0x84, 0xC0, 0x75, 0x09, 0x5F };
		BYTE sig5[] = { 0x0F, 0x94, 0xC0, 0x84, 0xC0, 0x74, 0x60 };
		BYTE sig6[] = { 0x0F, 0x95, 0xC1, 0x84, 0xC9, 0x74, 0x09, 0x83 };
		BYTE sig7[] = { 0x75, 0x02, 0xB3, 0x01, 0x8B, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xE8 };
		BYTE sig8[] = { 0x75, 0x17, 0x8B, 0x44, 0x24, 0x14 };

		if (FindSignature("Cheat (ignoreEquipVocation1)", sig1, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0xB0, 0x01 });
		if (FindSignature("Cheat (ignoreEquipVocation2)", sig2, &pOffset)) Set<BYTE>(pOffset + 4, { 0xEB });
		if (FindSignature("Cheat (ignoreEquipVocation3)", sig3, &pOffset)) Set<BYTE>(pOffset + 2, { 0x90, 0x90 });
		if (FindSignature("Cheat (ignoreEquipVocation4)", sig4, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0xB0, 0x01 });
		if (FindSignature("Cheat (ignoreEquipVocation5)", sig5, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0xB0, 0x01 });
		if (FindSignature("Cheat (ignoreEquipVocation6)", sig6, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x30, 0xC9 });
		if (FindSignature("Cheat (ignoreEquipVocation7)", sig7, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x90 });
		if (FindSignature("Cheat (ignoreEquipVocation8)", sig8, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x90 });
	}
	else
		logFile << "Cheat (ignoreEquipVocation): disabled" << std::endl;

	if (config.getBool(L"cheats", L"ignoreSkillVocation", false))
	{
		BYTE sig1[] = { 0x74, 0x2F, 0x8B, 0x47, 0x10 };
		BYTE sig2[] = { 0x74, 0x74, 0x8B, 0x44, 0x24, 0x1C };
		BYTE sig3[] = { 0x74, 0x24, 0x83, 0xBD, 0xCC, 0xCC, 0xCC, 0xCC, 0x05 };
		BYTE sig4[] = { 0x8B, 0x4A, 0x10, 0x49, 0x3B, 0xC1 };

		if (FindSignature("Cheat (ignoreSkillVocation1)", sig1, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x90 });
		if (FindSignature("Cheat (ignoreSkillVocation2)", sig2, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x90 });
		if (FindSignature("Cheat (ignoreSkillVocation3)", sig3, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x90 });
		if (FindSignature("Cheat (ignoreSkillVocation4)", sig4, &pOffset)) Set<BYTE>(pOffset, { 0x90, 0x90, 0x8B, 0xC1 });
	}
	else
		logFile << "Cheat (ignoreSkillVocation): disabled" << std::endl;
}