
#include "native_file_dialog.hpp"

#include <gtk/gtk.h>
#include <thread>

namespace securepath::drum::app {

void open_wav_file_dialog(std::function<void(std::string)> callback) {
    std::thread([callback = std::move(callback)]() {
        if (!gtk_init_check(nullptr, nullptr)) {
            callback({});
            return;
        }

        GtkWidget* dialog = gtk_file_chooser_dialog_new(
            "Select WAV file",
            nullptr,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Open",   GTK_RESPONSE_ACCEPT,
            nullptr
        );

        GtkFileFilter* filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "WAV files");
        gtk_file_filter_add_pattern(filter, "*.wav");
        gtk_file_filter_add_pattern(filter, "*.WAV");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

        std::string result;
        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
            if (filename) {
                result = filename;
                g_free(filename);
            }
        }

        gtk_widget_destroy(dialog);
        while (gtk_events_pending()) {
            gtk_main_iteration();
        }

        callback(std::move(result));
    }).detach();
}

}
