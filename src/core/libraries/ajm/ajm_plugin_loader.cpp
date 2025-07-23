/**
 * @file ajm_plugin_loader.cpp
 * @brief AJM Plugin Loader Implementation for ShadPS4
 * 
 * This file implements the plugin loader system that manages audio codec plugins.
 * It supports both built-in plugins and dynamic loading of external plugin DLLs.
 */

#include "plugin_interface.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <string>

#ifdef _WIN32
    #include <windows.h>
    #define PLUGIN_EXTENSION ".dll"
#else
    #include <dlfcn.h>
    #define PLUGIN_EXTENSION ".so"
#endif

namespace ShadPS4::Audio {

/**
 * @brief Plugin registry entry
 */
struct PluginEntry {
    std::unique_ptr<IAudioPlugin> plugin;
    PluginInfo info;
    bool isBuiltIn;
    void* libraryHandle; // For dynamically loaded plugins
    
    PluginEntry() : plugin(nullptr), isBuiltIn(true), libraryHandle(nullptr) {}
};

/**
 * @brief AJM Plugin Loader class
 * 
 * Manages the loading, registration, and lifecycle of audio codec plugins.
 */
class AjmPluginLoader {
public:
    static AjmPluginLoader& getInstance();
    
    bool initializePlugins();
    void shutdownPlugins();
    
    bool registerBuiltInPlugin(std::unique_ptr<IAudioPlugin> plugin);
    bool loadDynamicPlugin(const std::string& pluginPath);
    void unloadDynamicPlugin(const std::string& codecType);
    
    IAudioPlugin* getPlugin(const std::string& codecType);
    std::vector<PluginInfo> getAvailablePlugins() const;
    
    bool isInitialized() const { return initialized; }

private:
    AjmPluginLoader() = default;
    ~AjmPluginLoader();
    
    // Disable copy constructor and assignment
    AjmPluginLoader(const AjmPluginLoader&) = delete;
    AjmPluginLoader& operator=(const AjmPluginLoader&) = delete;
    
    void registerBuiltInPlugins();
    void* loadLibrary(const std::string& path);
    void unloadLibrary(void* handle);
    
    std::unordered_map<std::string, PluginEntry> plugins;
    mutable std::mutex pluginMutex;
    bool initialized = false;
};

AjmPluginLoader& AjmPluginLoader::getInstance() {
    static AjmPluginLoader instance;
    return instance;
}

AjmPluginLoader::~AjmPluginLoader() {
    shutdownPlugins();
}

bool AjmPluginLoader::initializePlugins() {
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    if (initialized) {
        std::cout << "[AjmPluginLoader] Already initialized" << std::endl;
        return true;
    }
    
    std::cout << "[AjmPluginLoader] Initializing plugin system" << std::endl;
    
    // Register built-in plugins
    registerBuiltInPlugins();
    
    // TODO: Scan for dynamic plugins in plugin directory
    // This would involve scanning a plugins/ directory for .dll/.so files
    
    initialized = true;
    
    std::cout << "[AjmPluginLoader] Plugin system initialized with " 
              << plugins.size() << " plugins" << std::endl;
    
    return true;
}

void AjmPluginLoader::shutdownPlugins() {
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    if (!initialized) {
        return;
    }
    
    std::cout << "[AjmPluginLoader] Shutting down plugin system" << std::endl;
    
    // Shutdown and unload all plugins
    for (auto& pair : plugins) {
        PluginEntry& entry = pair.second;
        
        if (entry.plugin) {
            entry.plugin->shutdown();
            entry.plugin.reset();
        }
        
        // Unload dynamic libraries
        if (!entry.isBuiltIn && entry.libraryHandle) {
            unloadLibrary(entry.libraryHandle);
            entry.libraryHandle = nullptr;
        }
    }
    
    plugins.clear();
    initialized = false;
    
    std::cout << "[AjmPluginLoader] Plugin system shutdown completed" << std::endl;
}

bool AjmPluginLoader::registerBuiltInPlugin(std::unique_ptr<IAudioPlugin> plugin) {
    if (!plugin) {
        std::cerr << "[AjmPluginLoader] Error: Null plugin provided" << std::endl;
        return false;
    }
    
    PluginInfo info = plugin->getPluginInfo();
    std::string codecType = info.codecType;
    
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    // Check if plugin already exists
    if (plugins.find(codecType) != plugins.end()) {
        std::cerr << "[AjmPluginLoader] Warning: Plugin for codec " << codecType 
                  << " already registered" << std::endl;
        return false;
    }
    
    // Create plugin entry
    PluginEntry entry;
    entry.plugin = std::move(plugin);
    entry.info = info;
    entry.isBuiltIn = true;
    entry.libraryHandle = nullptr;
    
    plugins[codecType] = std::move(entry);
    
    std::cout << "[AjmPluginLoader] Registered built-in plugin: " << info.name 
              << " (v" << info.version << ") for codec " << codecType << std::endl;
    
    return true;
}

bool AjmPluginLoader::loadDynamicPlugin(const std::string& pluginPath) {
    std::cout << "[AjmPluginLoader] Loading dynamic plugin: " << pluginPath << std::endl;
    
    // Load the library
    void* handle = loadLibrary(pluginPath);
    if (!handle) {
        std::cerr << "[AjmPluginLoader] Error: Failed to load plugin library: " 
                  << pluginPath << std::endl;
        return false;
    }
    
    // Get function pointers
    CreatePluginInstanceFunc createFunc = nullptr;
    DestroyPluginInstanceFunc destroyFunc = nullptr;
    
#ifdef _WIN32
    createFunc = reinterpret_cast<CreatePluginInstanceFunc>(
        GetProcAddress(static_cast<HMODULE>(handle), "createPluginInstance"));
    destroyFunc = reinterpret_cast<DestroyPluginInstanceFunc>(
        GetProcAddress(static_cast<HMODULE>(handle), "destroyPluginInstance"));
#else
    createFunc = reinterpret_cast<CreatePluginInstanceFunc>(
        dlsym(handle, "createPluginInstance"));
    destroyFunc = reinterpret_cast<DestroyPluginInstanceFunc>(
        dlsym(handle, "destroyPluginInstance"));
#endif
    
    if (!createFunc || !destroyFunc) {
        std::cerr << "[AjmPluginLoader] Error: Plugin missing required functions" << std::endl;
        unloadLibrary(handle);
        return false;
    }
    
    // Create plugin instance
    IAudioPlugin* pluginPtr = createFunc();
    if (!pluginPtr) {
        std::cerr << "[AjmPluginLoader] Error: Failed to create plugin instance" << std::endl;
        unloadLibrary(handle);
        return false;
    }
    
    std::unique_ptr<IAudioPlugin> plugin(pluginPtr);
    PluginInfo info = plugin->getPluginInfo();
    std::string codecType = info.codecType;
    
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    // Check if plugin already exists
    if (plugins.find(codecType) != plugins.end()) {
        std::cerr << "[AjmPluginLoader] Warning: Plugin for codec " << codecType 
                  << " already registered" << std::endl;
        destroyFunc(pluginPtr);
        unloadLibrary(handle);
        return false;
    }
    
    // Create plugin entry
    PluginEntry entry;
    entry.plugin = std::move(plugin);
    entry.info = info;
    entry.isBuiltIn = false;
    entry.libraryHandle = handle;
    
    plugins[codecType] = std::move(entry);
    
    std::cout << "[AjmPluginLoader] Successfully loaded dynamic plugin: " << info.name 
              << " (v" << info.version << ") for codec " << codecType << std::endl;
    
    return true;
}

void AjmPluginLoader::unloadDynamicPlugin(const std::string& codecType) {
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    auto it = plugins.find(codecType);
    if (it == plugins.end()) {
        std::cerr << "[AjmPluginLoader] Warning: Plugin for codec " << codecType 
                  << " not found" << std::endl;
        return;
    }
    
    PluginEntry& entry = it->second;
    
    if (entry.isBuiltIn) {
        std::cerr << "[AjmPluginLoader] Warning: Cannot unload built-in plugin for codec " 
                  << codecType << std::endl;
        return;
    }
    
    std::cout << "[AjmPluginLoader] Unloading dynamic plugin for codec: " << codecType << std::endl;
    
    // Shutdown plugin
    if (entry.plugin) {
        entry.plugin->shutdown();
        entry.plugin.reset();
    }
    
    // Unload library
    if (entry.libraryHandle) {
        unloadLibrary(entry.libraryHandle);
    }
    
    plugins.erase(it);
    
    std::cout << "[AjmPluginLoader] Successfully unloaded plugin for codec: " << codecType << std::endl;
}

IAudioPlugin* AjmPluginLoader::getPlugin(const std::string& codecType) {
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    auto it = plugins.find(codecType);
    if (it == plugins.end()) {
        std::cerr << "[AjmPluginLoader] Error: No plugin found for codec: " << codecType << std::endl;
        return nullptr;
    }
    
    return it->second.plugin.get();
}

std::vector<PluginInfo> AjmPluginLoader::getAvailablePlugins() const {
    std::lock_guard<std::mutex> lock(pluginMutex);
    
    std::vector<PluginInfo> result;
    result.reserve(plugins.size());
    
    for (const auto& pair : plugins) {
        result.push_back(pair.second.info);
    }
    
    return result;
}

void AjmPluginLoader::registerBuiltInPlugins() {
    // Forward declaration of built-in plugin creation functions
    extern IAudioPlugin* createM4aacPlugin();
    
    // Register M4AAC plugin
    std::unique_ptr<IAudioPlugin> m4aacPlugin(createM4aacPlugin());
    if (m4aacPlugin) {
        registerBuiltInPlugin(std::move(m4aacPlugin));
        std::cout << "[AjmPluginLoader] Plugin M4aacDec registered" << std::endl;
    } else {
        std::cerr << "[AjmPluginLoader] Error: Failed to create M4AAC plugin" << std::endl;
    }
}

void* AjmPluginLoader::loadLibrary(const std::string& path) {
#ifdef _WIN32
    return LoadLibraryA(path.c_str());
#else
    return dlopen(path.c_str(), RTLD_LAZY);
#endif
}

void AjmPluginLoader::unloadLibrary(void* handle) {
    if (!handle) return;
    
#ifdef _WIN32
    FreeLibrary(static_cast<HMODULE>(handle));
#else
    dlclose(handle);
#endif
}

} // namespace ShadPS4::Audio

// C interface for the plugin loader
extern "C" {

/**
 * @brief Initialize the AJM plugin system
 * @return 0 on success, negative error code on failure
 */
int sceAjmInstanceCreate() {
    std::cout << "[sceAjm] Creating AJM instance" << std::endl;
    
    auto& loader = ShadPS4::Audio::AjmPluginLoader::getInstance();
    if (!loader.initializePlugins()) {
        std::cerr << "[sceAjm] Error: Failed to initialize plugin system" << std::endl;
        return -1;
    }
    
    std::cout << "[sceAjm] AJM instance created successfully" << std::endl;
    return 0;
}

/**
 * @brief Shutdown the AJM plugin system
 * @return 0 on success, negative error code on failure
 */
int sceAjmInstanceDestroy() {
    std::cout << "[sceAjm] Destroying AJM instance" << std::endl;
    
    auto& loader = ShadPS4::Audio::AjmPluginLoader::getInstance();
    loader.shutdownPlugins();
    
    std::cout << "[sceAjm] AJM instance destroyed successfully" << std::endl;
    return 0;
}

/**
 * @brief Get a plugin for the specified codec type
 * @param codecType String identifier for the codec
 * @return Pointer to the plugin, or nullptr if not found
 */
ShadPS4::Audio::IAudioPlugin* sceAjmGetPlugin(const char* codecType) {
    if (!codecType) {
        std::cerr << "[sceAjm] Error: Invalid codec type" << std::endl;
        return nullptr;
    }
    
    auto& loader = ShadPS4::Audio::AjmPluginLoader::getInstance();
    return loader.getPlugin(std::string(codecType));
}

} // extern "C"

// Built-in plugin creation function
extern "C" ShadPS4::Audio::IAudioPlugin* createM4aacPlugin() {
    // Forward declaration from plugin_m4aac.cpp
    extern ShadPS4::Audio::IAudioPlugin* createPluginInstance();
    return createPluginInstance();
}
