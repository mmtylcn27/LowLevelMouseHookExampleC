#include <Windows.h>
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> is_active;

LRESULT WINAPI MouseCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (is_active && nCode >= 0)
	{
		auto* hook_struct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

		if (hook_struct->flags & LLMHF_INJECTED)
		{
			hook_struct->flags &= ~LLMHF_INJECTED;
			std::cout << "Flag Removed: LLMHF_INJECTED" << std::endl;
		}

		if (hook_struct->flags & LLMHF_LOWER_IL_INJECTED)
		{
			hook_struct->flags &= ~LLMHF_LOWER_IL_INJECTED;
			std::cout << "Flag Removed: LLMHF_LOWER_IL_INJECTED" << std::endl;
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT WINAPI KeyboardCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (is_active && nCode >= 0)
	{
		const auto key_event = static_cast<int>(wParam);

		if (key_event == WM_KEYDOWN || key_event == WM_KEYUP)
		{
			auto* hook_struct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

			if (hook_struct->flags & LLKHF_INJECTED)
			{
				hook_struct->flags &= ~LLKHF_INJECTED;
				std::cout << "Flag Removed: LLKHF_INJECTED" << std::endl;
			}

			if (hook_struct->flags & LLMHF_LOWER_IL_INJECTED)
			{
				hook_struct->flags &= ~LLMHF_LOWER_IL_INJECTED;
				std::cout << "Flag Removed: LLMHF_LOWER_IL_INJECTED" << std::endl;
			}
		}
	}

	return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void MessageProcess()
{
	const auto get_module = GetModuleHandle(nullptr);
	const auto hook_mouse = SetWindowsHookEx(WH_MOUSE_LL, &MouseCallback, get_module, 0);

	if (hook_mouse == nullptr)
	{
		std::cout << "hook_mouse error: " << GetLastError() << std::endl;
		return;
	}
	
	const auto hook_keyboard = SetWindowsHookEx(WH_KEYBOARD_LL, &KeyboardCallback, get_module, 0);

	if (hook_keyboard == nullptr)
	{
		std::cout << "hook_keyboard error: " << GetLastError() << std::endl;
		UnhookWindowsHookEx(hook_mouse);
		return;
	}

	std::cout << "get_module: 0x" << get_module << std::endl;
	std::cout << "hook_mouse: 0x" << hook_mouse << std::endl;
	std::cout << "hook_keyboard: 0x" << hook_keyboard << std::endl;
	
	for (MSG msg; is_active; std::this_thread::sleep_for(std::chrono::milliseconds(1)))
	{
		if (PeekMessage(&msg, nullptr, 0, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	UnhookWindowsHookEx(hook_mouse);
	UnhookWindowsHookEx(hook_keyboard);
}

int main()
{
	is_active = true;
	std::thread msg_thread(MessageProcess);
	std::cout << "msg_thread: " << msg_thread.get_id() << std::endl;
	
	while (true)
	{
		std::string input_console;
		std::cin >> input_console;

		if (input_console == "exit")
			break;
	}

	std::cout << "wait for exit..." << std::endl;
	is_active = false;
	msg_thread.join();
	std::cout << "exit success" << std::endl;
	std::cin.get();
	
	return 0;
}
