#ifndef ADDSTATECOMMAND_H
#define ADDSTATECOMMAND_H

#include <QUndoCommand>

class AddStateCommand : public QUndoCommand
{
public:
    AddStateCommand();
    void undo() override;
    void redo() override;
};

#endif // ADDSTATECOMMAND_H
