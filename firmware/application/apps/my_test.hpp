#include "ui.hpp"
#include "ui_widget.hpp"
#include "ui_navigation.hpp"
#include "string_format.hpp"
#include "usb_serial_asyncmsg.hpp"

namespace ui
{
    class NewAppView : public View                                // App class declaration
    {
    public:
        NewAppView(NavigationView &nav);                          // App class init function declaration
        std::string title() const override { return "Test App"; }; // App title

    private:
        Button button_helloWorld{
            {0, 0, ui::new_font_width*7,  ui::new_font_height*2},            // Coordinates are: int:x (px), int:y (px), int:width (px), int:height (px)
            "test"                 // Title
        };

        void update();                                            // Function declaration
        MessageHandlerRegistration message_handler_update{        // Example, not required: MessageHandlerRegistration class
            Message::ID::DisplayFrameSync,                        // relays messages to your app code from baseband. Every time you 
            [this](const Message *const) {                        // get a  DisplayFrameSync message the update() function will
                this->update();                                   // be triggered.  
            }};
    };
} 