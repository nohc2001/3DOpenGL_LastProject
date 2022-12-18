#pragma once
// Windows Header Files:
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glut32.lib")

#include <windows.h>

#include <gl/glew.h> //--- 필요한 헤더파일 include
#include <gl/GLU.h>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext.hpp>
#include <gl/glm/gtc/matrix_transform.hpp>

#include <string>
#include <fstream>

// C RunTime Header Files:
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <wchar.h>
#include <math.h>

#include <map>
#include <vector>
#include <random>
#include <string>
#include <TextStor.h>

#include <ft2build.h>
#include <freetype/freetype.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "SpaceMath.h"
#include "FreeMem.h"
#include "Music.h"

#define and2(A, B) (A && B)
#define and3(A, B, C) (A && B) && C
#define and4(A, B, C, D) (A && B) && (C && D)
#define and5(A, B, C, D, E) ((A && B) && (C && D)) && E

#define or2(A, B) (A || B)
#define or3(A, B, C) (A || B) || C
#define or4(A, B, C, D) (A || B) || (C || D)
#define or5(A, B, C, D, E) ((A || B) || (C || D)) || E

#define parametic3xyz(A) A.x, A.y, A.z

#define plus2(A, B) ++A; ++B
#define plus3(A, B, C) ++A; ++B; ++C;
#define plus4(A, B, C, D) ++A; ++B; ++C; ++D;
#define plus5(A, B, C, D, E) ++A; ++B; ++C; ++D; ++E;
#define plus6(A, B, C, D, E, F) ++A; ++B; ++C; ++D; ++E; ++F;
#define plus7(A, B, C, D, E, F, G) ++A; ++B; ++C; ++D; ++E; ++F ++G;
#define plus8(A, B, C, D, E, F, G, H) ++A; ++B; ++C; ++D; ++E; ++F; ++G; ++H;

#define VariableSet2(N, A, B) A = N; B = N;
#define VariableSet3(N, A, B, C) A = N; B = N; C = N;
#define VariableSet4(N, A, B, C, D) A = N; B = N; C = N; D = N;
#define VariableSet5(N, A, B, C, D, E) A = N; B = N; C = N; D = N; E = N;
#define VariableSet6(N, A, B, C, D, E, F) A = N; B = N; C = N; D = N; E = N; F = N;

FT_Library library;

//#include <freetype/freetype.h>
#include FT_FREETYPE_H

//#pragma comment(lib, "FreeTypeObj/Win32/Release/freetype.lib")

#define MAX_LOADSTRING 100

using namespace std;

HWND hWndMain;
HDC hdc;
HINSTANCE hInst;
HGLRC hrc;

void RunMessageLoop();
HRESULT Initialize();
static LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);

void DoDisplay();

RECT DisplayRt; // 디스플레이 좌표
RECT WindowRt; // 윈도우 전체 좌표
bool bAlias = false; // 안티 에일리어싱
bool bHint = false; // 고품질/저품질 안티 에일리어싱
int mx, my; // 마우스 좌표
char basicFont[128] = "C:\\Windows\\Fonts\\malgunsl.ttf";