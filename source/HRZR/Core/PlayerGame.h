#pragma once

#include "Player.h"

namespace HRZR
{
	class PlayerGame : public Player
	{
	public:
		char _pad0[0x220];
		bool m_RestartOnSpawned; // 0x350
	};
	assert_offset(PlayerGame, m_RestartOnSpawned, 0x350);
}
