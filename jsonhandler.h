#ifndef JSONHANDLER_H
#define JSONHANDLER_H
#include "activity.h"

class jsonHandler
{
public:
    jsonHandler(QString jsonfile = QString(), Activity *p_act = 0);

private:
    Activity *curr_act;

    void read_json(QString);

};

#endif // JSONHANDLER_H
