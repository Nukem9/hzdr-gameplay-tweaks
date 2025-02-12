#include "../DebugUI/MainMenuBar.h"

namespace HRZR
{
	class AIManagerGame;

	void(*AIManagerGameUpdateAIJob1)(void *Agent);
	void HookedAIManagerGameUpdateAIJob1(void *Agent)
	{
		if (DebugUI::MainMenuBar::m_PauseAIProcessing)
			return;

		AIManagerGameUpdateAIJob1(Agent);
	}

	void (*AIManagerGameUpdateAIJob2)(void *Agent);
	void HookedAIManagerGameUpdateAIJob2(void *Agent)
	{
		if (DebugUI::MainMenuBar::m_PauseAIProcessing)
			return;

		AIManagerGameUpdateAIJob2(Agent);
	}

	DECLARE_HOOK_TRANSACTION(AIManagerGame)
	{
		Hooks::WriteJump(
			Offsets::Signature("48 8D 05 ? ? ? ? 48 89 B7 C0 8E 03 00 FF 87 A8 8E 03 00").AsRipRelative(7),
			&HookedAIManagerGameUpdateAIJob1,
			&AIManagerGameUpdateAIJob1);

		Hooks::WriteJump(
			Offsets::Signature("48 8D 05 ? ? ? ? 48 89 83 68 37 00 00 48 8D 83 A0 37 00 00").AsRipRelative(7),
			&HookedAIManagerGameUpdateAIJob2,
			&AIManagerGameUpdateAIJob2);
	};
}
