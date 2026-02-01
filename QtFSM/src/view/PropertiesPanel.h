#ifndef PROPERTIESPANEL_H
#define PROPERTIESPANEL_H

#include <QWidget>

class FSM;

/**
 * @brief The PropertiesPanel class - Properties editor panel
 */
class PropertiesPanel : public QWidget {
  Q_OBJECT

public:
  explicit PropertiesPanel(QWidget *parent = nullptr);
  ~PropertiesPanel();

  void setFSM(FSM *fsm);

private slots:
  void updateStates();
  void updateTransitions();

private:
  FSM *m_fsm = nullptr;
  class QLineEdit *m_nameEdit = nullptr;
  class QFormLayout *m_formLayout = nullptr;
  QWidget *m_fsmSettingsContainer = nullptr;

  // Lists
  class QTreeWidget *m_statesTree = nullptr;
  class QTreeWidget *m_transitionsTree = nullptr;
};

#endif // PROPERTIESPANEL_H
