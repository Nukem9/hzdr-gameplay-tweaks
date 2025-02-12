#pragma once

#include "CoreObject.h"

namespace HRZR
{
	class Module : public CoreObject
	{
	public:
		int m_PauseRequests;						  // 0x20

		virtual const RTTI *GetRTTI() const override; // 0
		virtual ~Module() override;					  // 1
		virtual bool InitModule();					  // 14
		virtual void ExitModule();					  // 15
		virtual void UpdateModule();				  // 16
		virtual void DrawModule();					  // 17
		virtual bool Pause(bool);					  // 18
		virtual bool Continue();					  // 19

		bool IsPaused() const
		{
			return m_PauseRequests > 0;
		}
	};
}
