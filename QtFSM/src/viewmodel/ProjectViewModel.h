#ifndef PROJECTVIEWMODEL_H
#define PROJECTVIEWMODEL_H

#include <QObject>

/**
 * @brief The ProjectViewModel class - ViewModel for project-level operations
 */
class ProjectViewModel : public QObject
{
    Q_OBJECT

public:
    explicit ProjectViewModel(QObject *parent = nullptr);
    ~ProjectViewModel();
};

#endif // PROJECTVIEWMODEL_H
