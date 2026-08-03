#include "document.h"
#include "blockmodel.h"
#include "qjsonobject.h"
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

QList<Block*> Document::childrenOf(QUuid parentId)
{
    QList<Block*> children;
    for (Block& block : m_blocks) {
        if (block.parentId() == parentId)
            children.append(&block);
    }
    std::sort(children.begin(), children.end(), [](Block* a, Block* b) {
        return a->orderIndex() < b->orderIndex();
    });
    return children;
}

void Document::insertBlock(QUuid parentId, int row, const BlockData& data)
{
    Block newBlock(QUuid::createUuid(), parentId, row, data);
    insertBlockInternal(parentId, row, newBlock);
}

void Document::deleteBlock(QUuid blockId)
{
    removeBlockInternal(blockId);
}

void Document::moveBlock(QUuid blockId, QUuid newParentId, int newRow)
{
    // Stub – implement later
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

void Document::loadFromDatabase(NotesDatabase* db)
{
    Q_UNUSED(db);
}

void Document::saveToDatabase(NotesDatabase* db)
{
    Q_UNUSED(db);
}

QJsonObject Document::toJson() const
{
    return QJsonObject();
}

Document Document::fromJson(const QJsonObject& obj, NotesDatabase* db)
{
    Q_UNUSED(obj);
    Q_UNUSED(db);
    return Document(QUuid::createUuid(), "Untitled");
}

void Document::reorderSiblings(QUuid parentId)
{
    Q_UNUSED(parentId);
}

void Document::insertBlockInternal(QUuid parentId, int row, const Block& block)
{
    m_blocks.insert(row, block);
    if (m_blockModel) {
        m_blockModel->rebuildMaps();  // rebuild from m_blocks
    }
    emit blockInserted(block.id(), parentId, row);
    emit documentModified();
}

void Document::removeBlockInternal(QUuid blockId)
{
    int index = -1;
    for (int i = 0; i < m_blocks.size(); ++i) {
        if (m_blocks[i].id() == blockId) {
            index = i;
            break;
        }
    }
    if (index < 0) return;
    QUuid parentId = m_blocks[index].parentId();
    m_blocks.removeAt(index);
    if (m_blockModel) {
        m_blockModel->rebuildMaps();
    }
    emit blockDeleted(blockId);
    emit documentModified();
}