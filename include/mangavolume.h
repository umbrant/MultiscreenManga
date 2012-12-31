#ifndef MANGAVOLUME_H
#define MANGAVOLUME_H

#include "include/configuration.h"
#include <QObject>
#include <vector>
#include <QImage>
#include <QDir>
#include <poppler/qt4/poppler-qt4.h>
#include <memory>
#include <set>


class MangaPage
{
public:
    MangaPage(const QString & path):
        data(new QImage(path)) {}

    bool isNull() const {if(!data) return true; return data->isNull();}
    std::shared_ptr<const QImage> getData() const {return data;}

private:
    std::shared_ptr<QImage> data;
};


class MangaVolume : public QObject
{
    Q_OBJECT
public:
    explicit MangaVolume(bool do_cleanup, QObject * parent = 0): QObject(parent), m_do_cleanup(do_cleanup) {}
    ~MangaVolume() {
        if (m_do_cleanup) {
            cleanUp(m_file_dir);
        }
    }
    // Number of pages in this volume
    virtual uint numPages() const {return size();};
    // Number of pages in this volume and all subvolumes
    virtual uint size() const = 0;
    virtual std::shared_ptr<const QImage> getImage (uint page_num, QPointF scale=QPointF(1.0f,1.0f)) const = 0;
    virtual bool refreshOnResize() const {return false;}
protected:
    QString m_file_dir;
    virtual void cleanUp(const QString &path) {}
    bool m_do_cleanup;
};


class DirectoryMangaVolume : public MangaVolume
{
    Q_OBJECT
public:
    explicit DirectoryMangaVolume(bool cleanup=false, QObject * parent = 0);
    explicit DirectoryMangaVolume(const QString & dirpath, QObject *parent = 0);
    std::shared_ptr<const QImage> getImage(uint page_num, QPointF) const;
    // Number of pages in this volume
    virtual uint numPages() const;
    // Number of pages in this volume and all subvolumes
    virtual uint size() const;

protected:
    std::vector<MangaPage> m_pages;
    virtual void readImages(const QString & path);

private:
    QList<QString> m_child_volumes;
};

/**
 * CFMV subclasses DMV, since it's current behavior is to extract the archive,
 * then attempt to treat it like a DMV.
 */
class CompressedFileMangaVolume : public DirectoryMangaVolume
{
public:
    explicit CompressedFileMangaVolume(const QString & filepath, QObject *parent = 0);

private:
    void cleanUp(const QString & path);
};


class PDFMangaVolume : public MangaVolume
{
    Q_OBJECT
public:
    explicit PDFMangaVolume(const QString filepath, QObject *parent = 0);
    uint size() const;
    std::shared_ptr<const QImage> getImage (uint page_num, QPointF scale) const;
    bool refreshOnResize() const {return true;}

private:
    std::unique_ptr<Poppler::Document> m_doc;
    std::set<std::shared_ptr<const QImage> > m_active_pages;
    QString m_file_dir;

    void readImages(const QString & path);
};

#endif // MANGAVOLUME_H
