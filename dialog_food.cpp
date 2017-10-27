#include "dialog_food.h"
#include "ui_dialog_food.h"

Dialog_food::Dialog_food(QWidget *parent,schedule *ptrSchedule) :
    QDialog(parent),
    ui(new Ui::Dialog_food)
{
    ui->setupUi(this);    
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    schedulePtr = ptrSchedule;
    firstDayofWeek = QDate().currentDate().addDays(1 - QDate().currentDate().dayOfWeek());

    //calHeader << "Cal-Day" << "Cal-Convers" << "Cal-Diff";

    dayTags  << "Day" << "Meal" << "Food";
    mealsHeader << "Breakfast" << "Lunch" << "Coffee" << "Supper" << "Snacks";

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QDate::longDayName(d);
    }

    planerModel = new QStandardItemModel(mealsHeader.count(),7,this);
    planerModel->setHorizontalHeaderLabels(dayHeader);
    planerModel->setVerticalHeaderLabels(mealsHeader);
    ui->tableView_week->setModel(planerModel);
    ui->tableView_week->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_week->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    filePath = settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner");

    weekModel = new QStandardItemModel(this);
    weekModel->setColumnCount(2);

    weekProxy = new QSortFilterProxyModel(this);
    weekProxy->setSourceModel(weekModel);

    planerXML = "foodplan.xml";
    mealXML = "meals.xml";

    if(!filePath.isEmpty())
    {
        this->check_File(filePath,planerXML);
        this->check_File(filePath,mealXML);
        this->read_foodPlan(this->load_XMLFile(filePath,planerXML));
    }

    ui->treeView_week->setModel(weekModel);
    this->set_headerInfo(firstDayofWeek);
}

Dialog_food::~Dialog_food()
{
    delete ui;
}

void Dialog_food::on_toolButton_close_clicked()
{
    reject();
}

void Dialog_food::set_headerInfo(QDate firstDay)
{
    int weekNumber = firstDay.weekNumber();
    ui->labelHeader->setText("Foodplan Kw " + QString::number(weekNumber)+" - Start: "+firstDay.toString("dd.MM.yyyy"));
    firstDayofWeek = firstDay;
    this->fill_weekModel(firstDayofWeek);
}

void Dialog_food::read_foodPlan(QDomDocument xmldoc)
{
    QStandardItem *rootItem = weekModel->invisibleRootItem();
    QDomElement childLevel;
    QDomNodeList xmlList;

    xmlList = xmldoc.firstChildElement().elementsByTagName("week");

    for(int weeks = 0; weeks < xmlList.count(); ++weeks)
    {
        QList<QStandardItem*> intItems;
        childLevel = xmlList.at(weeks).toElement();
        intItems << new QStandardItem(childLevel.attribute("id")+"_"+childLevel.attribute("year"));
        rootItem->appendRow(intItems);
        build_weekFoodTree(childLevel,intItems.at(0),0);
    }
}

void Dialog_food::build_weekFoodTree(QDomElement element,QStandardItem *parentItem, int level)
{
    QDomNodeList childList = element.childNodes();
    QDomElement childElement;

    if(childList.count() > 0)
    {
        for(int childs = 0; childs < childList.count(); ++childs)
        {
            childElement = childList.at(childs).toElement();

            if(childElement.tagName() == "Day")
            {
                QList<QStandardItem*> intItems;
                intItems << new QStandardItem(childElement.attribute("date"));
                parentItem->appendRow(intItems);
                if(childElement.hasChildNodes())
                {
                    build_weekFoodTree(childElement,intItems.at(0),1);
                }
            }
            else if(childElement.tagName() == "Meal")
            {
                QList<QStandardItem*> intItems;
                intItems << new QStandardItem(childElement.attribute("name"));
                parentItem->appendRow(intItems);
                if(childElement.hasChildNodes())
                {
                    build_weekFoodTree(childElement,intItems.at(0),2);
                }
            }
            else if(childElement.tagName() == "Food")
            {
                QList<QStandardItem*> intItems;
                intItems << new QStandardItem(childElement.attribute("name"));
                intItems << new QStandardItem(childElement.attribute("cal"));
                parentItem->appendRow(intItems);
            }
        }
    }
}

void Dialog_food::fill_weekModel(QDate firstday)
{
    QString weekID = QString::number(firstday.weekNumber())+"_"+QString::number(firstday.year());
    QList<QStandardItem*> week;
    week = weekModel->findItems(weekID,Qt::MatchExactly,0);
    QStandardItem *item = weekModel->itemFromIndex(weekModel->indexFromItem(week.at(0)));

    qDebug() << item->data() << weekModel->indexFromItem(week.at(0)).data();

    for(int row = 0; row < 7; ++row)
    {
        qDebug() << item->child(row,0)->data();
    }
}

void Dialog_food::on_toolButton_next_clicked()
{
    this->set_headerInfo(firstDayofWeek.addDays(7));
}

void Dialog_food::on_toolButton_prev_clicked()
{
    this->set_headerInfo(firstDayofWeek.addDays(-7));
}
