// OSHw3.cpp : 定义应用程序的入口点。
//
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include "framework.h"
#include "OSHw3.h"

#define MAX_LOADSTRING 100

void InitializeRAM(HWND);
void ShowRAM(void);
void CreateEditRect(char *, int);
void GetEditInt(void);
#define BUTTONID		10000
#define FF_RAM	0
#define BF_RAM	1
#define WF_RAM   2
int  AllocateAlgorithm;
HWND hWind;

#define MAXRAMSIZE		256
#define MAXAREANO		15
#define MAXPROCESSNO	100

// 定义空闲分区表
struct {
	int Address;
	int Length;
} FreeAreaList[MAXAREANO];

// 定义进程内存分配表
struct {
	int Flag;
	int Address;
	int Length;
} ProcessList[MAXPROCESSNO];

void Allocate_FF_RAM(int);
void Allocate_BF_RAM(int);
void FreeRAM(int);

int FreeAreaMaxNo;								// 空闲分区最大数目
int ProcessMaxNo;								// 进程最大数目
HDC RAM_DC;


// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//---------------------------------------------------------------------------
//     动态分区内存分配与回收之初始化函数
//---------------------------------------------------------------------------
void InitializeRAM(HWND hWnd)
{
	FreeAreaMaxNo = 1;							// 空闲分区数目1个
	ProcessMaxNo = 0;							// 进程数目0个
	FreeAreaList[0].Address = 0;				// 第0空闲分区首址
	FreeAreaList[0].Length = MAXRAMSIZE;		// 第0空闲分区大小
	AllocateAlgorithm = FF_RAM;					// 内存分区分配算法置为首次适应算法

	hWind = hWnd;
	RAM_DC = GetDC(hWnd);
}
//---------------------------------------------------------------------------
//     显示空闲分区表及内存分配表
//---------------------------------------------------------------------------
RECT RAMRect = { 700, 100, 900, 100 + 2 * MAXRAMSIZE };
RECT AreaRect = { 400, 130, 460, 130 + 30 };
void ShowRAM()
{
	int  i;
	char ProcessString[32], FreeAreaString[3][32];
	HBRUSH ProcessBrush;
	RECT ProcessRect, faRect;

	if (AllocateAlgorithm == FF_RAM)
		SetWindowTextW(hWind, (LPCWSTR)"动态分区分配实验----采用首次适应算法分配内存");
	if (AllocateAlgorithm == BF_RAM)
		SetWindowTextW(hWind, (LPCWSTR)"动态分区分配实验----采用最佳适应算法分配内存");
	if (AllocateAlgorithm == WF_RAM)
		SetWindowTextW(hWind, (LPCWSTR)"动态分区分配实验----采用最坏适应算法分配内存");

	ProcessRect = RAMRect;
	TextOut(RAM_DC, ProcessRect.right + 4, ProcessRect.top - 8, "0K", 2);	// 写进程首址
	wsprintf(ProcessString, "%dK", MAXRAMSIZE);
	TextOut(RAM_DC, ProcessRect.right + 4, ProcessRect.top - 8 + 2 * MAXRAMSIZE,
		ProcessString, strlen(ProcessString));	// 写进程首址
	Rectangle(RAM_DC, RAMRect.left - 2, RAMRect.top - 2, RAMRect.right + 2, RAMRect.bottom + 2);
	for (i = 0; i < ProcessMaxNo; i++) {								// 画出内存动态分区图
		if (ProcessList[i].Flag != 0) {
			ProcessRect = RAMRect;
			ProcessRect.top = ProcessRect.top + 2 * ProcessList[i].Address;
			ProcessRect.bottom = ProcessRect.top + 2 * ProcessList[i].Length;
			Rectangle(RAM_DC, ProcessRect.left - 1, ProcessRect.top - 1,
				ProcessRect.right + 1, ProcessRect.bottom + 1);
			ProcessBrush = CreateSolidBrush(RGB(255, 255, 0));
			FillRect(RAM_DC, &ProcessRect, ProcessBrush);			// 画出进程所占内存区域
			DeleteObject(ProcessBrush);

			wsprintf(ProcessString, "%dK", ProcessList[i].Address);
			TextOut(RAM_DC, ProcessRect.right + 4, ProcessRect.top - 8,
				ProcessString, strlen(ProcessString));	// 写进程首址
			wsprintf(ProcessString, "%dK", ProcessList[i].Address + ProcessList[i].Length);
			TextOut(RAM_DC, ProcessRect.right + 4, ProcessRect.top - 8 + 2 * ProcessList[i].Length,
				ProcessString, strlen(ProcessString));

			wsprintf(ProcessString, "进程%d = %dK", i, ProcessList[i].Length);
			ProcessRect.top += ProcessList[i].Length - 8;
			DrawText(RAM_DC, ProcessString, strlen(ProcessString), &ProcessRect, DT_CENTER);																			// 写出进程号
		}
	}

	TextOut(RAM_DC, AreaRect.left + 60, AreaRect.top - 60,
		"空闲分区表　　　　　　　　　　　　　　　内存", 44);
	for (i = -1; i < FreeAreaMaxNo; i++) {									// 画出空闲分区表
		if (i == -1) {
			strcpy(FreeAreaString[0], "分区号");
			strcpy(FreeAreaString[1], "分区首址");
			strcpy(FreeAreaString[2], "分区大小");
		}
		else {
			wsprintf(FreeAreaString[0], "%d", i);
			wsprintf(FreeAreaString[1], "%d", FreeAreaList[i].Address);
			wsprintf(FreeAreaString[2], "%d", FreeAreaList[i].Length);
		}
		faRect = AreaRect;
		faRect.top += i * 29;
		faRect.bottom += i * 29;
		Rectangle(RAM_DC, faRect.left, faRect.top, faRect.right, faRect.bottom);
		TextOut(RAM_DC, faRect.left / 2 + faRect.right / 2 - strlen(FreeAreaString[0]) * 4,
			faRect.top + 6, FreeAreaString[0], strlen(FreeAreaString[0]));
		faRect.left += 59;
		faRect.right += 80;
		Rectangle(RAM_DC, faRect.left, faRect.top, faRect.right, faRect.bottom);
		TextOut(RAM_DC, faRect.left / 2 + faRect.right / 2 - strlen(FreeAreaString[1]) * 4,
			faRect.top + 6, FreeAreaString[1], strlen(FreeAreaString[1]));
		faRect.left += 80;
		faRect.right += 80;
		Rectangle(RAM_DC, faRect.left, faRect.top, faRect.right, faRect.bottom);
		TextOut(RAM_DC, faRect.left / 2 + faRect.right / 2 - strlen(FreeAreaString[2]) * 4,
			faRect.top + 6, FreeAreaString[2], strlen(FreeAreaString[2]));
	}
}
//---------------------------------------------------------------------------
//     内存分配函数
//---------------------------------------------------------------------------
void AllocateRAM(int AllocatRamSize)
{
	int i, j, faID, bfSize;
	int tail = 0;

	if (AllocateAlgorithm == FF_RAM) {		// 首次适应算法
		for (i = 0; i < FreeAreaMaxNo; i++) {
			if (FreeAreaList[i].Length >= AllocatRamSize) break;
		}
	}
	if (AllocateAlgorithm == BF_RAM) {		// 最佳适应算法
		faID = FreeAreaMaxNo;
		bfSize = MAXRAMSIZE;
		for (i = 0; i < FreeAreaMaxNo; i++) {
			if (FreeAreaList[i].Length >= AllocatRamSize) {
				if (bfSize >= FreeAreaList[i].Length - AllocatRamSize) {
					faID = i;
					bfSize = FreeAreaList[i].Length - AllocatRamSize;
				}
			}
		}
		i = faID;
	}
	if (AllocateAlgorithm == WF_RAM) {		// 最坏适应算法
		faID = FreeAreaMaxNo;
		bfSize = 0;
		for (i = 0; i < FreeAreaMaxNo; i++) {
			if (FreeAreaList[i].Length >= AllocatRamSize) {
				if (bfSize <= FreeAreaList[i].Length - AllocatRamSize) {
					faID = i;
					bfSize = FreeAreaList[i].Length - AllocatRamSize;
				}
			}
		}
		i = faID;
	}

	if (i >= FreeAreaMaxNo) {
		MessageBox(NULL, "进程需要内存太多，没有足够内存可供分配！", NULL, MB_OK);
		return;
	}

	ProcessList[ProcessMaxNo].Flag = 1;
	ProcessList[ProcessMaxNo].Address = FreeAreaList[i].Address; // 进程内存首址
	ProcessList[ProcessMaxNo].Length = AllocatRamSize;
	ProcessMaxNo++;

	FreeAreaList[i].Address += AllocatRamSize;					// 空闲分区首址
	FreeAreaList[i].Length -= AllocatRamSize;					// 空闲分区大小
	if (FreeAreaList[i].Length == 0) {							// 如果空闲分区大小为0，撤消
		for (j = i; j < FreeAreaMaxNo - 1; j++) {
			FreeAreaList[j].Address = FreeAreaList[j + 1].Address;
			FreeAreaList[j].Length = FreeAreaList[j + 1].Length;
		}
		FreeAreaMaxNo--;
	}

	InvalidateRect(hWind, NULL, TRUE);
	UpdateWindow(hWind);
	ShowRAM();
}
//---------------------------------------------------------------------------
//     内存回收函数
//---------------------------------------------------------------------------
void FreeRAM(int ProcessID)
{
	int  i, j;
	char FreeString[32];
	int  Address, Length;

	if (ProcessID > MAXPROCESSNO) {
		MessageBox(NULL, "进程号太大，没有这个进程！", NULL, MB_OK);
		return;
	}
	if (ProcessList[ProcessID].Flag == 0) {
		wsprintf(FreeString, "进程%d不存在！", ProcessID);
		MessageBox(NULL, FreeString, NULL, MB_OK);
		return;
	}
	ProcessList[ProcessID].Flag = 0;
	Address = ProcessList[ProcessID].Address;
	Length = ProcessList[ProcessID].Length;

	for (i = 0; i < FreeAreaMaxNo; i++) {
		if (FreeAreaList[i].Address + FreeAreaList[i].Length == Address) {// 与上边分区相邻，合并
			FreeAreaList[i].Length += Length;
			if (FreeAreaList[i].Address + FreeAreaList[i].Length != FreeAreaList[i + 1].Address)
				goto ShowRAMLoop;
			FreeAreaList[i].Length += FreeAreaList[i + 1].Length;		// 与上、下边分区皆相邻，合并
			for (j = i + 1; j < FreeAreaMaxNo - 1; j++) {					// 撤消下边分区项
				FreeAreaList[j].Address = FreeAreaList[j + 1].Address;
				FreeAreaList[j].Length = FreeAreaList[j + 1].Length;
			}
			FreeAreaMaxNo--;
			goto ShowRAMLoop;
		}
		if (Address + Length == FreeAreaList[i].Address) {			// 与下边分区相邻，合并
			FreeAreaList[i].Address = Address;
			FreeAreaList[i].Length += Length;
			goto ShowRAMLoop;
		}
	}
	for (i = FreeAreaMaxNo; i > 0; i--) {							// 上、下都没有空闲分区相邻
		if (FreeAreaList[i - 1].Address < Address) break;
		FreeAreaList[i].Address = FreeAreaList[i - 1].Address;
		FreeAreaList[i].Length = FreeAreaList[i - 1].Length;
	}
	FreeAreaList[i].Address = Address;
	FreeAreaList[i].Length = Length;
	FreeAreaMaxNo++;

ShowRAMLoop:
	InvalidateRect(hWind, NULL, TRUE);
	UpdateWindow(hWind);
	ShowRAM();
}
//---------------------------------------------------------------------------
//     数据输入函数
//---------------------------------------------------------------------------
HWND hEdit, hButton;
RECT hEditRect = { 170, 100, 300, 130 };
int  AllocFreeID;
void CreateEditRect(char *EditStatic, int afID)
{
	AllocFreeID = afID;
	TextOut(RAM_DC, hEditRect.left - 120, hEditRect.top + 4, EditStatic, strlen(EditStatic));
	hEdit = CreateWindow("EDIT", NULL, WS_CHILD | WS_VISIBLE | ES_LEFT | WS_THICKFRAME,
		hEditRect.left, hEditRect.top, hEditRect.right - hEditRect.left,
		hEditRect.bottom - hEditRect.top, hWind, NULL, hInst, NULL);  		// 生成编辑框
	hButton = CreateWindow("BUTTON", "确定", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		hEditRect.left - 50, hEditRect.top + 60, hEditRect.right - hEditRect.left - 40,
		hEditRect.bottom - hEditRect.top, hWind, (HMENU)BUTTONID, hInst, NULL); //生成确定键
}
//---------------------------------------------------------------------------
//     获取数据函数
//---------------------------------------------------------------------------
void GetEditInt()
{
	char hEditString[16];
	int hEditInt;

	GetWindowText(hEdit, hEditString, 15);
	hEditInt = atoi(hEditString);
	DestroyWindow(hEdit);
	DestroyWindow(hButton);
	if (AllocFreeID == IDM_RAMALLOCATE) AllocateRAM(hEditInt);
	if (AllocFreeID == IDM_RAMFREE)     FreeRAM(hEditInt);
}



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_OSHW3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_OSHW3));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_OSHW3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_OSHW3);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		InitializeRAM(hWnd);
		break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
			int wmEvent = HIWORD(wParam);
            // 分析菜单选择:
            switch (wmId)
            {
			case IDM_RAMALLOCATE:
				CreateEditRect("请输入进程长度：", IDM_RAMALLOCATE);
				break;

			case IDM_RAMFREE:
				CreateEditRect("请输入撤消进程：", IDM_RAMFREE);
				break;

			case IDM_ALLOCATE_FF_RAM:
				AllocateAlgorithm = FF_RAM;
				ShowRAM();
				break;
			case IDM_ALLOCATE_BF_RAM:
				AllocateAlgorithm = BF_RAM;
				ShowRAM();
				break;
			case IDM_ALLOCATE_WF_RAM:
				AllocateAlgorithm = WF_RAM;
				ShowRAM();
				break;

			case BUTTONID:
				GetEditInt();
				break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
