#include <Windows.h>
#include "WindowWIN32.h"
#include "tempD3d.h"
//---------------------------------------------
#include "InputManager.h"
#include "InputDeviceDirectInput.h"

//-----------------------------------------------
#include <stdio.h>
void tempShowMousePos(LONG x, LONG y, LONG z)
{
    char str[64] = "";
    sprintf_s(str, sizeof(str), "MOUSE:: X->%ld Y->%ld Z->%ld\n",
        x, y, z);
    OutputDebugString(str);
}
//-----------------------------------------------

int WINAPI WinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPSTR szCmdLine,
    _In_ int iCmdShow
)
{
    WindowWIN32* w = new WindowWIN32();
    w->CreateMyWindow("a test window",
        hInstance, iCmdShow, false);

    //---------------------------------------------
    InputManager im(w);
    im.CreateDirectInputMain();
    im.EnumAllInputDevices();

    if (FAILED(TEMP::InitD3D11Device(w->GetWndHandle())))
    {
        return -1;
    }
#ifdef SHOW_CUBE
    if (FAILED(TEMP::PrepareCube(w->GetWndHandle())))
    {
        return -2;
    }
#endif // SHOW_CUBE

    MSG msg = { 0 };
    while (WM_QUIT != msg.message)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            im.PollAllInputDevices();
            TEMP::Render();
            //-------------------------
            if (im.IsThisKeyBeingPushedInSingle(KB_ESCAPE))
            {
                PostQuitMessage(0);
            }

            LONG scrollOffset = 0;
            if (im.IsMouseScrollingUp())
            {
                scrollOffset = -10;
            }
            if (im.IsMouseScrollingDown())
            {
                scrollOffset = 10;
            }
            TEMP::setLightOffsetX(im.GetMouseOffset().x);
            TEMP::setLightOffsetY(im.GetMouseOffset().y +
                scrollOffset);
        }
    }

    TEMP::CleanupDevice();
    //--------------------------------------
    im.CloseDirectInputMain();
    //--------------------------------------
    delete w;

    return (int)msg.wParam;
}