#ifndef QPLOTOBJECT_H
#define QPLOTOBJECT_H

#include <qcustomplot.h>

class qplotobject : public QCustomPlot
{

public:
    qplotobject();

protected:
    QCPGraph *get_QCPLine(QCustomPlot *plot, QString name,QColor gColor)
    {
        QCPGraph *graph = plot->addGraph();
        graph->setName(name);
        graph->setLineStyle(QCPGraph::lsLine);
        graph->setAntialiased(true);
        graph->setPen(QPen(gColor,2));

        return graph;
    }
    /*
    QCPBars *get_QCPBar(QCustomPlot *plot,QColor gColor, QString name,int width, int factor,bool secondAxis)
    {
        QCPBars *bar = new QCPBars(plot->xAxis,plot->yAxis);
        bar->setName(name);
        return bar;
    }
    */
};
#endif // QPLOTOBJECT_H
