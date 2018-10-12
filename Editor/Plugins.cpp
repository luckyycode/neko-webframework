#include "../../Editor/WorldEditor.h"
#include "../../Editor/StudioApp.h"
#include "../../Engine/Core/Log.h"
#include "../../Editor/ui/UI.h"

namespace Neko
{
    struct StudioServerPlugin : public IStudioApp::IPlugin
    {
        StudioServerPlugin(IStudioApp& app)
        : App(app)
        {
        }
        
        ~StudioServerPlugin()
        {
        }
        
        
        void Update(float) override
        {
        }
        
        void OnWindowGUI() override
        {
            if (!UI::BeginDock("Http Server"))
            {
                UI::EndDock();
                return;
            }
            
            UI::EndDock();
        }
        
        
        const char* GetName() const override { return "http_server"; }
        
        IStudioApp& App;
    };
    
    NEKO_STUDIO_ENTRY(neko_httpserver)
    {
        WorldEditor& editor = *app.GetWorldEditor();
        IAllocator& allocator = editor.GetAllocator();
        StudioServerPlugin* plugin = NEKO_NEW(allocator, StudioServerPlugin)(app);
        app.AddPlugin(*plugin);
    }
}
