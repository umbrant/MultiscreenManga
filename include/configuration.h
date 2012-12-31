#ifndef CONFIG_H
#define CONFIG_H

#include <QList>
#include <QString>
#include <QImageReader>

/**
 * This should never be instantiated. Initialization happens in 
 * ConfigurationHidden's constructor, and a static instance of
 * ConfigurationHidden is embedded in Configuration (which is a
 * friend class).
 *
 * This class should have no public fields or methods.
 */
class ConfigurationHidden
{
public:
    friend class Configuration;
    QList<QString> supportedImageFormats;
protected:
    static ConfigurationHidden & Instance() {
        static ConfigurationHidden * ch = new ConfigurationHidden();
        return *ch;
    }
private:
    ConfigurationHidden()
    {
        // Read in Qt's reported supported image formats
        QList<QByteArray> formatsBytes = QImageReader::supportedImageFormats();
        for (int i=0; i<formatsBytes.size(); i++) {
            const QString* str = new QString(formatsBytes.at(i));
            supportedImageFormats.push_back(*str);
        }
    }

};

class Configuration
{
private:
    ConfigurationHidden config;
public:
    Configuration(): config(ConfigurationHidden::Instance()) {}
    const QList<QString> & getSupportedImageFormats() const {
        return config.supportedImageFormats;
    }
};

#endif
