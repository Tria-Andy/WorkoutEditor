#ifndef FOODMACRO_POPUP_H
#define FOODMACRO_POPUP_H

#include <QDialog>
#include <QStyledItemDelegate>
#include <foodplanner.h>
#include <settings.h>

namespace Ui {
class foodmacro_popup;
}

class del_foodmacro : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit del_foodmacro(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;
        QStringList macroHeader = settings::get_listValues("MacroHeader");
        double macroRange = settings::getdoubleMapPointer(settings::dMap::Double)->value("Macrorange")/100.0;
        QFont foodFont;
        foodFont.setPixelSize(settings::get_fontValue("fontSmall"));
        painter->setFont(foodFont);
        double macroValue = 0;
        itemColor = settings::get_colorMap().value(macroHeader.at(index.row()));
        gradColor.setHsv(0,0,200,150);

        if(index.row() %2 == 1)
        {
            macroValue = index.siblingAtRow(index.row()-1).data().toInt()*macroRange;

            if(index.data().toInt() > index.siblingAtRow(index.row()-1).data().toInt() && index.data().toInt() < (index.siblingAtRow(index.row()-1).data().toInt()+macroValue))
            {
                itemColor.setHsv(60,125,255,200);
                gradColor.setHsv(0,255,255,150);
                painter->setPen(Qt::darkGray);
            }
            else if(index.data().toInt() > index.siblingAtRow(index.row()-1).data().toInt())
            {
                itemColor.setHsv(0,125,255,150);
                gradColor.setHsv(0,255,255,150);
                painter->setPen(Qt::white);
            }
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};

class foodmacro_popup : public QDialog
{
    Q_OBJECT

public:
    explicit foodmacro_popup(QWidget *parent = nullptr,foodplanner *pFood = nullptr,const QDate startDate = QDate());
    ~foodmacro_popup();

private slots:
    void on_toolButton_close_clicked();

private:
    Ui::foodmacro_popup *ui;
    foodplanner *foodplan;
    del_foodmacro del_foodMacro;
    QDate startDay;
    double yMax;
    int dayCount;
    QList<QDateTime> weekDates;
    QStringList macroHeader,dateHeader;
    QVector<double> carbValues,proteinValues, fatValues, fiberValues,sugarValues,xValues;
    QVector<double> carbMarco,proteinMacro, fatMacro, fiberMacro,sugarMacro;
    QMap<QString,QColor> macroColors;
    void set_tableItem(QVector<double>,int,int);
    void set_plotValues(QDate);
    void set_graph(QDate);
};

#endif // FOODMACRO_POPUP_H
