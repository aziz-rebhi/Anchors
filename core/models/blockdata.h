#ifndef BLOCKDATA_H
#define BLOCKDATA_H

#include <QString>
#include <QJsonObject>
#include <QJsonArray>

struct ParagraphData {
    QString text;
};

struct HeadingData {
    int level = 1;
    QString text;
};

struct CodeData {
    QString language;
    QString code;
};

struct ImageData {
    QString source;
    QString caption;
    int width = 0;
    int height = 0;
};

struct TableData {
    int rows = 0;
    int cols = 0;
    QVector<QVector<QString>> cells;
};

struct TodoData {
    QString text;
    bool checked = false;
};

struct DividerData {
    int orientation = 0;
};

struct QuoteData {
    QString text;
};

#include <variant>
using BlockData = std::variant<ParagraphData, HeadingData, CodeData, ImageData, TableData, TodoData, DividerData, QuoteData>;

// Type indices (matches variant order):
//  0 = Paragraph
//  1 = Heading (H1)
//  2 = Heading (H2)  -- shared HeadingData, disambiguated by level
//  3 = Code
//  4 = Image
//  5 = Table
//  6 = Todo
//  7 = Divider
//  8 = Quote
//
// We map HeadingData variants to int codes via level:
//   level 1 -> TypeRole 1, level 2 -> TypeRole 2, level 3 -> TypeRole 3

// For QML TypeRole we use semantic codes:
//   0 = Paragraph
//   1 = H1
//   2 = H2
//   3 = H3
//   4 = Todo
//   5 = Code
//   6 = Image
//   7 = Table
//   8 = Divider
//   9 = Quote

// C++ insertBlock type param mapping:
//   0 = Paragraph
//   1 = H1
//   2 = H2
//   3 = H3
//   4 = Todo
//   5 = Code
//   6 = Image
//   7 = Table
//   8 = Divider
//   9 = Quote

#endif // BLOCKDATA_H
