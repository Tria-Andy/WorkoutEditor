#ifndef LOGGER_H
#define LOGGER_H
#include <QFile>

class logger
{
public:
    logger(bool logging = 0);

private:
    bool enableLog;

};

#endif // LOGGER_H
