#ifndef MANGAVOLUME_H
#define MANGAVOLUME_H

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
    MangaPage(const QString & path): filepath(path)
      , filename(filepath.split("/").last())
      ,data(QImage(path))
    {}
    MangaPage(const QImage & data, const QString & path):
        filepath(path)
      , filename(filepath.split("/").last())
      , data(data)
    {}


    bool isNull()const {return data.isNull();}
    const QString & getFilepath()const {return filepath;}
    const QString & getFilename()const {return filename;}
    const QImage & getData()const {return data;}


private:
    QString filepath;
    QString filename;
    QImage data;
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
    virtual uint size() const = 0;
    virtual std::shared_ptr<const QImage> getImage (uint page_num, QPointF scale=QPointF(1.0f,1.0f)) const = 0;
    virtual bool refreshOnResize() const {return false;}
protected:
    QString m_file_dir;
    virtual void cleanUp(const QString &path) {}
    bool m_do_cleanup;
};

class CompressedFileMangaVolume : public MangaVolume
{
    Q_OBJECT
public:
    explicit CompressedFileMangaVolume(const QString filepath, QObject *parent = 0);
    uint size() const {return m_pages.size();}
    std::shared_ptr<const QImage> getImage (uint page_num, QPointF) const;

private:
    std::vector<MangaPage> m_pages;

    void cleanUp(const QString & path);
    void readImages(const QString & path);
};


class PDFMangaVolume : public MangaVolume
{
    Q_OBJECT
public:
    explicit PDFMangaVolume(const QString filepath, QObject *parent = 0);
    uint size() const {if ( m_doc ) {return m_doc->numPages();} else {return 0;}}
    std::shared_ptr<const QImage> getImage (uint page_num, QPointF scale) const;
    bool refreshOnResize() const {return true;}

private:
    std::unique_ptr<Poppler::Document> m_doc;
    std::set<std::shared_ptr<const QImage> > m_active_pages;
    QString m_file_dir;

    void readImages(const QString & path);
};

#endif // MANGAVOLUME_H
