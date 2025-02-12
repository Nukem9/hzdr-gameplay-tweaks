#pragma once

#include "Module.h"
#include "NetMessageListener.h"
#include "NetSubSystemListener.h"

namespace HRZR
{
	class WorldState;
	class WeatherSystem;

	class GameModule : public Module /*, public NetMessageListener, private NetSubSystemListener*/
	{
	public:
		char _pad28[0x30];							  // 0x28
		Ref<WorldState> m_WorldState;				  // 0x58
		char _pad60[0x20];							  // 0x60
		float m_WorldRepresentationTimeTimeScale;	  // 0x80
		char _pad84[0x3C];							  // 0x84
		float m_UserInterfaceTimeTimeScale;			  // 0xC0
		char _padC4[0xC8];							  // 0xC4
		float m_WorldTimeScale;						  // 0x18C
		float m_TargetWorldTimeScale;				  // 0x190
		float m_WorldTimeScaleSpeed;				  // 0x194
		char _pad198[0x138];						  // 0x198
		WeatherSystem *m_WeatherSystem;				  // 0x2D0

		virtual const RTTI *GetRTTI() const override; // 0
		virtual ~GameModule() override;				  // 1
		virtual bool InitModule() override;			  // 14
		virtual void ExitModule() override;			  // 15
		virtual void UpdateModule() override;		  // 16
		virtual void DrawModule() override;			  // 17
		virtual bool Pause(bool) override;			  // 18
		virtual bool Continue() override;			  // 19

		void SetTimescale(float Timescale, float TransitionTime)
		{
			Timescale = std::clamp(Timescale, 0.0f, 1.0f);
			m_TargetWorldTimeScale = Timescale;

			if (TransitionTime > 0.0f)
			{
				m_WorldTimeScaleSpeed = (Timescale - m_WorldTimeScale) / TransitionTime;
			}
			else
			{
				m_WorldTimeScale = Timescale;

				if (!IsPaused())
				{
					m_WorldRepresentationTimeTimeScale = Timescale;
					m_UserInterfaceTimeTimeScale = 1.0f;
				}
				else
				{
					m_WorldRepresentationTimeTimeScale = 0.0f;
					m_UserInterfaceTimeTimeScale = 0.0f;
				}
			}
		}

		static GameModule *GetInstance()
		{
			const auto ptr = Offsets::Signature("48 8B 0D ? ? ? ? C5 FA ? ? ? ? ? ? C5 D8 57 E4")
				.AsRipRelative(7)
				.ToPointer<GameModule *>();

			return *ptr;
		}
	};
	assert_offset(GameModule, m_WorldRepresentationTimeTimeScale, 0x80);
	assert_offset(GameModule, m_UserInterfaceTimeTimeScale, 0xC0);
	assert_offset(GameModule, m_WorldTimeScale, 0x18C);
	assert_offset(GameModule, m_WeatherSystem, 0x2D0);
	assert_offset(GameModule, m_WeatherSystem, 0x2D0);
}
