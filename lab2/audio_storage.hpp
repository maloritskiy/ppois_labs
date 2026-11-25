#ifndef AUDIO_STORAGE_HPP
#define AUDIO_STORAGE_HPP

#include "audio_core.hpp"

// 30. Управление дисковым пространством.
class StorageManager {
public:
    std::string rootPath;
    std::size_t usedBytes = 0;
    std::size_t limitBytes = 1024U * 1024U * 1024U;

    bool canStore(std::size_t bytes) const {
        return bytes <= limitBytes - std::min(usedBytes, limitBytes);
    }
    bool canStoreFile(const AudioFile& file) const { return canStore(file.sizeBytes); }
    void reserve(std::size_t bytes) {
        if (!canStore(bytes)) throw StorageLimitExceededException();
        usedBytes += bytes;
    }
};

// 31. Контрольная сумма и размер аудиофайла.
class FileFingerprint {
public:
    std::string checksum;
    std::size_t sizeBytes = 0;

    bool sameContent(const AudioFile& file) const {
        return !checksum.empty() && checksum == file.checksum && sizeBytes == file.sizeBytes;
    }
    void updateFrom(const AudioFile& file) {
        checksum = file.checksum;
        sizeBytes = file.sizeBytes;
    }
};

// 32. Поиск точных дубликатов по контрольной сумме.
class DuplicateDetector {
public:
    std::size_t checkedFiles = 0;
    std::size_t duplicateFiles = 0;

    bool inspect(const AudioFile& first, const AudioFile& second) {
        ++checkedFiles;
        const bool duplicate = !first.checksum.empty() && first.checksum == second.checksum;
        if (duplicate) ++duplicateFiles;
        return duplicate;
    }
    double duplicateRatio() const {
        return checkedFiles == 0 ? 0.0 : static_cast<double>(duplicateFiles) / checkedFiles;
    }
};

// 33. Описание резервной копии коллекции.
class Backup {
public:
    std::string destination;
    std::string createdAt;
    std::vector<std::string> filePaths;

    void include(const AudioFile& file) {
        if (file.hasContent()) filePaths.push_back(file.path);
    }
    bool isUsable() const { return !destination.empty() && !filePaths.empty(); }
};

// 34. Управление списком резервных копий.
class BackupManager {
public:
    std::string backupDirectory;
    std::size_t maximumBackups = 5;
    std::vector<Backup> backups;

    void registerBackup(const Backup& backup) {
        if (!backup.isUsable() || maximumBackups == 0) return;
        while (backups.size() >= maximumBackups) backups.erase(backups.begin());
        backups.push_back(backup);
    }
    void removeOldest() {
        if (!backups.empty()) backups.erase(backups.begin());
    }
};

// 35. Результаты импорта файлов.
class ImportReport {
public:
    int imported = 0;
    int skipped = 0;
    int errors = 0;

    void recordImported() { ++imported; }
    void recordSkipped() { ++skipped; }
    void recordFailure(const std::string& sourcePath) {
        if (sourcePath.empty()) throw ImportFailedException("Не указан путь к файлу, импорт которого завершился ошибкой");
        ++errors;
    }
    double successRate() const {
        const int total = imported + skipped + errors;
        return total == 0 ? 0.0 : static_cast<double>(imported) / total;
    }
};

// 36. Результаты экспорта коллекции.
class ExportReport {
public:
    int exported = 0;
    std::string destination;
    std::string format = "m3u";
    bool metadataIncluded = true;

    void recordExport(const std::string& outputFormat, bool includeMetadata) {
        ++exported;
        format = outputFormat;
        metadataIncluded = includeMetadata;
    }
    bool hasOutput() const { return exported > 0 && !destination.empty(); }
};

// 37. Проверка форматов при импорте аудиофайлов.
class LibraryImporter {
public:
    std::vector<std::string> supportedExtensions{"mp3", "wav", "flac", "ogg", "m4a"};
    int importedCount = 0;

    bool supports(const AudioFormat& format) const {
        return std::find(supportedExtensions.begin(), supportedExtensions.end(), format.extension)
               != supportedExtensions.end();
    }
    void noteImported() { ++importedCount; }
    void validateFormat(const AudioFormat& format) const {
        if (!supports(format)) throw UnsupportedAudioFormatException();
    }
};

// 38. Модель экспорта композиций из коллекции.
class LibraryExporter {
public:
    std::string format = "m3u";
    bool includeMetadata = true;
    int exportedCount = 0;

    void configure(const std::string& newFormat, bool metadata) {
        if (newFormat.empty()) throw InvalidTrackDataException("Формат экспорта не задан");
        format = newFormat;
        includeMetadata = metadata;
    }
    void exportTrack(const Track& track, ExportReport& report) {
        if (track.isValid()) {
            ++exportedCount;
            report.recordExport(format, includeMetadata);
        }
    }
};

// 39. Папка с аудиофайлами.
class AudioFolder {
public:
    std::string path;
    std::vector<AudioFile> files;

    void addFile(const AudioFile& file) {
        if (file.hasContent()) files.push_back(file);
    }
    std::size_t totalBytes() const {
        std::size_t total = 0;
        for (const AudioFile& file : files) total += file.sizeBytes;
        return total;
    }
};

#endif // AUDIO_STORAGE_HPP