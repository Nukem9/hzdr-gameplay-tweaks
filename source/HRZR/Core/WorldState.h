#pragma once

#include "RTTIRefObject.h"

namespace HRZR
{
	struct GameWorldTime
	{
		float m_TimeOfDay;
		float m_DurationOfOneDay;
		int m_Day;
	};

	class WorldState : public RTTIRefObject
	{
	public:
		char _pad20[0x78];				  // 0x20
		GameWorldTime m_CurrentWorldTime; // 0x98
		bool m_EnableDayNightCycle;		  // 0xA4
		bool m_PausedDayNightCycle;		  // 0xA5

		void SetTimeOfDay(float Time, float FastForwardDuration)
		{
			const auto func = Offsets::Signature("48 83 EC 28 C5 F8 57 C0 C5 F8 2F D0 C5 F8 28 D9 C5 FA 10 89 98 00 00 00")
								  .ToPointer<void(WorldState *, float, float)>();

			func(this, Time, FastForwardDuration);
		}
	};
	assert_offset(WorldState, m_CurrentWorldTime, 0x98);
	assert_offset(WorldState, m_EnableDayNightCycle, 0xA4);
}
