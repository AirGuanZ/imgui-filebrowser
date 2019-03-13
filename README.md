# imgui-filebrowser

[imgui-filebrowser](https://github.com/AirGuanZ/imgui-filebrowser) is a simple file browser implementation for [dear-imgui](https://github.com/ocornut/imgui).

![IMG](./screenshots/0.png)

## Getting Started

imgui-filebrowser is header-only and must be included after imgui.h:

```cpp
#include <imgui.h>
#include <imfilebrowser.h>
```

It's hard for me to design interface for a stateless file browser. Thus, instead of creating a file dialog by a immediate function call, user needs to create a `ImGui::FileBrowser` instance, open it with member function `Open()`, and calls `Display()` in each rendering frame. Here is a simple example:

```cpp
#include <imgui.h>
#include <imfilebrowser.h>

int main()
{
    //...initialize rendering window and imgui
    
    // create a file browser instance
    ImGui::FileBrowser fileDialog(ImGuiFileBrowserFlags_EnterNewFilename);
    
    // mainloop
    while(continueRendering)
    {
        //...do other stuff like ImGui::NewFrame();
        
        // open file dialog when user clicks this button
        if(ImGui::Button("open file dialog"))
            fileDialog.Open();
        
        fileDialog.Display();
        
        if(fileDialog.HasSelected())
        {
            std::cout << "Selected filename" << fileDialog.GetSelected() << std::endl;
            fileDialog.ClearSelected();
        }
        
        //...do other stuff like ImGui::Render();
    }
    
    //...shutdown
}
```

