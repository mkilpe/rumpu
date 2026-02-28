
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

}
