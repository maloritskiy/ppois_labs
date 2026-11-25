#ifndef AUDIO_METADATA_HPP
#define AUDIO_METADATA_HPP

#include "audio_core.hpp"

// 9. Дополнительные метаданные трека.
class TrackMetadata {
public:
    std::string composer;
    std::string publisher;
    int year = 0;

    void applyTo(Track& track) const { if (year > 0) track.releaseYear = year; }
    int missingFieldCount() const {
        return static_cast<int>(composer.empty()) + static_cast<int>(publisher.empty()) + static_cast<int>(year <= 0);
    }
};

// 10. Обложка альбома.
class CoverArt {
public:
    std::string imagePath;
    int width = 0;
    int height = 0;

    bool isSquare() const { return width > 0 && width == height; }
    bool isUsable() const { return !imagePath.empty() && width >= 100 && height >= 100; }
};

// 11. Текст песни.
class Lyrics {
public:
    std::string text;
    std::string language;
    bool synchronized = false;

    std::size_t wordCount() const {
        std::size_t count = 0;
        bool insideWord = false;
        for (unsigned char ch : text) {
            if (std::isspace(ch)) insideWord = false;
            else if (!insideWord) { insideWord = true; ++count; }
        }
        return count;
    }
    bool containsText(const std::string& fragment) const {
        return !fragment.empty() && text.find(fragment) != std::string::npos;
    }
};

// 12. Тег для классификации трека.
class Tag {
public:
    std::string key;
    std::string value;
    bool userCreated = true;

    bool matches(const std::string& expected) const { return value == expected; }
    void normalizeValue() {
        std::transform(value.begin(), value.end(), value.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    }
};

// 13. Оценка трека с текстовым отзывом.
class TrackRating {
public:
    int score = 0;
    std::string review;
    std::string reviewer;

    void applyTo(Track& track) const { track.setRating(score); }
    bool isPositive() const { return score >= 4; }
};

// 14. Одно событие прослушивания.
class ListeningEvent {
public:
    std::string trackId;
    int playedSeconds = 0;
    std::string timestamp;

    double completionRatio(const Track& track) const {
        return track.durationSeconds <= 0 ? 0.0 :
            std::min(1.0, static_cast<double>(playedSeconds) / track.durationSeconds);
    }
    bool isCompleted(const Track& track) const { return completionRatio(track) >= 0.9; }
};

// 15. История прослушиваний.
class ListeningHistory {
public:
    std::vector<ListeningEvent> events;
    std::size_t capacity = 100;
    int totalPlayedSeconds = 0;

    void record(const ListeningEvent& event) {
        if (capacity == 0) return;
        events.push_back(event);
        totalPlayedSeconds += std::max(0, event.playedSeconds);
        if (events.size() > capacity) {
            totalPlayedSeconds -= std::max(0, events.front().playedSeconds);
            events.erase(events.begin());
        }
    }
    std::size_t playCountFor(const std::string& trackId) const {
        return static_cast<std::size_t>(std::count_if(events.begin(), events.end(),
            [&trackId](const ListeningEvent& event) { return event.trackId == trackId; }));
    }
};

// 16. Профиль слушателя.
class UserProfile {
public:
    std::string username;
    std::string displayName;
    std::string preferredGenre;

    void changeDisplayName(const std::string& value) {
        if (value.empty()) throw InvalidTrackDataException("Отображаемое имя не может быть пустым");
        displayName = value;
    }
    bool likesGenre(const Genre& genre) const { return preferredGenre == genre.name; }
};

#endif // AUDIO_METADATA_HPP