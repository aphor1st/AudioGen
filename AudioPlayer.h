#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <vector>
#include <portaudio.h>

class AudioPlayer : public QObject {
    Q_OBJECT

public:
    explicit AudioPlayer(QObject* parent = nullptr);
    ~AudioPlayer();

    void loadAudio(const std::string& filePath);
    void playAudio();

private:
    static int audioCallback(const void* inputBuffer, void* outputBuffer,
        unsigned long framesPerBuffer,
        PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData);

    PaStream* stream;
    std::vector<float> audioData;  // Stocke les données audio
    bool isPlaying;
};

#endif // AUDIOPLAYER_H
