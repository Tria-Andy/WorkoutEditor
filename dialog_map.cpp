#include "dialog_map.h"
#include "ui_dialog_map.h"

Dialog_map::Dialog_map(QWidget *parent, QSortFilterProxyModel *workMeta,QString workID,QString mapPix) :
    QDialog(parent),
    ui(new Ui::Dialog_map)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    mapPath = settings::getStringMapPointer(settings::stingMap::GC)->value("maps");
    workoutID = workID;
    metaProxy = workMeta;

    if(!mapPix.isEmpty())
    {
        mapImage = mapPath + QDir::separator() + mapPix;
        this->setImage(mapImage);
    }
}

Dialog_map::~Dialog_map()
{
    delete ui;
}

void Dialog_map::openFile()
{
    QFileDialog dialog(this);
    dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg)"));
    dialog.setViewMode(QFileDialog::Detail);

    QString fileName = QFileDialog::getOpenFileName(this,
                      tr("Open Images"),mapPath, tr("Image Files (*.png *.jpg *.bmp)"));
        if (!fileName.isEmpty())
        {
            this->setImage(fileName);
        }
}

void Dialog_map::setImage(QString fileName)
{
    QMargins offset = ui->verticalLayout_dialog->contentsMargins();
    QImage image(fileName);

    ui->imageLabel->setPixmap(QPixmap::fromImage(image));
    this->resize(image.width()+offset.left()+offset.right(),image.height()+ui->progressBar_save->height()+ui->frame_head->height()+(offset.top()*2)+offset.bottom());
    ui->label_imageName->setText(QDir::toNativeSeparators(fileName).split(QDir::separator()).last());
    ui->label_imagePath->setText(fileName);
}

void Dialog_map::on_toolButton_close_clicked()
{
    reject();
}

void Dialog_map::on_toolButton_map_clicked()
{
    openFile();
}

void Dialog_map::on_toolButton_save_clicked()
{
    ui->progressBar_save->setValue(10);
    metaProxy->invalidate();
    metaProxy->setFilterRegExp("\\b"+workoutID+"\\b");
    metaProxy->setFilterKeyColumn(1);
    ui->progressBar_save->setValue(50);
    metaProxy->setData(metaProxy->index(0,9),ui->label_imageName->text());
    ui->progressBar_save->setValue(100);
    QTimer::singleShot(2000,ui->progressBar_save,SLOT(reset()));
}
