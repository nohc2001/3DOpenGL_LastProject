#pragma once

#include "SpaceMath.h"
#include "GLdrw.h"

namespace gui {
	enum class UIKind {
		NONE = 0,
		Button = 1,
		EditBox = 2,
		Tree = 3,
		TableW = 4,
		TableH = 5,
		TableWH = 6,
		Label = 7,
		Layout = 8,
		Slider = 9
	};

	class InputData {
	public:
		shp::vec2f mouse;
	};

	class OpenGLUI {
	public:
		static vector<OpenGLUI*> UIArray;
		static int focusID;
		static InputData input;
		static int UpdateID;

		UIKind kind;
		string name;
		int id;
		bool active; // 활성화 / 비활성화
		shp::rect4f location;

		OpenGLUI() { kind = UIKind::NONE; name = "none"; id = UpdateID + 1; active = false; UpdateID++; }
		virtual ~OpenGLUI() {}
		virtual void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam) {}
		virtual void Render() {}
		virtual void Release() {}

		void SetName(string str) {
			name = str;
		}
		void Active(bool b) { active = b; }

		static void AllEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		static void AllRelease();
		static void AllRender();
	};

	class OpenGLButton : OpenGLUI {
	public:
		wchar_t* wstr;

		void (*OnClick)(OpenGLButton*) = nullptr; // 클릭했을때 작동되는 함수
		void (*RenderFunc)(OpenGLButton*) = nullptr; // 그릴때 작동되는 함수

		OpenGLButton() { location = shp::rect4f(0, 0, 0, 0); wstr = nullptr; kind = UIKind::Button; }
		OpenGLButton(shp::rect4f loc, const wchar_t* text, void (*clickfunc)(OpenGLButton*), void (*renderfunc)(OpenGLButton*)) {
			location = loc;
			wstr = new wchar_t[wcslen(text) + 1];
			int i;
			for (i = 0; i < wcslen(text) + 1; i++) {
				wstr[i] = text[i];
			}
			OnClick = clickfunc; RenderFunc = renderfunc;
			active = true; kind = UIKind::Button;
		}
		virtual ~OpenGLButton() {}

		void AddFuncOnClick(void (*func)(OpenGLButton*)) { OnClick = func; }
		void AddFuncOnRender(void (*func)(OpenGLButton*)) { RenderFunc = func; }
		void SetLocation(shp::rect4f rt) { location = rt; }
		void SetText(const wchar_t* text) { wcscpy_s(wstr, wcslen(text), text); }

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLEditBox : OpenGLUI {
	public:
		int cursor = 0;
		wstring wstr;

		const char* font;
		int fontsiz;
		drw::TextSortStruct tss;

		void (*OnAdd)(OpenGLEditBox*, wchar_t) = nullptr; // 글자를 추가하려 했을때 작동되는 함수 - 제한할 수 있음.
		void (*RenderFunc)(OpenGLEditBox*) = nullptr; // 그릴때 작동되는 함수

		OpenGLEditBox() { location = shp::rect4f(0, 0, 0, 0); kind = UIKind::EditBox; }
		OpenGLEditBox(shp::rect4f loc, const wchar_t* text, void (*renderfunc)(OpenGLEditBox*), const char* Font, int Fontsiz, drw::TextSortStruct Tss) {
			location = loc;
			int i;
			for (i = 0; i < wcslen(text) + 1; i++) {
				wstr.push_back(text[i]);
			}
			RenderFunc = renderfunc;
			active = true; kind = UIKind::EditBox;

			font = Font;
			fontsiz = Fontsiz;
			tss = Tss;
		}
		virtual ~OpenGLEditBox() {}

		void AddFuncOnAdd(void (*func)(OpenGLEditBox*, wchar_t)) { OnAdd = func; }
		void AddFuncOnRender(void (*func)(OpenGLEditBox*)) { RenderFunc = func; }
		void SetLocation(shp::rect4f rt) { location = rt; }
		void SetText(const wchar_t* text) {
			wstr.clear();
			for (int i = 0; i < wcslen(text) + 1; i++) {
				wstr.push_back(text[i]);
			}
		}

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLSlider : OpenGLUI {
	public:
		float value; // 0~1까지의 값을 가짐.
		bool onDown = false;

		void (*RenderFunc)(OpenGLSlider*) = nullptr; // 그릴때 작동되는 함수

		OpenGLSlider() { location = shp::rect4f(0, 0, 0, 0); kind = UIKind::Slider; }
		OpenGLSlider(shp::rect4f loc) { location = loc; active = true; kind = UIKind::Slider; }
		virtual ~OpenGLSlider() {}

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLTree : OpenGLUI {
	public:
		shp::rect4f contentsLoc; // 맨위 항목의 width, 한 항목의 height
		wchar_t* wstr;
		vector<OpenGLUI*> contents;
		bool HideContents;

		void (*OnClick)(OpenGLTree*) = nullptr; // 클릭했을때 작동되는 함수
		void (*RenderFunc)(OpenGLTree*) = nullptr; // 그릴때 작동되는 함수

		OpenGLTree() { location = shp::rect4f(0, 0, 0, 0); wstr = nullptr; kind = UIKind::Button; HideContents = false; }
		OpenGLTree(shp::rect4f loc, int cloch, const wchar_t* text, void (*clickfunc)(OpenGLTree*), void (*renderfunc)(OpenGLTree*)) {
			//여기에서 cloch은 단순히 한항목의 세로 크기만 결정한다. 고로 h만 본다.
			location = loc;
			contentsLoc = shp::rect4f(loc.fx, loc.fy, loc.lx, loc.fy + cloch);
			wstr = new wchar_t[wcslen(text) + 1];
			int i;
			for (i = 0; i < wcslen(text) + 1; i++) {
				wstr[i] = text[i];
			}
			OnClick = clickfunc; RenderFunc = renderfunc;
			active = true; kind = UIKind::Tree;
			HideContents = false;
		}
		virtual ~OpenGLTree() {}

		void AddFuncOnClick(void (*func)(OpenGLTree*)) { OnClick = func; }
		void AddFuncOnRender(void (*func)(OpenGLTree*)) { RenderFunc = func; }
		void SetLocation(shp::rect4f rt) { location = rt; }
		void SetText(const wchar_t* text) { wcscpy_s(wstr, wcslen(text), text); }

		void AddTreeContent(const wchar_t* text);
		void AddUIContent(OpenGLUI* ui);
		OpenGLUI* GetContentWithNum(int n);
		OpenGLUI* GetContentWithName(string name);

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLTableW : OpenGLUI {
	public:
		vector<OpenGLUI*> contents;
		vector<float> cx;

		void (*OnClick)(OpenGLTableW*) = nullptr; // 클릭했을때 작동되는 함수
		void (*RenderFunc)(OpenGLTableW*) = nullptr; // 그릴때 작동되는 함수

		OpenGLTableW() { location = shp::rect4f(0, 0, 0, 0); kind = UIKind::TableW; }
		OpenGLTableW(shp::rect4f loc) { location = loc; kind = UIKind::TableW; active = true; }
		virtual ~OpenGLTableW() {}

		void AddFuncOnClick(void (*func)(OpenGLTableW*)) { OnClick = func; }
		void AddFuncOnRender(void (*func)(OpenGLTableW*)) { RenderFunc = func; }

		void AddContent(OpenGLUI* ui, float x);

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLTableH : OpenGLUI {
	public:
		vector<OpenGLUI*> contents;
		vector<float> cy;

		void (*OnClick)(OpenGLTableH*) = nullptr; // 클릭했을때 작동되는 함수
		void (*RenderFunc)(OpenGLTableH*) = nullptr; // 그릴때 작동되는 함수

		OpenGLTableH() { location = shp::rect4f(0, 0, 0, 0); kind = UIKind::TableH; }
		OpenGLTableH(shp::rect4f loc) { location = loc; kind = UIKind::TableH; active = true; }
		virtual ~OpenGLTableH() {}

		void AddFuncOnClick(void (*func)(OpenGLTableH*)) { OnClick = func; }
		void AddFuncOnRender(void (*func)(OpenGLTableH*)) { RenderFunc = func; }

		void AddContent(OpenGLUI* ui, float y);

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLLabel : OpenGLUI {
	public:
		wstring Text;
		wstring popupText;

		const char* font;
		int fontsiz;
		drw::TextSortStruct tss;

		void (*RenderFunc)(OpenGLLabel*) = nullptr; // 그릴때 작동되는 함수

		OpenGLLabel() { location = shp::rect4f(0, 0, 0, 0); kind = UIKind::Label; }
		OpenGLLabel(shp::rect4f loc, const wchar_t* text, const char* fontfile, int fontSiz, drw::TextSortStruct Tss = drw::TextSortStruct(shp::vec2f(-1, -1), false, false)) {
			location = loc; kind = UIKind::Label;
			for (int i = 0; i < wcslen(text) + 1; i++) {
				Text.push_back(text[i]);
			}
			font = fontfile;
			fontsiz = fontSiz;
			tss = Tss;
			active = true;
		}
		virtual ~OpenGLLabel() {}

		void SetPopUpText(const wchar_t* popUpText) {
			for (int i = 0; i < wcslen(popUpText) + 1; i++) {
				popupText.push_back(popUpText[i]);
			}
		}
		void AddFuncOnRender(void (*func)(OpenGLLabel*)) { RenderFunc = func; }

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};

	class OpenGLLayout : OpenGLUI {
	public:
		vector<OpenGLUI*> contents;

		void (*EventFunc)(OpenGLLayout*, HWND, UINT, WPARAM, LPARAM) = nullptr;
		void (*RenderFunc)(OpenGLLayout*) = nullptr; // 그릴때 작동되는 함수

		OpenGLLayout() { location = shp::rect4f(0, 0, 0, 0); kind = UIKind::Layout; }
		OpenGLLayout(shp::rect4f loc) { location = loc; kind = UIKind::Layout; active = true; }
		virtual ~OpenGLLayout() {}

		void OnEvent(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
		void Render();
		void Release();

		void AddContent(OpenGLUI* ui);

		OpenGLUI GetOrigin() { return *((OpenGLUI*)this); }
	};
};