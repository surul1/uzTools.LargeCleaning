// FindALLbigFileToListProj.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "FindALLbigFileToListProj.h"
#include <algorithm> // For std::sort
#include <Shlwapi.h> // For PathRemoveFileSpecW

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

HWND g_hListBox = nullptr; // 全局列表框句柄
std::vector<std::pair<ULONGLONG, std::wstring>> g_bigFiles; // 存储大文件信息
std::atomic<bool> g_bScanning(false); // 扫描状态
std::future<void> g_scanFuture; // 用于存储 std::async 的返回值
HFONT g_hFont = nullptr; // 全局字体句柄
int g_sortColumn = 0; // 0: 文件大小, 1: 文件路径
bool g_sortAscending = false; // true: 升序, false: 降序
std::atomic<int> g_updateCounter(0); // 用于控制阶段性更新的计数器
const int UPDATE_THRESHOLD = 50; // 每找到50个文件更新一次UI
int g_totalDrives = 0; // 总固定驱动器数量
std::atomic<int> g_scannedDrives(0); // 已扫描驱动器数量

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 辅助函数声明 (已在 .h 中声明，此处为实现准备)
void StartScan(HWND hWnd);
void ScanDrive(const std::wstring& drivePath, HWND hWnd);
void AddFileToListBox(HWND hListBox, ULONGLONG fileSize, const std::wstring& filePath);
void DeleteSelectedFile(HWND hListBox, bool forceDelete); // 修改为支持强制删除
void OpenInExplorer(HWND hListBox); // 新增函数
BOOL IsSystemDirectory(const std::wstring& path);
void SortAndRefreshListView(HWND hWnd); // 新增排序并刷新列表的函数
void UpdateWindowTitle(HWND hWnd, const std::wstring& statusText); // 新增更新窗口标题的函数

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
    LoadStringW(hInstance, IDC_FINDALLBIGFILETOLISTPROJ, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FINDALLBIGFILETOLISTPROJ));

    MSG msg = {}; // 显式初始化

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
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

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FINDALLBIGFILETOLISTPROJ));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_FINDALLBIGFILETOLISTPROJ);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

    // 初始化通用控件库
    INITCOMMONCONTROLSEX icex = {}; // 零初始化
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_LISTVIEW_CLASSES;
    InitCommonControlsEx(&icex);

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, 0, 1000, 600, nullptr, nullptr, hInstance, nullptr); // 调整窗口大小

    if (!hWnd)
    {
        return FALSE;
    }

    // 创建“开始扫描”按钮
    HWND hScanButton = CreateWindowW(
        L"BUTTON", L"开始扫描",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 120, 30, // 位置和大小
        hWnd, (HMENU)IDC_SCAN_BUTTON, hInstance, nullptr);

    // 设置按钮字体为微软雅黑
    g_hFont = CreateFontW( // 将字体句柄存储到全局变量
        -16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"微软雅黑");
    SendMessage(hScanButton, WM_SETFONT, (WPARAM)g_hFont, TRUE);

    // 创建列表视图控件 (ListView)
    g_hListBox = CreateWindowW(
        WC_LISTVIEW, L"",
        WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT | LVS_SINGLESEL,
        10, 50, 760, 500, // 位置和大小
        hWnd, (HMENU)IDC_FILE_LISTBOX, hInstance, nullptr);

    // 初始化ListView列
    LVCOLUMNW lvc = {}; // 零初始化
    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

    // 第一列：文件大小 (GB)
    lvc.iSubItem = 0;
    WCHAR szCol1Text[] = L"文件大小 (GB)";
    lvc.pszText = szCol1Text;
    lvc.cx = 120;
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(g_hListBox, 0, &lvc);

    // 第二列：文件绝对路径
    lvc.iSubItem = 1;
    WCHAR szCol2Text[] = L"文件绝对路径";
    lvc.pszText = szCol2Text;
    lvc.cx = 640; // 调整宽度以适应路径
    lvc.fmt = LVCFMT_LEFT;
    ListView_InsertColumn(g_hListBox, 1, &lvc);

    // 设置ListView的扩展样式，例如整行选择
    ListView_SetExtendedListViewStyle(g_hListBox, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);


    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    // 初始状态：设置窗口标题为“就绪”
    UpdateWindowTitle(hWnd, L"就绪");

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
        // 确保ListView在窗口创建时被正确初始化
        // 实际上，ListView已经在InitInstance中创建，这里可以做一些额外的初始化
        break;
    case WM_SIZE:
    {
        // 调整按钮和列表视图的大小
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // 调整按钮位置和大小
        HWND hScanButton = GetDlgItem(hWnd, IDC_SCAN_BUTTON);
        if (hScanButton)
        {
            MoveWindow(hScanButton, 10, 10, 120, 30, TRUE);
        }

        // 调整列表视图位置和大小
        if (g_hListBox)
        {
            // C26454 警告修复：确保计算结果不会是负数
            int listviewWidth = width - 30;
            int listviewHeight = height - 80;

            // 防止尺寸为负值
            if (listviewWidth < 0) listviewWidth = 0;
            if (listviewHeight < 0) listviewHeight = 0;

            MoveWindow(g_hListBox, 10, 50, listviewWidth, listviewHeight, TRUE);
            // 调整列宽
            ListView_SetColumnWidth(g_hListBox, 0, LVSCW_AUTOSIZE_USEHEADER);
            // 警告 C26454 在此行 (288) 可能是误报，因为 LVSCW_AUTOSIZE_USEHEADER 是常量
            ListView_SetColumnWidth(g_hListBox, 1, LVSCW_AUTOSIZE_USEHEADER);
        }
    }
    break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        // 分析菜单选择:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDC_SCAN_BUTTON: // 处理“开始扫描”按钮点击
            if (!g_bScanning) // 避免重复扫描
            {
                ListView_DeleteAllItems(g_hListBox); // 清空列表
                g_bigFiles.clear(); // 清空存储的文件列表
                StartScan(hWnd);
            }
            break;
        case IDM_DELETE_FILE: // 处理删除文件命令 (发送到回收站)
            DeleteSelectedFile(g_hListBox, false);
            break;
        case IDM_FORCE_DELETE: // 处理强制删除命令
            DeleteSelectedFile(g_hListBox, true);
            break;
        case IDM_OPEN_EXPLORER: // 处理在资源管理器中打开命令
            OpenInExplorer(g_hListBox);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_NOTIFY:
    {
        LPNMHDR lpnmh = (LPNMHDR)lParam;
        if (lpnmh->hwndFrom == g_hListBox)
        {
            if (lpnmh->code == NM_RCLICK)
            {
                // 处理ListView右键点击事件
                LPNMITEMACTIVATE lpnmia = (LPNMITEMACTIVATE)lParam;
                if (lpnmia->iItem != -1) // 确保点击了有效项
                {
                    POINT pt;
                    GetCursorPos(&pt); // 获取鼠标位置
                    HMENU hMenu = CreatePopupMenu();
                    AppendMenuW(hMenu, MF_STRING, IDM_DELETE_FILE, L"删除文件 (到回收站)");
                    AppendMenuW(hMenu, MF_STRING, IDM_FORCE_DELETE, L"强制删除");
                    AppendMenuW(hMenu, MF_STRING, IDM_OPEN_EXPLORER, L"在资源管理器中打开");
                    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON | TPM_VERPOSANIMATION, pt.x, pt.y, 0, hWnd, nullptr);
                    DestroyMenu(hMenu);
                }
            }
            else if (lpnmh->code == LVN_COLUMNCLICK)
            {
                // 处理ListView列头点击事件
                LPNMLISTVIEW pNMLV = (LPNMLISTVIEW)lParam;
                int clickedColumn = pNMLV->iSubItem;

                if (clickedColumn == g_sortColumn)
                {
                    // 如果点击的是当前排序的列，则反转排序方向
                    g_sortAscending = !g_sortAscending;
                }
                else
                {
                    // 如果点击的是新列，则按新列升序排序
                    g_sortColumn = clickedColumn;
                    g_sortAscending = true; // 默认升序
                }
                SortAndRefreshListView(hWnd); // 排序并刷新列表
            }
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
        if (g_hFont)
        {
            DeleteObject(g_hFont); // 释放字体资源
            g_hFont = nullptr;
        }
        break;
    case WM_APP + 1: // 自定义消息：文件找到，触发阶段性更新（如果达到阈值） - 此消息目前未直接使用
        break;
    case WM_APP + 2: // 自定义消息：扫描完成
        EnableWindow(GetDlgItem(hWnd, IDC_SCAN_BUTTON), TRUE); // 重新启用按钮
        // 扫描完成后，设置窗口标题为“就绪”
        UpdateWindowTitle(hWnd, L"就绪");
        MessageBoxW(hWnd, L"文件扫描完成！", L"扫描完成", MB_OK | MB_ICONINFORMATION);
        break;
    case WM_APP + 3: // 自定义消息：重新填充ListView
    {
        ListView_DeleteAllItems(g_hListBox); // 清空ListView
        for (const auto& file : g_bigFiles)
        {
            AddFileToListBox(g_hListBox, file.first, file.second);
        }
    }
    break;
    case WM_APP + 4: // 自定义消息：更新扫描进度
    {
        int progress = (int)wParam; // wParam 携带百分比进度
        std::wstring statusText = L"任务进度: " + std::to_wstring(progress) + L"%";
        UpdateWindowTitle(hWnd, statusText);
    }
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

// 辅助函数实现

// 判断是否为系统目录
BOOL IsSystemDirectory(const std::wstring& path)
{
    if (path.find(L"C:\\Windows") == 0 ||
        path.find(L"C:\\Program Files") == 0 ||
        path.find(L"C:\\Program Files (x86)") == 0 ||
        path.find(L"C:\\PerfLogs") == 0 ||
        path.find(L"$Recycle.Bin") != std::wstring::npos || // 回收站
        path.find(L"System Volume Information") != std::wstring::npos) // 系统卷信息
    {
        return TRUE;
    }
    return FALSE;
}

// 将文件信息添加到ListView
void AddFileToListBox(HWND hListBox, ULONGLONG fileSize, const std::wstring& filePath)
{
    LVITEMW lvI = {}; // 零初始化
    int iItem;

    // 插入新行
    lvI.mask = LVIF_TEXT;
    lvI.iItem = ListView_GetItemCount(hListBox);
    lvI.iSubItem = 0;

    // 文件大小转换为GB
    double fileSizeGB = (double)fileSize / (1024.0 * 1024.0 * 1024.0);
    WCHAR szFileSize[50];
    swprintf_s(szFileSize, L"%.2f GB", fileSizeGB);
    lvI.pszText = szFileSize;

    iItem = ListView_InsertItem(hListBox, &lvI);

    // 设置文件路径子项
    ListView_SetItemText(hListBox, iItem, 1, (LPWSTR)filePath.c_str());
}

// 扫描单个驱动器 (此函数本身不直接更新进度，进度更新在调用它的循环中进行)
void ScanDrive(const std::wstring& drivePath, HWND hWnd)
{
    namespace fs = std::filesystem; // 使用标准版本

    // 检查是否为系统目录或网络驱动器
    if (IsSystemDirectory(drivePath) || PathIsNetworkPathW(drivePath.c_str()))
    {
        return;
    }

    try
    {
        // 使用 recursive_directory_iterator 遍历目录
        // C++17 的 std::filesystem::recursive_directory_iterator 默认会跳过权限拒绝的目录
        for (const auto& entry : fs::recursive_directory_iterator(drivePath))
        {
            if (!g_bScanning) // 如果扫描被取消
            {
                break;
            }

            try
            {
                if (fs::is_regular_file(entry.status()))
                {
                    ULONGLONG fileSize = fs::file_size(entry.path());
                    if (fileSize >= (1024ULL * 1024 * 1024)) // 1GB
                    {
                        // 将文件信息添加到全局列表
                        g_bigFiles.push_back({ fileSize, entry.path().wstring() });
                        // 阶段性更新UI
                        g_updateCounter++;
                        if (g_updateCounter >= UPDATE_THRESHOLD)
                        {
                            PostMessage(hWnd, WM_APP + 3, 0, 0); // 发送消息通知主线程重新填充ListView
                            g_updateCounter = 0; // 重置计数器
                        }
                    }
                }
            }
            catch (const fs::filesystem_error& e)
            {
                // 忽略权限拒绝或其他文件系统错误
                // 将 char* 转换为 wstring
                int len = MultiByteToWideChar(CP_ACP, 0, e.what(), -1, nullptr, 0);
                std::vector<WCHAR> wbuf(len);
                MultiByteToWideChar(CP_ACP, 0, e.what(), -1, wbuf.data(), len);
                OutputDebugStringW((L"Filesystem error (file): " + std::wstring(wbuf.data())).c_str());
            }
        }
    }
    catch (const fs::filesystem_error& e)
    {
        // 忽略无法访问的目录
        // 将 char* 转换为 wstring
        int len = MultiByteToWideChar(CP_ACP, 0, e.what(), -1, nullptr, 0);
        std::vector<WCHAR> wbuf(len);
        MultiByteToWideChar(CP_ACP, 0, e.what(), -1, wbuf.data(), len);
        OutputDebugStringW((L"Filesystem error (directory): " + std::wstring(wbuf.data())).c_str());
    }
}

// 启动扫描
void StartScan(HWND hWnd)
{
    g_bScanning = true;
    g_updateCounter = 0; // 重置文件更新计数器
    g_scannedDrives = 0; // 重置已扫描驱动器计数

    // 计算总固定驱动器数量
    g_totalDrives = 0;
    WCHAR driveLetters[MAX_PATH];
    GetLogicalDriveStringsW(MAX_PATH, driveLetters);
    WCHAR* drive = driveLetters;
    while (*drive)
    {
        if (GetDriveTypeW(drive) == DRIVE_FIXED)
        {
            g_totalDrives++;
        }
        drive += wcslen(drive) + 1;
    }

    // 禁用按钮，防止重复点击
    EnableWindow(GetDlgItem(hWnd, IDC_SCAN_BUTTON), FALSE);

    // 设置初始标题为0%进度
    UpdateWindowTitle(hWnd, L"任务进度: 0%");

    // 使用异步任务在后台线程执行扫描
    g_scanFuture = std::async(std::launch::async, [hWnd]() { // 存储返回值
        WCHAR driveLettersBuffer[MAX_PATH]; // 使用不同的变量名以避免混淆
        GetLogicalDriveStringsW(MAX_PATH, driveLettersBuffer);

        WCHAR* currentDrive = driveLettersBuffer;
        while (*currentDrive)
        {
            UINT driveType = GetDriveTypeW(currentDrive);
            // 只扫描固定磁盘
            if (driveType == DRIVE_FIXED)
            {
                ScanDrive(currentDrive, hWnd);
                // 每完成一个驱动器扫描，更新进度
                g_scannedDrives++;
                if (g_totalDrives > 0) { // 避免除以零
                    int progress = (int)((static_cast<double>(g_scannedDrives) / g_totalDrives) * 100);
                    // 确保进度不超过100%
                    if (progress > 100) progress = 100;
                    PostMessage(hWnd, WM_APP + 4, (WPARAM)progress, 0); // 发送消息通知主线程更新进度
                }
            }
            currentDrive += wcslen(currentDrive) + 1;
        }

        // 扫描完成后，对g_bigFiles进行排序（从大到小）
        // 初始排序，确保列表显示时是按大小降序
        g_sortColumn = 0; // 默认按大小
        g_sortAscending = false; // 默认降序
        SortAndRefreshListView(hWnd); // 调用新的排序并刷新函数

        g_bScanning = false;
        // 扫描完成后重新启用按钮，并发送消息通知主线程扫描完成
        PostMessage(hWnd, WM_APP + 2, 0, 0);
        });
}

// 删除选中的文件
void DeleteSelectedFile(HWND hListBox, bool forceDelete)
{
    int iSelected = ListView_GetNextItem(hListBox, -1, LVNI_SELECTED);
    if (iSelected != -1)
    {
        // C6054 警告修复：初始化缓冲区以确保零终止
        WCHAR szFilePath[MAX_PATH] = { 0 };
        ListView_GetItemText(hListBox, iSelected, 1, szFilePath, MAX_PATH);
        std::wstring filePath = szFilePath; // 此时 filePath 保证零终止

        UINT confirmFlags = MB_YESNO | MB_ICONWARNING;
        std::wstring confirmMessage;
        UINT fileOperationFlags = FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT;

        if (forceDelete)
        {
            confirmMessage = L"确定要强制删除文件:\n" + filePath + L"\n此操作不可撤销，文件将永久删除！";
            // FOF_ALLOWUNDO 标志控制是否发送到回收站。不设置此标志则为强制删除。
        }
        else
        {
            confirmMessage = L"确定要删除文件:\n" + filePath + L"\n此操作可撤销，文件将发送到回收站。";
            fileOperationFlags |= FOF_ALLOWUNDO; // 发送到回收站
        }

        if (MessageBoxW(nullptr, confirmMessage.c_str(), L"确认删除", confirmFlags) == IDYES)
        {
            try
            {
                SHFILEOPSTRUCTW sf = {}; // 零初始化
                sf.hwnd = nullptr;
                sf.wFunc = FO_DELETE;
                // SHFileOperation需要双空终止符
                // wcscpy_s 会在复制后添加一个零终止符
                // 我们需要手动添加第二个零终止符
                std::vector<WCHAR> fromBuffer(filePath.length() + 2); // 字符串长度 + 两个零终止符
                wcscpy_s(fromBuffer.data(), fromBuffer.size(), filePath.c_str());
                fromBuffer[filePath.length() + 1] = L'\0'; // 添加第二个零终止符
                sf.pFrom = fromBuffer.data();
                sf.fFlags = fileOperationFlags;

                int result = SHFileOperationW(&sf);

                if (result == 0 && !sf.fAnyOperationsAborted)
                {
                    MessageBoxW(nullptr, L"文件已成功删除。", L"删除成功", MB_OK | MB_ICONINFORMATION);
                    // 从ListView和g_bigFiles中移除该项
                    ListView_DeleteItem(hListBox, iSelected);
                    // 找到g_bigFiles中对应的项并移除
                    // 注意：这里需要根据路径来查找，因为ListView的索引可能在删除后发生变化
                    for (size_t i = 0; i < g_bigFiles.size(); ++i)
                    {
                        if (g_bigFiles[i].second == filePath)
                        {
                            g_bigFiles.erase(g_bigFiles.begin() + i);
                            break;
                        }
                    }
                }
                else
                {
                    MessageBoxW(nullptr, L"文件删除失败。", L"删除失败", MB_OK | MB_ICONERROR);
                }
            }
            catch (const std::exception& e)
            {
                // 将 char* 转换为 wstring
                int len = MultiByteToWideChar(CP_ACP, 0, e.what(), -1, nullptr, 0);
                std::vector<WCHAR> wbuf(len);
                MultiByteToWideChar(CP_ACP, 0, e.what(), -1, wbuf.data(), len);
                MessageBoxW(nullptr, (L"删除时发生异常: " + std::wstring(wbuf.data())).c_str(), L"错误", MB_OK | MB_ICONERROR);
            }
            catch (...) // 捕获其他未知异常
            {
                MessageBoxW(nullptr, L"删除时发生未知异常。", L"错误", MB_OK | MB_ICONERROR);
            }
        }
    }
    else
    {
        MessageBoxW(nullptr, L"请选择要删除的文件。", L"提示", MB_OK | MB_ICONINFORMATION);
    }
}

// 在资源管理器中打开选中的文件所在的目录
void OpenInExplorer(HWND hListBox)
{
    int iSelected = ListView_GetNextItem(hListBox, -1, LVNI_SELECTED);
    if (iSelected != -1)
    {
        // C6054 警告修复：初始化缓冲区以确保零终止
        WCHAR szFilePath[MAX_PATH] = { 0 };
        ListView_GetItemText(hListBox, iSelected, 1, szFilePath, MAX_PATH);
        std::wstring filePath = szFilePath;

        // 获取文件所在的目录
        WCHAR szDirectoryPath[MAX_PATH];
        wcscpy_s(szDirectoryPath, MAX_PATH, filePath.c_str());
        PathRemoveFileSpecW(szDirectoryPath); // 移除文件名，留下目录路径

        // 使用ShellExecuteW打开资源管理器并选中文件
        // "explore"动词可以打开资源管理器并选中指定文件或目录
        // 如果是文件，它会打开文件所在的目录并选中该文件
        // 如果是目录，它会直接打开该目录
        // 另一种方法是直接执行 explorer.exe /select,path
        // 这种方式可以确保文件被选中
        ShellExecuteW(nullptr, nullptr, L"explorer.exe", (L"/select,\"" + filePath + L"\"").c_str(), nullptr, SW_SHOWNORMAL);
    }
    else
    {
        MessageBoxW(nullptr, L"请选择一个文件。", L"提示", MB_OK | MB_ICONINFORMATION);
    }
}

// 排序并刷新ListView
void SortAndRefreshListView(HWND hWnd)
{
    // 确保在UI线程进行操作
    if (g_hListBox == nullptr) return;

    // 根据g_sortColumn和g_sortAscending对g_bigFiles进行排序
    std::sort(g_bigFiles.begin(), g_bigFiles.end(),
        [](const auto& a, const auto& b) {
            if (g_sortColumn == 0) // 按文件大小排序
            {
                return g_sortAscending ? (a.first < b.first) : (a.first > b.first);
            }
            else // 按文件路径排序
            {
                return g_sortAscending ? (a.second < b.second) : (a.second > b.second);
            }
        });

    // 清空ListView并重新填充排序后的数据
    ListView_DeleteAllItems(g_hListBox);
    for (const auto& file : g_bigFiles)
    {
        AddFileToListBox(g_hListBox, file.first, file.second);
    }
}

// 更新窗口标题
void UpdateWindowTitle(HWND hWnd, const std::wstring& statusText)
{
    // 使用 LoadStringW 获取 IDS_APP_TITLE
    WCHAR szAppTitle[MAX_LOADSTRING];
    LoadStringW(hInst, IDS_APP_TITLE, szAppTitle, MAX_LOADSTRING);
    std::wstring windowTitle = szAppTitle; // 获取原始标题
    windowTitle += L" - " + statusText;
    SetWindowTextW(hWnd, windowTitle.c_str());
}
