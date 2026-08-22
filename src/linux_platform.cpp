#include "../include/linux_platform.hpp"
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

KeyPressed detectKey(char &tecla) {
    KeyPressed keypressed;
    char c;
    ssize_t bytes = read(STDIN_FILENO, &c, 1);
    if (bytes > 0) {
        tecla = c;
        keypressed.key = c;
        keypressed.pressed = true;
        return keypressed;
    }
    keypressed.key = '\0';
    keypressed.pressed = false;
    return keypressed;
}

void configureTerminal(bool enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        const tcflag_t flagsToDisable = static_cast<tcflag_t>(ICANON) | static_cast<tcflag_t>(ECHO);
		newt.c_lflag &= static_cast<tcflag_t>(~flagsToDisable);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
        fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
    }
}
