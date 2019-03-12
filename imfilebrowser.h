#pragma once

#include <array>
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
        bool IsOpened() const;

        // 显示文件窗口
        void Display();

        // 是否选中了某个文件并已确认
        bool HasSelected() const;

        // 设置当前显示的路径
        bool SetPwd(std::filesystem::path pwd);

        // 取得当前选择的文件路径，仅在IsSelected返回true时有意义
        std::string GetSelectedFilename() const;

    private:

        void SetPwdUncatched(std::filesystem::path pwd);

        const Flags flags_;

        std::string title_;
        std::string openLabel_;

        bool openFlag_;
        bool closeFlag_;
        bool isOpened_;

        std::string statusStr_;
        bool sortByName_;

        std::filesystem::path pwd_;
        std::string selectedFilename_;

        struct FileRecord
        {
            bool isDir;
            std::string name;
        };
        std::vector<FileRecord> fileRecords_;

        std::array<char, 512> inputNameBuf_;
    };
} // namespace ImGui

inline ImGui::FileBrowser::FileBrowser(Flags flags)
    : flags_(flags),
      openFlag_(false), closeFlag_(false), isOpened_(false),
      sortByName_(true)
{
    inputNameBuf_[0] = '\0';
    SetTitle("file browser");
    SetPwd(std::filesystem::current_path());
}

inline void ImGui::FileBrowser::SetTitle(std::string title)
{
    title_ = std::move(title);
    openLabel_ = title_ + "##filebrowser_" + title_ + std::to_string(reinterpret_cast<size_t>(this));
}

inline void ImGui::FileBrowser::Open()
{
    if(!IsOpened())
    {
        openFlag_ = true;
        closeFlag_ = false;
    }
}

inline void ImGui::FileBrowser::Close()
{
    if(IsOpened())
    {
        closeFlag_ = true;
        openFlag_ = false;
    }
}

inline bool ImGui::FileBrowser::IsOpened() const
{
    return isOpened_ || openFlag_;
}

inline void ImGui::FileBrowser::Display()
{
    // TODO
}

inline bool ImGui::FileBrowser::HasSelected() const
{
    return !selectedFilename_.empty();
}

inline bool ImGui::FileBrowser::SetPwd(std::filesystem::path pwd)
{
    try
    {
        SetPwdUncatched(std::move(pwd));
        return true;
    }
    catch(const std::exception &err)
    {
        statusStr_ = std::string("error: ") + err.what();
    }
    catch(...)
    {
        statusStr_ = "error: unknown";
    }

    SetPwdUncatched(std::filesystem::current_path());
    return false;
}

inline std::string ImGui::FileBrowser::GetSelectedFilename() const
{
    return (pwd_ / selectedFilename_).string();
}

inline void ImGui::FileBrowser::SetPwdUncatched(std::filesystem::path pwd)
{
    fileRecords_ = { FileRecord{ true, ".." } };

    for(auto &p : std::filesystem::directory_iterator(pwd_))
    {
        FileRecord rcd;

        if(p.is_regular_file())
            rcd.isDir = false;
        else if(p.is_directory())
            rcd.isDir = true;
        else
            continue;

        rcd.name = p.path().filename().string();
        if(rcd.name.empty() || rcd.name[0] == '$')
            continue;

        fileRecords_.push_back(rcd);
    }

    if(sortByName_)
    {
        std::sort(fileRecords_.begin(), fileRecords_.end(),
            [](auto &L, auto &R)
        {
            return (L.isDir ^ R.isDir) ? L.isDir : (L.name < R.name);
        });
    }
}
