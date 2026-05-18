#ifndef SPELL_CHECKER_HIGHLIGHTER_H
#define SPELL_CHECKER_HIGHLIGHTER_H

#include "spell_checker.h"

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class spell_checker_highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit spell_checker_highlighter(const spell_checker& checker, QTextDocument* parent = nullptr)
        : QSyntaxHighlighter(parent)
        , checker(checker)
    {
        misspelled_format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        misspelled_format.setUnderlineColor(Qt::red);
    }

    void set_enabled(bool enabled)
    {
        spell_check_enabled = enabled;
        rehighlight();
    }

    [[nodiscard]] bool is_enabled() const { return spell_check_enabled; }

protected:
    void highlightBlock(const QString& text) override
    {
        if (!spell_check_enabled) {
            return;
        }

        static const QRegularExpression word_pattern(R"(\b[A-Za-z]+\b)");
        auto it = word_pattern.globalMatch(text);

        while (it.hasNext()) {
            const auto match = it.next();
            const std::string word = match.captured().toStdString();
            if (!checker.is_correct(word)) {
                setFormat(match.capturedStart(), match.capturedLength(), misspelled_format);
            }
        }
    }

private:
    const spell_checker& checker;
    QTextCharFormat misspelled_format;
    bool spell_check_enabled = true;
};

#endif // SPELL_CHECKER_HIGHLIGHTER_H