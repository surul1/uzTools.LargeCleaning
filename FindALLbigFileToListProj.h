#pragma once

#include "resource.h"
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <atomic>
#include <Shlwapi.h>
#include <filesystem> // C++17 filesystem library
#include <CommCtrl.h> // For ListView control
#include <shellapi.h> // For SHFileOperation

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Comctl32.lib") // Link with Comctl32.lib for ListView
#pragma comment(lib, "Shell32.lib") // Link with Shell32.lib for SHFileOperation

// Global variables
extern HWND g_hListBox;
extern std::vector<std::pair<ULONGLONG, std::wstring>> g_bigFiles;
extern std::atomic<bool> g_bScanning;
extern std::future<void> g_scanFuture; // 用于存储 std::async 的返回值

// Function prototypes
void StartScan(HWND hWnd);
void ScanDrive(const std::wstring& drivePath, HWND hWnd);
void AddFileToListBox(HWND hListBox, ULONGLONG fileSize, const std::wstring& filePath);
void DeleteSelectedFile(HWND hListBox, bool forceDelete);
void OpenInExplorer(HWND hListBox);
BOOL IsSystemDirectory(const std::wstring& path);
