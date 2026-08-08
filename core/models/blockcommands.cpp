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
    int insertAt = m_row;

    if (!m_parentId.isNull()) {
        const int parentIdx = m_doc->findBlockIndex(m_parentId);
        if (parentIdx < 0)
            return;

        int childCount = 0;
        for (const Block& b : m_doc->blocks()) {
            if (b.parentId() == m_parentId)
                ++childCount;
        }

        if (m_row >= 0 && m_row <= childCount)
            insertAt = parentIdx + 1 + m_row;
        else
            insertAt = parentIdx + 1 + childCount;
    }

    if (insertAt < 0 || insertAt > m_doc->blocks().size())
        insertAt = m_doc->blocks().size();

    Block block(m_generatedId, m_parentId, insertAt, m_data);
    m_doc->insertBlockInternal(m_parentId, insertAt, block);
}

// --- DeleteBlockCommand ---

DeleteBlockCommand::DeleteBlockCommand(Document* doc, QUuid blockId, QUndoCommand* parentCmd)
    : QUndoCommand("Delete block", parentCmd)
    , m_doc(doc)
    , m_blockId(blockId)
{
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
            } else if constexpr (std::is_same_v<T, BulletData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, CalloutData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, NumberedData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, ToggleData>) {
                m_oldText = arg.text;
            } else if constexpr (std::is_same_v<T, EquationData>) {
                m_oldText = arg.latex;
            } else {
                m_oldText = QString();
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
        if constexpr (std::is_same_v<T, ParagraphData>) return ParagraphData{m_oldText};
        else if constexpr (std::is_same_v<T, HeadingData>) return HeadingData{arg.level, m_oldText};
        else if constexpr (std::is_same_v<T, TodoData>) return TodoData{m_oldText, arg.checked};
        else if constexpr (std::is_same_v<T, QuoteData>) return QuoteData{m_oldText};
        else if constexpr (std::is_same_v<T, CodeData>) return CodeData{arg.language, m_oldText};
        else if constexpr (std::is_same_v<T, BulletData>) return BulletData{m_oldText, arg.indent};
        else if constexpr (std::is_same_v<T, CalloutData>) return CalloutData{m_oldText, arg.emoji};
        else if constexpr (std::is_same_v<T, NumberedData>) return NumberedData{m_oldText, arg.indent};
        else if constexpr (std::is_same_v<T, ToggleData>) return ToggleData{m_oldText, arg.collapsed};
        else if constexpr (std::is_same_v<T, EquationData>) return EquationData{m_oldText, arg.displayMode};
        else return arg;
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
        if constexpr (std::is_same_v<T, ParagraphData>) return ParagraphData{m_newText};
        else if constexpr (std::is_same_v<T, HeadingData>) return HeadingData{arg.level, m_newText};
        else if constexpr (std::is_same_v<T, TodoData>) return TodoData{m_newText, arg.checked};
        else if constexpr (std::is_same_v<T, QuoteData>) return QuoteData{m_newText};
        else if constexpr (std::is_same_v<T, CodeData>) return CodeData{arg.language, m_newText};
        else if constexpr (std::is_same_v<T, BulletData>) return BulletData{m_newText, arg.indent};
        else if constexpr (std::is_same_v<T, CalloutData>) return CalloutData{m_newText, arg.emoji};
        else if constexpr (std::is_same_v<T, NumberedData>) return NumberedData{m_newText, arg.indent};
        else if constexpr (std::is_same_v<T, ToggleData>) return ToggleData{m_newText, arg.collapsed};
        else if constexpr (std::is_same_v<T, EquationData>) return EquationData{m_newText, arg.displayMode};
        else return arg;
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