#include <wx/wx.h>
#include "gui.h"

class AudioGenApp : public wxApp {
public:
    virtual bool OnInit() {
        GUI* gui = new GUI("AudioGen");
        gui->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(AudioGenApp);