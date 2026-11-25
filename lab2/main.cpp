#include "audio_collection.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

int readInteger(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        std::string input;
        if (!std::getline(std::cin, input)) return 0;
        try {
            std::size_t used = 0;
            const int value = std::stoi(input, &used);
            if (used == input.size()) return value;
        } catch (const std::exception&) {
            // Повторный запрос при некорректном вводе.
        }
        std::cout << "Введите целое число.\n";
    }
}

void addTrack(AudioLibrary& library) {
    const std::string title = readLine("Название: ");
    const std::string artist = readLine("Исполнитель: ");
    const int duration = readInteger("Длительность в секундах: ");
    const std::string genre = readLine("Жанр: ");
    const std::string album = readLine("Альбом (можно оставить пустым): ");

    Track track(title, artist, duration, genre.empty() ? "Не указан" : genre, album);
    track.filePath = readLine("Путь к аудиофайлу (можно оставить пустым): ");
    const Track saved = library.addTrack(track);
    std::cout << "Трек добавлен. Его ID: " << saved.id << '\n';
}

void printTracks(const AudioLibrary& library) {
    if (library.tracks.empty()) {
        std::cout << "Коллекция пуста.\n";
        return;
    }
    std::cout << "\n--- Аудиоколлекция: " << library.name << " ---\n";
    for (const Track& track : library.tracks) {
        std::cout << track.id << " | " << track.title << " | " << track.artist
                  << " | " << track.formattedDuration() << " | " << track.genre
                  << " | рейтинг: " << track.rating
                  << (track.favorite ? " | избранное" : "") << '\n';
    }
}

void searchTracks(const AudioLibrary& library) {
    const std::string query = readLine("Введите часть названия: ");
    const std::vector<Track> results = library.searchByTitle(query);
    if (results.empty()) {
        std::cout << "Совпадений не найдено.\n";
        return;
    }
    for (const Track& track : results) {
        std::cout << track.id << " | " << track.title << " | " << track.artist << '\n';
    }
}

void createPlaylist(PlaylistManager& playlists, const AudioLibrary& library) {
    Playlist playlist;
    playlist.name = readLine("Название нового плейлиста: ");
    if (playlist.name.empty()) {
        std::cout << "Название не может быть пустым.\n";
        return;
    }
    while (true) {
        const std::string id = readLine("Введите ID трека для добавления (пустая строка — завершить): ");
        if (id.empty()) break;
        const Track* track = library.findTrack(id);
        if (track == nullptr) {
            std::cout << "Трек с таким ID не найден.\n";
            continue;
        }
        playlist.addTrack(*track);
        std::cout << "Трек добавлен в плейлист.\n";
    }
    playlists.addPlaylist(playlist);
    std::cout << "Плейлист создан.\n";
}

void showPlaylists(const PlaylistManager& playlists, const AudioLibrary& library) {
    if (playlists.playlists.empty()) {
        std::cout << "Плейлистов пока нет.\n";
        return;
    }
    for (const Playlist& playlist : playlists.playlists) {
        std::cout << "\n" << playlist.name << " (" << playlist.trackIds.size() << " треков)\n";
        for (const std::string& id : playlist.trackIds) {
            const Track* track = library.findTrack(id);
            if (track != nullptr) std::cout << "  " << track->title << " — " << track->artist << '\n';
        }
    }
}

void printMenu() {
    std::cout << "\n========== АУДИО КОЛЛЕКЦИЯ ==========\n"
              << "1. Добавить трек\n"
              << "2. Показать коллекцию\n"
              << "3. Найти трек по названию\n"
              << "4. Удалить трек\n"
              << "5. Создать плейлист\n"
              << "6. Показать плейлисты\n"
              << "7. Оценить трек\n"
              << "8. Добавить/убрать из избранного\n"
              << "9. Показать статистику\n"
              << "0. Выход\n";
}

} // namespace

int main() {
    AudioLibrary library;
    PlaylistManager playlists;

    while (true) {
        printMenu();
        const int choice = readInteger("Выберите действие: ");
        try {
            switch (choice) {
                case 1:
                    addTrack(library);
                    break;
                case 2:
                    printTracks(library);
                    break;
                case 3:
                    searchTracks(library);
                    break;
                case 4: {
                    const std::string id = readLine("ID удаляемого трека: ");
                    library.removeTrack(id);
                    const std::size_t updatedPlaylists = playlists.removeTrackFromAll(id);
                    std::cout << "Трек удалён. Обновлено плейлистов: "
                              << updatedPlaylists << '\n';
                    break;
                }
                case 5:
                    createPlaylist(playlists, library);
                    break;
                case 6:
                    showPlaylists(playlists, library);
                    break;
                case 7: {
                    const std::string id = readLine("ID трека: ");
                    const int rating = readInteger("Оценка (1–5): ");
                    library.rateTrack(id, rating);
                    std::cout << "Оценка сохранена.\n";
                    break;
                }
                case 8: {
                    const std::string id = readLine("ID трека: ");
                    library.toggleFavorite(id);
                    const Track* track = library.findTrack(id);
                    std::cout << (track->favorite ? "Добавлен в избранное.\n" : "Убран из избранного.\n");
                    break;
                }
                case 9: {
                    LibraryStatistics statistics;
                    for (const Track& track : library.tracks) statistics.include(track);
                    std::cout << "Количество треков: " << statistics.tracks << '\n'
                              << "Общая длительность: " << library.totalDuration() << " сек.\n"
                              << "Средняя длительность: " << statistics.averageDuration() << " сек.\n";
                    break;
                }
                case 0:
                    std::cout << "Работа программы завершена.\n";
                    return 0;
                default:
                    std::cout << "Неизвестный пункт меню.\n";
            }
        } catch (const AudioCollectionException& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}
