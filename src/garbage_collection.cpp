#include <string>
#include <map>
#include <deque>
#include <queue>
#include <set>
#include <typeinfo>
#include <genlang/garbage_collection.h>
#include <genlang/runtime_support.h>

#include "genlang/garbage_collection.h"

namespace GenLang {
    int garbage_collector::auto_clean() {
        std::set<object *> vis;
        std::queue<object *> qu;
        for (auto e : roots) {
            qu.push(*e);
            vis.insert(*e);
        }
        while (!qu.empty()) {
            object *obj = qu.front();
            qu.pop();
            if (!obj || vis.count(obj)) continue;
            obj->gc_walk(vis, qu);

        }

        std::vector<object *> todel;
        int cnt = 0;
        for (auto e : objects) {
            if (!vis.count(e)) {
                todel.push_back(e);
                class_manager::instance().erase_type(e);
                delete e;
                ++cnt;
            }
        }
        for (auto e : todel) {
            objects.erase(e);
        }
        std::cerr << "deleted " << cnt << "" << std::endl;
        return cnt;
    }


    object *garbage_collector::detach(object *dt) {
        objects.erase(dt);
        return dt;
    }

    void garbage_collector::detach(garbage_collector &gc) {
        gc.objects.insert(objects.begin(), objects.end());
        objects.clear();
    }

    void garbage_collector::attach_root_ptr(object **pPtr) {
        roots.insert(pPtr);
    }

    void garbage_collector::detach_root_ptr(object **pPtr) {
        roots.erase(pPtr);
    }

    garbage_collector &garbage_collector::instance() {
        static garbage_collector inst;
        return inst;

    }

    garbage_collector::garbage_collector() {}

    garbage_collector::~garbage_collector() {
        auto_clean();
    }


    void garbage_collector::signin(object *t) {
        objects.insert(t);
    }

    void class_manager::push(meta_object &o) {
        std::cerr << this << " registered " << o.name << " " << o.info.name() << std::endl;
        objs.push_back(&o);
    }

    meta_object *class_manager::find(const char *name) {
        if (!name)
            return nullptr;
        for (auto &obj : objs)
            if (std::strcmp(name, obj->name) == 0)
                return obj;
        return nullptr;
    }

    meta_object *class_manager::find(const std::type_info &info) {
        for (auto &obj : objs)
            if (obj->info == info)
                return obj;
        return nullptr;
    }

    class_manager::~class_manager() {
        for (auto e : objs)
            delete e;
    }

    void class_manager::put_type(void *o, meta_object *meta) {
        types[o] = meta;
    }

    void class_manager::erase_type(void *o) {
        types.erase(o);
    }

    string class_manager::find_name(void *p) {
        if(!p)
            return "NULL";
        return types[p]->name;
    }

    class_manager &class_manager::instance() {
        static class_manager inst;
        return inst;

    }

    class_manager::class_manager() {}


}
