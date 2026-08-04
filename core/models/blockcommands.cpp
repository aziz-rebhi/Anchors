#include "blockcommands.h"
#include "document.h"
#include "blockmodel.h"

// ----------------------------------------------------------------
// InsertBlockCommand
// ----------------------------------------------------------------

InsertBlockCommand::InsertBlockCommand(Document* doc, QUuid parentId, int row,
                                       const BlockData& data, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_doc(doc)
    , m_parentId(parentId)
    , m_row(row)
    , m_data(data)
{
}

void InsertBlockCommand::redo()
{
    if (m_blockId.isNull())
        m_blockId = QUuid::createUuid();

    Block block(m_blockId, m_parentId, m_row, m_data);
    m_doc->m_blocks.insert(m_row, block);
    m_doc->reorderSiblings(m_parentId);

    if (m_doc->m_blockModel)
        m_doc->m_blockModel->notifyInserted(m_parentId, m_row);

    emit m_doc->blockInserted(m_blockId, m_parentId, m_row);
    emit m_doc->documentModified();
}

void InsertBlockCommand::undo()
{
    int idx = m_doc->findBlockIndex(m_blockId);
    if (idx < 0) return;

    m_doc->m_blocks.removeAt(idx);

    if (m_doc->m_blockModel)
        m_doc->m_blockModel->notifyRemove(m_parentId, idx);

    emit m_doc->blockDeleted(m_blockId);
    emit m_doc->documentModified();
}

// ----------------------------------------------------------------
// DeleteBlockCommand
// ----------------------------------------------------------------

DeleteBlockCommand::DeleteBlockCommand(Document* doc, QUuid blockId, QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_doc(doc)
    , m_blockId(blockId)
    , m_row(0)
{
    // Capture state BEFORE deletion
    Block* block = m_doc->findBlock(blockId);
    if (block) {
        m_parentId = block->parentId();
        m_row = m_doc->findBlockIndex(blockId);
        m_data = block->data();
    }
}

void DeleteBlockCommand::redo()
{
    int idx = m_doc->findBlockIndex(m_blockId);
    if (idx < 0) return;

    m_doc->m_blocks.removeAt(idx);

    if (m_doc->m_blockModel)
        m_doc->m_blockModel->notifyRemove(m_parentId, idx);

    emit m_doc->blockDeleted(m_blockId);
    emit m_doc->documentModified();
}

void DeleteBlockCommand::undo()
{
    Block block(m_blockId, m_parentId, m_row, m_data);
    m_doc->m_blocks.insert(m_row, block);
    m_doc->reorderSiblings(m_parentId);

    if (m_doc->m_blockModel)
        m_doc->m_blockModel->notifyInserted(m_parentId, m_row);

    emit m_doc->blockInserted(m_blockId, m_parentId, m_row);
    emit m_doc->documentModified();
}

// ----------------------------------------------------------------
// EditTextCommand
// ----------------------------------------------------------------

EditTextCommand::EditTextCommand(Document* doc, QUuid blockId,
                                 const BlockData& oldData, const BlockData& newData,
                                 QUndoCommand* parent)
    : QUndoCommand(parent)
    , m_doc(doc)
    , m_blockId(blockId)
    , m_oldData(oldData)
    , m_newData(newData)
{
}

void EditTextCommand::redo()
{
    Block* block = m_doc->findBlock(m_blockId);
    if (!block) return;
    block->setData(m_newData);
    emit m_doc->blockDataChanged(m_blockId);
    emit m_doc->documentModified();
}

void EditTextCommand::undo()
{
    Block* block = m_doc->findBlock(m_blockId);
    if (!block) return;
    block->setData(m_oldData);
    emit m_doc->blockDataChanged(m_blockId);
    emit m_doc->documentModified();
}

int EditTextCommand::id() const { return COMMAND_ID; }

bool EditTextCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    const auto* otherEdit = static_cast<const EditTextCommand*>(other);
    if (otherEdit->m_blockId != m_blockId) return false;
    m_newData = otherEdit->m_newData;
    return true;
}