#ifndef LINUX_PLATFORM_HPP
#define LINUX_PLATFORM_HPP

// linux struct
struct KeyPressed {
    bool pressed;
    char key;
};

KeyPressed detectKey(char &tecla);

void configureTerminal(bool enable);

#endif
