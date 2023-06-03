#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    keywordFormat1.setForeground(Qt::darkBlue);
    keywordFormat1.setFontWeight(QFont::Bold);
    QStringList keywordPatterns1;
    keywordPatterns1 << "\\bchar\\b" << "\\bclass\\b" << "\\bstruct\\b"
                    << "\\bdouble\\b" << "\\benum\\b" << "\\bexplicit\\b"
                    << "\\bfriend\\b" << "\\binline\\b" << "\\bint\\b"
                    << "\\blong\\b" << "\\bnamespace\\b" << "\\boperator\\b"
                    << "\\bprivate\\b" << "\\bprotected\\b" << "\\bpublic\\b"
                    << "\\bshort\\b" << "\\bsignals\\b" << "\\bsigned\\b"
                    << "\\bslots\\b" << "\\bstatic\\b" << "\\bstruct\\b"
                    << "\\btemplate\\b" << "\\btypedef\\b" << "\\btypename\\b"
                    << "\\bunion\\b" << "\\bunsigned\\b" << "\\bvirtual\\b"
                    << "\\bvoid\\b" << "\\bvolatile\\b" << "\\bbool\\b"
                    << "\\bfloat\\b" << "\\binclude\\b" << "\\bendl\\b"
                    << "\\balignas\\b" << "\\balignof\\b" << "\\band\\b"
                    << "\\band_eq\\b" << "\\basm\\b" << "\\bauto\\b"
                    << "\\bbitand\\b" << "\\bbitor\\b" <<"\\bcase\\b"
                    << "\\bcatch\\b" << "\\bchar8_t\\b" << "\\bchar16_t\\b"
                    << "\\bchar32_t\\b" <<"\\bcompl\\b" << "\\bconcept\\b"
                    << "\\bcontinue\\b" << "\\bdefault\\b" << "\\bfalse\\b"
                    << "\\btrue\\b" << "\\belse\\b" << "\\bgoto\\b"
                    << "\\btypeid\\b" << "\\btrue\\b" << "\\bsizeof\\b"
                    << "\\bQtCore\\b" << "\\bQtGui\\b" << "\\bQtWidgets\\b" << "\\bQtQML\\b" << "\\bQtNetwork\\b"
                    << "\\bQtOpenGL\\b" << "\\bQtSql\\b" << "\\bQtScript\\b" << "\\bQtSvg\\b" << "\\bQtXml\\b"
                    << "\\bQtScript\\b" << "\\bQtTest\\b" << "\\bActiveQt\\b" << "\\bQtDeclarative\\b";


    foreach (const QString &pattern, keywordPatterns1) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat1;
        highlightingRules.append(rule);
    }
    keywordFormat2.setForeground(Qt::darkRed);
    keywordFormat2.setFontWeight(QFont::Bold);
    QStringList keywordPatterns2;
    keywordPatterns2 << "\\bfor\\b" << "\\bvector\\b" << "\\bconst\\b"
                     << "\\busing\\b" << "\\bwhile\\b" << "\\bdo\\b"
                     << "\\bcout\\b" << "\\bcin\\b" << "\\bstring\\b"
                     << "\\bif\\b" <<"\\bbreak\\b";
    foreach (const QString &pattern, keywordPatterns2) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat2;
        highlightingRules.append(rule);
    }

    keywordFormat3.setForeground(Qt::darkGreen);
    keywordFormat3.setFontWeight(QFont::Bold);
    QStringList keywordPatterns3;
    keywordPatterns3 << "<iostream>" << "<cmath>" << "<string>"
                     << "<vector>" << "<algorithm>" << "<map>"
                     << "<set>";
    foreach (const QString &pattern, keywordPatterns3) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat3;
        highlightingRules.append(rule);
    }

    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression("\\bQ[A-Za-z]+\\b");
    rule.format = classFormat;
    highlightingRules.append(rule);

    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);

    multiLineCommentFormat.setForeground(Qt::red);

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression("\".*\"");
    rule.format = quotationFormat;
    highlightingRules.append(rule);

    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    rule.format = functionFormat;
    highlightingRules.append(rule);

    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");
}

void Highlighter::highlightBlock(const QString &text)
{
    foreach (const HighlightingRule &rule, highlightingRules) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    setCurrentBlockState(0);

    int startIndex = 0;
    if (previousBlockState() != 1)
        startIndex = text.indexOf(commentStartExpression);


    while (startIndex >= 0) {
        QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        } else {
            commentLength = endIndex - startIndex
                            + match.capturedLength();
        }
        setFormat(startIndex, commentLength, multiLineCommentFormat);
        startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
    }
}
