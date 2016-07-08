#include "widget_dragdrop.h"
#include <QDebug>


widget_dragdrop::widget_dragdrop(QWidget *parent)
    : QTreeWidget(parent)
{
    setSelectionMode(QAbstractItemView::SingleSelection);
    setDragEnabled(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
    setDragDropMode(QAbstractItemView::InternalMove);
}
void widget_dragdrop::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "MouseEvent";
}

void widget_dragdrop::dragEnterEvent(QDragEnterEvent *event)
{
    qDebug() << "DragEnterEvent";
}

void widget_dragdrop::dropEvent(QDropEvent *event)
{

    qDebug() << "DropEvent";

}

bool widget_dragdrop::dropMimeData(QTreeWidgetItem *parent, int index, const QMimeData *data, Qt::DropAction action)
{
    qDebug() << "DropMime";
    QList<QUrl> urlList;
    QTreeWidgetItem *item;

    urlList = data->urls(); // retrieve list of urls

    foreach(QUrl url, urlList) // iterate over list
    {
        // make new QTreeWidgetItem and set its text
        // if parent is null - add top level item (this parent)
        if (parent == NULL) item = new QTreeWidgetItem(this);
        else
        // else add QTreeWidgetItem with parent and expand parent
        {
            item = new QTreeWidgetItem(parent);
            parent->setExpanded( true );
        }

        // set item text
        item->setText( 0, url.toLocalFile() );
    }

    return true;
}


QStringList widget_dragdrop::mimeTypes () const
{
    QStringList qstrList;
    // list of accepted mime types for drop
    qstrList.append("text/uri-list");
    return qstrList;
}


Qt::DropActions widget_dragdrop::supportedDropActions () const
{
    // returns what actions are supported when dropping
    return Qt::CopyAction | Qt::MoveAction;
}
