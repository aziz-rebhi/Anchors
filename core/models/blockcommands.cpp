#include "blockcommands.h"
#include "document.h"
#include <QUuid>

// --- InsertBlockCommand ---

InsertBlockCommand::InsertBlockCommand(Document* doc, QUuid parentId, int row, const BlockData& data,
                                       QUuid suggestedId, QUndoCommand* parentCmd)
    : QUndoCommand("Insert block", parentCmd)
    , m_doc(doc)
    , m_parentId(parentId)
    , m_row(row)
    , m_data(data)
    , m_generatedId(suggestedId.isNull() ? QUuid::createUuid() : suggestedId)
    , m_firstRedo(true)
{
}

void InsertBlockCommand::undo()
{
    m_doc->removeBlockInternal(m_generatedId);
}

void InsertBlockCommand::redo()
{
    Block block(m_generatedId, m_parentId, m_row, m_data);
    m_doc->insertBlockInternal(m_parentId, m_row, block);
}

// --- DeleteBlockCommand ---

DeleteBlockCommand::DeleteBlockCommand(Document* doc, QUuid blockId, QUndoCommand* parentCmd)
    : QUndoCommand("Delete block", parentCmd)
    , m_doc(doc)
    , m_blockId(blockId)
{
    // Capture current state
    int idx = m_doc->findBlockIndex(blockId);
    if (idx >= 0) {
        Block* b = m_doc->blockAt(idx);
        m_parentId = b->parentId();
        m_row = idx;
        m_data = b->data();
    }
}

void DeleteBlockCommand::undo()
{
    Block block(m_blockId, m_parentId, m_row, m_data);
    m_doc->insertBlockInternal(m_parentId, m_row, block);
}

void DeleteBlockCommand::redo()
{
    m_doc->removeBlockInternal(m_blockId);
}

// --- EditTextCommand ---

EditTextCommand::EditTextCommand(Document* doc, QUuid blockId, const QString& newText, QUndoCommand* parentCmd)
    : QUndoCommand("Edit text", parentCmd)
    , m_doc(doc)
    , m_blockId(blockId)
    , m_newText(newText)
{
    // Capture old text
    Block* block = doc->findBlock(blockId);
    if (block) {
        std::visit([&](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, ParagraphData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, HeadingData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, TodoData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, QuoteData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, CodeData>) {
                m_oldText = arg.code;
            } else {
                m_oldText = "";
            }
        }, block->data());
    }
}

void EditTextCommand::undo()
{
    Block* block = m_doc->findBlock(m_blockId);
    if (!block) return;

    BlockData newData = std::visit([&](auto&& arg) -> BlockData {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) {
            return ParagraphData{m_oldText};
        } else if constexpr (std::is_same_v<T, HeadingData>) {
            return HeadingData{arg.level, m_oldText};
        } else if constexpr (std::is_same_v<T, TodoData>) {
            return TodoData{m_oldText, arg.checked};
        } else if constexpr (std::is_same_v<T, QuoteData>) {
            return QuoteData{m_oldText};
        } else if constexpr (std::is_same_v<T, CodeData>) {
            return CodeData{arg.language, m_oldText};
        } else {
            return arg;
        }
    }, block->data());

    block->setData(newData);
    emit m_doc->blockDataChanged(m_blockId);
}

void EditTextCommand::redo()
{
    Block* block = m_doc->findBlock(m_blockId);
    if (!block) return;

    BlockData newData = std::visit([&](auto&& arg) -> BlockData {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, ParagraphData>) {
            return ParagraphData{m_newText};
        } else if constexpr (std::is_same_v<T, HeadingData>) {
            return HeadingData{arg.level, m_newText};
        } else if constexpr (std::is_same_v<T, TodoData>) {
            return TodoData{m_newText, arg.checked};
        } else if constexpr (std::is_same_v<T, QuoteData>) {
            return QuoteData{m_newText};
        } else if constexpr (std::is_same_v<T, CodeData>) {
            return CodeData{arg.language, m_newText};
        } else {
            return arg;
        }
    }, block->data());

    block->setData(newData);
    emit m_doc->blockDataChanged(m_blockId);
}

int EditTextCommand::id() const
{
    return 1001;
}

bool EditTextCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id()) return false;
    const auto* otherEdit = static_cast<const EditTextCommand*>(other);
    if (otherEdit->m_blockId != m_blockId) return false;
    m_newText = otherEdit->m_newText;
    return true;
}