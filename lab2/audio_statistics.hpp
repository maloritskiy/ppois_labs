#ifndef AUDIO_STATISTICS_HPP
#define AUDIO_STATISTICS_HPP

#include "audio_core.hpp"

#include <cctype>

// 40. Общая статистика библиотеки.
class LibraryStatistics {
public:
    std::size_t tracks = 0;
    long long totalSeconds = 0;
    std::size_t plays = 0;

    void include(const Track& track) { ++tracks; totalSeconds += track.durationSeconds; }
    void recordPlay() { ++plays; }
    double averageDuration() const { return tracks == 0 ? 0.0 : static_cast<double>(totalSeconds) / tracks; }
};

// 41. Статистика по жанру.
class GenreStatistics {
public:
    std::string genreName;
    std::size_t trackCount = 0;
    std::size_t plays = 0;

    void include(const Track& track) { if (track.genre == genreName) ++trackCount; }
    void recordPlay() { ++plays; }
    double averagePlays() const { return trackCount == 0 ? 0.0 : static_cast<double>(plays) / trackCount; }
};

// 42. Статистика по исполнителю.
class ArtistStatistics {
public:
    std::string artistName;
    std::size_t trackCount = 0;
    std::size_t totalPlays = 0;

    void include(const Track& track) { if (track.artist == artistName) ++trackCount; }
    void recordPlay() { ++totalPlays; }
    double popularityScore() const { return static_cast<double>(totalPlays) * 1.0 + trackCount * 0.25; }
};

// 43. Статистика по альбому.
class AlbumStatistics {
public:
    std::string albumTitle;
    std::size_t trackCount = 0;
    long long totalSeconds = 0;

    void include(const Track& track) { if (track.album == albumTitle) { ++trackCount; totalSeconds += track.durationSeconds; } }
    double averageDuration() const { return trackCount == 0 ? 0.0 : static_cast<double>(totalSeconds) / trackCount; }
};

// 44. Умный плейлист с условием по рейтингу.
class SmartPlaylist {
public:
    std::string name;
    int minimumRating = 0;
    std::string requiredGenre;

    bool matches(const Track& track) const {
        return track.rating >= minimumRating && (requiredGenre.empty() || track.genre == requiredGenre);
    }
    void relaxRating() { if (minimumRating > 0) --minimumRating; }
};

// 45. Правило фильтрации по текстовому полю.
class PlaylistRule {
public:
    std::string field = "title";
    std::string expected;
    bool negated = false;

    bool matchesText(const Track& track) const {
        const std::string& actual = field == "artist" ? track.artist :
                                    field == "genre" ? track.genre : track.title;
        const bool found = actual.find(expected) != std::string::npos;
        return negated ? !found : found;
    }
    void invert() { negated = !negated; }
};

// 46. Фильтр коллекции.
class CollectionFilter {
public:
    std::string text;
    std::string genre;
    int minimumRating = 0;

    bool matches(const Track& track) const {
        const bool textMatch = text.empty() || track.title.find(text) != std::string::npos ||
                               track.artist.find(text) != std::string::npos;
        const bool genreMatch = genre.empty() || track.genre == genre;
        return textMatch && genreMatch && track.rating >= minimumRating;
    }
    void clear() { text.clear(); genre.clear(); minimumRating = 0; }
};

// 47. Параметры сортировки.
class SortOptions {
public:
    std::string field = "title";
    bool ascending = true;
    bool ignoreCase = true;

    void reverseDirection() { ascending = !ascending; }

    bool before(const Track& first, const Track& second) const {
        if (field == "duration") {
            return ascending ? first.durationSeconds < second.durationSeconds
                             : first.durationSeconds > second.durationSeconds;
        }

        std::string firstValue = field == "artist" ? first.artist : first.title;
        std::string secondValue = field == "artist" ? second.artist : second.title;
        if (ignoreCase) {
            normalize(firstValue);
            normalize(secondValue);
        }
        return ascending ? firstValue < secondValue : firstValue > secondValue;
    }

private:
    static void normalize(std::string& value) {
        for (char& character : value) {
            character = static_cast<char>(
                std::tolower(static_cast<unsigned char>(character))
            );
        }
    }
};

// 48. Компаратор треков.
class TrackComparator {
public:
    std::string field = "title";
    bool descending = false;
    bool less(const Track& first, const Track& second) const {
        if (field == "artist") return descending ? first.artist > second.artist : first.artist < second.artist;
        if (field == "duration") return descending ? first.durationSeconds > second.durationSeconds
                                                    : first.durationSeconds < second.durationSeconds;
        return descending ? first.title > second.title : first.title < second.title;
    }
    void reverse() { descending = !descending; }
};

// 49. Список недавно проигранных треков.
class RecentlyPlayed {
public:
    std::vector<std::string> trackIds;
    std::size_t capacity = 10;
    bool newestFirst = true;

    void record(const Track& track) {
        trackIds.erase(std::remove(trackIds.begin(), trackIds.end(), track.id), trackIds.end());
        if (newestFirst) trackIds.insert(trackIds.begin(), track.id);
        else trackIds.push_back(track.id);
        if (trackIds.size() > capacity) trackIds.resize(capacity);
    }
    bool contains(const std::string& trackId) const {
        return std::find(trackIds.begin(), trackIds.end(), trackId) != trackIds.end();
    }
};

// 50. Коллекция избранного.
class FavoriteCollection {
public:
    std::string name = "Избранное";
    std::vector<std::string> trackIds;
    std::size_t maximumSize = 500;

    void add(const Track& track) {
        if (!track.id.empty() && trackIds.size() < maximumSize && !contains(track.id)) trackIds.push_back(track.id);
    }
    bool remove(const std::string& trackId) {
        const auto oldSize = trackIds.size();
        trackIds.erase(std::remove(trackIds.begin(), trackIds.end(), trackId), trackIds.end());
        return oldSize != trackIds.size();
    }
    bool contains(const std::string& trackId) const {
        return std::find(trackIds.begin(), trackIds.end(), trackId) != trackIds.end();
    }
};

#endif // AUDIO_STATISTICS_HPP