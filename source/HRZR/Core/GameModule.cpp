#include "../DebugUI/MainMenuBar.h"
#include "GameModule.h"

namespace HRZR
{
	void HookedSetTimescale(void *, float Timescale, float TransitionTime)
	{
		auto gameModule = GameModule::GetInstance();

		if (DebugUI::MainMenuBar::m_TimescaleOverride && (!gameModule->IsPaused() || DebugUI::MainMenuBar::m_TimescaleOverrideInMenus))
		{
			gameModule->m_WorldTimeScale = DebugUI::MainMenuBar::m_Timescale;
			gameModule->m_TargetWorldTimeScale = DebugUI::MainMenuBar::m_Timescale;
			gameModule->m_WorldRepresentationTimeTimeScale = DebugUI::MainMenuBar::m_Timescale;
			return;
		}

		gameModule->SetTimescale(Timescale, TransitionTime);
	}

	DECLARE_HOOK_TRANSACTION(GameModule)
	{
		Hooks::WriteCall(Offsets::Signature("E8 ? ? ? ? 48 8B 05 ? ? ? ? 4C 8D 3D ? ? ? ? 45 8B F5"), &HookedSetTimescale);
	};
}
