#include <wx/wx.h>
#include <portaudio.h>
#include <vector>
#include <cmath>

// Structure représentant une piste audio
struct AudioTrack {
    std::vector<float> samples;
    float sampleRate = 44100.0f;
};

// Application principale
class AudioGenApp : public wxApp {
public:
    virtual bool OnInit();
    virtual int OnExit();
};

// Fenêtre principale
class AudioGenFrame : public wxFrame {
public:
    AudioGenFrame(const wxString& title);
    ~AudioGenFrame();
    void OnAddTrack(wxCommandEvent& event);
    void OnPlayAudio(wxCommandEvent& event);
    void OnMixTracks(wxCommandEvent& event);

private:
    wxButton* addTrackButton;
    wxButton* playButton;
    wxButton* mixButton;
    PaStream* stream;
    std::vector<AudioTrack> tracks;
    std::vector<float> mixedAudio;

    void MixTracks();
};

bool AudioGenApp::OnInit() {
    if (!wxApp::OnInit())
        return false;

    AudioGenFrame* frame = new AudioGenFrame("AudioGen DAW");
    frame->Show(true);
    return true;
}

int AudioGenApp::OnExit() {
    Pa_Terminate();
    return wxApp::OnExit();
}

AudioGenFrame::AudioGenFrame(const wxString& title)
    : wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(400, 300)) {
    Pa_Initialize();

    // Interface
    addTrackButton = new wxButton(this, wxID_ANY, "Add Track", wxPoint(20, 20), wxSize(100, 50));
    playButton = new wxButton(this, wxID_ANY, "Play Audio", wxPoint(140, 20), wxSize(100, 50));
    mixButton = new wxButton(this, wxID_ANY, "Mix Tracks", wxPoint(260, 20), wxSize(100, 50));

    // Bind events
    Bind(wxEVT_BUTTON, &AudioGenFrame::OnAddTrack, this, addTrackButton->GetId());
    Bind(wxEVT_BUTTON, &AudioGenFrame::OnPlayAudio, this, playButton->GetId());
    Bind(wxEVT_BUTTON, &AudioGenFrame::OnMixTracks, this, mixButton->GetId());

    // Initialize PortAudio
    stream = nullptr;
}

AudioGenFrame::~AudioGenFrame() {
    if (stream) {
        Pa_StopStream(stream);
        Pa_CloseStream(stream);
    }
    Pa_Terminate();
}

void AudioGenFrame::OnAddTrack(wxCommandEvent& event) {
    // Créer une piste de 440 Hz pour 2 secondes
    AudioTrack track;
    float frequency = 440.0f;
    size_t numSamples = static_cast<size_t>(track.sampleRate * 2);
    track.samples.resize(numSamples);

    for (size_t i = 0; i < numSamples; ++i) {
        track.samples[i] = sin(2.0f * M_PI * frequency * i / track.sampleRate);
    }

    tracks.push_back(track);
    wxMessageBox("Track added!", "Info", wxOK | wxICON_INFORMATION);
}

void AudioGenFrame::MixTracks() {
    if (tracks.empty()) return;

    // Déterminer la longueur maximale des pistes
    size_t maxSamples = 0;
    for (const auto& track : tracks) {
        maxSamples = std::max(maxSamples, track.samples.size());
    }

    mixedAudio.resize(maxSamples, 0.0f);

    // Mixer toutes les pistes
    for (const auto& track : tracks) {
        for (size_t i = 0; i < track.samples.size(); ++i) {
            mixedAudio[i] += track.samples[i];
        }
    }

    // Normaliser le mix
    float maxAmplitude = 0.0f;
    for (float sample : mixedAudio) {
        maxAmplitude = std::max(maxAmplitude, std::abs(sample));
    }
    if (maxAmplitude > 0.0f) {
        for (float& sample : mixedAudio) {
            sample /= maxAmplitude;
        }
    }
}

void AudioGenFrame::OnMixTracks(wxCommandEvent& event) {
    MixTracks();
    wxMessageBox("Tracks mixed!", "Info", wxOK | wxICON_INFORMATION);
}

void AudioGenFrame::OnPlayAudio(wxCommandEvent& event) {
    if (mixedAudio.empty()) {
        wxMessageBox("No audio to play. Add and mix tracks first.", "Error", wxOK | wxICON_ERROR);
        return;
    }

    auto callback = [](const void* inputBuffer, void* outputBuffer, unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) -> int {
            auto* mixedAudio = static_cast<std::vector<float>*>(userData);
            static size_t currentSample = 0;
            float* out = static_cast<float*>(outputBuffer);

            for (unsigned long i = 0; i < framesPerBuffer; ++i) {
                if (currentSample < mixedAudio->size()) {
                    *out++ = (*mixedAudio)[currentSample];
                    *out++ = (*mixedAudio)[currentSample];
                    currentSample++;
                } else {
                    *out++ = 0.0f;
                    *out++ = 0.0f;
                }
            }

            return currentSample < mixedAudio->size() ? paContinue : paComplete;
        };

    Pa_OpenDefaultStream(&stream,
        0,
        2,
        paFloat32,
        44100,
        256,
        callback,
        &mixedAudio);

    Pa_StartStream(stream);
}

wxIMPLEMENT_APP(AudioGenApp);
