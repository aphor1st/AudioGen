#include "AudioPlayer.h"
#include <iostream>
#include <fstream>
#include <sndfile.h>  // Pour lire les fichiers WAV

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent), stream(nullptr), isPlaying(false) {
    Pa_Initialize();
}

AudioPlayer::~AudioPlayer() {
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }
    Pa_Terminate();
}

void AudioPlayer::loadAudio(const std::string& filePath) {
    // Charger le fichier WAV
    SF_INFO sfInfo;
    SNDFILE* file = sf_open(filePath.c_str(), SFM_READ, &sfInfo);
    if (!file) {
        std::cerr << "Erreur lors de l'ouverture du fichier audio: " << sf_strerror(file) << std::endl;
        return;
    }

    // Lire les données du fichier
    audioData.resize(sfInfo.frames * sfInfo.channels);
    sf_read_float(file, audioData.data(), audioData.size());
    sf_close(file);
}

void AudioPlayer::playAudio() {
    if (isPlaying) {
        std::cout << "Déjà en train de jouer." << std::endl;
        return;
    }

    // Créer un stream audio avec PortAudio
    PaError err = Pa_OpenDefaultStream(&stream,
        0,          // Pas d'entrée
        1,          // Sortie mono
        paFloat32,  // Format audio
        44100,      // Fréquence d'échantillonnage
        256,        // Taille du buffer
        audioCallback,
        this);  // Passer l'objet actuel comme userData

    if (err != paNoError) {
        std::cerr << "Erreur lors de l'ouverture du stream audio: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Erreur lors du démarrage du stream: " << Pa_GetErrorText(err) << std::endl;
        return;
    }

    isPlaying = true;
}

int AudioPlayer::audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    AudioPlayer* player = static_cast<AudioPlayer*>(userData);
    float* out = (float*)outputBuffer;

    // Remplir le buffer avec des données audio
    for (unsigned long i = 0; i < framesPerBuffer; i++) {
        *out++ = player->audioData[i % player->audioData.size()]; // Cycle les données
    }

    return paContinue;
}