#ifndef AUDIO_CORE_HPP
#define AUDIO_CORE_HPP

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Базовое исключение предметной области и специализированные ошибки.
class AudioCollectionException : public std::runtime_error {
public:
    explicit AudioCollectionException(const std::string& message)
        : std::runtime_error(message) {}
};

class TrackNotFoundException : public AudioCollectionException {
public:
    explicit TrackNotFoundException(const std::string& message = "Трек не найден")
        : AudioCollectionException(message) {}
};
class DuplicateTrackException : public AudioCollectionException {
public:
    explicit DuplicateTrackException(const std::string& message = "Трек уже существует")
        : AudioCollectionException(message) {}
};
class DuplicatePlaylistException : public AudioCollectionException {
public:
    explicit DuplicatePlaylistException(const std::string& message = "Плейлист с таким именем уже существует")
        : AudioCollectionException(message) {}
};
class InvalidTrackDataException : public AudioCollectionException {
public:
    explicit InvalidTrackDataException(const std::string& message = "Некорректные данные трека")
        : AudioCollectionException(message) {}
};
class InvalidRatingException : public AudioCollectionException {
public:
    explicit InvalidRatingException(const std::string& message = "Оценка должна быть от 1 до 5")
        : AudioCollectionException(message) {}
};
class InvalidDurationException : public AudioCollectionException {
public:
    explicit InvalidDurationException(const std::string& message = "Длительность должна быть положительной")
        : AudioCollectionException(message) {}
};
class PlaylistNotFoundException : public AudioCollectionException {
public:
    explicit PlaylistNotFoundException(const std::string& message = "Плейлист не найден")
        : AudioCollectionException(message) {}
};
class EmptyPlaylistException : public AudioCollectionException {
public:
    explicit EmptyPlaylistException(const std::string& message = "Плейлист пуст")
        : AudioCollectionException(message) {}
};
class UnsupportedAudioFormatException : public AudioCollectionException {
public:
    explicit UnsupportedAudioFormatException(const std::string& message = "Формат аудио не поддерживается")
        : AudioCollectionException(message) {}
};
class StorageLimitExceededException : public AudioCollectionException {
public:
    explicit StorageLimitExceededException(const std::string& message = "Недостаточно места в хранилище")
        : AudioCollectionException(message) {}
};
class DeviceUnavailableException : public AudioCollectionException {
public:
    explicit DeviceUnavailableException(const std::string& message = "Аудиоустройство недоступно")
        : AudioCollectionException(message) {}
};
class InvalidSearchQueryException : public AudioCollectionException {
public:
    explicit InvalidSearchQueryException(const std::string& message = "Поисковый запрос не может быть пустым")
        : AudioCollectionException(message) {}
};
class ImportFailedException : public AudioCollectionException {
public:
    explicit ImportFailedException(const std::string& message = "Не удалось импортировать аудиофайл")
        : AudioCollectionException(message) {}
};

// 1. Жанр музыкального произведения.
class Genre {
public:
    std::string name;
    std::string description;
    int popularity = 0;

    std::string normalizedName() const {
        std::string result = name;
        for (char& character : result) {
            character = static_cast<char>(
                std::tolower(static_cast<unsigned char>(character))
            );
        }
        return result;
    }
    bool isPopular() const { return popularity >= 70; }
};

// 2. Исполнитель.
class Artist {
public:
    std::string name;
    std::string country;
    int monthlyListeners = 0;

    void rename(const std::string& newName) {
        if (newName.empty()) throw InvalidTrackDataException("Имя исполнителя не может быть пустым");
        name = newName;
    }
    bool matchesTrack(const class Track& track) const;
};

// 3. Формат аудиофайла.
class AudioFormat {
public:
    std::string extension;
    int bitrate = 0;
    bool lossless = false;

    int qualityScore() const { return lossless ? 100 : std::min(100, bitrate / 2); }
    bool isSupported() const {
        const std::string ext = extension;
        return ext == "mp3" || ext == "wav" || ext == "flac" || ext == "ogg" || ext == "m4a";
    }
};

// 4. Файл, содержащий аудиоданные.
class AudioFile {
public:
    std::string path;
    std::string checksum;
    std::size_t sizeBytes = 0;

    void moveTo(const std::string& newPath) {
        if (newPath.empty()) throw InvalidTrackDataException("Путь к файлу не может быть пустым");
        path = newPath;
    }
    bool hasContent() const { return !path.empty() && sizeBytes > 0; }
};

// 5. Музыкальный трек — центральная сущность коллекции.
class Track {
public:
    std::string id;
    std::string title;
    std::string artist;
    std::string album;
    std::string genre;
    int durationSeconds = 0;
    int releaseYear = 0;
    int rating = 0;
    bool favorite = false;
    std::string filePath;

    Track() = default;
    Track(std::string trackTitle, std::string trackArtist, int duration,
          std::string trackGenre = "Не указан", std::string trackAlbum = "")
        : title(std::move(trackTitle)), artist(std::move(trackArtist)), album(std::move(trackAlbum)),
          genre(std::move(trackGenre)), durationSeconds(duration) {}

    void rename(const std::string& newTitle) {
        if (newTitle.empty()) throw InvalidTrackDataException("Название трека не может быть пустым");
        title = newTitle;
    }
    void setRating(int value) {
        if (value < 1 || value > 5) throw InvalidRatingException();
        rating = value;
    }
    void toggleFavorite() { favorite = !favorite; }
    bool isValid() const { return !title.empty() && !artist.empty() && durationSeconds > 0; }
    std::string formattedDuration() const {
        const int minutes = durationSeconds / 60;
        const int seconds = durationSeconds % 60;
        return std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
    }
};

inline bool Artist::matchesTrack(const Track& track) const {
    return track.artist == name;
}

// 6. Альбом и список входящих в него треков.
class Album {
public:
    std::string title;
    std::string artist;
    int releaseYear = 0;
    std::vector<std::string> trackIds;

    void addTrack(const Track& track) {
        if (track.id.empty()) throw InvalidTrackDataException("Для добавления в альбом треку нужен ID");
        if (std::find(trackIds.begin(), trackIds.end(), track.id) == trackIds.end()) trackIds.push_back(track.id);
    }
    bool removeTrack(const std::string& trackId) {
        const auto oldSize = trackIds.size();
        trackIds.erase(std::remove(trackIds.begin(), trackIds.end(), trackId), trackIds.end());
        return oldSize != trackIds.size();
    }
};

// 7. Пользовательский плейлист.
class Playlist {
public:
    std::string name;
    std::vector<std::string> trackIds;
    bool isPublic = false;

    void addTrack(const Track& track) {
        if (track.id.empty()) throw InvalidTrackDataException("Для плейлиста треку нужен ID");
        if (std::find(trackIds.begin(), trackIds.end(), track.id) == trackIds.end()) trackIds.push_back(track.id);
    }
    bool removeTrack(const std::string& trackId) {
        const auto oldSize = trackIds.size();
        trackIds.erase(std::remove(trackIds.begin(), trackIds.end(), trackId), trackIds.end());
        return oldSize != trackIds.size();
    }
    void moveTrack(std::size_t from, std::size_t to) {
        if (from >= trackIds.size() || to >= trackIds.size()) return;
        const std::string value = trackIds[from];
        trackIds.erase(trackIds.begin() + static_cast<std::ptrdiff_t>(from));
        trackIds.insert(trackIds.begin() + static_cast<std::ptrdiff_t>(to), value);
    }
};

// 8. Управление именованными плейлистами.
class PlaylistManager {
public:
    std::vector<Playlist> playlists;
    std::size_t maximumPlaylists = 100;
    bool allowPublic = true;

    void addPlaylist(const Playlist& playlist) {
        if (playlist.name.empty()) throw InvalidTrackDataException("Название плейлиста не может быть пустым");
        if (findPlaylist(playlist.name) != nullptr) throw DuplicatePlaylistException();
        if (playlist.isPublic && !allowPublic) throw InvalidTrackDataException("Создание публичных плейлистов запрещено");
        if (playlists.size() >= maximumPlaylists) throw InvalidTrackDataException("Достигнут лимит плейлистов");
        playlists.push_back(playlist);
    }
    Playlist* findPlaylist(const std::string& playlistName) {
        const auto it = std::find_if(playlists.begin(), playlists.end(),
            [&playlistName](const Playlist& item) { return item.name == playlistName; });
        return it == playlists.end() ? nullptr : &(*it);
    }
    const Playlist* findPlaylist(const std::string& playlistName) const {
        const auto it = std::find_if(playlists.begin(), playlists.end(),
            [&playlistName](const Playlist& item) { return item.name == playlistName; });
        return it == playlists.end() ? nullptr : &(*it);
    }
    bool removePlaylist(const std::string& playlistName) {
        const auto it = std::find_if(playlists.begin(), playlists.end(),
            [&playlistName](const Playlist& item) { return item.name == playlistName; });
        if (it == playlists.end()) throw PlaylistNotFoundException();
        playlists.erase(it);
        return true;
    }
    void addTrack(const std::string& playlistName, const Track& track) {
        Playlist* playlist = findPlaylist(playlistName);
        if (playlist == nullptr) throw PlaylistNotFoundException();
        playlist->addTrack(track);
    }
    std::size_t removeTrackFromAll(const std::string& trackId) {
        std::size_t updatedPlaylists = 0;
        for (Playlist& playlist : playlists) {
            if (playlist.removeTrack(trackId)) ++updatedPlaylists;
        }
        return updatedPlaylists;
    }

    std::size_t playlistCount() const { return playlists.size(); }
};

// 9. Основной каталог аудиоколлекции.
class AudioLibrary {
public:
    std::string name = "Моя аудиоколлекция";
    std::vector<Track> tracks;
    int nextId = 1;

    Track addTrack(Track track) {
        if (track.title.empty() || track.artist.empty()) throw InvalidTrackDataException();
        if (track.durationSeconds <= 0) throw InvalidDurationException();
        if (track.id.empty()) track.id = "T" + std::to_string(nextId++);
        if (findTrack(track.id) != nullptr) throw DuplicateTrackException();
        tracks.push_back(track);
        return track;
    }
    bool removeTrack(const std::string& trackId) {
        const auto it = std::find_if(tracks.begin(), tracks.end(),
                                     [&trackId](const Track& item) { return item.id == trackId; });
        if (it == tracks.end()) throw TrackNotFoundException();
        tracks.erase(it);
        return true;
    }
    Track* findTrack(const std::string& trackId) {
        const auto it = std::find_if(tracks.begin(), tracks.end(),
                                     [&trackId](const Track& item) { return item.id == trackId; });
        return it == tracks.end() ? nullptr : &(*it);
    }
    const Track* findTrack(const std::string& trackId) const {
        const auto it = std::find_if(tracks.begin(), tracks.end(),
                                     [&trackId](const Track& item) { return item.id == trackId; });
        return it == tracks.end() ? nullptr : &(*it);
    }
    std::vector<Track> searchByTitle(const std::string& query) const {
        if (query.empty()) throw InvalidSearchQueryException();
        std::vector<Track> result;
        for (const Track& track : tracks) {
            if (track.title.find(query) != std::string::npos) result.push_back(track);
        }
        return result;
    }
    void rateTrack(const std::string& trackId, int ratingValue) {
        Track* track = findTrack(trackId);
        if (track == nullptr) throw TrackNotFoundException();
        track->setRating(ratingValue);
    }
    void toggleFavorite(const std::string& trackId) {
        Track* track = findTrack(trackId);
        if (track == nullptr) throw TrackNotFoundException();
        track->toggleFavorite();
    }
    std::size_t trackCount() const { return tracks.size(); }
    int totalDuration() const {
        int total = 0;
        for (const Track& track : tracks) total += track.durationSeconds;
        return total;
    }
};

#endif // AUDIO_CORE_HPP