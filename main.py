import sys
import numpy as np
import pyaudio
import wave
import mido
from PyQt5.QtWidgets import QApplication, QMainWindow, QPushButton, QVBoxLayout, QWidget, QHBoxLayout, QLabel, QSlider, QTabWidget, QVBoxLayout, QGridLayout, QGraphicsView, QGraphicsScene
from PyQt5.QtCore import Qt, QRectF  # Import Qt for using Qt constants
from pedalboard import Pedalboard, Reverb  # Use pedalboard for audio effects
from PyQt5.QtGui import QPainter, QPen

# Audio settings
FORMAT = pyaudio.paInt16  # 16-bit audio
CHANNELS = 1              # Mono
RATE = 44100              # Sample rate (44.1 kHz)
CHUNK = 1024              # Buffer size

class Timeline(QGraphicsView):
    def __init__(self):
        super().__init__()
        self.setScene(QGraphicsScene(self))
        self.setRenderHint(QPainter.Antialiasing)
        self.setFixedHeight(100)  # Set a fixed height for the timeline view
        self.label = QLabel("Timeline (Drag to add clips)")  # Create the label once
        self.label.setAlignment(Qt.AlignCenter)
        self.scene().addWidget(self.label)  # Add the label to the scene once
        self.draw_timeline()

    def draw_timeline(self):
        # Clear previous drawings
        scene = self.scene()
        scene.clear()  

        # Re-add the label after clearing the scene
        scene.addWidget(self.label)  # Add the label back to the scene

        # Draw horizontal lines for the timeline
        for i in range(0, 10):  # 10 divisions
            line = scene.addLine(0, i * 10, 800, i * 10, QPen(Qt.black))
            scene.addItem(line)

class DAW(QMainWindow):
    def __init__(self):
        super().__init__()
        self.initUI()
        self.audio = pyaudio.PyAudio()
        self.stream = None
        self.recording = False
        self.frames = []
        self.midi_input = None
        self.midi_output = None
        self.init_midi()
        self.board = Pedalboard()  # Initialize pedalboard for effects

    def init_midi(self):
        input_names = mido.get_input_names()
        output_names = mido.get_output_names()

        if input_names:
            self.midi_input = mido.open_input(input_names[0])  # Open the first available input port
            print(f"Opened MIDI input port: {input_names[0]}")
        else:
            print("No MIDI input ports available.")

        if output_names:
            self.midi_output = mido.open_output(output_names[1])  # Open the first available output port
            print(f"Opened MIDI output port: {output_names[0]} + {output_names[1]}")
        else:
            print("No MIDI output ports available.")

    def apply_effects(self):
        # Example of applying a reverb effect
        if self.frames:
            audio_data = np.frombuffer(b''.join(self.frames), dtype=np.int16)
            audio_data = audio_data.astype(np.float32) / 32768.0  # Normalize to [-1, 1]
            processed_audio = self.board(audio_data)  # Apply effects
            self.frames = [processed_audio.astype(np.int16).tobytes()]  # Convert back to bytes

    def initUI(self):
        self.setWindowTitle("Simple DAW")
        self.setGeometry(100, 100, 800, 600)  # Increased size for better layout

        # Create a tab widget for different views
        self.tabs = QTabWidget()
        self.setCentralWidget(self.tabs)

        # Create tabs for Timeline, Mixer, and Piano Roll
        self.timeline_tab = QWidget()
        self.mixer_tab = QWidget()
        self.piano_roll_tab = QWidget()

        self.tabs.addTab(self.timeline_tab, "Timeline")
        self.tabs.addTab(self.mixer_tab, "Mixer")
        self.tabs.addTab(self.piano_roll_tab, "Piano Roll")

        # Initialize each tab
        self.init_timeline()
        self.init_mixer()
        self.init_piano_roll()

    def init_timeline(self):
        layout = QVBoxLayout()
        self.timeline_view = Timeline()  # Create an instance of the Timeline class
        layout.addWidget(self.timeline_view)
        self.timeline_tab.setLayout(layout)

    def init_mixer(self):
        layout = QVBoxLayout()
        layout.addWidget(QLabel("Mixer (Placeholder)"))

        # Example of adding volume sliders for channels
        for i in range(1, 5):  # Assuming 4 channels
            channel_layout = QHBoxLayout()
            channel_label = QLabel(f"Channel {i}")
            volume_slider = QSlider()
            volume_slider.setOrientation(Qt.Horizontal)  # Set orientation to horizontal
            volume_slider.setRange(0, 100)  # Volume range from 0 to 100
            channel_layout.addWidget(channel_label)
            channel_layout.addWidget(volume_slider)
            layout.addLayout(channel_layout)

        self.mixer_tab.setLayout(layout)

    def init_piano_roll(self):
        layout = QVBoxLayout()
        layout.addWidget(QLabel("Piano Roll (Placeholder)"))
        # Add more piano roll functionality here
        self.piano_roll_tab.setLayout(layout)

    def toggle_record(self):
        if not self.recording:
            self.recording = True
            self.record_button.setText("Stop Recording")
            self.frames = []
            self.stream = self.audio.open(format=FORMAT, channels=CHANNELS,
                                          rate=RATE, input=True,
                                          frames_per_buffer=CHUNK,
                                          stream_callback=self.record_callback)
        else:
            self.recording = False
            self.record_button.setText("Record")
            if self.stream:
                self.stream.stop_stream()
                self.stream.close()

    def record_callback(self, in_data, frame_count, time_info, status):
        self.frames.append(in_data)
        return (in_data, pyaudio.paContinue)

    def play_audio(self):
        if not self.frames:
            return
        self.stream = self.audio.open(format=FORMAT, channels=CHANNELS,
                                      rate=RATE, output=True)
        for frame in self.frames:
            self.stream.write(frame)
        self.stream.stop_stream()
        self.stream.close()

    def stop_audio(self):
        if self.stream:
            self.stream.stop_stream()
            self.stream.close()

    def save_audio(self):
        if not self.frames:
            return
        with wave.open("output.wav", "wb") as wf:
            wf.setnchannels(CHANNELS)
            wf.setsampwidth(self.audio.get_sample_size(FORMAT))
            wf.setframerate(RATE)
            wf.writeframes(b"".join(self.frames))

    def closeEvent(self, event):
        if self.stream:
            self.stream.stop_stream()
            self.stream.close()
        self.audio.terminate()
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    daw = DAW()
    daw.show()

    sys.exit(app.exec_())