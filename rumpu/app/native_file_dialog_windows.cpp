
#include "native_file_dialog.hpp"

#include <windows.h>
#include <shobjidl.h>
#include <thread>

namespace securepath::drum::app {

void open_wav_file_dialog(std::function<void(std::string)> callback) {
    std::thread([callback = std::move(callback)]() {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        std::string result;
        IFileOpenDialog* dialog = nullptr;

        if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_ALL,
                                       IID_IFileOpenDialog, reinterpret_cast<void**>(&dialog)))) {
            COMDLG_FILTERSPEC filters[] = {
                {L"WAV Files", L"*.wav;*.WAV"},
                {L"All Files", L"*.*"}
            };
            dialog->SetFileTypes(2, filters);
            dialog->SetDefaultExtension(L"wav");

            if (SUCCEEDED(dialog->Show(nullptr))) {
                IShellItem* item = nullptr;
                if (SUCCEEDED(dialog->GetResult(&item))) {
                    PWSTR path = nullptr;
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                        int size = WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                                       nullptr, 0, nullptr, nullptr);
                        result.resize(size - 1);
                        WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                            result.data(), size, nullptr, nullptr);
                        CoTaskMemFree(path);
                    }
                    item->Release();
                }
            }
            dialog->Release();
        }

        CoUninitialize();
        callback(std::move(result));
    }).detach();
}

template<typename DialogT, REFCLSID clsid, REFIID iid>
static void run_project_dialog(DWORD extra_options, const wchar_t* default_name, std::function<void(std::string)> callback) {
    std::thread([extra_options, default_name, callback = std::move(callback)]() {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        std::string result;
        DialogT* dialog = nullptr;

        if (SUCCEEDED(CoCreateInstance(clsid, nullptr, CLSCTX_ALL, iid, reinterpret_cast<void**>(&dialog)))) {
            COMDLG_FILTERSPEC filters[] = {
                {L"Rumpu Project Files", L"*.spd"},
                {L"All Files", L"*.*"}
            };
            dialog->SetFileTypes(2, filters);
            dialog->SetDefaultExtension(L"spd");

            if (extra_options) {
                FILEOPENDIALOGOPTIONS opts{};
                dialog->GetOptions(&opts);
                dialog->SetOptions(opts | extra_options);
            }

            if (default_name) {
                dialog->SetFileName(default_name);
            }

            if (SUCCEEDED(dialog->Show(nullptr))) {
                IShellItem* item = nullptr;
                if (SUCCEEDED(dialog->GetResult(&item))) {
                    PWSTR path = nullptr;
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                        int size = WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                                       nullptr, 0, nullptr, nullptr);
                        result.resize(size - 1);
                        WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                            result.data(), size, nullptr, nullptr);
                        CoTaskMemFree(path);
                    }
                    item->Release();
                }
            }
            dialog->Release();
        }

        CoUninitialize();
        callback(std::move(result));
    }).detach();
}

void save_wav_file_dialog(std::function<void(std::string)> callback) {
    std::thread([callback = std::move(callback)]() {
        CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

        std::string result;
        IFileSaveDialog* dialog = nullptr;

        if (SUCCEEDED(CoCreateInstance(CLSID_FileSaveDialog, nullptr, CLSCTX_ALL,
                                       IID_IFileSaveDialog, reinterpret_cast<void**>(&dialog)))) {
            COMDLG_FILTERSPEC filters[] = {
                {L"WAV Files", L"*.wav;*.WAV"},
                {L"All Files", L"*.*"}
            };
            dialog->SetFileTypes(2, filters);
            dialog->SetDefaultExtension(L"wav");

            FILEOPENDIALOGOPTIONS opts{};
            dialog->GetOptions(&opts);
            dialog->SetOptions(opts | FOS_OVERWRITEPROMPT);
            dialog->SetFileName(L"export.wav");

            if (SUCCEEDED(dialog->Show(nullptr))) {
                IShellItem* item = nullptr;
                if (SUCCEEDED(dialog->GetResult(&item))) {
                    PWSTR path = nullptr;
                    if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &path))) {
                        int size = WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                                       nullptr, 0, nullptr, nullptr);
                        result.resize(size - 1);
                        WideCharToMultiByte(CP_UTF8, 0, path, -1,
                                            result.data(), size, nullptr, nullptr);
                        CoTaskMemFree(path);
                    }
                    item->Release();
                }
            }
            dialog->Release();
        }

        CoUninitialize();
        callback(std::move(result));
    }).detach();
}

void open_project_file_dialog(std::function<void(std::string)> callback) {
    run_project_dialog<IFileOpenDialog, CLSID_FileOpenDialog, IID_IFileOpenDialog>(0, nullptr, std::move(callback));
}

void save_project_file_dialog(std::function<void(std::string)> callback) {
    run_project_dialog<IFileSaveDialog, CLSID_FileSaveDialog, IID_IFileSaveDialog>(FOS_OVERWRITEPROMPT, L"untitled.spd", std::move(callback));
}

}
