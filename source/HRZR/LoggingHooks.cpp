#include <Windows.h>
#include "../ModConfiguration.h"

namespace LoggingHooks
{
	void WINAPI hk_OutputDebugStringA(LPCSTR OutputString)
	{
		if (!OutputString)
			return;

		// Remove trailing newline
		auto finalLength = strlen(OutputString);

		if (finalLength > 0 && OutputString[finalLength - 1] == '\n')
			finalLength--;

		spdlog::info("{:.{}s}", OutputString, finalLength);
	}

	DECLARE_HOOK_TRANSACTION(LoggingHooks)
	{
		Hooks::RedirectImport(nullptr, "KERNEL32.dll", "OutputDebugStringA", &hk_OutputDebugStringA);

		if (ModConfiguration.SkipPSNAccountLinking)
		{
			// Kill PSN SDK error message when not installed
			Memory::Patch(Offsets::Signature("33 C9 48 8B 5C 24 30 48 8B 74 24 38 48 83 C4 20 5F 48 FF") + 0x11, { 0xC3 });

			// Kill mandatory PSN account linking
			Memory::Fill(Offsets::Signature("74 1B 80 78 10 00 74 15 E8 ? ? ? ? 84 C0 74 0C"), 0x90, 17);

			// Kill GPU driver version check
			Memory::Patch(Offsets::Signature("0F 85 ? ? ? ? C6 45 ? 00 4C 89 ? ? 48 C7 ? ? 07 00 00 00"), { 0x90, 0xE9 });
		}

		// Kill crash reporter/logger
		Memory::Patch(Offsets::Signature("40 53 48 83 EC 20 80 79 38 00 48 8B D9 75 4F E8"), { 0xC3 });
	};
}
