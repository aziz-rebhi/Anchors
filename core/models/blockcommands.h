#pragma once
#include <QUndoCommand>
#include <QUuid>
#include "block.h"
#include "blockdata.h"

class Document;

class InsertBlockCommand :public QUndoCommand {
public :
    InsertBlockCommand(Document* doc, QUuid parentId, int row, const BlockData& data, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private :
    Document* m_doc;
    QUuid m_blockId;
    QUuid m_parentId;
    int m_row;
    BlockData m_data;

};

class DeleteBlockCommand : public QUndoCommand {
public :
    DeleteBlockCommand(Document* doc, QUuid blockId, QUndoCommand* parentId = nullptr);
    void undo() override;
    void redo() override;

private :
    Document* m_doc;
    QUuid m_blockId;
    QUuid m_parentId;
    int m_row;
    BlockData m_data;
};

class EditTextCommand : public QUndoCommand {
public :
    EditTextCommand(Document* doc, QUuid blockId, const BlockData& oldData, const BlockData& newData, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private :
    static constexpr int COMMAND_ID = 1001;
    Document* m_doc;
    QUuid m_blockId;
    BlockData m_oldData;
    BlockData m_newData;

};

