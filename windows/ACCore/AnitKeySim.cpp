#include "../ThirdParty/phnt/phnt.h"

HHOOK MouseHook = 0;
HHOOK KeyboardHook = 0;

LRESULT OnKeyboardHook(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
    if ((reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam)->flags &
         (LLKHF_INJECTED | LLKHF_LOWER_IL_INJECTED))) {
      return true;
    }

    if (!(GetAsyncKeyState(reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam)->vkCode) &
          0x01)) {
      return true;
    }
  }

  return CallNextHookEx(KeyboardHook, nCode, wParam, lParam);
}

LRESULT OnMouseHook(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION &&
      (wParam == WM_RBUTTONDOWN || wParam == WM_LBUTTONDOWN)) {
    if ((reinterpret_cast<PMSLLHOOKSTRUCT>(lParam)->flags &
         (LLMHF_INJECTED | LLMHF_LOWER_IL_INJECTED))) {
      return true;
    }
  }

  return CallNextHookEx(MouseHook, nCode, wParam, lParam);
}

INT HookAndPeekMessageThread() {
  MouseHook = SetWindowsHookEx(WH_MOUSE_LL, OnMouseHook, nullptr, nullptr);
  KeyboardHook =
      SetWindowsHookEx(WH_KEYBOARD_LL, OnKeyboardHook, nullptr, nullptr);

  if (MouseHook || KeyboardHook) {
    MSG message;
    while (GetMessage(&message, nullptr, 0, 0)) {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
  }

  return 0;
}