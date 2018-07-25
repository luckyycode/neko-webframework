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
//  _   _      _           _____                                            _
// | \ | | ___| | _____   |  ___| __ __ _ _ __ ___   _____      _____  _ __| | __
// |  \| |/ _ \ |/ / _ \  | |_ | '__/ _` | '_ ` _ \ / _ \ \ /\ / / _ \| '__| |/ /
// | |\  |  __/   < (_) | |  _|| | | (_| | | | | | |  __/\ V  V / (_) | |  |   <
// |_| \_|\___|_|\_\___/  |_|  |_|  \__,_|_| |_| |_|\___| \_/\_/ \___/|_|  |_|\_\
//
//  Router.cpp
//  Neko Framework
//
//  Copyright © 2018 Neko Vision. All rights reserved.
//

#include "Router.h"

#include "Engine/Containers/AssociativeArray.h"
#include "Engine/Core/Profiler.h"
#include "Engine/Core/Log.h"

#include "../Utils.h"

#define ROUTE_PARAMS_TAG    "[params]"
#define ROUTE_PARAM_TAG     "[param]"

namespace Neko
{
    using namespace Neko::Skylar;
    namespace Nova
    {
        bool Router::AddRoute(const Http::Method method, const String& path, const String& controllerAction)
        {
            assert(method >= Http::Method::Get && method <= Http::Method::Patch);
            
            // assert
            if (path.Find(ROUTE_PARAMS_TAG) != INDEX_NONE and !EndsWith(*path, ROUTE_PARAMS_TAG))
            {
                LogError.log("Nova") << "[params] tag must be set as last parameter.";
                return false;
            }
            
            Route route(Allocator);
            
            route.Method = method;
            
            // parse path
            SplitPath(route.ComponentList, path);
            
            for (int32 i = 0; i < route.ComponentList.GetSize(); ++i)
            {
                const auto& item = route.ComponentList[i];
                
                // count amount of param tags in url
                if (item == ROUTE_PARAM_TAG)
                {
                    route.ParameterNum++;
                }
            }
            
            // multiple variable params tag
            route.HasVariableParams = route.ComponentList.Contains(ROUTE_PARAMS_TAG);
            
            for (int32 i = 0; i < route.ComponentList.GetSize(); ++i)
            {
                const String& item = route.ComponentList[i];
                
                // assert param tags
                if (item[0] == '[')
                {
                    if (item != ROUTE_PARAM_TAG && item != ROUTE_PARAMS_TAG)
                    {
                        LogError.log("Nova") << "Found tag begin symbol '[' but didn't match any of existing ones.";
                        
                        return false;
                    }
                }
                else
                {
                    route.KeywordIndexes.Push((int16)i);
                }
            }
            
            if (controllerAction[0] != '/')
            {
                // parse controller and action
                TArray<String> list(Allocator);
                controllerAction.ParseIntoArray(list, "#", false);
                
                if (list.GetSize() == 2)
                {
                    // full controller name
                    route.Controller = list[0].ToLower();
                    route.Controller += "controller";
                    
                    route.Action = list[1];
                }
                else
                {
                    LogError.log("Nova") << "Invalid controller action - " << *controllerAction;
                    return false;
                }
            }
            else
            {
                route.Controller = controllerAction;
            }
            
            // @todo perhaps check for dupes?
            
            // save
            Routes.Push(route);
            
            LogInfo.log("Nova") << "Added Url route: " << *path << ", method /" << route.Method << ", controller \"" << route.Controller.c_str() << "\", action \"" << route.Action.c_str() << "\", params: " << route.HasVariableParams;
            
            return true;
        }
        
        Routing Router::FindRouting(Http::Method method, TArray<String>& components) const
        {
            PROFILE_FUNCTION()
            
            if (this->Routes.IsEmpty())
            {
                LogWarning.log("Nova") << "FindRouting: No routes";
                
                return Routing(Allocator);
            }
            
            for (auto& route : this->Routes)
            {
                // check lengths
                if (route.HasVariableParams)
                {
                    // ignore last token
                    if (route.ComponentList.GetSize() - 1 > components.GetSize())
                    {
                        continue;
                    }
                }
                else
                {
                    if (route.ComponentList.GetSize() != components.GetSize())
                    {
                        continue;
                    }
                }
                
                // compare nodes
                for (int32 index : route.KeywordIndexes)
                {
                    if (route.ComponentList[index] != components[index])
                    {
                        // continue outside
                        goto continueNext;
                    }
                }
                
                // check if method types match
                if (route.Method == method or route.Method == Http::Any)
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
                        TArray<int16> it(route.KeywordIndexes);
                        
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
        
        // uh oh no other way
        class RouteMethodStringMap : public TAssociativeArray<String, Http::Method>
        {
        public:
            
            explicit RouteMethodStringMap(IAllocator& allocator)
            : TAssociativeArray<String, Http::Method>(allocator)
            {
                Insert("match",    Http::Any);
                Insert("get",      Http::Get);
                Insert("post",     Http::Post);
                Insert("put",      Http::Put);
                Insert("patch",    Http::Patch);
                Insert("delete",   Http::Delete);
                Insert("trace",    Http::Trace);
                Insert("connect",  Http::Connect);
                Insert("patch",    Http::Patch);
            }
        };
        
        static RouteMethodStringMap& RouteMethodCache()
        {
            static DefaultAllocator allocator;
            static RouteMethodStringMap hash(allocator);
            return hash;
        }
        
        Routing Router::FindRouting(const String& method, const String& uri) const
        {
            TArray< String > components(Allocator);
            
            Router::SplitPath(components, uri); // for parsing
            
            Http::Method methodType = RouteMethodCache().Get(method);
            return FindRouting(methodType, components);
        }
        
        Routing Router::FindRouting(const String& method, TArray<String>& components) const
        {
            Http::Method methodType = RouteMethodCache().Get(method);
            return FindRouting(methodType, components);
        }
        
        void Router::SplitPath(TArray<String>& outArray, const String& path)
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
            path.Mid(start, length - start).ParseIntoArray(outArray, slash, false);
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
                    << "\t# of parameters: " << route.ParameterNum << " (variable params: " << route.HasVariableParams << ")";
            }
        }
        
        String Router::FindUrl(const String& controller, const String& action, const TArray<String>& params) const
        {
            PROFILE_FUNCTION()
            
            if (Routes.IsEmpty())
            {
                LogWarning.log("Nova") << "FindUrl: No routes";
                
                return String::Empty;
            }
            
            for (const auto& route : Routes)
            {
                if (route.Controller == controller and route.Action == action
                    and ((route.ParameterNum == params.GetSize() and !route.HasVariableParams)
                        or (route.ParameterNum <= params.GetSize() and route.HasVariableParams)))
                {
                    return GeneratePath(route.ComponentList, params);
                }
            }
            
            return String::Empty;
        }
        
        String Router::GeneratePath(const TArray<String>& components, const TArray<String>& params) const
        {
            String result("/", Allocator);
            int count = 0;
            
            for (auto& component : components)
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
//            // remove last character
//            if (result.Length() > 1)
//            {
//                result.Erase(result.Length() - 1, 1);
//            }
            
            return result;
        }
    }
}
