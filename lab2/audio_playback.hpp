#ifndef AUDIO_PLAYBACK_HPP
#define AUDIO_PLAYBACK_HPP

#include "audio_core.hpp"

// 17. Очередь воспроизведения.
class PlaybackQueue {
public:
    std::vector<std::string> trackIds;
    std::size_t currentIndex = 0;
    bool repeat = false;

    void enqueue(const Track& track) { if (!track.id.empty()) trackIds.push_back(track.id); }
    std::string next() {
        if (trackIds.empty()) throw EmptyPlaylistException();
        if (currentIndex + 1 < trackIds.size()) ++currentIndex;
        else if (repeat) currentIndex = 0;
        return trackIds[currentIndex];
    }
};

// 18. Текущий сеанс воспроизведения.
class PlaybackSession {
public:
    std::string trackId;
    int positionSeconds = 0;
    bool active = false;

    void start(const Track& track) {
        if (!track.isValid() || track.id.empty()) {
            throw InvalidTrackDataException("Для воспроизведения нужен корректный трек с ID");
        }
        trackId = track.id;
        positionSeconds = 0;
        active = true;
    }
    void seek(int seconds, const Track& track) {
        if (seconds < 0) seconds = 0;
        positionSeconds = std::min(seconds, track.durationSeconds);
    }
    void finish() { active = false; positionSeconds = 0; }
};

// 19. Настройки проигрывателя.
class PlaybackSettings {
public:
    int volume = 70;
    bool shuffle = false;
    bool repeat = false;

    void setVolume(int value) { volume = std::max(0, std::min(100, value)); }
    void toggleShuffle() { shuffle = !shuffle; }
    void toggleRepeat() { repeat = !repeat; }
};

// 20. Полоса эквалайзера.
class EqualizerBand {
public:
    int frequency = 1000;
    double gain = 0.0;
    double bandwidth = 1.0;

    void adjustGain(double delta) { gain = std::max(-12.0, std::min(12.0, gain + delta)); }
    bool isBoosted() const { return gain > 0.0; }
};

// 21. Эквалайзер из нескольких полос.
class Equalizer {
public:
    std::vector<EqualizerBand> bands;
    bool enabled = false;

    void addBand(const EqualizerBand& band) { bands.push_back(band); }
    void toggleEnabled() { enabled = !enabled; }
    void reset() { for (EqualizerBand& band : bands) band.gain = 0.0; }
};

// 22. Устройство вывода звука.
class AudioDevice {
public:
    std::string name;
    std::string deviceType;
    int volume = 70;

    bool supportsFormat(const AudioFormat& format) const { return format.isSupported(); }
    void adjustVolume(int delta) { volume = std::max(0, std::min(100, volume + delta)); }
};

// 23. Соединение с устройством.
class DeviceConnection {
public:
    std::string deviceName;
    bool connected = false;
    int latencyMs = 0;

    void connect(const AudioDevice& device) {
        if (device.name.empty()) throw DeviceUnavailableException();
        deviceName = device.name;
        connected = true;
    }
    void updateLatency(int milliseconds) {
        if (milliseconds < 0) throw InvalidTrackDataException("Задержка не может быть отрицательной");
        latencyMs = milliseconds;
    }
    void disconnect() { connected = false; }
};

#endif // AUDIO_PLAYBACK_HPP