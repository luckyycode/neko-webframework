#include "../../Editor/WorldEditor.h"
#include "../../Editor/StudioApp.h"
#include "../../Engine/Core/Log.h"
#include "../../Editor/ui/UI.h"

namespace Neko
{
    struct StudioServerPlugin : public IStudioApp::UiPlugin
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
