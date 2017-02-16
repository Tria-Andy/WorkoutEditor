#ifndef DEL_DAYPOP_H
#define DEL_DAYPOP_H
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDebug>
#include "calculation.h"

class del_daypop: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_daypop(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
         painter->save();
         const QAbstractItemModel *model = index.model();
         int textMargin = 2;
         QPainterPath rectRow;
         QString sport = model->data(model->index(1,index.column())).toString();
         QString workEntry = index.data(Qt::DisplayRole).toString();
         QRect rectEntry(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
         rectRow.addRoundedRect(rectEntry,2,2);
         QRect rectText(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,option.rect.height());

         QLinearGradient rectGradient;
         rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
         rectGradient.setSpread(QGradient::RepeatSpread);
         QColor gradColor;
         gradColor.setHsv(0,0,180,200);
         QColor rectColor;

         if(option.state & QStyle::State_Selected)
         {
             rectColor.setHsv(240,255,150,200);
             rectColor.setAlpha(200);
             rectGradient.setColorAt(0,rectColor);
             rectGradient.setColorAt(1,gradColor);
             painter->setPen(Qt::white);
         }
         else
         {
             rectColor = settings::get_itemColor(sport).toHsv();
             rectColor.setAlpha(200);
             rectGradient.setColorAt(0,rectColor);
             rectGradient.setColorAt(1,gradColor);
             painter->setPen(Qt::black);
         }

         painter->setBrush(rectGradient);
         painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
         painter->drawPath(rectRow);
         painter->drawText(rectText,Qt::AlignLeft | Qt::AlignVCenter, workEntry);
         painter->restore();
    }



};
#endif // DEL_DAYPOP_H
