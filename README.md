# Windows Large File Management Tool

A simple and efficient Windows desktop application built with C++ Win32 API, designed to help users identify and manage large files (1GB and above) on their local fixed drives. It leverages the C++17 `std::filesystem` library for efficient file scanning and `std::async` for background operations, ensuring a responsive user interface.

### ✨ Features

*   **Comprehensive Scanning:** Scans all fixed drives for files larger than 1GB.
*   **User-Friendly List View:** Displays scan results in a sortable ListView (by file size and absolute path).
*   **Real-time Progress:** Shows scanning progress directly in the window title.
*   **Flexible Deletion:**
    *   Delete selected files to Recycle Bin (undoable).
    *   Force delete selected files (irreversible).
*   **Easy Navigation:** Open the location of selected files in Windows Explorer.
*   **System Directory Exclusion:** Automatically skips system directories (e.g., `C:\Windows`, `Program Files`, `Recycle Bin`, `System Volume Information`) to enhance performance and prevent accidental critical file operations.
*   **Responsive UI:** Background scanning thread prevents UI freezing.

### 🚀 Getting Started

#### Installation

1.  **Download:** Go to the [Releases](https://github.com/CNMengHan/uzTools.LargeCleaning/releases) page and download the latest `FindALLbigFileToListProj.zip` (or `.exe` if provided directly).
2.  **Extract & Run:** Extract the contents of the zip file. You can then simply run `FindALLbigFileToListProj.exe`.

#### Usage

1.  **Start Scan:** Click the "开始扫描" (Start Scan) button. The application will begin scanning all fixed drives for files larger than 1GB. The window title will update with the scanning progress.
2.  **View Results:** Files found will be listed in the main window.
3.  **Sort:** Click on the "文件大小 (GB)" (File Size (GB)) or "文件绝对路径" (File Absolute Path) column headers to sort the results. Clicking again will reverse the sort order.
4.  **Manage Files (Right-Click Menu):**
    *   **删除文件 (到回收站)** (Delete File (to Recycle Bin)): Moves the selected file to the Recycle Bin.
    *   **强制删除** (Force Delete): Permanently deletes the selected file, bypassing the Recycle Bin. *Use with caution!*
    *   **在资源管理器中打开** (Open in Explorer): Opens the directory containing the selected file in Windows Explorer, with the file highlighted.

### 💻 Development Setup

To build this project from source, you will need:

*   **Visual Studio:** Visual Studio 2022 (or a compatible newer version) is recommended.
*   **Workload:** Ensure the "Desktop development with C++" workload is installed with Visual Studio.

**Steps:**

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/CNMengHan/uzTools.LargeCleaning.git
    cd uzTools.LargeCleaning/FindALLbigFileToListProj
    ```
2.  **Open Solution:** Open the `FindALLbigFileToListProj.sln` file with Visual Studio.
3.  **Build:** Build the solution (e.g., F7 or Build -> Build Solution). The executable will be generated in the `x64/Debug` or `x64/Release` directory, depending on your build configuration.

### 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

## 中文 | 🇨🇳

一个简单高效的 Windows 桌面应用程序，使用 C++ Win32 API 构建，旨在帮助用户识别和管理本地固定驱动器上大于 1GB 的大型文件。它利用 C++17 的 `std::filesystem` 库进行高效的文件扫描，并使用 `std::async` 进行后台操作，以确保用户界面的响应性。

### ✨ 功能特性

*   **全面扫描：** 扫描所有固定驱动器上大于 1GB 的文件。
*   **友好的列表视图：** 在可排序的列表视图 (ListView) 中显示扫描结果（支持按文件大小和绝对路径排序）。
*   **实时进度显示：** 在窗口标题中实时显示扫描进度。
*   **灵活删除：**
    *   将选定的文件删除到回收站（可撤销）。
    *   强制删除选定的文件（此操作不可逆，请谨慎使用！）。
*   **轻松导航：** 在 Windows 资源管理器中打开选定文件所在的目录。
*   **系统目录排除：** 自动跳过系统目录（例如 `C:\Windows`、`Program Files`、`Recycle Bin`、`System Volume Information`），以提高扫描性能并防止误操作关键系统文件。
*   **响应式界面：** 后台扫描线程确保 UI 不会冻结。

### 🚀 快速开始

#### 安装

1.  **下载：** 访问 [Releases](https://github.com/CNMengHan/uzTools.LargeCleaning/releases) 页面，下载最新的 `FindALLbigFileToListProj.zip` (或直接提供的 `.exe` 文件)。
2.  **解压运行：** 解压 zip 文件内容。然后直接运行 `FindALLbigFileToListProj.exe` 即可。

#### 使用方法

1.  **开始扫描：** 点击 "开始扫描" 按钮。应用程序将开始扫描所有固定驱动器上大于 1GB 的文件。窗口标题将实时更新扫描进度。
2.  **查看结果：** 找到的文件将显示在主窗口的列表中。
3.  **排序：** 点击 "文件大小 (GB)" 或 "文件绝对路径" 列头可以对结果进行排序。再次点击将反转排序顺序。
4.  **文件管理（右键菜单）：**
    *   **删除文件 (到回收站)：** 将选定的文件移动到回收站。
    *   **强制删除：** 永久删除选定的文件，绕过回收站。*请务必谨慎操作！*
    *   **在资源管理器中打开：** 在 Windows 资源管理器中打开选定文件所在的目录，并突出显示该文件。

### 💻 开发环境设置

要从源代码构建此项目，您需要：

*   **Visual Studio：** 建议使用 Visual Studio 2022 (或兼容的更新版本)。
*   **工作负载：** 确保 Visual Studio 已安装 "使用 C++ 的桌面开发" 工作负载。

**步骤：**

1.  **克隆仓库：**
    ```bash
    git clone https://github.com/CNMengHan/uzTools.LargeCleaning.git
    cd uzTools.LargeCleaning/FindALLbigFileToListProj
    ```
2.  **打开解决方案：** 使用 Visual Studio 打开 `FindALLbigFileToListProj.sln` 解决方案文件。
3.  **构建：** 构建解决方案 (例如，按 F7 或选择 "生成" -> "生成解决方案")。可执行文件将生成在 `x64/Debug` 或 `x64/Release` 目录中，具体取决于您的构建配置。

### 📄 许可证

本项目采用 MIT 许可证开源 - 详情请参阅 [LICENSE](LICENSE) 文件。
