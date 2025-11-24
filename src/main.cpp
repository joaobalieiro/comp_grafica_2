#include <windows.h>
#include <algorithm>

#include "render.h"
#include "ui.h"


// =============================
// WndProc
// =============================

// Funcao principal de tratamento de mensagens da janela.
// Aqui a gente conecta a UI (botoes, teclado, redimensionamento) com
// o estado global (objeto, shading, projecao) e com o renderScene().
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Janela foi criada: pega tamanho inicial da area de cliente
            // e configura os buffers (framebuffer, zbuffer, DIB).
            RECT rc;
            GetClientRect(hwnd, &rc);
            ResizeBuffers(rc.right - rc.left, rc.bottom - rc.top);

            // Cria os botoes da interface (objetos, shading, projecao).
            HINSTANCE hInst = ((LPCREATESTRUCT)lParam)->hInstance;
            CreateUIControls(hwnd, hInst);
            return 0;
        }

        case WM_SIZE: {
            // Sempre que a janela muda de tamanho, atualiza os buffers
            // para a nova largura/altura e pede um redraw.
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            ResizeBuffers(w, h);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_COMMAND: {
            // Tratamento de cliques nos botoes (via IDs).
            int id = LOWORD(wParam);
            switch (id) {
                // Selecao de objeto 3D
                case ID_BTN_OBJ_CUBE:     currentObject = ObjectType::CUBE;           break;
                case ID_BTN_OBJ_PRISM:    currentObject = ObjectType::TRIANGLE_PRISM; break;
                case ID_BTN_OBJ_CYLINDER: currentObject = ObjectType::CYLINDER;       break;
                case ID_BTN_OBJ_SPHERE:   currentObject = ObjectType::SPHERE;         break;

                    // Selecao de modo de iluminacao
                case ID_BTN_SHADE_FLAT:    shadingMode = ShadingMode::FLAT;    break;
                case ID_BTN_SHADE_GOURAUD: shadingMode = ShadingMode::GOURAUD; break;
                case ID_BTN_SHADE_PHONG:   shadingMode = ShadingMode::PHONG;   break;

                    // Selecao de tipo de projecao
                case ID_BTN_PROJ_PERSP:
                    projectionMode = ProjectionMode::PERSPECTIVE;
                    break;
                case ID_BTN_PROJ_ORTHO:
                    projectionMode = ProjectionMode::ORTHOGRAPHIC;
                    break;
                default: break;
            }
            // Devolve foco para a janela principal apos clicar em qualquer botao
            // (importante para o teclado continuar funcionando).
            SetFocus(hwnd);
            // Pede para redesenhar a cena com o novo estado.
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_KEYDOWN: {
            WPARAM vk = wParam;
            if (vk == VK_ESCAPE) {
                PostQuitMessage(0);
                return 0;
            } switch (vk) {
                // 1–4: troca de objeto 3D
                case '1': currentObject = ObjectType::CUBE;           break;
                case '2': currentObject = ObjectType::TRIANGLE_PRISM; break;
                case '3': currentObject = ObjectType::CYLINDER;       break;
                case '4': currentObject = ObjectType::SPHERE;         break;

                // F/G/H: troca de modo de shading
                case 'F': shadingMode = ShadingMode::FLAT;    break;
                case 'G': shadingMode = ShadingMode::GOURAUD; break;
                case 'H': shadingMode = ShadingMode::PHONG;   break;

                // P: alterna entre projecao perspectiva e ortografica
                case 'P':
                    projectionMode = (projectionMode == ProjectionMode::PERSPECTIVE)
                                     ? ProjectionMode::ORTHOGRAPHIC
                                     : ProjectionMode::PERSPECTIVE;
                    break;

                // +/-: controla zoom (aproxima/afasta o objeto)
                case VK_OEM_PLUS:
                case VK_ADD:
                    zoom = std::max(2.0f, zoom - 0.5f);
                    break;
                case VK_OEM_MINUS:
                case VK_SUBTRACT:
                    zoom = std::min(15.0f, zoom + 0.5f);
                    break;

                // WASD: rotacao da cena em X e Y
                case 'W': angleX += 10.0f; break;
                case 'S': angleX -= 10.0f; break;
                case 'A': angleY += 10.0f; break;
                case 'D': angleY -= 10.0f; break;


                // === CONTROLES DE LUZ E MATERIAL ===
                // Setas: Move lightPos no plano XZ
                case VK_LEFT:   lightPos.x -= 0.5f; break;
                case VK_RIGHT:  lightPos.x += 0.5f; break;
                case VK_UP:     lightPos.z -= 0.5f; break;
                case VK_DOWN:   lightPos.z += 0.5f; break;
                
                // PgUp/PgDn: Move lightPos no eixo Y
                case VK_PRIOR:  lightPos.y += 0.5f; break; 
                case VK_NEXT:   lightPos.y -= 0.5f; break;
                
                default: break;
                
            }
            // Depois de mudar qualquer parametro, pede novo redraw.
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }

        case WM_PAINT: {
            // Evento de pintura: redesenha a cena inteira.
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Calcula o frame atual (pipeline completo CPU).
            renderScene();
            // Converte framebuffer para DIB e manda para a tela.
            Present(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            // Quando a janela e destruida, encerra o loop de mensagens.
            PostQuitMessage(0);
            return 0;
    }
    // Tratamento padrao para mensagens que nao foram tratadas acima.
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Loop Principal
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    // Nome da classe de janela que vamos registrar
    const wchar_t CLASS_NAME[] = L"CGScanline3DWindowClass";

    // Preenche estrutura da classe de janela
    WNDCLASSW wc{};
    wc.style         = CS_HREDRAW | CS_VREDRAW;      // redesenha em resize horizontal/vertical
    wc.lpfnWndProc   = WndProc;                      // funcao que trata as mensagens (callback)
    wc.hInstance     = hInstance;                    // instancia da aplicacao
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);     // cor de fundo padrao
    wc.lpszClassName = CLASS_NAME;                   // nome da classe

    // Registra a classe de janela no sistema
    if (!RegisterClassW(&wc)) {
        MessageBoxW(nullptr, L"Erro ao registrar classe de janela.", L"Erro", MB_ICONERROR);
        return 0;
    }

    // Calcula o tamanho externo da janela a partir do tamanho interno desejado 
    RECT rc = {0,0,WIDTH,HEIGHT};
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;

    // Cria a janela principal
    HWND hwnd = CreateWindowExW(
            0,                                              // estilos extras
            CLASS_NAME,                                     // classe registrada acima
            L"Trabalho CG - Scan-line 3D (CPU + Win32)",    // titulo da janela
            WS_OVERLAPPEDWINDOW,                            // estilo de janela padrao (com barra de titulo, etc.)
            CW_USEDEFAULT, CW_USEDEFAULT,                   // posicao inicial
            winW, winH,                                     // tamanho externo calculado
            nullptr, nullptr, hInstance, nullptr
    );

    // Se falhar, mostra mensagem de erro
    if (!hwnd) {
        MessageBoxW(nullptr, L"Erro ao criar janela.", L"Erro", MB_ICONERROR);
        return 0;
    }

    // Exibe a janela e dispara WM_PAINT inicial
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // Loop principal de mensagens do Windows:
    // enquanto houver mensagens (teclado, mouse, paint, etc.)
    // elas sao tratadas e repassadas para o WndProc.
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    // Quando GetMessage retorna 0 (WM_QUIT), encerramos a aplicacao.
    return 0;
}
