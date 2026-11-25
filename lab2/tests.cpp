#include "audio_collection.hpp"

#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <exception>

namespace {
int assertions = 0;

void check(bool condition, const std::string& message) {
    ++assertions;
    if (!condition) throw std::runtime_error("Проверка не пройдена: " + message);
}

template <typename Exception, typename Function>
void expectThrow(Function function, const std::string& message) {
    ++assertions;
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error("Ожидалось исключение: " + message);
}

void testCatalogCore() {
    Genre rock{"Rock", "Рок-музыка", 80};
    check(rock.normalizedName() == "rock" && rock.isPopular(), "жанр нормализуется и считается популярным");
    Genre ambient{"Ambient", "", 20};
    check(!ambient.isPopular(), "непопулярный жанр");

    Track track("Night Drive", "The Example", 185, "Rock", "First Album");
    track.id = "T1";
    Artist artist{"The Example", "UK", 1000};
    check(artist.matchesTrack(track), "исполнитель сопоставляется с треком");
    artist.rename("Example Band");
    check(artist.name == "Example Band", "переименование исполнителя");
    expectThrow<InvalidTrackDataException>([&] { artist.rename(""); }, "пустое имя исполнителя");

    AudioFormat mp3{"mp3", 320, false};
    AudioFormat flac{"flac", 900, true};
    AudioFormat strange{"xyz", 0, false};
    check(mp3.isSupported() && mp3.qualityScore() == 100, "поддерживаемый MP3");
    check(flac.qualityScore() == 100 && !strange.isSupported(), "lossless и неподдерживаемый формат");

    AudioFile file{"music/night-drive.mp3", "abc123", 2048};
    check(file.hasContent(), "аудиофайл содержит данные");
    file.moveTo("archive/night-drive.mp3");
    check(file.path == "archive/night-drive.mp3", "перемещение файла");
    expectThrow<InvalidTrackDataException>([&] { file.moveTo(""); }, "пустой путь");

    check(track.isValid() && track.formattedDuration() == "3:05", "проверка трека и формат длительности");
    track.rename("Night Drive (Remaster)");
    track.setRating(5);
    track.toggleFavorite();
    check(track.title == "Night Drive (Remaster)" && track.rating == 5 && track.favorite,
          "изменение данных трека");
    expectThrow<InvalidRatingException>([&] { track.setRating(6); }, "оценка вне диапазона");
    expectThrow<InvalidTrackDataException>([&] { track.rename(""); }, "пустое название трека");
    Track invalid;
    check(!invalid.isValid(), "пустой трек невалиден");

    Album album{"First Album", "The Example", 2024, {}};
    album.addTrack(track);
    album.addTrack(track);
    check(album.trackIds.size() == 1 && album.removeTrack("T1"), "добавление и удаление трека альбома");
    check(!album.removeTrack("missing"), "удаление отсутствующего трека из альбома");
    Track noId("Untitled", "Someone", 10);
    expectThrow<InvalidTrackDataException>([&] { album.addTrack(noId); }, "добавление трека без ID в альбом");
    noId.id = "T-extra";

    Playlist playlist{"Road", {}, false};
    playlist.addTrack(track);
    playlist.addTrack(noId);
    playlist.addTrack(track);
    playlist.moveTrack(0, 1);
    check(playlist.trackIds.size() == 2 && playlist.trackIds[1] == "T1", "операции с плейлистом");
    check(playlist.removeTrack("T1") && !playlist.removeTrack("none"), "удаление из плейлиста");
    playlist.moveTrack(100, 0);
    playlist.addTrack(track);

    PlaylistManager playlistManager;
    playlistManager.addPlaylist(playlist);
    check(playlistManager.findPlaylist("Road") != nullptr && playlistManager.playlistCount() == 1,
          "управление плейлистами");
    playlistManager.addTrack("Road", track);
    expectThrow<PlaylistNotFoundException>([&] { playlistManager.addTrack("Missing", track); },
                                            "добавление трека в отсутствующий плейлист");
    expectThrow<DuplicatePlaylistException>([&] { playlistManager.addPlaylist(playlist); },
                                            "дублирующий плейлист");
    check(playlistManager.removeTrackFromAll("T1") == 1,
          "удаление ID трека из всех плейлистов");
    check(playlistManager.removePlaylist("Road") && playlistManager.playlistCount() == 0,
          "удаление плейлиста");
    expectThrow<PlaylistNotFoundException>([&] { playlistManager.removePlaylist("Missing"); },
                                            "удаление отсутствующего плейлиста");
    PlaylistManager privateManager;
    privateManager.allowPublic = false;
    expectThrow<InvalidTrackDataException>([&] { privateManager.addPlaylist(Playlist{"Public", {}, true}); },
                                            "запрет публичных плейлистов");
    PlaylistManager limitedManager;
    limitedManager.maximumPlaylists = 0;
    expectThrow<InvalidTrackDataException>([&] { limitedManager.addPlaylist(Playlist{"Limited", {}, false}); },
                                            "лимит плейлистов");

    AudioLibrary library;
    Track saved = library.addTrack(track);
    check(saved.id == "T1" && library.trackCount() == 1, "добавление трека в библиотеку");
    check(library.findTrack("T1") != nullptr && library.findTrack("absent") == nullptr, "поиск трека по ID");
    check(library.searchByTitle("Night").size() == 1, "поиск по названию");
    check(library.totalDuration() == 185, "общая длительность библиотеки");
    library.rateTrack("T1", 4);
    library.toggleFavorite("T1");
    check(library.findTrack("T1")->rating == 4 && !library.findTrack("T1")->favorite,
          "оценка и избранное через библиотеку");
    expectThrow<DuplicateTrackException>([&] { library.addTrack(track); }, "дублирующий ID");
    expectThrow<InvalidDurationException>([&] { library.addTrack(Track("Bad", "Artist", 0)); }, "нулевая длительность");
    expectThrow<InvalidTrackDataException>([&] { library.addTrack(Track("", "Artist", 50)); }, "пустое название");
    expectThrow<InvalidSearchQueryException>([&] { library.searchByTitle(""); }, "пустой поиск библиотеки");
    expectThrow<TrackNotFoundException>([&] { library.removeTrack("missing"); }, "удаление неизвестного трека");
    expectThrow<TrackNotFoundException>([&] { library.rateTrack("missing", 3); }, "оценка неизвестного трека");
    expectThrow<TrackNotFoundException>([&] { library.toggleFavorite("missing"); }, "избранное неизвестного трека");
    check(library.removeTrack("T1") && library.trackCount() == 0, "удаление из библиотеки");
}

void testMetadataAndListening() {
    Track track("Song", "Singer", 100, "Pop", "");
    track.id = "T2";
    TrackMetadata metadata{"Composer", "Publisher", 2020};
    metadata.applyTo(track);
    check(track.releaseYear == 2020 && metadata.missingFieldCount() == 0, "метаданные трека");
    TrackMetadata incomplete;
    check(incomplete.missingFieldCount() == 3, "подсчёт незаполненных метаданных");

    CoverArt cover{"cover.png", 500, 500};
    check(cover.isSquare() && cover.isUsable(), "обложка корректна");
    CoverArt badCover{"", 20, 40};
    check(!badCover.isSquare() && !badCover.isUsable(), "некорректная обложка");

    Lyrics lyrics{"One two\nthree", "en", false};
    check(lyrics.wordCount() == 3 && lyrics.containsText("two"), "анализ текста песни");
    check(!lyrics.containsText("absent") && !lyrics.containsText(""), "поиск отсутствующего фрагмента");

    Tag tag{"mood", "Happy", true};
    check(tag.matches("Happy"), "сопоставление тега");
    tag.normalizeValue();
    check(tag.value == "happy", "нормализация тега");

    TrackRating rating{5, "Great", "Listener"};
    rating.applyTo(track);
    check(track.rating == 5 && rating.isPositive(), "применение оценки");
    TrackRating neutral{2, "", ""};
    check(!neutral.isPositive(), "отрицательная оценка");

    ListeningEvent event{"T2", 95, "2026-11-18"};
    check(event.completionRatio(track) == 0.95 && event.isCompleted(track), "процент прослушивания");
    Track zeroDuration("Zero", "Singer", 0);
    check(event.completionRatio(zeroDuration) == 0.0, "защита от нулевой длительности");

    ListeningHistory history;
    history.capacity = 2;
    history.record(event);
    history.record(event);
    history.record(ListeningEvent{"other", 10, "later"});
    check(history.events.size() == 2 && history.playCountFor("T2") == 1 && history.totalPlayedSeconds == 105,
          "история ограничивает размер и считает прослушивания");
    ListeningHistory disabled;
    disabled.capacity = 0;
    disabled.record(event);
    check(disabled.events.empty(), "история с нулевой ёмкостью");

    Genre pop{"Pop", "", 50};
    UserProfile profile{"user", "Listener", "Pop"};
    check(profile.likesGenre(pop), "предпочтительный жанр");
    profile.changeDisplayName("New Name");
    check(profile.displayName == "New Name", "смена отображаемого имени");
    expectThrow<InvalidTrackDataException>([&] { profile.changeDisplayName(""); }, "пустое отображаемое имя");
}

void testPlaybackAndSearch() {
    Track track("Blue Sky", "Singer", 200, "Jazz", "Album");
    track.id = "T3";
    PlaybackQueue queue;
    expectThrow<EmptyPlaylistException>([&] { queue.next(); }, "следующий трек пустой очереди");
    queue.enqueue(track);
    Track second("Red Moon", "Other", 100);
    second.id = "T4";
    queue.enqueue(second);
    check(queue.next() == "T4", "переход к следующему треку");
    check(queue.next() == "T4", "последний трек без повтора остаётся текущим");
    queue.repeat = true;
    check(queue.next() == "T3", "зацикливание очереди");

    PlaybackSession session;
    session.start(track);
    check(session.active && session.trackId == "T3", "начало сеанса воспроизведения");
    session.seek(150, track);
    check(session.positionSeconds == 150, "перемотка воспроизведения");
    session.seek(-10, track);
    check(session.positionSeconds == 0, "отрицательная позиция ограничивается нулём");
    session.seek(500, track);
    check(session.positionSeconds == 200, "перемотка не выходит за длительность");
    session.finish();
    check(!session.active && session.positionSeconds == 0, "завершение сеанса");

    PlaybackSettings settings;
    settings.setVolume(140);
    settings.toggleShuffle();
    settings.toggleRepeat();
    check(settings.volume == 100 && settings.shuffle && settings.repeat,
          "настройки воспроизведения");
    settings.setVolume(-5);
    check(settings.volume == 0, "минимальная громкость");

    EqualizerBand band{1000, 2.0, 1.0};
    band.adjustGain(20);
    check(band.gain == 12.0 && band.isBoosted(), "ограничение усиления эквалайзера");
    band.adjustGain(-30);
    check(band.gain == -12.0 && !band.isBoosted(), "ограничение ослабления эквалайзера");
    Equalizer equalizer;
    equalizer.addBand(band);
    equalizer.toggleEnabled();
    equalizer.reset();
    check(equalizer.bands.size() == 1 && equalizer.bands.front().gain == 0.0 && equalizer.enabled,
          "включение эквалайзера и сброс полос");

    AudioFormat format{"flac", 900, true};
    AudioDevice device{"Headphones", "Bluetooth", 50};
    check(device.supportsFormat(format), "устройство поддерживает формат");
    device.adjustVolume(80);
    check(device.volume == 100, "изменение громкости устройства");
    DeviceConnection connection;
    connection.connect(device);
    connection.updateLatency(25);
    check(connection.connected && connection.deviceName == "Headphones" && connection.latencyMs == 25,
          "подключение устройства и установка задержки");
    expectThrow<InvalidTrackDataException>([&] { connection.updateLatency(-1); },
                                           "отрицательная задержка устройства");
    connection.disconnect();
    check(!connection.connected, "отключение устройства");
    expectThrow<DeviceUnavailableException>([&] { connection.connect(AudioDevice{}); }, "подключение устройства без имени");

    SearchQuery query{"Blue", "title", false};
    query.validate();
    check(query.matches(track), "поиск по названию без учёта регистра");
    SearchQuery insensitiveQuery{"blue", "title", false};
    check(insensitiveQuery.matches(track), "поиск без учёта регистра");
    SearchQuery caseSensitiveQuery{"blue", "title", true};
    check(!caseSensitiveQuery.matches(track), "поиск с учётом регистра");
    SearchQuery artistQuery{"Singer", "artist", false};
    check(artistQuery.matches(track), "поиск по исполнителю");
    SearchQuery genreQuery{"Jazz", "genre", false};
    check(genreQuery.matches(track), "поиск по жанру");
    expectThrow<InvalidSearchQueryException>([] { SearchQuery empty; empty.validate(); }, "пустой поисковый запрос");

    SearchResult result{"T3", 0.4, "title"};
    check(!result.isRelevant(), "низкая релевантность");
    result.improveScore(0.2);
    check(result.isRelevant(), "повышение релевантности");
    SearchEngine engine;
    check(engine.rank(track, query) == 0.7 && engine.lastQuery == "Blue",
          "ранжирование частичного совпадения и сохранение запроса");
    query.text = "Blue Sky";
    check(engine.rank(track, query) == 1.0, "ранжирование точного совпадения");
    query.text = "Absent";
    check(engine.rank(track, query) == 0.0, "ранжирование отсутствующего совпадения");
    engine.exactMatch = true;
    query.text = "Blue";
    check(engine.rank(track, query) == 0.0, "режим точного совпадения отклоняет частичный результат");
    query.text = "Blue Sky";
    check(engine.search(std::vector<Track>{track, second}, query).size() == 1,
          "поиск возвращает только точные совпадения");
    engine.exactMatch = false;
    engine.setResultLimit(1);
    query.text = "";
    expectThrow<InvalidSearchQueryException>([&] { engine.search(std::vector<Track>{track}, query); },
                                             "пустой запрос движка поиска");
    query.text = "e";
    check(engine.search(std::vector<Track>{track, second}, query).size() == 1,
          "ограничение количества результатов");
    engine.setResultLimit(0);
    check(engine.resultLimit == 1, "минимальное число результатов");
}

void testRecommendationsStorageAndBackup() {
    Recommendation recommendation{"T3", "Similar genre", 0.4};
    recommendation.boost(0.3);
    check(recommendation.qualifies(0.6), "оценка рекомендации");
    recommendation.boost(5.0);
    check(recommendation.score == 1.0, "верхняя граница рекомендации");
    RecommendationEngine engine;
    check(engine.accepts(recommendation), "движок принимает рекомендацию");
    engine.configure(2.0, 0);
    check(engine.minimumScore == 1.0 && engine.maximumResults == 1, "ограничение настроек рекомендаций");

    DownloadTask download{"T3", "https://example.test/file", 90};
    download.advance(20);
    check(download.isComplete(), "завершение скачивания");
    download.advance(-200);
    check(download.progressPercent == 0, "нижняя граница загрузки");

    StorageManager storage{"music", 80, 100};
    check(storage.canStore(20) && !storage.canStore(21), "проверка свободного места");
    check(storage.canStoreFile(AudioFile{"file", "hash", 15}), "проверка размера аудиофайла");
    check(!storage.canStoreFile(AudioFile{"large", "hash", 100}), "аудиофайл не помещается в хранилище");
    storage.reserve(10);
    check(storage.usedBytes == 90, "резервирование пространства");
    expectThrow<StorageLimitExceededException>([&] { storage.reserve(20); }, "превышение лимита хранилища");

    AudioFile file{"song.flac", "hash-1", 1000};
    FileFingerprint fingerprint{"hash-1", 1000};
    check(fingerprint.sameContent(file), "совпадение отпечатка");
    file.checksum = "hash-2";
    check(!fingerprint.sameContent(file), "различие отпечатков");
    fingerprint.updateFrom(file);
    check(fingerprint.sameContent(file), "обновление отпечатка");

    DuplicateDetector detector;
    AudioFile copy{"copy.flac", "hash-2", 1000};
    check(detector.inspect(file, copy), "поиск дубликата");
    check(!detector.inspect(file, AudioFile{"other", "different", 10}), "отсутствие дубликата");
    check(detector.duplicateRatio() == 0.5, "доля дубликатов");
    DuplicateDetector emptyDetector;
    check(emptyDetector.duplicateRatio() == 0.0, "доля при отсутствии проверок");

    Backup backup{"backup", "2026-11-18", {}};
    backup.include(file);
    check(backup.isUsable(), "резервная копия включает файл");
    Backup emptyBackup;
    check(!emptyBackup.isUsable(), "пустая резервная копия непригодна");
    BackupManager manager;
    manager.removeOldest();
    manager.maximumBackups = 2;
    manager.registerBackup(backup);
    manager.registerBackup(emptyBackup);
    manager.registerBackup(backup);
    manager.registerBackup(backup);
    check(manager.backups.size() == 2, "регистрация с ограничением числа резервных копий");
    manager.removeOldest();
    check(manager.backups.size() == 1, "удаление старой копии");
    manager.maximumBackups = 0;
    manager.registerBackup(backup);
    check(manager.backups.size() == 1, "нулевой лимит запрещает регистрацию копий");
}

void testReportsStatisticsAndCollections() {
    ImportReport importReport;
    check(importReport.successRate() == 0.0, "пустой отчёт импорта");
    importReport.recordImported();
    importReport.recordSkipped();
    importReport.recordFailure("bad-file.mp3");
    expectThrow<ImportFailedException>([&] { importReport.recordFailure(""); }, "ошибка импорта без пути");
    check(std::abs(importReport.successRate() - 1.0 / 3.0) < 0.0001, "успешность импорта");

    ExportReport exportReport;
    check(!exportReport.hasOutput(), "пустой отчёт экспорта");
    exportReport.destination = "playlist.m3u";
    LibraryExporter exporter;
    exporter.exportTrack(Track{}, exportReport);
    exporter.exportTrack(Track("Valid", "Artist", 60), exportReport);
    check(exportReport.hasOutput() && exporter.exportedCount == 1 &&
          exportReport.format == "m3u" && exportReport.metadataIncluded,
          "экспорт корректного трека с параметрами отчёта");
    exporter.configure("pls", false);
    check(exporter.format == "pls" && !exporter.includeMetadata, "настройка экспорта");
    exporter.exportTrack(Track("Another", "Artist", 70), exportReport);
    check(exportReport.format == "pls" && !exportReport.metadataIncluded,
          "экспорт применяет выбранный формат и настройку метаданных");
    expectThrow<InvalidTrackDataException>([&] { exporter.configure("", true); }, "пустой формат экспорта");

    LibraryImporter importer;
    check(importer.supports(AudioFormat{"mp3", 320, false}), "импорт поддерживаемого формата");
    check(!importer.supports(AudioFormat{"aacx", 1, false}), "импорт неподдерживаемого формата");
    importer.validateFormat(AudioFormat{"mp3", 320, false});
    expectThrow<UnsupportedAudioFormatException>([&] { importer.validateFormat(AudioFormat{"aacx", 1, false}); },
                                                   "импорт неподдерживаемого формата вызывает исключение");
    importer.noteImported();
    check(importer.importedCount == 1, "счётчик импорта");

    Track track("Song", "Singer", 120, "Jazz", "Album");
    track.id = "T5";
    LibraryStatistics statistics;
    check(statistics.averageDuration() == 0.0, "средняя длительность пустой библиотеки");
    statistics.include(track);
    statistics.include(Track("Second", "Singer", 60, "Jazz", "Album"));
    statistics.recordPlay();
    check(statistics.tracks == 2 && statistics.averageDuration() == 90.0 && statistics.plays == 1,
          "общая статистика и число прослушиваний");

    GenreStatistics genreStats{"Jazz", 0, 0};
    genreStats.include(track);
    genreStats.include(Track("Other", "Singer", 10, "Pop"));
    genreStats.recordPlay();
    genreStats.recordPlay();
    check(genreStats.trackCount == 1 && genreStats.averagePlays() == 2.0, "статистика жанра");
    ArtistStatistics artistStats{"Singer", 0, 0};
    artistStats.include(track);
    artistStats.include(Track("Other", "Another", 10));
    artistStats.recordPlay();
    artistStats.recordPlay();
    check(artistStats.trackCount == 1 && artistStats.popularityScore() == 2.25,
          "статистика исполнителя");
    AlbumStatistics albumStats{"Album", 0, 0};
    albumStats.include(track);
    albumStats.include(Track("Other", "Singer", 60, "Jazz", "Album"));
    check(albumStats.trackCount == 2 && albumStats.averageDuration() == 90.0, "статистика альбома");
    AlbumStatistics emptyAlbumStats;
    check(emptyAlbumStats.averageDuration() == 0.0, "средняя длительность пустого альбома");

    SmartPlaylist smart{"Top Jazz", 4, "Jazz"};
    track.rating = 5;
    check(smart.matches(track), "условия умного плейлиста");
    smart.relaxRating();
    check(smart.minimumRating == 3, "ослабление рейтингового ограничения");
    PlaylistRule rule{"artist", "Singer", false};
    check(rule.matchesText(track), "правило по исполнителю");
    rule.invert();
    check(!rule.matchesText(track), "инверсия правила");

    CollectionFilter filter{"Song", "Jazz", 4};
    check(filter.matches(track), "фильтр коллекции");
    filter.clear();
    check(filter.matches(track) && filter.minimumRating == 0, "сброс фильтра");

    SortOptions sort;
    check(sort.before(track, Track("Zulu", "Someone", 1)), "сортировка по названию без учёта регистра");
    sort.reverseDirection();
    check(!sort.before(track, Track("Zulu", "Someone", 1)), "сортировка по убыванию");
    sort.ascending = true;
    sort.field = "artist";
    check(sort.before(track, Track("A", "Zulu", 1)), "сортировка по исполнителю");
    sort.field = "duration";
    check(sort.before(track, Track("A", "B", 250)), "сортировка по длительности");
    sort.field = "title";
    sort.ignoreCase = true;
    check(sort.before(Track("alpha", "A", 1), Track("Beta", "B", 1)),
          "сортировка без учёта регистра");
    sort.ignoreCase = false;
    check(!sort.before(Track("alpha", "A", 1), Track("Beta", "B", 1)),
          "сортировка с учётом регистра");
    TrackComparator comparator;
    check(comparator.less(track, Track("Zulu", "Someone", 1)), "сравнение по названию");
    comparator.field = "artist";
    check(comparator.less(track, Track("A", "Zulu", 1)), "сравнение по исполнителю");
    comparator.field = "duration";
    check(comparator.less(track, Track("A", "B", 150)), "сравнение по длительности");
    comparator.reverse();
    check(!comparator.less(track, Track("A", "B", 150)), "обратный порядок сравнения");

    RecentlyPlayed recent;
    recent.capacity = 2;
    recent.record(track);
    recent.record(Track("B", "B", 20));
    recent.record(track);
    check(recent.contains("T5") && recent.trackIds.size() == 2, "недавно прослушанные с удалением повтора");
    RecentlyPlayed oldestFirst;
    oldestFirst.newestFirst = false;
    oldestFirst.record(track);
    check(oldestFirst.trackIds.front() == "T5", "режим добавления в конец");

    FavoriteCollection favorites;
    favorites.add(track);
    favorites.add(track);
    check(favorites.contains("T5") && favorites.trackIds.size() == 1, "добавление в избранное без дублей");
    check(favorites.remove("T5") && !favorites.remove("absent"), "удаление из избранного");
    favorites.maximumSize = 0;
    favorites.add(track);
    check(favorites.trackIds.empty(), "ограничение размера избранного");

    AudioFolder folder{"music", {}};
    folder.addFile(AudioFile{"a.mp3", "a", 100});
    folder.addFile(AudioFile{"", "", 0});
    check(folder.files.size() == 1 && folder.totalBytes() == 100, "содержимое папки и общий размер");
}

} // namespace

int main() {
    try {
        testCatalogCore();
        testMetadataAndListening();
        testPlaybackAndSearch();
        testRecommendationsStorageAndBackup();
        testReportsStatisticsAndCollections();
        std::cout << "Все тесты пройдены. Проверок: " << assertions << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}