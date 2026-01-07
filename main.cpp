#include "devices.hpp"

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::cout << "Usage: " << argv[0] << '\n';
        std::cout << "No parameters or flags required.\n";
        exit(0);
    }    
    
    Listener listener;
    Mouse mouse(MOUSE_DEVICE_PATH);
    Keyboard keyboard(KEYBOARD_DEVICE_PATH);

    listener.bind(mouse);
    listener.bind(keyboard);

    listener.run();
}

