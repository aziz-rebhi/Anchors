#pragma once
#include <QString>
#include <QList>
#include <QVariant>

// Forward declarations
struct ParagraphData;
struct HeadingData;
struct CodeData;
struct ImageData;
struct TableData;
// ... more types later

using BlockData = std::variant<
    ParagraphData,
    HeadingData,
    CodeData,
    ImageData,
    TableData
    >;

struct ParagraphData {
    QString text;
};

struct HeadingData {
    int level; // 1,2,3
    QString text;
};

struct CodeData {
    QString language;
    QString code;
};

struct ImageData {
    QString filePath;
    QString caption;
    int width = 0;
    int height = 0;
};

struct TableData {
    int rows = 0;
    int cols = 0;
    QList<QList<QString>> cells;
};