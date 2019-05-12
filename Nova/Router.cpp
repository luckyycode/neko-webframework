//
//          *                  *
//             __                *
//           ,db'    *     *
//          ,d8/       *        *    *
//          888
//          `db\       *     *
//            `o`_                    **
//               *                 / )
//             *    /\__/\ *       ( (  *
//           ,-.,-.,)    (.,-.,-.,-.) ).,-.,-.
//          | @|  ={      }= | @|  / / | @|o |
//         _j__j__j_)     `-------/ /__j__j__j_
//          ________(               /___________
//          |  | @| \              || o|O | @|
//          |o |  |,'\       ,   ,'"|  |  |  |  hjw
//          vV\|/vV|`-'\  ,---\   | \Vv\hjwVv\//v
//                     _) )    `. \ /
//                    (__/       ) )
//
//  Router.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "../Utils.h"
#include "Engine/Containers/AssociativeArray.h"
#include "Engine/Core/Log.h"
#include "Engine/Core/Profiler.h"
#include "Router.h"
#include "../Skylar/RouteMethodStringMap.h"

namespace Neko::Nova
{
    using namespace Neko::Skylar;
    
    static const char* RouteParamsTag = "[params]";
    static const char* RouteParamTag = "[param]";

    bool Router::AddRoute(const Http::Method method, const String& path, const String& controller, const String& action)
    {
        assert(method >= Http::Method::Get and method <= Http::Method::Patch);
        
        // assert
        if (path.Find(RouteParamsTag) != INDEX_NONE and not EndsWith(*path, RouteParamsTag))
        {
            LogError.log("Nova") << "[params] tag must be set as last parameter.";
            return false;
        }
        
        Route route(Allocator);
        
        route.Method = method;
        
        // parse path
        ParsePathForRoute(route.Components, path);
        
        for (int32 i = 0; i < route.Components.GetSize(); ++i)
        {
            const auto& item = route.Components[i];
            
            // count amount of param tags in url
            if (item == RouteParamTag)
            {
                route.ParamCount++;
            }
        }
        
        // multiple variable params tag
        route.HasCustomParams = route.Components.Contains(RouteParamsTag);
        
        for (int32 i = 0; i < route.Components.GetSize(); ++i)
        {
            const auto& item = route.Components[i];
            
            // assert param tags
            if (item[0] == '[')
            {
                if (item != RouteParamTag && item != RouteParamsTag)
                {
                    LogError.log("Nova") << "Found tag begin symbol '[' but didn't match any of existing ones.";
                    return false;
                }
            }
            else
            {
                route.ParamIndexes.Push((int16)i);
            }
        }
        
        if (controller[0] != '/')
        {
            // full controller name
            route.Controller = controller.ToLower();
            route.Controller += "controller";
            
            if (action.IsEmpty())
            {
                LogError.log("Nova") << "Invalid controller action supplied.";
                return false;
            }
            
            route.Action = action;
        }
        else
        {
            route.Controller = controller;
        }
        
        // @todo perhaps check for dupes?
        
        // save
        Routes.Push(route);
        
        auto methodName = route.Method; // @todo string
        
        LogInfo.log("Nova") << "Added Url route: "
            << *path << ", method /" << (int)methodName << ", controller \""
            << route.Controller.c_str()
            << "\", action \"" << route.Action.c_str() << "\", params: " << route.HasCustomParams;
        
        return true;
    }
    
    Routing Router::FindRoute(Http::Method method, const TArray<String>& components) const
    {
        PROFILE_FUNCTION()
        
        if (this->Routes.IsEmpty())
        {
            LogWarning.log("Nova") << "FindRoute: No routes";
            
            return Routing(Allocator);
        }
        
        for (auto& route : this->Routes)
        {
            // check lengths
            if (route.HasCustomParams)
            {
                // ignore last token
                if (route.Components.GetSize() - 1 > components.GetSize())
                {
                    continue;
                }
            }
            else
            {
                if (route.Components.GetSize() != components.GetSize())
                {
                    continue;
                }
            }
            
            // compare nodes
            for (int32 index : route.ParamIndexes)
            {
                if (route.Components[index] != components[index])
                {
                    // continue outside
                    goto continueNext;
                }
            }
            
            // check if method types match
            if (route.Method == method or route.Method == Http::Method::Any)
            {
                // Generates parameters for action
                TArray<String> params(components);
                
                if (params.GetSize() == 1 and params[0].IsEmpty())
                {
                    // empty path ( or / )
                    params.Clear();
                }
                else
                {
                    // Remove everything but parameters
                    TArray<int16> it(route.ParamIndexes);
                    
                    for (int32 i = it.GetSize() - 1; i >= 0; --i)
                    {
                        params.RemoveAt(it[i], 1);
                    }
                }
                
                // it's ok
                
                Routing routing(route.Controller, route.Action, params, true);
                return routing;
            }
            
        continueNext:
            continue;
        }
        // Not found
        return Routing(Allocator);
    }
    
    Routing Router::FindRoute(Http::Method method, const String& uri) const
    {
        TArray< String > components(Allocator);
        
        const static char* slash = "/";
        
        uri.ParseIntoArray(components, slash, true); // for parsing
        
        return FindRoute(method, components);
    }
    
    Routing Router::FindRoute(const String& method, const TArray<String>& components) const
    {
        uint32 hash = Crc32(*method, method.Length());
        Http::Method methodType = RouteMethodCache()[hash];
        return FindRoute(methodType, components);
    }
    
    void Router::ParsePathForRoute(TArray<String>& outArray, const String& path)
    {
        const char* slash = "/";
        
        // whether start from slash position or no
        int32 start = StartsWith(*path, slash) ? 1 : 0;
        int32 length = path.Length();
        
        // remove last slash if present
        if (length > 1 and EndsWith(*path, slash))
        {
            --length;
        }
        // parse
        path.Mid(start, length - start)
            .ParseIntoArray(outArray, slash, false);
    }
    
    void Router::Clear()
    {
        Routes.Clear();
    }
    
    void Router::PrintAllRoutes()
    {
        for (const auto& route : Routes)
        {
            LogInfo.log("Nova") << "Route: \n"
                << "\tController: " << route.Controller << "\n"
                << "\tAction: " << route.Action << "\n"
                << "\t# of parameters: " << route.ParamCount << " (variable params: " << route.HasCustomParams << ")";
        }
    }
    
    String Router::FindUrlByController(const String& controller, const String& action, const TArray<String>& params) const
    {
        PROFILE_FUNCTION()
        
        if (Routes.IsEmpty())
        {
            LogWarning.log("Nova") << "FindUrlByController: No routes";
            return String::Empty;
        }
        
        for (const auto& route : Routes)
        {
            if (route.Controller == controller and route.Action == action
                and ((route.ParamCount == params.GetSize() and !route.HasCustomParams)
                     or (route.ParamCount <= params.GetSize() and route.HasCustomParams)))
            {
                return GeneratePathFromComponents(route.Components, params);
            }
        }
        return String::Empty;
    }
    
    String Router::GeneratePathFromComponents(const TArray<String>& components, const TArray<String>& params) const
    {
        String result("/", Allocator);
        int count = 0;
        
        for (const auto& component : components)
        {
            if (component == "[param]")
            {
                result += params[count++];
            }
            else if (component == "[params]")
            {
                TArray<String> left(Allocator);
                left.Append(params, count, params.GetSize() - count);
                
                result += String::Join(left, "/", Allocator);
                break;
            }
            else
            {
                result += component;
            }
            result += "/";
        }
        
        // @todo check if last character is a slash?
        return result;
    }
}

