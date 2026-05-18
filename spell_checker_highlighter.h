#ifndef SPELL_CHECKER_HIGHLIGHTER_H
#define SPELL_CHECKER_HIGHLIGHTER_H

#include "spell_checker.h"

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class spell_checker_highlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    spell_checker_highlighter(const spell_checker& checker, QTextDocument* parent)
        : QSyntaxHighlighter(parent)
        , checker(checker)
    {
        format.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
        format.setUnderlineColor(Qt::red);
    }

protected:
    void highlightBlock(const QString& text) override
    {
        QRegularExpression word_pattern("\\b[A-Za-z]+\\b");
        QRegularExpressionMatchIterator it = word_pattern.globalMatch(text);

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            std::string word = match.captured().toStdString();
            if (!checker.is_correct(word)) {
                setFormat(match.capturedStart(), match.capturedLength(), format);
            }
        }
    }

private:
    const spell_checker& checker;
    QTextCharFormat format;
};

#endif // SPELL_CHECKER_HIGHLIGHTER_H

