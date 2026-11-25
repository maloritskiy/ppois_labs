#ifndef AUDIO_SEARCH_HPP
#define AUDIO_SEARCH_HPP

#include "audio_core.hpp"

#include <cctype>
#include <vector>

// 24. Параметры поискового запроса.
class SearchQuery {
public:
    std::string text;
    std::string field = "title";
    bool caseSensitive = false;

    void validate() const {
        if (text.empty()) throw InvalidSearchQueryException();
    }

    const std::string& valueFor(const Track& track) const {
        if (field == "artist") return track.artist;
        if (field == "genre") return track.genre;
        return track.title;
    }

    bool matches(const Track& track) const {
        validate();
        const std::string& source = valueFor(track);
        if (caseSensitive) return source.find(text) != std::string::npos;
        return normalize(source).find(normalize(text)) != std::string::npos;
    }

    bool matchesExactly(const Track& track) const {
        validate();
        const std::string& source = valueFor(track);
        if (caseSensitive) return source == text;
        return normalize(source) == normalize(text);
    }

private:
    static std::string normalize(const std::string& value) {
        std::string result = value;
        for (char& character : result) {
            character = static_cast<char>(
                std::tolower(static_cast<unsigned char>(character))
            );
        }
        return result;
    }
};

// 25. Результат поиска одной композиции.
class SearchResult {
public:
    std::string trackId;
    double score = 0.0;
    std::string matchedField;

    void improveScore(double bonus) { score = std::max(0.0, score + bonus); }
    bool isRelevant() const { return score >= 0.5; }
};

// 26. Механизм поиска и ранжирования результатов.
class SearchEngine {
public:
    std::string lastQuery;
    std::size_t resultLimit = 20;
    bool exactMatch = false;

    double rank(const Track& track, const SearchQuery& query) {
        lastQuery = query.text;
        if (!query.matches(track)) return 0.0;

        const bool exact = query.matchesExactly(track);
        if (exactMatch && !exact) return 0.0;
        return exact ? 1.0 : 0.7;
    }

    std::vector<SearchResult> search(const std::vector<Track>& tracks,
                                     const SearchQuery& query) {
        lastQuery = query.text;
        query.validate();

        std::vector<SearchResult> results;
        for (const Track& track : tracks) {
            const double score = rank(track, query);
            if (score == 0.0) continue;

            results.push_back(SearchResult{track.id, score, query.field});
            if (results.size() >= resultLimit) break;
        }
        return results;
    }

    void setResultLimit(std::size_t limit) {
        resultLimit = std::max<std::size_t>(1, limit);
    }
};

// 27. Одна рекомендация для пользователя.
class Recommendation {
public:
    std::string trackId;
    std::string reason;
    double score = 0.0;

    void boost(double amount) { score = std::max(0.0, std::min(1.0, score + amount)); }
    bool qualifies(double minimum) const { return score >= minimum; }
};

// 28. Настройки отбора рекомендаций.
class RecommendationEngine {
public:
    double minimumScore = 0.5;
    std::size_t maximumResults = 10;

    bool accepts(const Recommendation& recommendation) const {
        return recommendation.qualifies(minimumScore);
    }
    void configure(double minimum, std::size_t maximum) {
        minimumScore = std::max(0.0, std::min(1.0, minimum));
        maximumResults = std::max<std::size_t>(1, maximum);
    }
};

// 29. Модель задачи скачивания аудиофайла.
class DownloadTask {
public:
    std::string trackId;
    std::string sourceUrl;
    int progressPercent = 0;

    void advance(int percent) { progressPercent = std::max(0, std::min(100, progressPercent + percent)); }
    bool isComplete() const { return progressPercent == 100; }
};

#endif // AUDIO_SEARCH_HPP