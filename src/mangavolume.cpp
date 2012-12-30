#include "include/mangavolume.h"
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QCryptographicHash>
#include <QTime>
#include <QDebug>
#include <QDir>

const size_t num_extensions = 6;
QString extensions[num_extensions] = {
    "png",
    "jpg",
    "jpeg",
    "tif",
    "tiff",
    "bmp"
};
const std::set<QString> DirectoryMangaVolume::m_valid_extensions(extensions,extensions+num_extensions);

DirectoryMangaVolume::DirectoryMangaVolume(bool cleanup, QObject * parent)
    : MangaVolume(cleanup, parent)
{}
DirectoryMangaVolume::DirectoryMangaVolume(const QString & dirpath, QObject *parent)
    : MangaVolume(false, parent)
{
    readImages(dirpath);
}

CompressedFileMangaVolume::CompressedFileMangaVolume(const QString & filepath, QObject *parent)
    : DirectoryMangaVolume(true,parent) {

    QByteArray ba = str1.toLocal8Bit();
    const char *filename = ba.data();

    struct archive *a;
    struct archive_entry *entry;
    int r;

    // Initialize archive struct
    a = archive_read_new();
    archive_read_support_filter_all(a);
    archive_read_support_format_all(a);
    // Attempt opening the file as an archive
    r = archive_read_open_filename(a, filename, 8*1024*1024);
    if (r != ARCHIVE_OK) {
        return -1;
    }
    // Iterate through the files in the archive
    while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
        printf("%s\n",archive_entry_pathname(entry));
        archive_read_data_skip(a);
    }
    // Cleanup
    r = archive_read_free(a);
    if (r != ARCHIVE_OK) {
        return -1;
    }

    /*
    QStringList path_split = filepath.split("/");
    QString filename = path_split.last();
    QStringList filename_split = filename.split(".");
    if (filename_split.length() > 1) {
        filename_split.pop_back();
    }

    QDir dir;
    do {
        // keep trying hashes until dir exists.
        // no one could have taken all hashes
        QTime time = QTime::currentTime();
        QString toHash = filepath+time.toString();
        qWarning() <<  toHash;
        QString hash = QString(QCryptographicHash::hash(
                                   toHash.toAscii(),
                                   QCryptographicHash::Sha1).toHex());
        m_file_dir = tr("/tmp/") + filename_split.join(tr("."))
                + tr("-") + hash;
        qWarning() << "Making directory: " << m_file_dir;
        dir = QDir(m_file_dir);
    } while (dir.exists());
    dir.mkpath(".");

    QString program = "";
    QStringList arguments;
    if (filename.endsWith(tr(".zip"))) {
        program = tr("unzip");
        arguments << tr("-d") << m_file_dir;
        arguments << filepath;
    } else if (filename.endsWith(tr(".rar"))) {
        program = tr("unrar");
        arguments << tr("x");
        arguments << filepath;
        arguments << m_file_dir;
    } else {
        qWarning() << "Unknown filetype for file " << filename;
        return;
    }

    qWarning() << "Open file?: " << filename;
    QProcess * myProcess = new QProcess(this);

    // Start the extraction program
    myProcess->start(program, arguments);

    // Check to make sure it started correctly
    if (!myProcess->waitForStarted()) {
        switch (myProcess->error()) {
        case QProcess::FailedToStart:
            qWarning() << "Failed to start program" << program << ". Is it installed correctly?";
            break;
        case QProcess::Crashed:
            qWarning() << "Program" << program << "crashed.";
            break;
        default:
            qWarning() << "QProcess::ProcessError code " << myProcess->error();
        }
        return;
    }

    // Check to make sure it finished correctly
    if (!myProcess->waitForFinished()) {
        qWarning() << program << "was unable to extract file " << filepath;
        // TODO(umbrant): capture stdout/stderr to show the user
        return;
    }

    // Successful extraction
    qWarning() << "Extracted successfully";
    m_do_cleanup = true;

    // Read each of the extracted files into m_pages<MangaPage>
    readImages(m_file_dir);

    // XXX(umbrant): Looks like dead code?
    for (const MangaPage& page: m_pages) {
        page.getFilename().size();
        // TODO(mtao): processing?
    }
    */
}


std::shared_ptr<const QImage> DirectoryMangaVolume::getImage(uint page_num, QPointF) const {
    if (page_num >= m_pages.size()) {
        return std::shared_ptr<const QImage>();
    } else {
        return m_pages.at(page_num).getData();
    }
}


void DirectoryMangaVolume::readImages(const QString & path) {
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        // Recurse into sub-directories
        QDir dir(path);
        QStringList fileList = dir.entryList(
                    QDir::AllEntries | QDir::NoDotAndDotDot
                    | QDir::NoSymLinks | QDir::Hidden, QDir::Name
                    | QDir::IgnoreCase);
        for (int i = 0; i < fileList.count(); ++i) {
            readImages(path + tr("/")+fileList.at(i));
        }
    } else {
        QStringList filename_split = path.split(".");
        if ( m_valid_extensions.find(filename_split.last()) == m_valid_extensions.end() ) {
            return;
        } else {
            MangaPage img(path);
            if (!img.isNull()) {
                m_pages.push_back(img);
            }
        }
    }
}


void CompressedFileMangaVolume::cleanUp(const QString &path) {
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        QDir dir(path);
        QStringList fileList = dir.entryList(
                    QDir::AllEntries | QDir::NoDotAndDotDot
                    | QDir::NoSymLinks | QDir::Hidden, QDir::Name
                    | QDir::IgnoreCase);
        for (int i = 0; i < fileList.count(); ++i) {
            cleanUp(path + tr("/")+fileList.at(i));
        }
        dir.rmpath(tr("."));
    } else {
        QFile::remove(path);
    }
}


PDFMangaVolume::PDFMangaVolume(const QString filepath, QObject *parent): MangaVolume(false, parent){
    m_doc.reset(Poppler::Document::load(filepath));
    m_doc->setRenderHint(Poppler::Document::Antialiasing);
    m_doc->setRenderHint(Poppler::Document::TextAntialiasing);
}

std::shared_ptr<const QImage> PDFMangaVolume::getImage(uint page_num, QPointF scale) const {
    if(scale.x() != scale.x()) {
        scale.setX(1.0f);
    }
    if(scale.y() != scale.y()) {
        scale.setY(1.0f);
    }

    std::shared_ptr<const QImage> img;
    qWarning() << scale;
    if(m_doc && page_num < m_doc->numPages()) {
        img = std::shared_ptr<const QImage>(
                    new const QImage(m_doc->page(page_num)->renderToImage(72.0f*scale.x(),72.0f*scale.y()))
                    );
        //m_active_pages.insert(img);
    } else {
        return std::shared_ptr<const QImage>();
    }/*
    for (auto it = m_active_pages.begin(); it != m_active_pages.end();) {
        if(!*it)
        {
            it = m_active_pages.erase(it);
        } else {
            ++it;
        }
    }*/
    return img;
}
