#ifndef STATE_H
#define STATE_H

#include <QWidget>

namespace Ui {
class State;
}

class State : public QWidget
{
    Q_OBJECT

public:
    explicit State(QWidget *parent = nullptr);
    ~State();

    void displayLog(const QString &text);

private slots:
    void on_back_clicked();

    void on_logClear_clicked();

private:
    QWidget *mParent;
    Ui::State *ui;
};

#endif // STATE_H
