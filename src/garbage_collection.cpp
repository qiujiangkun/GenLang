#include <string>
#include <map>
#include <deque>
#include <queue>
#include <set>
#include <typeinfo>
#include "genlang/garbage_collection.h"
namespace GenLang {
int GC::autoClean(object *root) {
    std::set<object *> se;
    std::queue<object *> qu;
    qu.push(root);
    se.insert(root);
    while(!qu.empty())
    {
        object *dt = qu.front();
        qu.pop();
        if(!dt) continue;
        if(dt->get_type()->same_as(typeid(map_object)))
        {
            map_object *obj = (map_object *)dt;
            for (map_object::iterator it = obj->begin(); it != obj->end(); ++it) {
                if(it->second && !se.count(it->second))
                {
                    se.insert(it->second);
                    qu.push(it->second);
                }
            }
        } if(dt->get_type()->same_as(typeid(list))) {
            list *lst = (list *)dt;
            for (list::iterator it = lst->begin(); it != lst->end(); ++it) {
                if(!se.count(*it))
                {
                    se.insert(*it);
                    qu.push(*it);
                }
            }
        }
    }
    
    std::vector<object *> todel;
    int cnt = 0;
    for (auto e : objects) {
        if(!se.count(e)) {
            todel.push_back(e);
            delete e;
            ++cnt;
        }
    }
    for (auto e : todel) {
        objects.erase(e);
    }
    return cnt;
}

}