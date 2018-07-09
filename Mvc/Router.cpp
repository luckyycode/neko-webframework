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

#include "../../Engine/Containers/AssociativeArray.h"
#include "../../Engine/Core/Log.h"

#include "../Utils.h"

namespace Neko
{
	namespace Http
    {
        bool Router::AddRoute(const Net::Http::Method method, const String& path, const String& controllerAction)
        {
            assert(method >= Net::Http::Method::Get && method <= Net::Http::Method::Patch);
            
            // assert
            if (path.Find("[params]") != INDEX_NONE && !EndsWith(*path, "[params]"))
            {
                GLogError.log("Http") << "[params] tag must be set as last parameter.";
                return false;
            }
            
            Route route(Allocator);
            
            route.Method = method;
            
            // parse path
            SplitPath(route.ComponentList, path);
            
            for (int32 i = 0; i < route.ComponentList.GetSize(); ++i)
            {
                const auto& item = route.ComponentList[i];
                if (item == "[param]")
                {
                    route.ParameterNum++;
                }
            }
            
            // multiple variable params tag
            route.HasVariableParams = route.ComponentList.Contains("[params]");
            
            for (int32 i = 0; i < route.ComponentList.GetSize(); ++i)
            {
                const String& item = route.ComponentList[i];
                // assert param tags
                if (item[0] == '[')
                {
                    if (item != "[param]" && item != "[params]")
                    {
                        return false;
                    }
                }
                else
                {
                    route.KeywordIndexes.Push(i);
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
                    GLogError.log("Http") << "Invalid controller action - " << controllerAction.c_str();
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
            
            GLogInfo.log("Http") << "Added URL route: method /" << route.Method << ", controller \"" << route.Controller.c_str() << "\", action \"" << route.Action.c_str() << "\", params: " << route.HasVariableParams;
            
            return true;
        }
        
        Routing Router::FindRouting(Net::Http::Method method, TArray<String>& components) const
        {
            if (Routes.IsEmpty())
            {
                GLogWarning.log("Http") << "FindRouting: No routes";
                
                return Routing(Allocator);
            }
            
            for (auto& route : Routes)
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
                if (route.Method == method || route.Method == Net::Http::Any)
                {
                    // Generates parameters for action
                    TArray<String> params(components);
                    
                    if (params.GetSize() == 1 && params[0].IsEmpty())
                    {
                        // empty path ( or / )
                        params.Clear();
                    }
                    else
                    {
                        // Remove everything but parameters
                        TArray<int> it(route.KeywordIndexes);
                        
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
        class RouteMethodStringMap : public TAssociativeArray<String, Net::Http::Method>
        {
        public:
            
            explicit RouteMethodStringMap(IAllocator& allocator)
            : TAssociativeArray<String, Net::Http::Method>(allocator)
            {
                Insert("match",    Net::Http::Any);
                Insert("get",      Net::Http::Get);
                Insert("post",     Net::Http::Post);
                Insert("put",      Net::Http::Put);
                Insert("patch",    Net::Http::Patch);
                Insert("delete",   Net::Http::Delete);
                Insert("trace",    Net::Http::Trace);
                Insert("connect",  Net::Http::Connect);
                Insert("patch",    Net::Http::Patch);
            }
        };
        
        static RouteMethodStringMap& RouteMethodCache()
        {
            static DefaultAllocator allocator;
            static RouteMethodStringMap hash(allocator);
            return hash;
        }
        
        Routing Router::FindRouting(const String& method, TArray<String>& components) const
        {
            Net::Http::Method methodType = RouteMethodCache().Get(method);
            return FindRouting(methodType, components);
        }
        
        void Router::SplitPath(TArray<String>& outArray, const String& path)
        {
            const char* slash = "/";
            
            // whether start from slash position or no
            int start = StartsWith(*path, slash) ? 1 : 0;
            int length = path.Length();
            
            // remove last slash if present
            if (length > 1 && EndsWith(*path, slash))
            {
                --length;
            }
            // parse
            String substr = path.Mid(start, length - start);
            substr.ParseIntoArray(outArray, slash, false);
        }
        
        void Router::Clear()
        {
            Routes.Clear();
        }
        
        void Router::PrintAllRoutes()
        {
            for (const auto& route : Routes)
            {
                GLogInfo.log("Http") << "Route: \n"
                    << "\tController: " << route.Controller << "\n"
                    << "\tAction: " << route.Action << "\n"
                    << "\t# of parameters: " << route.ParameterNum << " (variable params: " << route.HasVariableParams << ")";
            }
        }
        
        String Router::FindUrl(const String& controller, const String& action, const TArray<String>& params) const
        {
            if (Routes.IsEmpty())
            {
                return String(Allocator);
            }
            
            for (const auto& route : Routes)
            {
                if (route.Controller == controller && route.Action == action
                    && ((route.ParameterNum == params.GetSize() && !route.HasVariableParams)
                        || (route.ParameterNum <= params.GetSize() && route.HasVariableParams)))
                {
                    return GeneratePath(route.ComponentList, params);
                }
            }
            
            return String(Allocator);
        }
        
        String Router::GeneratePath(const TArray<String>& components, const TArray<String>& params) const
        {
            String result("/", Allocator);
            int count = 0;
            
            for (auto& c : components)
            {
                if (c == "[param]")
                {
                    result += params[count++];
                }
                else if (c == "[params]")
                {
                    TArray<String> left(Allocator);
                    left.Append(params, count, params.GetSize() - count);
                    
                    result += String::Join(left, "/", Allocator);
                    break;
                }
                else
                {
                    result += c;
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
