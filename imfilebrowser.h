#pragma once

#include <filesystem>
#include <string>

#ifndef IMGUI_VERSION
#   error "include imgui.h before this header"
#endif

namespace ImGui
{
    class FileBrowser
    {
    public:

        enum Flags
        {
            Flags_SelectRegularFile  = 1 << 0,
            Flags_SelectDirectory    = 1 << 1,
            Flags_EnterNewFilename   = 1 << 2,
            Flags_CanCreateDirectory = 1 << 3,
            Flags_NoModal            = 1 << 4,
            Flags_CloseOnEsc         = 1 << 5,
            Flags_NoStatusBar        = 1 << 6
        };

        // 默认将pwd设置为当前工作目录，可通过SetPwd修改
        explicit FileBrowser(Flags flags = Flags_SelectRegularFile);

        // 设置窗口标题文字，以utf-8编码
        void SetTitle(std::string title);

        // 打开文件窗口
        void Open();

        // 关闭文件窗口
        void Close();

        // 是否处于打开状态
        bool IsOpened();

        // 显示文件窗口
        void Display();

        // 是否选中了某个文件并已确认
        bool IsSelected() const;

        // 设置当前显示的路径
        bool SetPwd(std::string pwd);

        // 清除除了flags外的所有状态
        void Clear();

        // 取得当前选择的文件路径，仅在IsSelected返回true时有意义
        std::string GetSelectedFilename() const;

    private:

        std::filesystem::path pwd_;
        std::string selectedFilename_;

        struct FileRecord
        {
            bool isDir;
            std::string name_;
        };
        std::vector<FileRecord> fileRecords_;

        static constexpr size_t INPUT_NAME_BUF_SIZE = 512;
        char inputNameBuf_[INPUT_NAME_BUF_SIZE];
    };
}
