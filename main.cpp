#include <cerrno>
#include <cstdlib>
#include <linux/input-event-codes.h>
#include <vector>
#include <iostream>
#include <sys/epoll.h> // for epoll 
#include <sys/time.h> 
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <linux/input.h> // for struct input_event, event codes
                    
constexpr const char * KEYBOARD_DEVICE_PATH = "/dev/input/event16";
constexpr const char * MOUSE_DEVICE_PATH = "/dev/input/event8";
constexpr const int MAX_EVENTS = 15;


int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        std::cout << "Usage: " << argv[0] << '\n';
        std::cout << "No parameters or flags required.\n";
        exit(0);
    }
    
    int keyboardListener = open(KEYBOARD_DEVICE_PATH, O_RDONLY, 0);
    int mouseListener = open(MOUSE_DEVICE_PATH, O_RDONLY, 0);
    
    if (mouseListener < 0 || keyboardListener < 0)
    {
        std::cerr << "Unable to instantiante keyboard or mouse file descriptor: " << errno << '\n';
        exit(1);
    }

    // creates a epoll, 2 is optional but its a hint of how many fd
    int epoll = epoll_create(2);

    // generating a struct of events that we are interested
    struct epoll_event event {};
    event.events = EPOLLIN; // read only
    event.data.fd = mouseListener;

    if (epoll < 0)
    {
        std::cerr << "Unable to create epoll" << errno << '\n';
        exit(1);
    }

    // adding file descriptors to epoll
    epoll_ctl(epoll, EPOLL_CTL_ADD, mouseListener, &event);
    event.data.fd = keyboardListener;
    epoll_ctl(epoll, EPOLL_CTL_ADD, keyboardListener, &event);

    std::vector<struct epoll_event> events;
    events.resize(MAX_EVENTS);

    // runing event loop
    int mouse_x {};  
    int mouse_y {};

    while (true)
    { 
        int epollCheck= epoll_wait(epoll, events.data(), MAX_EVENTS, -1);
        struct input_event inputEvent;

        for (int i = 0; i < epollCheck; ++i)
        {
            if (epollCheck < 0)
            {
                std::cerr << "Epoll error: " << errno << '\n';
                exit(1);
            }

            ssize_t mouseBytesRead = read(events.at(i).data.fd, &inputEvent, sizeof(inputEvent));
            if (mouseBytesRead < 0)
            {
                std::cerr << "Error while fetching mouse data: " << errno << '\n';
                exit(1);
            }
            
            if (inputEvent.type == EV_REL)
            {
                if (inputEvent.code == REL_X)
                {
                    mouse_x += inputEvent.value;
                }
                else if (inputEvent.code == REL_Y)
                {
                    mouse_y += inputEvent.value;
                }
                std::cout << "[MOUSE POS (X, Y)]" << '(' << mouse_x << ", " << mouse_y << ")\n";
            }
        }
    }

    close(keyboardListener);
    close(mouseListener);
    close(epoll);
}

