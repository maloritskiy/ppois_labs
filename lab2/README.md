# Лабораторная работа №2 — «Аудио коллекция»

## 1. Назначение проекта

Программа моделирует цифровую аудиоколлекцию. Пользователь может добавлять и удалять музыкальные треки, искать их по названию, выставлять оценки, управлять избранным, создавать плейлисты и просматривать статистику коллекции.

Предметная модель также описывает связанные процессы: воспроизведение, историю прослушиваний, метаданные, обложки и тексты песен, поиск, рекомендации, импорт и экспорт, хранение файлов, резервное копирование, статистику и фильтрацию.

Проект написан на **C++17**. Описание классов разделено по смысловым модулям: `audio_core.hpp`, `audio_metadata.hpp`, `audio_playback.hpp`, `audio_search.hpp`, `audio_storage.hpp` и `audio_statistics.hpp`. Заголовок `audio_collection.hpp` подключает эти модули. Консольный интерфейс находится в `main.cpp`, модульные тесты — в `tests.cpp`.

## 2. Состав объектной модели

В предметной модели реализован **51 основной класс**. Дополнительно объявлено **14 классов исключений**: одно базовое и 13 специализированных. Таким образом, в проекте 65 классов, если учитывать классы исключений.

| Класс | Поля | Содержательные методы | Ассоциации |
|---|---:|---:|---|
| Genre | 3 | 2 | — |
| Artist | 3 | 2 | Track |
| AudioFormat | 3 | 2 | — |
| AudioFile | 3 | 2 | — |
| Track | 10 | 5 | — |
| Album | 4 | 2 | Track |
| Playlist | 3 | 3 | Track |
| PlaylistManager | 3 | 7 | Playlist, Track |
| AudioLibrary | 3 | 9 | Track |
| TrackMetadata | 3 | 2 | Track |
| CoverArt | 3 | 2 | — |
| Lyrics | 3 | 2 | — |
| Tag | 3 | 2 | — |
| TrackRating | 3 | 2 | Track |
| ListeningEvent | 3 | 2 | Track |
| ListeningHistory | 3 | 2 | ListeningEvent |
| UserProfile | 3 | 2 | Genre |
| PlaybackQueue | 3 | 2 | Track |
| PlaybackSession | 3 | 3 | Track |
| PlaybackSettings | 3 | 3 | — |
| EqualizerBand | 3 | 2 | — |
| Equalizer | 2 | 3 | EqualizerBand |
| AudioDevice | 3 | 2 | AudioFormat |
| DeviceConnection | 3 | 3 | AudioDevice |
| SearchQuery | 3 | 4 | Track |
| SearchResult | 3 | 2 | — |
| SearchEngine | 3 | 3 | Track, SearchQuery |
| Recommendation | 3 | 2 | — |
| RecommendationEngine | 2 | 2 | Recommendation |
| DownloadTask | 3 | 2 | — |
| StorageManager | 3 | 3 | AudioFile |
| FileFingerprint | 2 | 2 | AudioFile |
| DuplicateDetector | 2 | 2 | AudioFile |
| Backup | 3 | 2 | AudioFile |
| BackupManager | 3 | 2 | Backup |
| ImportReport | 3 | 4 | — |
| ExportReport | 4 | 2 | — |
| LibraryImporter | 2 | 3 | AudioFormat |
| LibraryExporter | 3 | 2 | Track, ExportReport |
| LibraryStatistics | 3 | 3 | Track |
| GenreStatistics | 3 | 3 | Track |
| ArtistStatistics | 3 | 3 | Track |
| AlbumStatistics | 3 | 2 | Track |
| SmartPlaylist | 3 | 2 | Track |
| PlaylistRule | 3 | 2 | Track |
| CollectionFilter | 3 | 2 | Track |
| SortOptions | 3 | 2 | Track |
| TrackComparator | 2 | 2 | Track |
| RecentlyPlayed | 3 | 2 | Track |
| FavoriteCollection | 3 | 3 | Track |
| AudioFolder | 2 | 2 | AudioFile |

### 2.1. Поля, методы и связи

Ниже перечислены поля и содержательные методы каждого класса. Конструкторы и простые операции доступа к полям в число содержательных методов не включены.

| Класс | Поля | Методы |
|---|---|---|
| `Genre` | `name`, `description`, `popularity` | `normalizedName()`, `isPopular()` |
| `Artist` | `name`, `country`, `monthlyListeners` | `rename()`, `matchesTrack()` |
| `AudioFormat` | `extension`, `bitrate`, `lossless` | `qualityScore()`, `isSupported()` |
| `AudioFile` | `path`, `checksum`, `sizeBytes` | `moveTo()`, `hasContent()` |
| `Track` | `id`, `title`, `artist`, `album`, `genre`, `durationSeconds`, `releaseYear`, `rating`, `favorite`, `filePath` | `rename()`, `setRating()`, `toggleFavorite()`, `isValid()`, `formattedDuration()` |
| `Album` | `title`, `artist`, `releaseYear`, `trackIds` | `addTrack()`, `removeTrack()` |
| `Playlist` | `name`, `trackIds`, `isPublic` | `addTrack()`, `removeTrack()`, `moveTrack()` |
| `PlaylistManager` | `playlists`, `maximumPlaylists`, `allowPublic` | `addPlaylist()`, `findPlaylist()` (изменяемый и константный варианты), `removePlaylist()`, `addTrack()`, `removeTrackFromAll()`, `playlistCount()` |
| `AudioLibrary` | `name`, `tracks`, `nextId` | `addTrack()`, `removeTrack()`, `findTrack()` (изменяемый и константный варианты), `searchByTitle()`, `rateTrack()`, `toggleFavorite()`, `trackCount()`, `totalDuration()` |
| `TrackMetadata` | `composer`, `publisher`, `year` | `applyTo()`, `missingFieldCount()` |
| `CoverArt` | `imagePath`, `width`, `height` | `isSquare()`, `isUsable()` |
| `Lyrics` | `text`, `language`, `synchronized` | `wordCount()`, `containsText()` |
| `Tag` | `key`, `value`, `userCreated` | `matches()`, `normalizeValue()` |
| `TrackRating` | `score`, `review`, `reviewer` | `applyTo()`, `isPositive()` |
| `ListeningEvent` | `trackId`, `playedSeconds`, `timestamp` | `completionRatio()`, `isCompleted()` |
| `ListeningHistory` | `events`, `capacity`, `totalPlayedSeconds` | `record()`, `playCountFor()` |
| `UserProfile` | `username`, `displayName`, `preferredGenre` | `changeDisplayName()`, `likesGenre()` |
| `PlaybackQueue` | `trackIds`, `currentIndex`, `repeat` | `enqueue()`, `next()` |
| `PlaybackSession` | `trackId`, `positionSeconds`, `active` | `start()`, `seek()`, `finish()` |
| `PlaybackSettings` | `volume`, `shuffle`, `repeat` | `setVolume()`, `toggleShuffle()`, `toggleRepeat()` |
| `EqualizerBand` | `frequency`, `gain`, `bandwidth` | `adjustGain()`, `isBoosted()` |
| `Equalizer` | `bands`, `enabled` | `addBand()`, `toggleEnabled()`, `reset()` |
| `AudioDevice` | `name`, `deviceType`, `volume` | `supportsFormat()`, `adjustVolume()` |
| `DeviceConnection` | `deviceName`, `connected`, `latencyMs` | `connect()`, `updateLatency()`, `disconnect()` |
| `SearchQuery` | `text`, `field`, `caseSensitive` | `validate()`, `valueFor()`, `matches()`, `matchesExactly()` |
| `SearchResult` | `trackId`, `score`, `matchedField` | `improveScore()`, `isRelevant()` |
| `SearchEngine` | `lastQuery`, `resultLimit`, `exactMatch` | `rank()`, `search()`, `setResultLimit()` |
| `Recommendation` | `trackId`, `reason`, `score` | `boost()`, `qualifies()` |
| `RecommendationEngine` | `minimumScore`, `maximumResults` | `accepts()`, `configure()` |
| `DownloadTask` | `trackId`, `sourceUrl`, `progressPercent` | `advance()`, `isComplete()` |
| `StorageManager` | `rootPath`, `usedBytes`, `limitBytes` | `canStore()`, `canStoreFile()`, `reserve()` |
| `FileFingerprint` | `checksum`, `sizeBytes` | `sameContent()`, `updateFrom()` |
| `DuplicateDetector` | `checkedFiles`, `duplicateFiles` | `inspect()`, `duplicateRatio()` |
| `Backup` | `destination`, `createdAt`, `filePaths` | `include()`, `isUsable()` |
| `BackupManager` | `backupDirectory`, `maximumBackups`, `backups` | `registerBackup()`, `removeOldest()` |
| `ImportReport` | `imported`, `skipped`, `errors` | `recordImported()`, `recordSkipped()`, `recordFailure()`, `successRate()` |
| `ExportReport` | `exported`, `destination`, `format`, `metadataIncluded` | `recordExport()`, `hasOutput()` |
| `LibraryImporter` | `supportedExtensions`, `importedCount` | `supports()`, `noteImported()`, `validateFormat()` |
| `LibraryExporter` | `format`, `includeMetadata`, `exportedCount` | `configure()`, `exportTrack()` |
| `LibraryStatistics` | `tracks`, `totalSeconds`, `plays` | `include()`, `recordPlay()`, `averageDuration()` |
| `GenreStatistics` | `genreName`, `trackCount`, `plays` | `include()`, `recordPlay()`, `averagePlays()` |
| `ArtistStatistics` | `artistName`, `trackCount`, `totalPlays` | `include()`, `recordPlay()`, `popularityScore()` |
| `AlbumStatistics` | `albumTitle`, `trackCount`, `totalSeconds` | `include()`, `averageDuration()` |
| `SmartPlaylist` | `name`, `minimumRating`, `requiredGenre` | `matches()`, `relaxRating()` |
| `PlaylistRule` | `field`, `expected`, `negated` | `matchesText()`, `invert()` |
| `CollectionFilter` | `text`, `genre`, `minimumRating` | `matches()`, `clear()` |
| `SortOptions` | `field`, `ascending`, `ignoreCase` | `reverseDirection()`, `before()` |
| `TrackComparator` | `field`, `descending` | `less()`, `reverse()` |
| `RecentlyPlayed` | `trackIds`, `capacity`, `newestFirst` | `record()`, `contains()` |
| `FavoriteCollection` | `name`, `trackIds`, `maximumSize` | `add()`, `remove()`, `contains()` |
| `AudioFolder` | `path`, `files` | `addFile()`, `totalBytes()` |

Ассоциация учитывается, когда один класс принимает объект другого класса в методе либо содержит его в поле или контейнере. Например, `AudioLibrary` хранит объекты `Track`, `PlaylistManager` управляет объектами `Playlist`, `ListeningHistory` содержит события `ListeningEvent`, а `Equalizer` содержит полосы `EqualizerBand`. `AudioLibrary` отвечает за сами композиции, а `PlaylistManager` — за списки ID этих композиций. Когда трек удаляется из библиотеки, консольная программа вызывает `PlaylistManager::removeTrackFromAll()`, чтобы убрать устаревший ID из каждого плейлиста.

### 2.2. Исключения

Все исключения наследуются от `AudioCollectionException`, который, в свою очередь, наследуется от `std::runtime_error`.

1. `TrackNotFoundException` — трек с указанным идентификатором не найден.
2. `DuplicateTrackException` — обнаружен повторяющийся трек.
3. `DuplicatePlaylistException` — уже существует плейлист с таким именем.
4. `InvalidTrackDataException` — переданы некорректные данные трека или плейлиста.
5. `InvalidRatingException` — оценка не входит в диапазон от 1 до 5.
6. `InvalidDurationException` — длительность трека не является положительной.
7. `PlaylistNotFoundException` — плейлист с указанным именем не найден.
8. `EmptyPlaylistException` — операция запрошена для пустой очереди воспроизведения.
9. `UnsupportedAudioFormatException` — формат файла не поддерживается импортёром.
10. `StorageLimitExceededException` — для файла недостаточно свободного места.
11. `DeviceUnavailableException` — невозможно подключить устройство.
12. `InvalidSearchQueryException` — поисковый запрос пуст.
13. `ImportFailedException` — не удалось зафиксировать ошибку импорта без указания исходного файла.

### 2.3. Итоговая статистика модели

| Показатель | Реализовано | Требуется |
|---|---:|---:|
| Основные классы предметной области | 51 | не менее 50 |
| Поля основных классов | 155 | не менее 150 |
| Содержательные методы | 117 | не менее 100 |
| Различные ассоциации | 40 | не менее 30 |
| Специализированные классы исключений | 13 | не менее 12 |

## 3. Возможности консольного интерфейса

После запуска доступны следующие действия:

1. Добавление трека с названием, исполнителем, длительностью, жанром и альбомом.
2. Просмотр всех записей аудиоколлекции.
3. Поиск трека по части названия.
4. Удаление трека по идентификатору.
5. Создание плейлиста и добавление в него треков по ID.
6. Просмотр созданных плейлистов.
7. Установка оценки трека от 1 до 5.
8. Добавление трека в избранное или удаление из него.
9. Просмотр количества треков, общей и средней длительности коллекции.

Идентификаторы новых треков назначаются автоматически (`T1`, `T2` и т. д.). Предметная логика находится отдельно от функций чтения ввода и вывода меню.