#pragma once

#include "RTTIObject.h"

namespace HRZR
{
	enum class EGodMode : int
	{
		Off = 0,
		On = 1,
		Invulnerable = 2,
	};

	class DebugSettings : public RTTIObject
	{
	public:
		char _pad8[0x4];							  // 0x8
		bool m_PlayerCoverEnabled;					  // 0xC
		char _padD[0x3];							  // 0xB
		bool m_ShowInfo;							  // 0x10
		char _pad11[0xE];							  // 0x11
		bool m_SPAllUnlocked;						  // 0x1F
		bool m_MPAllUnlocked;						  // 0x20
		char _pad21[0x1];							  // 0x21
		bool m_NoInactivityCheck;					  // 0x22
		char _pad23[0x1];							  // 0x23
		EGodMode m_GodModeState;					  // 0x24
		bool m_InfiniteAmmo;						  // 0x28
		bool m_Inexhaustible;						  // 0x29
		char _pad29[0x1];							  // 0x2A
		bool m_ApplyPhotoModeSettingsInGame;		  // 0x2B
		char _pad2C[0x8];							  // 0x2C
		bool m_InfiniteSizeClip;					  // 0x34
		char _pad35[0x8];							  // 0x35

		virtual const RTTI *GetRTTI() const override; // 0
		virtual ~DebugSettings() override;			  // 1

		static DebugSettings *GetInstance()
		{
			const auto ptr = Offsets::Signature("48 8B 05 ? ? ? ? 80 78 2B 00 74 2E 48 8B 05")
				.AsRipRelative(7)
				.ToPointer<DebugSettings *>();

			return *ptr;
		}
	};
	assert_offset(DebugSettings, m_SPAllUnlocked, 0x1F);
	assert_offset(DebugSettings, m_GodModeState, 0x24);
	assert_offset(DebugSettings, m_InfiniteSizeClip, 0x34);
}
