#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <windowsx.h>

#define WM_INIT WM_APP
#define MAX_NUM (7*3)
#define MARGIN 50

TCHAR szClassName[] = TEXT("Window");
INT DefaultSite[MAX_NUM] = { 0, 0, 1, 1, 1, 0, 0,
0, 1, 1, 1, 1, 1, 0,
1, 1, 1, 1, 1, 1, 1 };

// 棒がすべて消されているかどうか
BOOL IsGameOver(const int *list)
{
	for (int i = 0; i < MAX_NUM; i++)
	{
		if (list[i])
		{
			return FALSE;
		}
	}
	return TRUE;
}

// 後手必勝かどうか
BOOL IsCertainVictory(const int *list)
{
	int i, total;
	INT nList[MAX_NUM];
	int bitflag;
	int index;
	ZeroMemory(nList, (sizeof INT)*MAX_NUM);
	bitflag = 0;
	index = -1;
	for (i = 0; i < MAX_NUM; i++)
	{
		if (bitflag == 0 && list[i] == 1)
		{
			bitflag = 1;
			index++;
			nList[index]++;
			continue;
		}
		if (bitflag == 1 && list[i] == 1)
		{
			nList[index]++;
			continue;
		}
		if (bitflag == 1 && (list[i] == 0 || (i + 1) % 7 == 0))
		{
			bitflag = 0;
			continue;
		}
	}
	for (i = 0; i < index + 1; i++)
	{
		if (nList[i] != 1)break;
	}
	if (i == index + 1)
	{
		// 隣接する棒が無いときは奇数かどうか
		if (i % 2)
		{
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		// 隣接する棒を組として各組の棒数の排他的論理和がゼロになるか
		total = 0;
		for (i = 0; i < index + 1; i++)
		{
			total ^= nList[i];
		}
		return (total == 0);
	}
}

// 渡された2点の座標で棒をとることができるかどうか
BOOL IsValid(int x1, int y1, int x2, int y2, const int*list)
{
	// 斜めに引いてないかチェック
	if (y1 != y2)
	{
		return FALSE;
	}
	// 既に消されている棒をまたがっていないかチェック
	int count = 0;
	for (int i = x1; i < x2; i++)
	{
		if (count == 0 && list[y1 * 7 + i] == 1)
		{
			count = 1;
		}
		else if (count == 1 && list[y1 * 7 + i] == 0)
		{
			count = 2;
		}
		else if (count == 2 && list[y1 * 7 + i] == 1)
		{
			return FALSE;
		}
	}
	// 少なくとも1つ以上消しているか
	if (count == 0)
	{
		return FALSE;
	}
	return TRUE;
}

// ウィンドウプロシージャ
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static INT nSite[MAX_NUM];
	static INT nTemp[MAX_NUM];
	static HPEN hPen1;
	static HPEN hPen2; // ユーザーのペン
	static HPEN hPen3; // コンピュータのペン
	static BOOL bDrag;
	static POINT point1;
	static POINT point2;
	static BOOL bUserTurn;
	static INT nAnimationStep;
	switch (msg)
	{
	case WM_CREATE:
		hPen1 = CreatePen(PS_SOLID, 16, RGB(0, 0, 0));
		hPen2 = CreatePen(PS_SOLID, 8, RGB(255, 0, 0));
		hPen3 = CreatePen(PS_SOLID, 8, RGB(0, 255, 0));
		PostMessage(hWnd, WM_INIT, 0, 0);
		break;
	case WM_INIT:
	{
		CopyMemory(nSite, DefaultSite, sizeof nSite);
		InvalidateRect(hWnd, 0, 1);
		SetWindowText(hWnd, TEXT("七五三 [あなたの番です]"));
		bUserTurn = TRUE;
	}
	break;
	case WM_LBUTTONDOWN:
		if (bUserTurn && !bDrag)
		{
			bDrag = TRUE;
			point1.x = GET_X_LPARAM(lParam);
			point1.y = GET_Y_LPARAM(lParam);
			SetCapture(hWnd);
		}
		break;
	case WM_MOUSEMOVE:
		if (bUserTurn && bDrag)
		{
			InvalidateRect(hWnd, 0, 1);
			UpdateWindow(hWnd);
			HDC hdc = GetDC(hWnd);
			HPEN hOldPen = (HPEN)SelectObject(hdc, hPen2);
			MoveToEx(hdc, point1.x, point1.y, 0);
			LineTo(hdc, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			SelectObject(hdc, hOldPen);
			ReleaseDC(hWnd, hdc);
		}
		break;
	case WM_LBUTTONUP:
		if (bUserTurn && bDrag)
		{
			ReleaseCapture();
			bDrag = FALSE;
			RECT rect;
			GetClientRect(hWnd, &rect);
			point2.x = GET_X_LPARAM(lParam);
			point2.y = GET_Y_LPARAM(lParam);
			const int x1 = min(point1.x, point2.x) / (rect.right / 8);
			const int x2 = max(point1.x, point2.x) / (rect.right / 8);
			const int y1 = point1.y / (rect.bottom / 3);
			const int y2 = point2.y / (rect.bottom / 3);
			if (IsValid(x1, y1, x2, y2, nSite))
			{
				for (int i = x1; i < x2; i++)
				{
					nSite[y1 * 7 + i] = 0;
				}
				if (IsGameOver(nSite))
				{
					MessageBox(hWnd, TEXT("GameOver!\n\nコンピュータの勝ちです。"), TEXT("確認"), 0);
					PostMessage(hWnd, WM_INIT, 0, 0);
					return 0;
				}
				SetWindowText(hWnd, TEXT("七五三 [コンピュータが考えています]"));
				bUserTurn = FALSE;
				SetTimer(hWnd, 0x1234, 500, 0);
			}
			InvalidateRect(hWnd, 0, 1);
		}
		break;
	case WM_TIMER:
		if (wParam == 0x1234)
		{
			KillTimer(hWnd, 0x1234);
			const BOOL bIsCertainVictory = IsCertainVictory(nSite);
			srand(GetTickCount());
			int t1, t2;
			int x1, x2;
			int y1;
			for (;;)
			{
				CopyMemory(nTemp, nSite, sizeof nTemp);
				do
				{
					do
					{
						t1 = rand() % 8;
						t2 = rand() % 8;
					} while (t1 == t2);
					x1 = min(t1, t2);
					x2 = max(t1, t2);
					y1 = rand() % 3;
				} while (!IsValid(x1, y1, x2, y1, nTemp));
				for (; !nTemp[y1 * 7 + x1]; x1++);
				for (; !nTemp[y1 * 7 + x2 - 1]; x2--);
				for (int i = x1; i < x2; i++) nTemp[y1 * 7 + i] = 0;
				if (bIsCertainVictory || IsCertainVictory(nTemp)) break;
			}
			RECT rect;
			GetClientRect(hWnd, &rect);
			point1.x = x1 * (rect.right / 8) + (rect.right / 8) / 2;
			point1.y = y1 * (rect.bottom - MARGIN * 4) / 3 + (rect.bottom - MARGIN * 4) / 6 + (y1 + 1) * MARGIN;
			point2.x = x2 * (rect.right / 8) + (rect.right / 8) / 2;
			point2.y = point1.y;
			nAnimationStep = 0;
			SetTimer(hWnd, 0x5678, 200, 0);
		}
		else if (wParam == 0x5678)
		{
			nAnimationStep++;
			if (nAnimationStep > 5)
			{
				KillTimer(hWnd, 0x5678);
				CopyMemory(nSite, nTemp, sizeof nSite);
				if (IsGameOver(nSite))
				{
					MessageBox(hWnd, TEXT("GameOver!\n\nあなたの勝ちです。"), TEXT("確認"), 0);
					PostMessage(hWnd, WM_INIT, 0, 0);
					return 0;
				}
				InvalidateRect(hWnd, 0, 1);
				SetWindowText(hWnd, TEXT("七五三 [あなたの番です]"));
				bUserTurn = TRUE;
			}
			else
			{
				HDC hdc = GetDC(hWnd);
				HPEN hOldPen = (HPEN)SelectObject(hdc, bUserTurn? hPen2:hPen3);
				MoveToEx(hdc, point1.x, point1.y, 0);
				LineTo(hdc, point2.x, point2.y);
				SelectObject(hdc, hOldPen);
				ReleaseDC(hWnd, hdc);
			}
		}
		break;
	case WM_ERASEBKGND:
	{
		RECT rect;
		GetClientRect(hWnd, &rect);
		PatBlt((HDC)wParam, 0, 0, rect.right, rect.bottom, WHITENESS);
		HPEN hOldPen = (HPEN)SelectObject((HDC)wParam, hPen1);
		for (int i = 0; i < MAX_NUM; i++)
		{
			if (nSite[i])
			{
				MoveToEx((HDC)wParam, rect.right / 8 * (i % 7 + 1), (i / 7)*(rect.bottom - MARGIN * 4) / 3 + MARGIN * (i / 7 + 1), 0);
				LineTo((HDC)wParam, rect.right / 8 * (i % 7 + 1), (i / 7 + 1)*(rect.bottom - MARGIN * 4) / 3 + MARGIN * (i / 7 + 1));
			}
		}
		SelectObject((HDC)wParam, hOldPen);
	}
	return 1;
	case WM_DESTROY:
		DeleteObject(hPen1);
		DeleteObject(hPen2);
		DeleteObject(hPen3);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		0,
		LoadCursor(0, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW + 1),
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("七五三"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
		);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
