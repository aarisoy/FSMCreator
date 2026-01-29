#ifndef DELETESTATECOMMAND_H
#define DELETESTATECOMMAND_H

#include <QUndoCommand>

class DeleteStateCommand : public QUndoCommand
{
public:
    DeleteStateCommand();
    void undo() override;
    void redo() override;
};

#endif // DELETESTATECOMMAND_H
