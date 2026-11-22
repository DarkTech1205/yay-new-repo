#include "Store.hpp"
#include <map>
#include <ctime>
#include <charconv>
#include <sstream>

using namespace geode::prelude;

namespace {
    std::map<std::string, int64_t>& table() {
        static std::map<std::string, int64_t> t;
        static bool loaded = false;
        if (!loaded) {
            loaded = true;
            auto raw = Mod::get()->getSavedValue<std::string>("state", "");
            std::stringstream ss(raw);
            std::string item;
            while (std::getline(ss, item, ';')) {
                auto eq = item.find('=');
                if (eq == std::string::npos || eq == 0) continue;
                int64_t v = 0;
                auto val = item.substr(eq + 1);
                auto r = std::from_chars(val.data(), val.data() + val.size(), v);
                if (r.ec != std::errc()) continue;
                t[item.substr(0, eq)] = v;
            }
        }
        return t;
    }
    uint64_t g_version = 1;
    void flush() {
        g_version++;
        std::string out;
        for (auto& [k, v] : table()) out += k + "=" + std::to_string(v) + ";";
        Mod::get()->setSavedValue<std::string>("state", out);
    }
}

namespace gcu {
    int64_t Store::get(std::string const& key, int64_t def) {
        auto& t = table(); auto it = t.find(key);
        return it == t.end() ? def : it->second;
    }
    bool Store::has(std::string const& key) { return table().contains(key); }
    void Store::set(std::string const& key, int64_t v) { table()[key] = v; flush(); }
    void Store::add(std::string const& key, int64_t d) { set(key, get(key) + d); }
    void Store::reset() { table().clear(); flush(); }
    uint64_t Store::version() { return g_version; }
    uint32_t Store::fnv(std::string const& s) { uint32_t h = 2166136261u; for (unsigned char c : s) { h ^= c; h *= 16777619u; } return h; }
    int64_t Store::now() { return static_cast<int64_t>(std::time(nullptr)); }
}
