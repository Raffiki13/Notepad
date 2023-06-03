
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QTableWidget>
#include <QDockWidget>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QVector>
#include <QMap>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "highlighter.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QAction;
class QMenu;
class QSessionManager;
class QPlainTextEdit;
QT_END_NAMESPACE

class MyQPlainTextEdit;

class MainWindow : public QMainWindow {
    friend class MyQPlainTextEdit;
    Q_OBJECT

public:
    MainWindow();

    QMap<QString, QVector<QPair<int, int>>> get_c() {
        return space;
    }

    QTabWidget * get_tab() {
        return tabWidget;
    }

    void cscope(QString);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void open();
    bool save(int);
    bool saveAs(int);
    void about();
    void documentWasModified();
    void closeTabs(int);
    void setFile(int);
    void openByCtags(int, int);
    void graph();
    void saveGraph();
    void xml();

#ifndef QT_NO_SESSIONMANAGER
    void commitData(QSessionManager &);
#endif

private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();

    void open_file(QString);
    bool maybeSave(int);
    bool saveFile(const QString &, int);
    void loadFile(const QString &, int);
    void setCurrentFile(const QString &, int);
    QString strippedName(const QString &);

    void ctags();
    void cflow();

    QString curFile;
    Highlighter *highlight;
    QTabWidget *tabWidget;
    QGridLayout *gridLayout;
    QMenu *graphMenu;

    QTableWidget *tableWidget;
    QDockWidget *dockWidget;
    QVector<QVector<QString>> doc;
    QMap<QString, QVector<QPair<int, int>>> space;
    QVector<QPair<QPair<std::string, int>, QPair<std::string, int>>> data;
    int maxTab = 0;
    QGraphicsScene *scene;
    QGraphicsView *view;
};

class MyQPlainTextEdit: public QPlainTextEdit {

protected:
    void mouseDoubleClickEvent(QMouseEvent * event);
};

#endif
