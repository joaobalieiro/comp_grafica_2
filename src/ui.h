#pragma once

#include <windows.h>

// IDs dos botoes
enum ButtonID {
    ID_BTN_OBJ_CUBE      = 1001,
    ID_BTN_OBJ_PRISM     = 1002,
    ID_BTN_OBJ_CYLINDER  = 1003,
    ID_BTN_OBJ_SPHERE    = 1004,

    ID_BTN_SHADE_FLAT    = 1101,
    ID_BTN_SHADE_GOURAUD = 1102,
    ID_BTN_SHADE_PHONG   = 1103,

    ID_BTN_PROJ_PERSP    = 1201,
    ID_BTN_PROJ_ORTHO    = 1202
};

// Criacao dos controles de interface
void CreateUIControls(HWND hwnd, HINSTANCE hInst);
