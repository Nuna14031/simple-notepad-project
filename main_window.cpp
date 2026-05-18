#include "main_window.h"

#include "notepad_exception.h"
#include "ui_find_replace_dialog.h"
#include "ui_word_frequency_dialog.h"

#include <QAction>
#include <QApplication>
#include <QFile>
#include <QFileDialog>
#include <QFont>
#include <QFontDialog>
#include <QHeaderView>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTableWidgetItem>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QTextStream>
#include <QToolBar>
#include <algorithm>
#include <map>
#include <sstream>
#include <string>
#include <vector>

main_window::main_window()
{
    setWindowTitle("Notepad");
    resize(800, 600);

    editor = new QTextEdit(this);
    setCentralWidget(editor);

    transforms.push_back(std::make_unique<uppercase_transform>());
    transforms.push_back(std::make_unique<lowercase_transform>());
    transforms.push_back(std::make_unique<capitalize_transform>());
    transforms.push_back(std::make_unique<sentence_case_transform>());
    transforms.push_back(std::make_unique<swap_case_transform>());

    spell_highlighter = new spell_checker_highlighter(checker, editor->document());

    editor->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(editor, &QTextEdit::customContextMenuRequested, this, &main_window::show_context_menu);

    setup_file_menu();
    setup_edit_menu();
    setup_format_menu();
    setup_format_toolbar();
    setup_search_menu();
    setup_tools_menu();
    setup_view_menu();

    update_status_bar();
    update_recent_files_menu();
}

main_window::~main_window() = default;

void main_window::setup_file_menu()
{
    auto* file_menu = menuBar()->addMenu("File");

    const auto* action_new = file_menu->addAction("New");
    connect(action_new, &QAction::triggered, this, [this] {
        editor->clear();
        current_file.clear();
        update_title();
    });

    file_menu->addSeparator();

    const auto* action_open = file_menu->addAction("Open...");
    connect(action_open, &QAction::triggered, this, [this] {
        open_file();
    });

    const auto* action_save = file_menu->addAction("Save");
    connect(action_save, &QAction::triggered, this, [this] {
        save_file();
    });

    const auto* action_save_as = file_menu->addAction("Save As...");
    connect(action_save_as, &QAction::triggered, this, [this] {
        save_file_as();
    });

    file_menu->addSeparator();

    recent_files_menu = file_menu->addMenu("Recent Files");

    file_menu->addSeparator();

    const auto* action_exit = file_menu->addAction("Exit");
    connect(action_exit, &QAction::triggered, this, [] {
        QApplication::quit();
    });
}

void main_window::setup_edit_menu()
{
    auto* edit_menu = menuBar()->addMenu("Edit");

    auto* action_undo = edit_menu->addAction("Undo");
    action_undo->setShortcut(QKeySequence::Undo);
    connect(action_undo, &QAction::triggered, editor, &QTextEdit::undo);

    auto* action_redo = edit_menu->addAction("Redo");
    action_redo->setShortcut(QKeySequence::Redo);
    connect(action_redo, &QAction::triggered, editor, &QTextEdit::redo);

    edit_menu->addSeparator();

    auto* action_cut = edit_menu->addAction("Cut");
    action_cut->setShortcut(QKeySequence::Cut);
    connect(action_cut, &QAction::triggered, editor, &QTextEdit::cut);

    auto* action_copy = edit_menu->addAction("Copy");
    action_copy->setShortcut(QKeySequence::Copy);
    connect(action_copy, &QAction::triggered, editor, &QTextEdit::copy);

    auto* action_paste = edit_menu->addAction("Paste");
    action_paste->setShortcut(QKeySequence::Paste);
    connect(action_paste, &QAction::triggered, editor, &QTextEdit::paste);

    edit_menu->addSeparator();

    auto* action_select_all = edit_menu->addAction("Select All");
    action_select_all->setShortcut(QKeySequence::SelectAll);
    connect(action_select_all, &QAction::triggered, editor, &QTextEdit::selectAll);

    connect(editor, &QTextEdit::textChanged, this, [this] {
        update_status_bar();
    });
}

void main_window::setup_format_menu()
{
    auto* format_menu = menuBar()->addMenu("Format");

    const auto* action_font = format_menu->addAction("Font...");
    connect(action_font, &QAction::triggered, this, [this] {
        bool ok = false;
        QFont current_font = editor->font();
        QFont selected_font = QFontDialog::getFont(&ok, current_font, this, "Font");
        if (!ok) {
            return;
        }

        QTextCursor cursor = editor->textCursor();
        if (cursor.hasSelection()) {
            QTextCharFormat fmt;
            fmt.setFont(selected_font);
            cursor.mergeCharFormat(fmt);
        } else {
            editor->setFont(selected_font);
        }
    });

    format_menu->addSeparator();

    auto* text_case_menu = format_menu->addMenu("Text Case");
    for (const auto& transform : transforms) {
        const auto* action = text_case_menu->addAction(QString::fromStdString(transform->name()));
        connect(action, &QAction::triggered, this, [this, &transform] {
            apply_transform(*transform);
        });
    }
}

void main_window::setup_format_toolbar()
{
    auto* toolbar = addToolBar("Format");
    toolbar->setIconSize(QSize(16, 16));

    auto* action_bold = toolbar->addAction(QIcon("data/images/bold.svg"), "Bold");
    action_bold->setCheckable(true);
    action_bold->setShortcut(QKeySequence("Ctrl+B"));
    connect(action_bold, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat fmt;
        fmt.setFontWeight(checked ? QFont::Bold : QFont::Normal);
        editor->mergeCurrentCharFormat(fmt);
    });

    auto* action_italic = toolbar->addAction(QIcon("data/images/italic.svg"), "Italic");
    action_italic->setCheckable(true);
    action_italic->setShortcut(QKeySequence("Ctrl+I"));
    connect(action_italic, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat fmt;
        fmt.setFontItalic(checked);
        editor->mergeCurrentCharFormat(fmt);
    });

    auto* action_underline = toolbar->addAction(QIcon("data/images/underline.svg"), "Underline");
    action_underline->setCheckable(true);
    action_underline->setShortcut(QKeySequence("Ctrl+U"));
    connect(action_underline, &QAction::triggered, this, [this](bool checked) {
        QTextCharFormat fmt;
        fmt.setFontUnderline(checked);
        editor->mergeCurrentCharFormat(fmt);
    });

    connect(editor, &QTextEdit::currentCharFormatChanged,
        this, [action_bold, action_italic, action_underline](const QTextCharFormat& fmt) {
            action_bold->setChecked(fmt.fontWeight() == QFont::Bold);
            action_italic->setChecked(fmt.fontItalic());
            action_underline->setChecked(fmt.fontUnderline());
        });
}

void main_window::setup_search_menu()
{
    auto* search_menu = menuBar()->addMenu("Search");

    auto* action_find_replace = search_menu->addAction("Find / Replace...");
    action_find_replace->setShortcut(QKeySequence::Find);
    connect(action_find_replace, &QAction::triggered, this, [this] {
        show_find_replace_dialog();
    });
}

void main_window::setup_tools_menu()
{
    auto* tools_menu = menuBar()->addMenu("Tools");

    const auto* action_word_freq = tools_menu->addAction("Word Frequency...");
    connect(action_word_freq, &QAction::triggered, this, [this] {
        show_word_frequency();
    });

    const auto* action_spell = tools_menu->addAction("Check Spelling...");
    connect(action_spell, &QAction::triggered, this, [this] {
        show_spell_check();
    });
}

void main_window::setup_view_menu()
{
    auto* view_menu = menuBar()->addMenu("View");

    auto* action_zoom_in = view_menu->addAction("Zoom In");
    action_zoom_in->setShortcut(QKeySequence("Ctrl++"));
    connect(action_zoom_in, &QAction::triggered, this, [this] {
        editor->zoomIn(2);
        zoom_factor++;
    });

    auto* action_zoom_out = view_menu->addAction("Zoom Out");
    action_zoom_out->setShortcut(QKeySequence("Ctrl+-"));
    connect(action_zoom_out, &QAction::triggered, this, [this] {
        editor->zoomOut(2);
        zoom_factor--;
    });

    auto* action_zoom_reset = view_menu->addAction("Reset Zoom");
    action_zoom_reset->setShortcut(QKeySequence("Ctrl+0"));
    connect(action_zoom_reset, &QAction::triggered, this, [this] {
        if (zoom_factor > 0) {
            editor->zoomOut(zoom_factor * 2);
        } else if (zoom_factor < 0) {
            editor->zoomIn(-zoom_factor * 2);
        }
        zoom_factor = 0;
    });
}

void main_window::open_file(const QString& path)
{
    QString file_path = path;
    if (file_path.isEmpty()) {
        file_path = QFileDialog::getOpenFileName(this, "Open File");
    }
    if (file_path.isEmpty()) {
        return;
    }
    try {
        QFile file(file_path);
        if (!file.exists()) {
            throw file_not_found_exception(file_path.toStdString());
        }
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw file_read_exception(file_path.toStdString());
        }
        QTextStream in(&file);
        QString contents = in.readAll();
        if (in.status() != QTextStream::Ok) {
            throw file_read_exception(file_path.toStdString());
        }
        editor->setPlainText(contents);
        current_file = file_path;
        update_title();
        add_to_recent_files(file_path);
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
    }
}

void main_window::save_file()
{
    if (current_file.isEmpty()) {
        save_file_as();
        return;
    }
    try {
        QFile file(current_file);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            throw file_write_exception(current_file.toStdString());
        }
        QTextStream out(&file);
        out << editor->toPlainText();
    } catch (const notepad_exception& ex) {
        QMessageBox::critical(this, "Error", ex.what());
    }
}

void main_window::save_file_as()
{
    QString path = QFileDialog::getSaveFileName(this, "Save File As");
    if (path.isEmpty()) {
        return;
    }
    current_file = path;
    save_file();
    update_title();
    add_to_recent_files(path);
}

void main_window::update_title()
{
    if (current_file.isEmpty()) {
        setWindowTitle("Notepad");
    } else {
        setWindowTitle("Notepad: " + current_file);
    }
}

void main_window::add_to_recent_files(const QString& path)
{
    QStringList recent = settings.value("recent_files").toStringList();
    recent.removeAll(path);
    recent.prepend(path);
    while (recent.size() > 5) {
        recent.removeLast();
    }
    settings.setValue("recent_files", recent);
    update_recent_files_menu();
}

void main_window::update_recent_files_menu()
{
    if (!recent_files_menu) {
        return;
    }
    recent_files_menu->clear();

    QStringList recent = settings.value("recent_files").toStringList();
    if (recent.isEmpty()) {
        QAction* empty = recent_files_menu->addAction("(empty)");
        empty->setEnabled(false);
        return;
    }

    for (const QString& path : recent) {
        QAction* action = recent_files_menu->addAction(path);
        connect(action, &QAction::triggered, this, [this, path] {
            open_file(path);
        });
    }

    recent_files_menu->addSeparator();
    QAction* clear = recent_files_menu->addAction("Clear Recent Files");
    connect(clear, &QAction::triggered, this, [this] {
        settings.remove("recent_files");
        update_recent_files_menu();
    });
}

void main_window::update_status_bar() const
{
    QString text = editor->toPlainText();
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    int word_count = words.size();
    int line_count = text.isEmpty() ? 0 : text.count('\n') + 1;
    statusBar()->showMessage(QString("Words: %1  Lines: %2").arg(word_count).arg(line_count));
}

void main_window::show_find_replace_dialog()
{
    if (!find_replace_dlg) {
        find_replace_dlg = new QDialog(this);
        find_replace_ui = std::make_unique<Ui::find_replace_dialog>();
        find_replace_ui->setupUi(find_replace_dlg);

        auto current_flags = [this] {
            auto flags = QTextDocument::FindFlags();
            if (find_replace_ui->case_sensitive_check->isChecked()) {
                flags |= QTextDocument::FindCaseSensitively;
            }
            return flags;
        };

        connect(find_replace_ui->find_next_button, &QPushButton::clicked,
            find_replace_dlg, [this, current_flags] {
                find_next(find_replace_ui->find_input->text(), current_flags());
            });
        connect(find_replace_ui->replace_button, &QPushButton::clicked,
            find_replace_dlg, [this, current_flags] {
                replace_current(find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(), current_flags());
            });
        connect(find_replace_ui->replace_all_button, &QPushButton::clicked,
            find_replace_dlg, [this, current_flags] {
                replace_all(find_replace_ui->find_input->text(),
                    find_replace_ui->replace_input->text(), current_flags());
            });
        connect(find_replace_ui->close_button, &QPushButton::clicked,
            find_replace_dlg, [this] { find_replace_dlg->hide(); });
    }

    find_replace_dlg->show();
    find_replace_dlg->raise();
    find_replace_dlg->activateWindow();
}

void main_window::find_next(const QString& term, const QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto found = editor->document()->find(term, editor->textCursor(), flags);
    if (found.isNull()) {
        auto from_start = editor->textCursor();
        from_start.movePosition(QTextCursor::Start);
        found = editor->document()->find(term, from_start, flags);
    }
    if (!found.isNull()) {
        editor->setTextCursor(found);
    }
}

void main_window::replace_current(const QString& term, const QString& replacement,
    const QTextDocument::FindFlags flags) const
{
    if (auto cursor = editor->textCursor(); cursor.hasSelection()) {
        cursor.insertText(replacement);
        editor->setTextCursor(cursor);
    }
    find_next(term, flags);
}

void main_window::replace_all(const QString& term, const QString& replacement,
    const QTextDocument::FindFlags flags) const
{
    if (term.isEmpty()) {
        return;
    }
    auto start_cursor = editor->textCursor();
    start_cursor.movePosition(QTextCursor::Start);
    editor->setTextCursor(start_cursor);

    while (true) {
        auto found = editor->document()->find(term, editor->textCursor(), flags);
        if (found.isNull()) {
            break;
        }
        editor->setTextCursor(found);
        auto c = editor->textCursor();
        c.insertText(replacement);
        editor->setTextCursor(c);
    }
}

void main_window::show_word_frequency()
{
    QString text = editor->toPlainText().toLower();
    std::string std_text = text.toStdString();

    std::map<std::string, int> freq;
    std::istringstream stream(std_text);
    std::string word;
    while (stream >> word) {
        std::erase_if(word, [](unsigned char c) { return !std::isalpha(c); });
        if (!word.empty()) {
            freq[word]++;
        }
    }

    std::vector<std::pair<std::string, int>> sorted_freq(freq.begin(), freq.end());
    std::sort(sorted_freq.begin(), sorted_freq.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });

    auto* dialog = new QDialog(this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    Ui::word_frequency_dialog ui;
    ui.setupUi(dialog);

    ui.frequency_table->verticalHeader()->setVisible(false);
    ui.frequency_table->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    ui.frequency_table->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);

    ui.frequency_table->setRowCount(static_cast<int>(sorted_freq.size()));
    for (int i = 0; i < static_cast<int>(sorted_freq.size()); ++i) {
        const auto& [w, count] = sorted_freq[i];
        ui.frequency_table->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(w)));
        auto* count_item = new QTableWidgetItem(QString::number(count));
        count_item->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui.frequency_table->setItem(i, 1, count_item);
    }
    ui.frequency_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui.frequency_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    dialog->exec();
}

void main_window::show_spell_check()
{
    if (!checker.loaded()) {
        QMessageBox::warning(this, "Spell Checker", "Dictionary not loaded. Make sure data/words.txt exists.");
        return;
    }
    spell_highlighter->rehighlight();
    QMessageBox::information(this, "Spell Checker", "Spell check complete. Misspelled words are underlined in red.");
}

void main_window::show_context_menu(const QPoint& pos)
{
    QMenu* menu = editor->createStandardContextMenu();

    QTextCursor cursor = editor->cursorForPosition(pos);
    cursor.select(QTextCursor::WordUnderCursor);
    QString word = cursor.selectedText();

    if (!word.isEmpty() && checker.loaded() && !checker.is_correct(word.toStdString())) {
        std::vector<std::string> suggestions = checker.suggestions(word.toStdString());

        if (!suggestions.empty()) {
            QAction* first = menu->actions().isEmpty() ? nullptr : menu->actions().first();

            QAction* sep = new QAction(menu);
            sep->setSeparator(true);
            menu->insertAction(first, sep);

            for (int i = static_cast<int>(suggestions.size()) - 1; i >= 0; i--) {
                QString suggestion = QString::fromStdString(suggestions[i]);
                QAction* sug_action = new QAction(suggestion, menu);
                connect(sug_action, &QAction::triggered, this, [this, cursor, suggestion]() mutable {
                    cursor.insertText(suggestion);
                    editor->setTextCursor(cursor);
                });
                menu->insertAction(first, sug_action);
            }

            QAction* label = new QAction("Suggestions:", menu);
            label->setEnabled(false);
            menu->insertAction(menu->actions().first(), label);
        }
    }

    menu->exec(editor->viewport()->mapToGlobal(pos));
    delete menu;
}

void main_window::apply_transform(const text_transform& transform) const
{
    auto cursor = editor->textCursor();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::Document);
    }
    int start = cursor.selectionStart();
    QString selected = cursor.selectedText().replace(QChar::ParagraphSeparator, '\n');
    std::string original = selected.toStdString();
    std::string result = transform.apply(original);

    cursor.beginEditBlock();
    for (int i = 0; i < static_cast<int>(result.size()); i++) {
        if (original[i] != result[i]) {
            cursor.setPosition(start + i);
            cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
            cursor.insertText(QString(QChar(result[i])), cursor.charFormat());
        }
    }
    cursor.endEditBlock();
}
