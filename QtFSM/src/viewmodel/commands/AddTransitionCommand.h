#ifndef ADDTRANSITIONCOMMAND_H
#define ADDTRANSITIONCOMMAND_H

#include <QUndoCommand>

class AddTransitionCommand : public QUndoCommand
{
public:
    AddTransitionCommand();
    void undo() override;
    void redo() override;
};

#endif // ADDTRANSITIONCOMMAND_H
