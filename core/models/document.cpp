#include "document.h"
#include "blockmodel.h"
#include "blockcommands.h"
#include "core/storage/notesdatabase.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QUndoStack>
#include <algorithm>

Document::Document(QUuid id, const QString& title, QObject* parent)
    : QObject(parent)
    , m_id(id)
    , m_title(title)
    , m_undoStack(new QUndoStack(this))
{
    m_blockModel = new BlockModel(this, this);
}

Document::~Document() = default;

QUuid Document::id() const { return m_id; }
QString Document::title() const { return m_title; }

void Document::setTitle(const QString& title)
{
    if (m_title == title) return;
    m_title = title;
    emit titleChanged(title);
    emit documentModified();
}

QList<Block> Document::blocks() const { return m_blocks; }

Block* Document::findBlock(QUuid id)
{
    for (Block& block : m_blocks) {
        if (block.id() == id)
            return &block;
    }
    return nullptr;
}

int Document::findBlockIndex(QUuid id) const
{
    for (int i = 0; i < m_blocks.size(); ++i) {
        if (m_blocks[i].id() == id)
            return i;
    }
    return -1;
}

Block* Document::blockAt(int index)
{
    if (index < 0 || index >= m_blocks.size()) return nullptr;
    return &m_blocks[index];
}

void Document::insertBlock(QUuid parentId, int row, const BlockData& data)
{
    // For children, physical insert position = after parent + existing children
    int insertAt = row;
    if (!parentId.isNull()) {
        int parentIdx = findBlockIndex(parentId);
        if (parentIdx < 0) return;
        insertAt = parentIdx + 1;
        int childCount = 0;
        for (const Block& b : m_blocks) {
            if (b.parentId() == parentId)
                ++childCount;
        }
        insertAt = parentIdx + 1 + childCount; // append child
        // If row is given as child index:
        if (row >= 0 && row < childCount)
            insertAt = parentIdx + 1 + row;
        else
            insertAt = parentIdx + 1 + childCount;
    }

    Block newBlock(QUuid::createUuid(), parentId, insertAt, data);
    insertBlockInternal(parentId, insertAt, newBlock);
}

void Document::deleteBlock(QUuid blockId)
{
    removeBlockInternal(blockId);
}

void Document::moveBlock(QUuid blockId, QUuid newParentId, int newRow)
{
    Q_UNUSED(blockId);
    Q_UNUSED(newParentId);
    Q_UNUSED(newRow);
}

void Document::updateBlockData(QUuid blockId, const BlockData& newData)
{
    Block* block = findBlock(blockId);
    if (!block) return;
    block->setData(newData);
    emit blockDataChanged(blockId);
    emit documentModified();
}

QUndoStack* Document::undoStack() const { return m_undoStack; }
BlockModel* Document::model() const { return m_blockModel; }

QJsonObject Document::toJson() const
{
    QJsonObject obj;
    obj["id"] = m_id.toString(QUuid::WithoutBraces);
    obj["title"] = m_title;
    QJsonArray blocksArr;
    for (const Block& block : m_blocks) {
        blocksArr.append(block.toJson());
    }
    obj["blocks"] = blocksArr;
    return obj;
}

Document* Document::fromJson(const QJsonObject& obj, NotesDatabase* db)
{
    Q_UNUSED(db);
    QUuid id = QUuid::fromString(obj["id"].toString());
    QString title = obj["title"].toString();
    auto* doc = new Document(id, title);

    QJsonArray blocksArr = obj["blocks"].toArray();
    for (const QJsonValue& val : blocksArr) {
        Block block = Block::fromJson(val.toObject());
        doc->m_blocks.append(block);
    }
    doc->m_blockModel->rebuildMaps();
    return doc;
}

void Document::insertBlockInternal(QUuid parentId, int row, const Block& block)
{
    if (row < 0 || row > m_blocks.size())
        row = m_blocks.size();
    m_blocks.insert(row, block);
    if (m_blockModel)
        m_blockModel->notifyInserted(row, block.id());
    emit blockInserted(block.id(), parentId, row);
    emit documentModified();
}

void Document::removeBlockInternal(QUuid blockId)
{
    // Remove children first (toggle contents)
    QList<QUuid> childIds;
    for (const Block& b : m_blocks) {
        if (b.parentId() == blockId)
            childIds.append(b.id());
    }
    for (const QUuid& cid : childIds)
        removeBlockInternal(cid);

    int index = findBlockIndex(blockId);
    if (index < 0) return;

    if (m_blockModel)
        m_blockModel->notifyRemove(index);

    m_blocks.removeAt(index);
    emit blockDeleted(blockId);
    emit documentModified();
}