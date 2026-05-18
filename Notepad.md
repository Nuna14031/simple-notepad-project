Notepad: Implementation Notes
Required Features
1. Exception Handling
   The exception hierarchy is defined in notepad_exception.h:

notepad_exception — base class, extends std::runtime_error
file_not_found_exception — thrown when the file does not exist
file_read_exception — thrown when the file cannot be opened for reading
file_write_exception — thrown when the file cannot be opened for writing

Both open_file() and save_file() are wrapped in try / catch blocks. Errors are shown to the user via QMessageBox::critical with the title "Error".
2. Spell Checker
   spell_checker.h
   Loads data/words.txt into a std::set<std::string> at startup. Has two methods:

is_correct(word) — strips non-alpha characters, lowercases the word, and checks if it is in the set.
suggestions(word) — goes through the dictionary and collects up to 5 words that start with the same letter and have a similar length.

spell_checker_highlighter.h
Subclasses QSyntaxHighlighter. In highlightBlock() it finds all words using a regular expression and calls is_correct() on each one. Misspelled words are underlined in red using QTextCharFormat::SpellCheckUnderline. This runs automatically every time the text changes.
In main_window

The highlighter is attached to editor->document() in the constructor, so red underlines appear in real time as you type.
Tools > Check Spelling... calls rehighlight() to re-check the whole document and shows a message when done.
Right-clicking a misspelled word opens a context menu with up to 5 suggestions at the top. Clicking a suggestion replaces the word.


Optional Features
1. Recent Files
   File > Recent Files shows the last 5 opened or saved files. The list is saved using QSettings so it persists between sessions. When you open or save a file, its path is added to the top of the list. Each item in the submenu opens that file directly. There is also a "Clear Recent Files" action at the bottom of the submenu.
2. Font Dialog
   Format > Font... opens the standard QFontDialog. If text is selected, the chosen font is applied only to the selection. If nothing is selected, it is applied to the whole document via editor->setFont().
3. Zoom
   View menu has three items: Zoom In (Ctrl++), Zoom Out (Ctrl+-), and Reset Zoom (Ctrl+0). Each zoom step changes the font size by 2 points using editor->zoomIn() and editor->zoomOut(). A zoom_factor counter keeps track of how many steps have been taken so that Reset Zoom can bring the size back to the original.