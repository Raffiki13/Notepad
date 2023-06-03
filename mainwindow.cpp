#include <QPlainTextEdit>
#include <QGuiApplication>
#include <QDialog>
#include <QFileDialog>
#include <QMenuBar>
#include <QAction>
#include <QMessageBox>
#include <QToolBar>
#include <QApplication>
#include <QStatusBar>
#include <QString>
#include <QSettings>
#include <QDesktopWidget>
#include <QTextStream>
#include <QSessionManager>
#include <QTableWidget>
#include <QDockWidget>
#include <QAbstractScrollArea>
#include <QTextCursor>
#include <iostream>
#include <fstream>
#include <string>
#include <QApplication>
#include <QTextBlock>
#include <QVector>
#include <sstream>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QVBoxLayout>
#include <QXmlStreamWriter>
#include "mainwindow.h"
#include "stdlib.h"

MainWindow::MainWindow() {
    tableWidget = new QTableWidget;
    connect(tableWidget, SIGNAL(cellClicked(int,int)), this, SLOT(openByCtags(int, int)));
    dockWidget = new QDockWidget(this);
    dockWidget->setWindowTitle("CTags");
    dockWidget->setMinimumWidth(700);
    dockWidget->setFeatures(QDockWidget::DockWidgetMovable);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea|Qt::RightDockWidgetArea);

    createActions();
    createStatusBar();
    readSettings();
    ctags();

#ifndef QT_NO_SESSIONMANAGER
    QGuiApplication::setFallbackSessionManagementEnabled(false);
    connect(qApp, &QGuiApplication::commitDataRequest, this, &MainWindow::commitData);
#endif

    setCurrentFile(QString(), tabWidget->currentIndex());
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    int c = 0;
    for (int i = 0; i < tabWidget->count(); ++i) {
        if (maybeSave(i))
            ++c;
    }
    if (c == tabWidget->count()) {
        writeSettings();
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::cscope(QString file_name) {
    int last_ind = file_name.length();
    if (last_ind >= 3) {
        if ((QString(file_name[last_ind-4]) + QString(file_name[last_ind-3]) + QString(file_name[last_ind-2]) + QString(file_name[last_ind-1]) == QString(".cpp") ||
             QString(file_name[last_ind-2]) + QString(file_name[last_ind-1]) == QString(".h"))) {
            std::string str = "cscope -b -k  " + file_name.toStdString();
            char *cstr = new char[str.size()+1];
            strcpy(cstr, str.c_str());
            system(cstr);
            delete[] cstr;
            QString path = QCoreApplication::applicationDirPath()+ "/cscope.out";
            std::fstream file;
            file.open(path.toStdString());

            std::string digits = "0123456789";
            std::string num = "";
            std::string cur;
            bool flag = false;
            while (std::getline(file, cur)) {
                if (num != "") {
                    if (cur.find("$") != -1 && !flag) {
                        space[file_name].push_back({stoi(num), -10});
                        flag = true;
                    }
                    if (cur.find("}") != -1 && flag) {
                        space[file_name].back().second = stoi(num);
                        flag = false;
                    }
                }
                std::string tmp;
                if (cur != "" ) {
                    int i = 0;
                    while (digits.find(cur[i]) != -1) {
                        tmp += cur[i];
                        i++;
                    }
                    if (tmp != "")
                        num = tmp;
                }
            }
            file.close();
        }
    }
}

void MainWindow::ctags() {
    doc.clear();
    for (int i = 0; i < tabWidget->count(); i++) {
        QString file_name = tabWidget->tabText(i);
        int last_ind = file_name.length();
        if (last_ind >= 3){
            if(QString(file_name[last_ind-4]) + QString(file_name[last_ind-3]) + QString(file_name[last_ind-2]) + QString(file_name[last_ind-1]) == QString(".cpp") ||
                              QString(file_name[last_ind-2]) + QString(file_name[last_ind-1]) == QString(".h")) {

                std::string str = "ctags " + file_name.toStdString();
                char *cstr = new char[str.size() + 1];
                strcpy(cstr, str.c_str());
                system(cstr);
                delete[] cstr;
                std::ifstream file;
                file.open("tags");
                std::string cur = "";
            while (std::getline(file, cur)) {
                if (cur[0] != '!') {
                    std::stringstream inp(cur);
                    std::string func_name, file_name, func;
                    func.resize(256);
                    inp >> func_name >> file_name;
                    inp.getline((char*)func.data(), 255);
                    func[func.find('$')] = 0;
                    func = func.data()+3;
                    doc.push_back({QString::fromStdString(file_name).mid(QString::fromStdString(file_name).lastIndexOf('/') + 1),
                                   QString::fromStdString(func_name), QString::fromStdString(func), QString::number(i)});
                }
            }
            file.close();
            std::ofstream out;
            out.open("tags");
            out << "";
            out.close();
        }
    }
    }
    tableWidget->setRowCount(doc.size());
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels(QString("Var File Def Docs").split(" "));
    for (int i = 0; i < doc.size(); i++){
        tableWidget->setItem(i, 0, new QTableWidgetItem(doc[i][0]));
        tableWidget->setItem(i, 1, new QTableWidgetItem(doc[i][1]));
        tableWidget->setItem(i, 2, new QTableWidgetItem(doc[i][2]));
        tableWidget->setItem(i, 3, new QTableWidgetItem(doc[i][3]));
    }
    dockWidget->setWidget(tableWidget);
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);
}

void MainWindow::openByCtags(int row, int col) {
    if (col == 2) {
        tabWidget->setCurrentWidget(tabWidget->widget(tableWidget->item(row, col + 1)->text().toInt()));
        if (((MyQPlainTextEdit *)(tabWidget->currentWidget()))->find(QString(tableWidget->item(row, col)->text()))) {
            QTextCursor cursor = ((MyQPlainTextEdit *)(tabWidget->currentWidget()))->textCursor();
            ((MyQPlainTextEdit *)(tabWidget->currentWidget()))->setTextCursor(cursor);
        }
    }
}

void MainWindow::cflow() {
    std::string str = "cflow " + (tabWidget->tabText(tabWidget->currentIndex())).toStdString() + " > " + (QCoreApplication::applicationDirPath()+"/cflow.txt").toStdString();
    char *cstr = new char[str.size() + 1];
    strcpy(cstr, str.c_str());
    system(cstr);
    delete[] cstr;
    std::ifstream file;
    file.open((QCoreApplication::applicationDirPath()+"/cflow.txt").toStdString());

    data.clear();
    QMap<int, QPair<std::string, int>> parent;
    std::string s;
    getline(file, s);
    std::stringstream in(s);
    std::string func_name;
    in >> func_name;
    parent[0] = {func_name, 0};

    int i = 0;
    while (getline(file, s)) {
        std::stringstream in(s);
        in >> func_name;
        int tabs = 0;
        while (s[tabs] == ' ')
            ++tabs;
        parent[tabs] = {func_name, tabs/4};
        data.resize(data.size() + 1);
        if (tabs == 0) {
            data[i].first.first = parent[0].first;
            data[i].first.second = 0;
        } else {
            data[i].first.first = parent[tabs-4].first;
            data[i].first.second = (tabs-4)/4;
            data[i].second.first = func_name;
            data[i].second.second = tabs/4;
        }
        ++i;
        maxTab = std::max(maxTab, tabs/4);
    }
}

void MainWindow::graph() {
    cflow();
    scene = new QGraphicsScene(this);
    scene->setSceneRect(0, 0, width(), height());

    QVBoxLayout *box = new QVBoxLayout;
    QMenuBar *bar = new QMenuBar();
    QMenu *menu = new QMenu("&File");
    menu->addAction(tr("&Save"), this, SLOT(saveGraph()));
    bar->addMenu(menu);

    view = new QGraphicsView;
    menu->addAction(tr("&Exit"), view, &QWidget::close);
    view->setScene(scene);
    view->setLayout(box);
    view->layout()->setMenuBar(bar);
    view->setWindowTitle(tabWidget->tabText(tabWidget->currentIndex()));
    view->show();

    QVector<QPair<QPair<int, int>, QPair<int, int>>> coord;
    QVector<std::string> parent;
    QVector<std::string> child;
    QVector<int> tab(maxTab + 1, 0);
    int x = 50;
    int y = 50;
    for (int i = 0; i < data.size(); ++i) {
        if (std::find(parent.begin(), parent.end(), data[i].first.first) == parent.end()) {
            parent.push_back(data[i].first.first);
            QGraphicsItem *text1 = scene->addText(QString::fromStdString(data[i].first.first));
            text1->setPos(x+200*data[i].first.second, y+50*tab[data[i].first.second]);
            ++tab[data[i].first.second];
        }
        for (int k = 0; k < data.size(); ++k) {
            if (data[i].first.first == data[k].first.first) {
                QGraphicsItem *text2 = scene->addText(QString::fromStdString(data[k].second.first));
                text2->setPos(x+200*data[k].second.second, y+50*tab[data[k].second.second]);
                ++tab[data[k].second.second];
                coord.push_back({{x+250*data[i].first.second, y+50*(tab[data[i].first.second]-1)}, {x+200*data[k].second.second, y+50*(tab[data[k].second.second]-1)}});
            }
        }
    }
    for (int i = 0; i < coord.size(); i++){
        scene->addLine(coord[i].first.first, coord[i].first.second, coord[i].second.first,coord[i].second.second);
    }
}

void MainWindow::saveGraph() {
    QString fileName = view->windowTitle();
    fileName.replace(".cpp", ".xml");
    QFile file(fileName);

    if (!file.open(QIODevice::WriteOnly)) {
        return;
    }
    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("SceneData");
    xmlWriter.writeAttribute("version", "v1.0");
    xmlWriter.writeStartElement("GraphicsItemList");

    QVector<int> tab(maxTab + 1, 0);
    QVector<std::string> parent;
    int x = 50;
    int y = 50;
    for (int i = 0; i < data.size(); ++i) {
        if (std::find(parent.begin(), parent.end(), data[i].first.first) == parent.end()) {
            parent.push_back(data[i].first.first);
            xmlWriter.writeStartElement("parent");
            xmlWriter.writeAttribute("xbeg", QString::number(x+200*data[i].first.second));
            xmlWriter.writeAttribute("ybeg", QString::number(y+50*tab[data[i].first.second]));
            xmlWriter.writeAttribute("funcName", QString::fromStdString(data[i].first.first));
            ++tab[data[i].first.second];
            for (int k = 0; k < data.size(); ++k) {
                if (data[i].first.first == data[k].first.first) {
                    xmlWriter.writeStartElement("child");
                    xmlWriter.writeAttribute("xend", QString::number(x+200*data[k].second.second));
                    xmlWriter.writeAttribute("yend", QString::number(y+50*tab[data[k].second.second]));
                    xmlWriter.writeAttribute("funcName", QString::fromStdString(data[k].second.first));
                    ++tab[data[k].second.second];
                    xmlWriter.writeEndElement();
                }
            }
        }
        xmlWriter.writeEndElement();
    }
    xmlWriter.writeEndElement();
    xmlWriter.writeEndElement();
    xmlWriter.writeEndDocument();
}

void MainWindow::xml() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open file"), "", tr("Graph Files  (*.xml)"));
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Ошибка файла", "Не удалось открыть файл", QMessageBox::Ok);
    } else {
        scene = new QGraphicsScene(this);
        scene->setSceneRect(0, 0, width(), height());
        view = new QGraphicsView;
        view->setScene(scene);
        view->setWindowTitle(fileName);
        view->show();

        QXmlStreamReader xmlReader;
        QVector<QPair<QPair<int, int>, QPair<int,int>>> coord;
        xmlReader.setDevice(&file);
        xmlReader.readNext();
        int xparent, yparent;
        while (!xmlReader.atEnd()) {
            if (xmlReader.isStartElement()) {
                if (xmlReader.name() == "parent"){
                    foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                        if (attr.name().toString() == "xbeg") {
                            xparent = attr.value().toString().toInt();
                        }
                        if (attr.name().toString() == "ybeg") {
                            yparent = attr.value().toString().toInt();
                        }
                        if (attr.name().toString() == "funcName"){
                            QGraphicsItem *text = scene->addText(attr.value().toString());
                            text->setPos(xparent, yparent);
                           }
                        }
                } else if (xmlReader.name() == "child"){
                    int xchild, ychild;
                    foreach(const QXmlStreamAttribute &attr, xmlReader.attributes()) {
                        if (attr.name().toString() == "xend") {
                            xchild = attr.value().toString().toInt();
                        }
                        if (attr.name().toString() == "yend") {
                            ychild = attr.value().toString().toInt();
                        }
                        if (attr.name().toString() == "funcName"){
                            QGraphicsItem *text = scene->addText(attr.value().toString());
                            text->setPos(xchild, ychild);
                            coord.push_back({{xparent, yparent},{xchild, ychild + 20}});

                           }
                        }
                    }
                }
                xmlReader.readNext();
            }
            for (int i = 0; i < coord.size(); i++){
                scene->addLine(coord[i].first.first, coord[i].first.second, coord[i].second.first, coord[i].second.second);
            }
        }
}

void MainWindow::newFile() {
    tabWidget->addTab(new MyQPlainTextEdit, "untitled.txt");
    setCurrentFile(QString(), tabWidget->currentIndex());
    ((MyQPlainTextEdit *)(tabWidget->currentWidget()))->document()->setModified(false);
}

void MainWindow::open() {
    QString fileName = QFileDialog::getOpenFileName(this);
    if (!fileName.isEmpty()) {
        tabWidget->addTab(new MyQPlainTextEdit, fileName);
        loadFile(fileName, tabWidget->count()-1);
        ((MyQPlainTextEdit *)(tabWidget->currentWidget()))->document()->setModified(false);
        ctags();
        cscope(fileName);
    }
}

void MainWindow::closeTabs(int index) {
    if (maybeSave(index)) {
        if (tabWidget->count() == 1) {
            tabWidget->removeTab(index);
            newFile();
        } else {
            tabWidget->removeTab(index);
            setCurrentFile(tabWidget->tabText(tabWidget->currentIndex()), tabWidget->currentIndex());
        }
    } else {
        saveFile(tabWidget->widget(index)->objectName(), index);
    }
}

bool MainWindow::save(int index) {
    if (index == 0 && tabWidget->tabText(index).isEmpty())
        index = tabWidget->currentIndex();
    if (tabWidget->tabText(index).isEmpty()) {
        return saveAs(index);
    } else {
        bool f = saveFile(tabWidget->tabText(index), index);
        ctags();
        cscope(tabWidget->tabText(index));
        return f;
    }
}

bool MainWindow::saveAs(int index) {
    QFileDialog dialog(this);
    dialog.setWindowModality(Qt::WindowModal);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    if (dialog.exec() != QDialog::Accepted)
        return false;
    return saveFile(dialog.selectedFiles().first(), index);
}

void MainWindow::about() {
   QMessageBox::about(this, tr("About Application"),
            tr("The <b>Application</b> example demonstrates how to "
               "write modern GUI applications using Qt, with a menu bar, "
               "toolbars, and a status bar."));
}

void MainWindow::documentWasModified() {
    setWindowModified(((MyQPlainTextEdit *)(tabWidget->currentWidget()))->document()->isModified());
}

void MainWindow::setFile(int index) {
    setCurrentFile(tabWidget->tabText(index), index);
}

void MainWindow::createActions() {
    tabWidget = new QTabWidget();
    tabWidget->setTabShape(QTabWidget::Triangular);
    tabWidget->setTabsClosable(true);
    tabWidget->setTabPosition(QTabWidget::North);
    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTabs(int)));
    connect(tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(setFile(int)));

    tabWidget->addTab(new MyQPlainTextEdit, "untitled.txt");
    gridLayout = new QGridLayout;
    gridLayout->addWidget(tabWidget);

    QWidget *cenrtal = new QWidget;
    cenrtal->setLayout(gridLayout);
    setCentralWidget(cenrtal);
    connect(((MyQPlainTextEdit *)(tabWidget->currentWidget()))->document(), &QTextDocument::contentsChanged, this, &MainWindow::documentWasModified);

    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    QToolBar *fileToolBar = addToolBar(tr("File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(":/images/new.png"));
    QAction *newAct = new QAction(newIcon, tr("&New"), this);
    newAct->setShortcuts(QKeySequence::New);
    newAct->setStatusTip(tr("Create a new file"));
    connect(newAct, &QAction::triggered, this, &MainWindow::newFile);
    fileMenu->addAction(newAct);
    fileToolBar->addAction(newAct);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(":/images/open.png"));
    QAction *openAct = new QAction(openIcon, tr("&Open..."), this);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, &QAction::triggered, this, &MainWindow::open);
    fileMenu->addAction(openAct);
    fileToolBar->addAction(openAct);

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(":/images/save.png"));
    QAction *saveAct = new QAction(saveIcon, tr("&Save"), this);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip(tr("Save the document to disk"));
    connect(saveAct, &QAction::triggered, this, &MainWindow::save);
    fileMenu->addAction(saveAct);
    fileToolBar->addAction(saveAct);

    const QIcon saveAsIcon = QIcon::fromTheme("document-save-as");
    QAction *saveAsAct = fileMenu->addAction(saveAsIcon, tr("Save &As..."), this, &MainWindow::saveAs);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    saveAsAct->setStatusTip(tr("Save the document under a new name"));

    QAction *xmlAct = new QAction(openIcon, tr("&Open .xml"), this);
    xmlAct->setShortcuts(QKeySequence::Open);
    xmlAct->setStatusTip(tr("Open xml file"));
    connect(xmlAct, &QAction::triggered, this, &MainWindow::xml);
    fileMenu->addAction(xmlAct);

    fileMenu->addSeparator();

    const QIcon exitIcon = QIcon::fromTheme("application-exit");
    QAction *exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction *aboutAct = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    aboutAct->setStatusTip(tr("Show the application's About box"));

    QAction *aboutQtAct = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));

    graphMenu = menuBar()->addMenu(tr("&Graph"));
    QAction *graphAct = graphMenu->addAction(tr("Graph"), this, &MainWindow::graph);
    graphAct->setStatusTip(tr("Graph"));

}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::open_file(QString fileName) {
    tabWidget->addTab(new MyQPlainTextEdit, fileName);
    loadFile(fileName, tabWidget->count()-1);
    cscope(fileName);
    ctags();
}

void MainWindow::readSettings() {
    QString path = QCoreApplication::applicationDirPath();
    QSettings config(QCoreApplication::applicationDirPath() + "/cofig.ini", QSettings::IniFormat);
    config.beginGroup("last_opened_files");
    QStringList key = config.allKeys();
    for (int i = 0; i < key.size(); ++i) {
        if (!config.value(key[i]).toString().isEmpty())
            open_file(config.value(key[i]).toString());
    }
    config.endGroup();
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
}

void MainWindow::writeSettings() {
    QSettings config(QCoreApplication::applicationDirPath() + "/cofig.ini", QSettings::IniFormat);
    config.beginGroup("last_opened_files");
    QStringList key = config.allKeys();
    for (int i = 0; i < key.size(); ++i) {
        QString cur;
        config.setValue("file"+cur.setNum(i), "");
    }

    for (int i = 0; i < tabWidget->count(); ++i) {
        QString cur;
        if (tabWidget->tabText(i) != "untitled.txt")
            config.setValue("file"+cur.setNum(i), tabWidget->tabText(i));
    }
    config.endGroup();
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
}

bool MainWindow::maybeSave(int index) {
    if (!(((MyQPlainTextEdit *)(tabWidget->widget(index)))->document()->isModified()))
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Application"),
                               tr("The document has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save(index);
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::loadFile(const QString &fileName, int index) {
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot read file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return;
    }

    QTextStream in(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    ((MyQPlainTextEdit *)(tabWidget->widget(index)))->document()->setPlainText(in.readAll());
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    setCurrentFile(fileName, index);
    statusBar()->showMessage(tr("File loaded"), 2000);
}



bool MainWindow::saveFile(const QString &fileName, int index) {
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("Application"),
                             tr("Cannot write file %1:\n%2.")
                             .arg(QDir::toNativeSeparators(fileName),
                                  file.errorString()));
        return false;
    }

    QTextStream out(&file);
#ifndef QT_NO_CURSOR
    QApplication::setOverrideCursor(Qt::WaitCursor);
#endif
    out << ((MyQPlainTextEdit *)(tabWidget->widget(index)))->document()->toPlainText();
#ifndef QT_NO_CURSOR
    QApplication::restoreOverrideCursor();
#endif

    tabWidget->setTabText(index, fileName);
    setCurrentFile(fileName, index);
    statusBar()->showMessage(tr("File saved"), 2000);
    return true;
}

void MainWindow::setCurrentFile(const QString &fileName, int index) {
    curFile = fileName;
    if (!(((MyQPlainTextEdit *)(tabWidget->widget(index)))->document()->isModified()))
        ((MyQPlainTextEdit *)(tabWidget->widget(index)))->document()->setModified(false);

    QString shownName = curFile;
    if (curFile.isEmpty())
        shownName = "untitled.txt";
    int last_ind = curFile.length();
    if (last_ind >= 3) {
        if ((QString(curFile[last_ind-4]) + QString(curFile[last_ind-3]) + QString(curFile[last_ind-2]) + QString(curFile[last_ind-1]) == QString(".cpp") ||
            QString(curFile[last_ind-2]) + QString(curFile[last_ind-1]) == QString(".h"))) {
                highlight = new Highlighter(((MyQPlainTextEdit *)(tabWidget->widget(index)))->document());
                setWindowModified(false);
                graphMenu->menuAction()->setEnabled(true);
        } else {
            graphMenu->menuAction()->setEnabled(false);
        }
    }
    setWindowFilePath(shownName);
}

QString MainWindow::strippedName(const QString &fullFileName) {
    return QFileInfo(fullFileName).fileName();
}

void MyQPlainTextEdit::mouseDoubleClickEvent(QMouseEvent * event) {
    MainWindow * m = (MainWindow *) QApplication::activeWindow();
    QString file_name = m->get_tab()->tabText(m->get_tab()->currentIndex());
    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < m->get_c()[file_name].size(); ++i) {
            if (m->get_c()[file_name][i].first == ((MyQPlainTextEdit *)(m->get_tab()->currentWidget()))->textCursor().blockNumber() + 1){
                QTextBlock block = ((MyQPlainTextEdit *)(m->get_tab()->currentWidget()))->document()->findBlockByLineNumber(m->get_c()[file_name][i].first);
                if (block.isVisible()) {
                    for (int k = m->get_c()[file_name][i].first; k < m->get_c()[file_name][i].second; ++k) {
                        QTextBlock block1 = ((MyQPlainTextEdit *)(m->get_tab()->currentWidget()))->document()->findBlockByLineNumber(k);
                        block1.setVisible(false);
                    }
                } else {
                    for (int k = m->get_c()[file_name][i].first; k < m->get_c()[file_name][i].second; ++k) {
                        QTextBlock block2 = ((MyQPlainTextEdit *)(m->get_tab()->currentWidget()))->document()->findBlockByLineNumber(k);
                        block2.setVisible(true);
                    }
                }
               ((MyQPlainTextEdit *)(m->get_tab()->currentWidget()))->viewport()->update();
            }
        }
    }
}

#ifndef QT_NO_SESSIONMANAGER
void MainWindow::commitData(QSessionManager &manager) {
    if (manager.allowsInteraction()) {
        if (!maybeSave(tabWidget->currentIndex()))
            manager.cancel();
    } else {
        if (((MyQPlainTextEdit *)(tabWidget->currentWidget()))->document()->isModified())
            save(tabWidget->currentIndex());
    }
}
#endif
