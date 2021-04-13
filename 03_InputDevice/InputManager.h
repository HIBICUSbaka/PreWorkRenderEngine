#pragma once

#include "ID_BasicMacro.h"
#include <Windows.h>
#include <dinput.h>
#include "WindowWIN32.h"
#include "ID_ExportMacro.h"
#include "InputDeviceBase.h"

typedef POINT MOUSE_OFFSET;

class INPUTDEVICE_EXPORT InputManager
{
public:
    InputManager(WindowWIN32* wnd);

    HRESULT CreateDirectInputMain();
    void CloseDirectInputMain();

    void EnumAllInputDevices();
    HRESULT PollAllInputDevices();

    const bool IsThisKeyBeingPushedInSingle(UINT keyCode);
    const MOUSE_OFFSET GetMouseOffset();
    const bool IsMouseScrollingUp();
    const bool IsMouseScrollingDown();

    InputDeviceBase* GetKeyBoard();
    InputDeviceBase* GetMouse();
    InputDeviceBase* GetGamePadByOffset(int offset = 0);

private:
    HINSTANCE mhInstance;
    HWND mhWindow;

    LPDIRECTINPUT8 mpDirectInput;

    InputDeviceBase* mpKeyBoard;
    InputDeviceBase* mpMouse;
    InputDeviceBase* mpGamePads[MAX_INPUTDEVICE_NUM];
};

