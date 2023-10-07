# imgui-filebrowser

[imgui-filebrowser](https://github.com/AirGuanZ/imgui-filebrowser) is a header-only file browser implementation for [dear-imgui](https://github.com/ocornut/imgui). C++ 17 is required.

![IMG](./screenshots/0.png)

## Getting Started

`imfilebrowser.h` should be included after `imgui.h`:

```cpp
#include <imgui.h>
#include <imfilebrowser.h>
```

Instead of creating a file dialog with an immediate function call, you need to create a `ImGui::FileBrowser` instance, open it with member function `Open()`, and call `Display()` in each frame. Here is a simple example:

```cpp
#include <imgui.h>
#include <imfilebrowser.h>

int main()
{
    //...initialize rendering window and imgui
    
    // create a file browser instance
    ImGui::FileBrowser fileDialog;
    
    // (optional) set browser properties
    fileDialog.SetTitle("title");
    fileDialog.SetTypeFilters({ ".h", ".cpp" });
    
    // mainloop
    while(continueRendering)
    {
        //...do other stuff like ImGui::NewFrame();
        
        if(ImGui::Begin("dummy window"))
        {
            // open file dialog when user clicks this button
            if(ImGui::Button("open file dialog"))
                fileDialog.Open();
        }
        ImGui::End();
        
        fileDialog.Display();
        
        if(fileDialog.HasSelected())
        {
            std::cout << "Selected filename" << fileDialog.GetSelected().string() << std::endl;
            fileDialog.ClearSelected();
        }
        
        //...do other stuff like ImGui::Render();
    }
    
    //...shutdown
}
```

## Options

Various options can be combined with '|' and passed to the constructor:

```cpp
enum ImGuiFileBrowserFlags_
{
    ImGuiFileBrowserFlags_SelectDirectory   = 1 << 0, // select directory instead of regular file
    ImGuiFileBrowserFlags_EnterNewFilename  = 1 << 1, // allow user to enter new filename when selecting regular file
    ImGuiFileBrowserFlags_NoModal           = 1 << 2, // file browsing window is modal by default. specify this to use a popup window
    ImGuiFileBrowserFlags_NoTitleBar        = 1 << 3, // hide window title bar
    ImGuiFileBrowserFlags_NoStatusBar       = 1 << 4, // hide status bar at the bottom of browsing window
    ImGuiFileBrowserFlags_CloseOnEsc        = 1 << 5, // close file browser when pressing 'ESC'
    ImGuiFileBrowserFlags_CreateNewDir      = 1 << 6, // allow user to create new directory
    ImGuiFileBrowserFlags_MultipleSelection = 1 << 7, // allow user to select multiple files. this will hide ImGuiFileBrowserFlags_EnterNewFilename
    ImGuiFileBrowserFlags_HideRegularFiles  = 1 << 8, // hide regular files when ImGuiFileBrowserFlags_SelectDirectory is enabled
    ImGuiFileBrowserFlags_ConfirmOnEnter    = 1 << 9, // confirm selection when pressnig 'ENTER'
};
```

When `ImGuiFileBrowserFlags_MultipleSelection` is enabled, use `fileBrowser.GetMultiSelected()` to get all selected filenames (instead of `fileBrowser.GetSelected()`, which returns only one of them).

Here are some common examples:

```cpp
// select single regular file for opening
0
// select multiple regular files for opening
ImGuiFileBrowserFlags_MultipleSelection
// select single directory for opening
ImGuiFileBrowserFlags_SelectDirectory
// select multiple directories for opening
ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_MultipleSelection
// select single regular file for saving
ImGuiFileBrowserFlags_EnterNewFilename | ImGuiFileBrowserFlags_CreateNewDir
// select single directory for saving
ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_CreateNewDir
// select single directory and hide regular files in browser
ImGuiFileBrowserFlags_SelectDirectory | ImGuiFileBrowserFlags_HideRegularFiles
```

## Usage

* double click to enter a directory
* single click to (de)select a regular file (or directory, when `ImGuiFileBrowserFlags_SelectDirectory` is enabled).
*  When `ImGuiFileBrowserFlags_SelectDirectory` is enabled and no directory is selected, click `ok` to choose the current directory as selected result.
*  When `ImGuiFileBrowserFlags_MultipleSelection` is enabled, hold  `Ctrl` for multi selection and `Shift` for range selection.  
*  When `ImGuiFileBrowserFlags_MultipleSelection` is enabled, use `Ctrl + A` to select all (filtered) items.
*  When `ImGuiFileBrowserFlags_CreateNewDir` is enabled, click the top-right little button "+" to create a new directory.
*  When `ImGuiFileBrowserFlags_SelectDirectory` is not specified,  double click to choose a regular file as selected result.

## Type Filters

* (optionally) use `browser.SetTypeFilters({".h", ".cpp"})` to set file extension filters.
* ".*" matches with any extension
* filters are case-insensitive on Windows platform

## Note

The filebrowser implementation queries drive list via Win32 API (only on Windows). Thus `<Windows.h>` is included in `<imfilebrowser.h>`, which may pollute the global namespace. This can be solved by simply moving the `GetDrivesBitMask()` definition into a cpp file.
