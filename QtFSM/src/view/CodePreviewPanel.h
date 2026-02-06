#ifndef CODEPREVIEWPANEL_H
#define CODEPREVIEWPANEL_H

#include <QWidget>

class QTextEdit;
class FSM;

/**
 * @brief The CodePreviewPanel class - Shows generated C++ config in real-time
 */
class CodePreviewPanel : public QWidget {
  Q_OBJECT

public:
  explicit CodePreviewPanel(QWidget *parent = nullptr);
  ~CodePreviewPanel();

  QString code() const;

public slots:
  void updateCode(FSM *fsm);
  void clearCode();
  void setReadOnly(bool readOnly);
  void setCode(const QString &code);

signals:
  void codeChanged(const QString &code);
  void updateDiagramRequested();
  void generateCodeRequested();

private:
  QTextEdit *m_codeEdit;
  bool m_isInternalUpdate;
};

#endif // CODEPREVIEWPANEL_H
