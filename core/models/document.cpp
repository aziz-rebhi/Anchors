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
    Q_UNUSED(newParentId); // v1: root-level reorder only
    int from = findBlockIndex(blockId);
    if (from < 0 || newRow < 0)
        return;

    // Only reorder top-level blocks (parent null) for v1
    Block* b = blockAt(from);
    if (!b || !b->parentId().isNull())
        return;

    if (m_undoStack) {
        m_undoStack->push(new MoveBlockCommand(this, blockId, from, newRow));
    } else {
        moveBlockInternal(blockId, from, newRow);
    }
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

void Document::moveBlockInternal(QUuid blockId, int fromRow, int toRow)
{
    int from = findBlockIndex(blockId);
    if (from < 0)
        return;

    // Unit = this block + contiguous descendants (toggle/columns children)
    auto isUnder = [&](const Block& cand, QUuid rootId) -> bool {
        QUuid p = cand.parentId();
        while (!p.isNull()) {
            if (p == rootId) return true;
            int pi = findBlockIndex(p);
            if (pi < 0) break;
            p = m_blocks[pi].parentId();
        }
        return false;
    };

    int end = from + 1;
    while (end < m_blocks.size() && isUnder(m_blocks[end], blockId))
        ++end;

    const int count = end - from;
    if (count <= 0) return;

    // Clamp destination into visible root slots (model rows ≈ top-level count)
    // Here toRow is index in the flat list among top-level targets:
    // We interpret toRow as destination index in m_blocks for the start of the unit.
    if (toRow > m_blocks.size()) toRow = m_blocks.size();
    if (toRow < 0) toRow = 0;

    // Extract unit
    QList<Block> unit;
    for (int i = 0; i < count; ++i)
        unit.append(m_blocks[from + i]);

    for (int i = 0; i < count; ++i)
        m_blocks.removeAt(from);

    // Adjust target after removal
    int dest = toRow;
    if (dest > from)
        dest -= count;
    if (dest < 0) dest = 0;
    if (dest > m_blocks.size()) dest = m_blocks.size();

    for (int i = 0; i < unit.size(); ++i)
        m_blocks.insert(dest + i, unit[i]);

    if (m_blockModel)
        m_blockModel->rebuildMaps();

    emit documentModified();
}