
#include "native_file_dialog.hpp"

#include <gtk/gtk.h>
#include <thread>

namespace securepath::drum::app {

static void run_project_dialog(GtkFileChooserAction action, const char* title, const char* accept_label, std::function<void(std::string)> callback) {
    std::thread([action, title, accept_label, callback = std::move(callback)]() {
        if (!gtk_init_check(nullptr, nullptr)) {
            callback({});
            return;
        }

        GtkWidget* dialog = gtk_file_chooser_dialog_new(
            title,
            nullptr,
            action,
            "_Cancel", GTK_RESPONSE_CANCEL,
            accept_label, GTK_RESPONSE_ACCEPT,
            nullptr
        );

        GtkFileFilter* filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "Rumpu project files");
        gtk_file_filter_add_pattern(filter, "*.spd");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

        if (action == GTK_FILE_CHOOSER_ACTION_SAVE) {
            gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
            gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "untitled.spd");
        }

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

void save_wav_file_dialog(std::function<void(std::string)> callback) {
    std::thread([callback = std::move(callback)]() {
        if (!gtk_init_check(nullptr, nullptr)) {
            callback({});
            return;
        }

        GtkWidget* dialog = gtk_file_chooser_dialog_new(
            "Export as WAV",
            nullptr,
            GTK_FILE_CHOOSER_ACTION_SAVE,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Export", GTK_RESPONSE_ACCEPT,
            nullptr
        );

        GtkFileFilter* filter = gtk_file_filter_new();
        gtk_file_filter_set_name(filter, "WAV files");
        gtk_file_filter_add_pattern(filter, "*.wav");
        gtk_file_filter_add_pattern(filter, "*.WAV");
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
        gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);
        gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "export.wav");

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

void open_project_file_dialog(std::function<void(std::string)> callback) {
    run_project_dialog(GTK_FILE_CHOOSER_ACTION_OPEN, "Open project", "_Open", std::move(callback));
}

void save_project_file_dialog(std::function<void(std::string)> callback) {
    run_project_dialog(GTK_FILE_CHOOSER_ACTION_SAVE, "Save project", "_Save", std::move(callback));
}

void open_folder_dialog(std::function<void(std::string)> callback) {
    std::thread([callback = std::move(callback)]() {
        if (!gtk_init_check(nullptr, nullptr)) {
            callback({});
            return;
        }

        GtkWidget* dialog = gtk_file_chooser_dialog_new(
            "Select folder",
            nullptr,
            GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            "_Cancel", GTK_RESPONSE_CANCEL,
            "_Select", GTK_RESPONSE_ACCEPT,
            nullptr
        );

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
