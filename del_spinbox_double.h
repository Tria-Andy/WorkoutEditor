#ifndef DEL_SPINBOX_DOUBLE
#define DEL_SPINBOX_DOUBLE
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QDebug>

class del_spinbox_double : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_spinbox_double(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        if(index.column() == 1)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setDecimals(3);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);

            return editor;
        }

        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        double value = index.model()->data(index, Qt::EditRole).toInt();

        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(index.column() == 1)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double value = spinBox->value();
            model->setData(index, value, Qt::EditRole);
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

};






#endif // DEL_SPINBOX_DOUBLE

