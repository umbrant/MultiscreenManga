#ifndef SIDEBAR_H
#define SIDEBAR_H
#include <QWidget>
#include <QTreeView>
class QFileSystemModel;

class FileViewer: public QTreeView
{
    Q_OBJECT
public:
    FileViewer(QWidget *parent = 0);
protected:
    void mousePressEvent(QMouseEvent * event);
public slots:
    void selectRoot();
};

class Sidebar: public QWidget
{
    Q_OBJECT

public:
    Sidebar(QWidget *parent = 0);
public slots:
    void modelItemSelected(const QModelIndex & modelIndex);
signals:
    void filePathSelected(const QString &);
private:
    FileViewer * tree = 0;
};
#endif
