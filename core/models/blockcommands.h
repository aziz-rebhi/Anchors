#ifndef BLOCKCOMMANDS_H
#define BLOCKCOMMANDS_H

#include <QUndoCommand>
#include <QUuid>
#include <QString>
#include "blockdata.h"

class Document;

class InsertBlockCommand : public QUndoCommand {
public:
    // If suggestedId is null, generates one automatically
    InsertBlockCommand(Document* doc, QUuid parentId, int row, const BlockData& data,
                       QUuid suggestedId = QUuid(), QUndoCommand* parentCmd = nullptr);
    void undo() override;
    void redo() override;
    QUuid insertedId() const { return m_generatedId; }

private:
    Document* m_doc;
    QUuid m_parentId;
    int m_row;
    BlockData m_data;
    QUuid m_generatedId;
    bool m_firstRedo = true;
};

class DeleteBlockCommand : public QUndoCommand {
public:
    DeleteBlockCommand(Document* doc, QUuid blockId, QUndoCommand* parentCmd = nullptr);
    void undo() override;
    void redo() override;

private:
    Document* m_doc;
    QUuid m_blockId;
    QUuid m_parentId;
    int m_row;
    BlockData m_data;
};

class EditTextCommand : public QUndoCommand {
public:
    EditTextCommand(Document* doc, QUuid blockId, const QString& newText, QUndoCommand* parentCmd = nullptr);
    void undo() override;
    void redo() override;
    int id() const override;
    bool mergeWith(const QUndoCommand* other) override;

private:
    Document* m_doc;
    QUuid m_blockId;
    QString m_oldText;
    QString m_newText;
};

#endif // BLOCKCOMMANDS_H
