// A DAW's executable directory differs from its plug-in directory.
#include <windows.h>
#include <delayimp.h>
#include <cstring>
#include <string>

extern "C" IMAGE_DOS_HEADER __ImageBase;

static FARPROC WINAPI loadSibling(unsigned event, PDelayLoadInfo info)
{
    if (event != dliNotePreLoadLibrary || std::strcmp(info->szDll, PLUGIN_RUNTIME_NAME) != 0)
        return nullptr;
    std::wstring path(32768, L'\0');
    const auto count = GetModuleFileNameW(reinterpret_cast<HMODULE>(&__ImageBase), path.data(),
                                          static_cast<DWORD>(path.size()));
    if (count == 0 || count >= path.size())
        return nullptr;
    path.resize(path.find_last_of(L'\\', count) + 1);
    const std::string name(PLUGIN_RUNTIME_NAME);
    path.append(name.begin(), name.end());
    return reinterpret_cast<FARPROC>(LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH));
}
extern "C" const PfnDliHook __pfnDliNotifyHook2 = loadSibling;
