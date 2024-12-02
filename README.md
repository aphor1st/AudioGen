# AudioGen

# Clone the vcpkg repository
git clone https://github.com/microsoft/vcpkg.git

# Navigate into the vcpkg directory
cd vcpkg

# Bootstrap vcpkg
./bootstrap-vcpkg.sh  # On Linux/macOS
.\bootstrap-vcpkg.bat # On Windows

# Install the wxWidgets library
vcpkg install wxwidgets

vcpkg integrate install

vcpkg install libsndfile
