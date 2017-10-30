#ifndef DIALOG_FOOD_H
#define DIALOG_FOOD_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "xmlhandler.h"
#include "schedule.h"

class del_foodplan : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit del_foodplan(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        //const QAbstractItemModel *model = index.model();

        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;

        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            itemColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            itemColor.setHsv(255,255,255,0);
            painter->setPen(Qt::black);
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        QFont eFont;
        eFont.setPixelSize(settings::get_fontValue("fontSmall"));

        QLineEdit *editor = new QLineEdit(parent);
        editor->setFrame(false);
        editor->setFont(eFont);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        qDebug() << "Data" << index.data(Qt::DisplayRole).toString();

        QLineEdit *lineText = static_cast<QLineEdit*>(editor);
        lineText->setText(index.data(Qt::DisplayRole).toString());
    }
};

namespace Ui {
class Dialog_food;
}


class Dialog_food : public QDialog, public xmlHandler
{
    Q_OBJECT

public:
    explicit Dialog_food(QWidget *parent = 0,schedule *ptrSchedule = 0);
    ~Dialog_food();

    QString itemText;

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_next_clicked();
    void on_toolButton_prev_clicked();
    void on_tableWidget_week_itemChanged(QTableWidgetItem *item);
    void on_tableWidget_week_cellActivated(int row, int column);

private:
    Ui::Dialog_food *ui;

    schedule *schedulePtr;
    QString filePath,planerXML,mealXML;
    QStandardItemModel *planerModel,*ingredModel,*weekModel,*sumModel,*mealModel;
    QSortFilterProxyModel *weekProxy;
    QStringList dayTags,mealTags,mealsHeader,weekHeader,dayHeader,mealList;
    QDate firstDayofWeek;
    del_foodplan foodPlan_del;

    void set_headerInfo(QDate);
    void read_foodPlan(QDomDocument);
    void read_meals(QDomDocument);
    void build_weekFoodTree(QDomElement,QStandardItem*,int);
    QStandardItem *create_item(QDomElement,QStandardItem*);
    void fill_weekModel(QDate);
};

#endif // DIALOG_FOOD_H
