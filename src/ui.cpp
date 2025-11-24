#include "ui.h"

void CreateUIControls(HWND hwnd, HINSTANCE hInst) {
    int y = 10;
    // Objetos
    CreateWindowW(L"BUTTON", L"Cubo",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  10, y, 60, 24,
                  hwnd, (HMENU)ID_BTN_OBJ_CUBE, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"Prisma",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  80, y, 60, 24,
                  hwnd, (HMENU)ID_BTN_OBJ_PRISM, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"Cilindro",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  150, y, 70, 24,
                  hwnd, (HMENU)ID_BTN_OBJ_CYLINDER, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"Esfera",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  230, y, 60, 24,
                  hwnd, (HMENU)ID_BTN_OBJ_SPHERE, hInst, nullptr);

    // Iluminação
    y += 30;
    CreateWindowW(L"BUTTON", L"Flat",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  10, y, 60, 24,
                  hwnd, (HMENU)ID_BTN_SHADE_FLAT, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"Gouraud",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  80, y, 70, 24,
                  hwnd, (HMENU)ID_BTN_SHADE_GOURAUD, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"Phong",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  160, y, 60, 24,
                  hwnd, (HMENU)ID_BTN_SHADE_PHONG, hInst, nullptr);

    // Projeção
    y += 30;
    CreateWindowW(L"BUTTON", L"Perspectiva",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  10, y, 100, 24,
                  hwnd, (HMENU)ID_BTN_PROJ_PERSP, hInst, nullptr);
    CreateWindowW(L"BUTTON", L"Ortografica",
                  WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                  120, y, 100, 24,
                  hwnd, (HMENU)ID_BTN_PROJ_ORTHO, hInst, nullptr);
}
