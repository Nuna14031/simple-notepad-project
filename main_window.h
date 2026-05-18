#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "spell_checker.h"
#include "spell_checker_highlighter.h"
#include "text_transform.h"

#include <QDialog>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QTextEdit>
#include <memory>
#include <vector>

namespace Ui {
class find_replace_dialog;
class word_frequency_dialog;
}

class main_window : public QMainWindow {
    Q_OBJECT

public:
    main_window();
    ~main_window() override;

private:
    // UI
    QTextEdit* editor = nullptr;
    QDialog* find_replace_dlg = nullptr;
    std::unique_ptr<Ui::find_replace_dialog> find_replace_ui;

    // Text transforms
    std::vector<std::unique_ptr<text_transform>> transforms;

    // State
    QString current_file;

    // Spell checker
    spell_checker checker { "data/words.txt" };
    spell_checker_highlighter* spell_highlighter = nullptr;

    // Recent files
    static constexpr int max_recent_files = 5;
    QMenu* recent_files_menu = nullptr;
    QSettings settings { "Notepad", "Notepad" };

    // Zoom
    int zoom_factor = 0; // steps from default

    // Setup
    void setup_file_menu();
    void setup_edit_menu();
    void setup_format_menu();
    void setup_format_toolbar();
    void setup_search_menu();
    void setup_tools_menu();
    void setup_view_menu();

    // File ops
    void open_file(const QString& path = {});
    void save_file();
    void save_file_as();
    void update_title();

    // Recent files
    void update_recent_files_menu();
    void add_to_recent_files(const QString& path);

    // Status bar
    void update_status_bar() const;

    // Find/replace
    void show_find_replace_dialog();
    void find_next(const QString& term, QTextDocument::FindFlags flags) const;
    void replace_current(const QString& term, const QString& replacement, QTextDocument::FindFlags flags) const;
    void replace_all(const QString& term, const QString& replacement, QTextDocument::FindFlags flags) const;

    // Tools
    void show_word_frequency();
    void show_spell_check();

    // Transform
    void apply_transform(const text_transform& transform) const;

    // Context menu
    void show_context_menu(const QPoint& pos);
};

#endif // MAIN_WINDOW_H

