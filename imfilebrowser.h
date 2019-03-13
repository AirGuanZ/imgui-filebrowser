#pragma once

#include <array>
#include <filesystem>
#include <string>

#ifndef IMGUI_VERSION
#   error "include imgui.h before this header"
#endif

using ImGuiFileBrowserFlags = int;

enum ImGuiFileBrowserFlags_
{
    ImGuiFileBrowserFlags_SelectDirectory    = 1 << 1,
    ImGuiFileBrowserFlags_EnterNewFilename   = 1 << 2,
    ImGuiFileBrowserFlags_NoModal            = 1 << 3,
    ImGuiFileBrowserFlags_NoTitleBar         = 1 << 4,
    ImGuiFileBrowserFlags_NoStatusBar        = 1 << 5,
};

namespace ImGui
{
    class FileBrowser
    {
    public:

        // 默认将pwd设置为当前工作目录，可通过SetPwd修改
        explicit FileBrowser(ImGuiFileBrowserFlags flags = 0);

        // 设置窗口标题文字，以utf-8编码
        void SetTitle(std::string title);

        // 打开文件窗口
        void Open();

        // 关闭文件窗口
        void Close();

        // 是否处于打开状态
        bool IsOpened() const noexcept;

        // 显示文件窗口
        // 当遭遇非法访问等异常时，当前路径会被自动定向到程序工作目录，且上一个错误会被显示窗口底部的状态栏中。
        // 若定向到程序工作目录时再次发生异常，则该异常会被原样抛出。即使如此，imgui的状态始终是一致的。
        void Display();

        // 是否选中了某个文件并已确认
        bool HasSelected() const noexcept;

        // 设置当前显示的路径
        bool SetPwd(const std::filesystem::path &pwd = std::filesystem::current_path());

        // 取得当前选择的文件路径，仅在IsSelected返回true时有意义
        std::string GetSelected() const;

        // 清除当前选择的文件
        void ClearSelected();

    private:

        class ScopeGuard
        {
            std::function<void()> func_;

        public:

            template<typename T>
            explicit ScopeGuard(T func) : func_(std::move(func)) { }
            ~ScopeGuard() { func_(); }
        };

        void SetPwdUncatched(const std::filesystem::path &pwd);

        ImGuiFileBrowserFlags flags_;

        std::string title_;
        std::string openLabel_;

        bool openFlag_;
        bool closeFlag_;
        bool isOpened_;
        bool ok_;

        std::string statusStr_;

        std::filesystem::path pwd_;
        std::string selectedFilename_;

        struct FileRecord
        {
            bool isDir;
            std::string name;
            std::string showName;
        };
        std::vector<FileRecord> fileRecords_;

        // IMPROVE: overflow when selectedFilename_.length() > inputNameBuf_.size() - 1
        std::array<char, 512> inputNameBuf_;
    };
} // namespace ImGui

inline ImGui::FileBrowser::FileBrowser(ImGuiFileBrowserFlags flags)
    : flags_(flags),
      openFlag_(false), closeFlag_(false), isOpened_(false), ok_(false)
{
    inputNameBuf_[0] = '\0';
    SetTitle("file browser");
    SetPwd(std::filesystem::current_path());
}

inline void ImGui::FileBrowser::SetTitle(std::string title)
{
    title_ = std::move(title);
    openLabel_ = title_ + "##filebrowser_" + std::to_string(reinterpret_cast<size_t>(this));
}

inline void ImGui::FileBrowser::Open()
{
    ClearSelected();
    openFlag_ = true;
    closeFlag_ = false;
}

inline void ImGui::FileBrowser::Close()
{
    ClearSelected();
    closeFlag_ = true;
    openFlag_ = false;
}

inline bool ImGui::FileBrowser::IsOpened() const noexcept
{
    return isOpened_;
}

inline void ImGui::FileBrowser::Display()
{
    PushID(this);
    ScopeGuard exitThis([this] { openFlag_ = false; closeFlag_ = false; PopID(); });

    if(openFlag_)
        OpenPopup(openLabel_.c_str());
    isOpened_ = false;

    // 尝试打开窗口

    if(openFlag_ && (flags_ & ImGuiFileBrowserFlags_NoModal))
        SetNextWindowSize(ImVec2(700, 450));
    else
        SetNextWindowSize(ImVec2(700, 450), ImGuiCond_FirstUseEver);
    if(flags_ & ImGuiFileBrowserFlags_NoModal)
    {
        if(!BeginPopup(openLabel_.c_str()))
            return;
    }
    else if(!BeginPopupModal(openLabel_.c_str(), nullptr, flags_ & ImGuiFileBrowserFlags_NoTitleBar ? ImGuiWindowFlags_NoTitleBar : 0))
        return;
    isOpened_ = true;
    ScopeGuard endPopup([] { EndPopup(); });

    // 显示路径中的一系列section

    int secIdx = 0, newPwdLastSecIdx = -1;
    for(auto &sec : pwd_)
    {
#ifdef _WIN32
        if(secIdx == 1)
        {
            ++secIdx;
            continue;
        }
#endif
        PushID(secIdx);
        if(secIdx > 0)
            SameLine();
        if(SmallButton(sec.u8string().c_str()))
            newPwdLastSecIdx = secIdx;
        PopID();
        ++secIdx;
    }

    if(newPwdLastSecIdx >= 0)
    {
        int i = 0;
        std::filesystem::path newPwd;
        for(auto &sec : pwd_)
        {
            if(i++ > newPwdLastSecIdx)
                break;
            newPwd /= sec;
        }
#ifdef _WIN32
        if(newPwdLastSecIdx == 0)
            newPwd /= "\\";
#endif
        SetPwd(newPwd);
    }

    // 显示文件列表的子窗口

    float reserveHeight = GetItemsLineHeightWithSpacing();
    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory) && (flags_ & ImGuiFileBrowserFlags_EnterNewFilename))
        reserveHeight += GetItemsLineHeightWithSpacing();
    {
        BeginChild("child", ImVec2(0, -reserveHeight), true,
            (flags_ & ImGuiFileBrowserFlags_NoModal) ? ImGuiWindowFlags_AlwaysHorizontalScrollbar : 0);
        ScopeGuard endChild([] { EndChild(); });

        for(auto &rsc : fileRecords_)
        {
            const bool selected = selectedFilename_ == rsc.name;
            if(Selectable(rsc.showName.c_str(), selected, ImGuiSelectableFlags_DontClosePopups))
            {
                if(selected)
                {
                    selectedFilename_ = std::string();
                    inputNameBuf_[0] = '\0';
                }
                else if(rsc.name != "..")
                {
                    if((rsc.isDir && (flags_ & ImGuiFileBrowserFlags_SelectDirectory)) ||
                        (!rsc.isDir && !(flags_ & ImGuiFileBrowserFlags_SelectDirectory)))
                    {
                        selectedFilename_ = rsc.name;
                        if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory))
                            std::strcpy(inputNameBuf_.data(), selectedFilename_.c_str());
                    }
                }
            }

            if(IsItemClicked(0) && IsMouseDoubleClicked(0) && rsc.isDir)
                SetPwd((rsc.name != "..") ? (pwd_ / rsc.name) : pwd_.parent_path());
        }
    }

    // 使用逻辑：
    //  选择文件时，input slot的内容始终是和selectedFilename一致的
    //  选择目录时，input slot无意义且不显示，只有在没有selectedFilename时才能ok，其他情况下都是open selected并进入其中

    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory) && (flags_ & ImGuiFileBrowserFlags_EnterNewFilename))
    {
        PushID(this);
        ScopeGuard popTextID([] { PopID(); });

        PushItemWidth(-1);
        if(InputText("", inputNameBuf_.data(), inputNameBuf_.size()))
            selectedFilename_ = inputNameBuf_.data();
        PopItemWidth();
    }

    if(statusStr_.empty() || (flags_ & ImGuiFileBrowserFlags_NoStatusBar))
        NewLine();
    else
        Text("%s", statusStr_.c_str());

    SameLine(GetWindowWidth() - 140);

    if(!(flags_ & ImGuiFileBrowserFlags_SelectDirectory))
    {
        if(Button(" ok ") && !selectedFilename_.empty())
        {
            ok_ = true;
            CloseCurrentPopup();
        }
    }
    else
    {
        if(selectedFilename_.empty())
        {
            if(Button(" ok "))
            {
                ok_ = true;
                CloseCurrentPopup();
            }
        }
        else if(Button("open"))
            SetPwd(pwd_ / selectedFilename_);
    }

    SameLine();

    if(Button("cancel") || closeFlag_)
        CloseCurrentPopup();
}

inline bool ImGui::FileBrowser::HasSelected() const noexcept
{
    return ok_;
}

inline bool ImGui::FileBrowser::SetPwd(const std::filesystem::path &pwd)
{
    try
    {
        SetPwdUncatched(pwd);
        return true;
    }
    catch(const std::exception &err)
    {
        statusStr_ = std::string("last error: ") + err.what();
    }
    catch(...)
    {
        statusStr_ = "last error: unknown";
    }

    SetPwdUncatched(std::filesystem::current_path());
    return false;
}

inline std::string ImGui::FileBrowser::GetSelected() const
{
    return (pwd_ / selectedFilename_).string();
}

inline void ImGui::FileBrowser::ClearSelected()
{
    selectedFilename_ = std::string();
    inputNameBuf_[0] = '\0';
    ok_ = false;
}

inline void ImGui::FileBrowser::SetPwdUncatched(const std::filesystem::path &pwd)
{
    fileRecords_ = { FileRecord{ true, "..", "[D] .." } };

    for(auto &p : std::filesystem::directory_iterator(pwd))
    {
        FileRecord rcd;

        if(p.is_regular_file())
            rcd.isDir = false;
        else if(p.is_directory())
            rcd.isDir = true;
        else
            continue;

        rcd.name = p.path().filename().string();
        if(rcd.name.empty())
            continue;

        rcd.showName = (rcd.isDir ? "[D] " : "[F] ") + rcd.name;
        fileRecords_.push_back(rcd);
    }

    std::sort(fileRecords_.begin(), fileRecords_.end(),
        [](const FileRecord &L, const FileRecord &R)
    {
        return (L.isDir ^ R.isDir) ? L.isDir : (L.name < R.name);
    });

    pwd_ = absolute(pwd);
    selectedFilename_ = std::string();
    inputNameBuf_[0] = '\0';
}
