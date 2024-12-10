#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFileDialog>
#include <portaudio.h>
#include "AudioPlayer.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr) : QMainWindow(parent), audioPlayer(new AudioPlayer(this)) {
        setWindowTitle("Audio Software");

        // Créer la fenêtre principale
        QWidget* centralWidget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(centralWidget);

        // Ajouter une étiquette
        QLabel* label = new QLabel("Bienvenue dans le logiciel audio", this);
        layout->addWidget(label);

        // Bouton de lecture
        QPushButton* playButton = new QPushButton("Play", this);
        connect(playButton, &QPushButton::clicked, audioPlayer, &AudioPlayer::playAudio);
        layout->addWidget(playButton);

        // Bouton pour charger un fichier audio
        QPushButton* loadButton = new QPushButton("Charger un fichier audio", this);
        connect(loadButton, &QPushButton::clicked, this, &MainWindow::loadAudioFile);
        layout->addWidget(loadButton);

        // Affichage de la fenêtre
        setCentralWidget(centralWidget);
    }

private slots:
    void loadAudioFile() {
        // Ouvrir un dialog pour choisir un fichier
        QString fileName = QFileDialog::getOpenFileName(this, "Ouvrir un fichier audio", "", "Audio Files (*.wav)");
        if (!fileName.isEmpty()) {
            audioPlayer->loadAudio(fileName.toStdString());
        }
    }

private:
    AudioPlayer* audioPlayer;
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
