#pragma once

#include "RTTIObject.h"

namespace HRZR
{
	enum class EDifficulty : int
	{
		DIFF_NONE = -1,
		DIFF_VERY_EASY = 0,
		DIFF_EASY = 1,
		DIFF_MEDIUM = 2,
		DIFF_HARD = 3,
		DIFF_ULTRA_HARD = 4,
		DIFF_IMPOSSIBLE = 5,
		DIFF_NUM = 6,
	};

	class SystemParams
	{
	public:
		char _pad0[0xE4];
		EDifficulty m_HighestCompletedNewGamePlusDifficulty; // 0xE4
	};

	class PlayerProfile : /*public StateObject, */ public RTTIObject
	{
	public:
		char _pad0[0x410];
		SystemParams m_SystemParams; // 0x418
	};
	assert_offset(PlayerProfile, m_SystemParams, 0x418);
}
