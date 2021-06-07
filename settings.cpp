/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "settings.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QDateTime>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QIcon>

settings::settings()
{
}
QString settings::settingFile;
QString settings::splitter = "/";
bool settings::settingsUpdated;
QDate settings::firstDayofWeek;
QPair<int,int> settings::screenSize;

QHash<QString,QString> settings::gcInfo;
QHash<QString,QString> settings::generalMap;
QHash<QString,QString> settings::formatMap;
QHash<QString,QString> settings::sportMap;
QHash<QString,QString> settings::fileMap;
QHash<QString,QColor> settings::colorMap;
QHash<QString,int> settings::intMap;
QHash<QString,QString> settings::sportIcon;
QString settings::valueFile;
QString settings::valueFilePath;

QString settings::SwimLabel;
QString settings::BikeLabel;
QString settings::RunLabel;
QString settings::JumpLabel;
QString settings::StrengthLabel;
QString settings::AltLabel;
QString settings::TriaLabel;
QString settings::AthLabel;
QString settings::OtherLabel;

QHash<QString,QStringList*> settings::headerMap;
QHash<QString,QStringList*> settings::xmlmapping;
QHash<QString,QStringList> settings::listMap;
QHash<QString,QStringList> settings::jsonTags;
QHash<QString,QMap<QString,QString>> settings::sportDistance;
QHash<QString,QVector<double>> settings::doubleVector;
QMap<int,QString> settings::sampList;
QMap<int,QString> settings::intList;
QMap<QDate,double> settings::weightMap;
QHash<QString,double> settings::thresholdMap;
QHash<QString,double> settings::athleteMap;
QHash<QString,double> settings::doubleMap;
QHash<QString,QString> settings::triaMap;
QHash<QString,QHash<QString,QString>> settings::rangeMap;
QStringList settings::keyList;
QStringList settings::extkeyList;

enum {LOADED,INITERROR,GCERROR,VALERROR};
enum {SPORT,LEVEL,PHASE,CYCLE,WCODE,JFILE,EDITOR};
enum {SPORTUSE};

void settings::fill_mapList(QMap<int,QString> *map, QString *values)
{
    QStringList list = values->split(splitter);

    for(int i = 0; i < list.count(); ++i)
    {
        map->insert(i,list.at(i));
    }
}

void settings::fill_mapColor(QStringList *stringList, QString *colorString,bool trans)
{
    for(int i = 0; i < stringList->count(); ++i)
    {
        colorMap.insert(stringList->at(i),settings::get_colorRGB(colorString->split(splitter).at(i),trans));
    }
}

QHash<QString, QString> settings::set_rangeLevel(QString values)
{
    QHash<QString,QString> levelValues;

    QStringList list = values.split(splitter);
    for(int i = 0; i < listMap.value("Level").count(); ++i)
    {
        levelValues.insert(listMap.value("Level").at(i),list.at(i));
    }
    return levelValues;
}

void settings::fill_mapRange(QHash<QString, QString> *map, QString values)
{
    QStringList list = values.split(splitter);
    for(int i = 0; i < listMap.value("Level").count(); ++i)
    {
        map->insert(listMap.value("Level").at(i),list.at(i));
    }
}

QStringList settings::get_colorStringList(QStringList *stringList)
{
    QStringList colorList;
    for(int i = 0; i < stringList->count(); ++i)
    {
        colorList << settings::set_colorString(colorMap.value(stringList->at(i)));
    }
    return colorList;
}

QColor settings::get_colorRGB(QString colorValue,bool trans)
{
    QColor color;
    QString cRed,cGreen,cBlue;
    int aValue = 0;
    cRed = colorValue.split("-").at(0);
    cGreen = colorValue.split("-").at(1);
    cBlue = colorValue.split("-").at(2);
    if(trans)
    {
        aValue = 125;
    }
    else
    {
        aValue = 255;
    }

    color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt(),aValue);

    return color;
}

void settings::readMappingFile(QDomDocument *xmlFile,QHash<QString,QStringList*> *valueMap)
{
    QDomNodeList xmlList;
    QDomElement xmlElement,childElement;
    QMap<int,QString> headerList;

    xmlList = xmlFile->firstChild().childNodes();

    for(int row = 0; row < xmlList.count(); ++row)
    {
       xmlElement =  xmlList.at(row).toElement();

       for(int i = 0; i < xmlElement.childNodes().count(); ++i)
       {
           childElement = xmlElement.childNodes().at(i).toElement();
           headerList.insert(childElement.attribute("id").toInt(),childElement.attribute("tag"));
       }

       QStringList *tagList = new QStringList();

       for(QMap<int,QString>::const_iterator it = headerList.cbegin(), end = headerList.cend(); it != end; ++it)
       {
           tagList->append(it.value());
       }

       valueMap->insert(xmlList.at(row).toElement().tagName(),tagList);
       headerList.clear();
    }
}

void settings::fill_sportDistance(QStringList *list, QSettings *key)
{
    QMap<QString,QString> raceDist;
    QStringList raceList,distList;
    QString sportName;

    for(int sport = 0; sport < list->count(); ++sport)
    {
        for(int child = 0; child < key->childKeys().count(); ++child)
        {
            if(key->childKeys().at(child).startsWith(list->at(sport),Qt::CaseInsensitive))
            {
                sportName = key->childKeys().at(child);
                if(sportName.contains("race"))
                {
                    raceList << key->value(sportName).toString().split(splitter);
                }
                if(sportName.contains("dist"))
                {
                    distList << key->value(sportName).toString().split(splitter);
                }
            }
            if(!raceList.isEmpty() && !distList.isEmpty())
            {
                for(int race = 0; race < raceList.count(); ++race)
                {
                    raceDist.insert(raceList.at(race),distList.at(race));
                }
                raceList.clear();
                distList.clear();

            }
        }
        sportDistance.insert(list->at(sport),raceDist);
        raceDist.clear();
    }
}

QVector<double> settings::set_doubleValues(QStringList *list)
{
    QVector<double> doubleVector;
    doubleVector.resize(list->count());

    for(int i = 0; i < list->count();++i)
    {
        doubleVector[i] = list->at(i).toDouble();
    }
    list->clear();
    return doubleVector;
}


int settings::loadSettings()
{
    firstDayofWeek = QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek());
    settingsUpdated = false;

    settingFile = QApplication::applicationDirPath() + QDir::separator() +"WorkoutEditor.ini";

    //General Settings
    if(QFile(settingFile).exists())
    {
        QSettings *mysettings = new QSettings(settingFile,QSettings::IniFormat);
        mysettings->beginGroup("GoldenCheetah");
            gcInfo.insert("regPath",mysettings->value("regPath").toString());
            gcInfo.insert("dir",mysettings->value("dir").toString());
            gcInfo.insert("athlete",mysettings->value("athlete").toString());
            gcInfo.insert("athletepref",mysettings->value("athletepref").toString());
            gcInfo.insert("folder",mysettings->value("folder").toString());
            gcInfo.insert("uploads",mysettings->value("manual").toString());
            gcInfo.insert("conf",mysettings->value("conf").toString());
            gcInfo.insert("gcpath",mysettings->value("gcpath").toString());
            gcInfo.insert("bodyfile",mysettings->value("bodyfile").toString());
            gcInfo.insert("nutritionfile",mysettings->value("nutritionfile").toString());
        mysettings->endGroup();

        if(gcInfo.value("gcpath").isEmpty())
        {
            QSettings gc_reg(gcInfo.value("regPath"),QSettings::NativeFormat);
            QString gc_dir = gc_reg.value(gcInfo.value("dir")).toString();
            QString gcPath = gc_dir + QDir::separator() + gcInfo.value("athlete") + QDir::separator() + gcInfo.value("folder");
            gcInfo.insert("gcpath",QDir::toNativeSeparators(gcPath));
            return GCERROR;
        }
        else
        {
            gcInfo.insert("actpath",gcInfo.value("gcpath") + QDir::separator() + gcInfo.value("athlete")+ QDir::separator() + gcInfo.value("folder"));
            gcInfo.insert("confpath",gcInfo.value("gcpath") + QDir::separator() + gcInfo.value("athlete")+ QDir::separator() + gcInfo.value("conf"));
            gcInfo.insert("uploadpath",gcInfo.value("gcpath") + QDir::separator() + gcInfo.value("athlete")+ QDir::separator() + gcInfo.value("uploads"));
        }

        mysettings->beginGroup("Filepath");
            gcInfo.insert("schedule",mysettings->value("schedule").toString());
            gcInfo.insert("workouts",mysettings->value("workouts").toString());
            gcInfo.insert("saisons",mysettings->value("saisons").toString());
            gcInfo.insert("foodplanner",mysettings->value("foodplanner").toString());
            gcInfo.insert("maps",mysettings->value("maps").toString());
        mysettings->endGroup();

        mysettings->beginGroup("Files");
            valueFile = mysettings->value("valuefile").toString();
            fileMap.insert("headerfile",mysettings->value("headerfile").toString());
            fileMap.insert("xmlmapping",mysettings->value("xmlmapping").toString());
            fileMap.insert("schedulefile",mysettings->value("schedulefile").toString());
            fileMap.insert("standardworkoutfile",mysettings->value("standardworkoutfile").toString());
            fileMap.insert("activityfile",mysettings->value("activityfile").toString());
            fileMap.insert("saisonfile",mysettings->value("saisonfile").toString());
            fileMap.insert("stressfile",mysettings->value("stressfile").toString());
            fileMap.insert("foodplanner",mysettings->value("foodplanner").toString());
            fileMap.insert("foodfile",mysettings->value("foodfile").toString());
            fileMap.insert("mealsfile",mysettings->value("mealsfile").toString());
            fileMap.insert("foodhistory",mysettings->value("foodhistory").toString());
            fileMap.insert("ingredfile",mysettings->value("ingredfile").toString());
            fileMap.insert("recipefile",mysettings->value("recipefile").toString());
            fileMap.insert("drinkfile",mysettings->value("drinkfile").toString());
        mysettings->endGroup();


        QFile file(gcInfo.value("confpath") + QDir::separator() + gcInfo.value("bodyfile"));
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            athleteMap.insert("weight",80.0);
            athleteMap.insert("boneskg",30.0);
            athleteMap.insert("musclekg",40.0);
        }
        else
        {
            QString jsonFile = file.readAll();
            QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
            QJsonObject jsonobj = d.object();
            QJsonArray bodyWeight = jsonobj["measures"].toArray();
            file.close();

            athleteMap.insert("weight",0.0);

            QJsonObject weightInfo;
            double currWeight = 0.0;
            double currBone = 0.0;
            double currMuscle = 0.0;
            double currBodyfat = 0.0;
            QDate weightDate;

            for(int i = 0; i < bodyWeight.count(); ++i)
            {
                weightInfo = bodyWeight.at(i).toObject();
                currWeight = weightInfo.value("weightkg").toDouble();
                currBone = weightInfo.value("boneskg").toDouble();
                currMuscle = weightInfo.value("musclekg").toDouble();
                currBodyfat = weightInfo.value("fatpercent").toDouble();
                weightDate = QDateTime().fromSecsSinceEpoch(weightInfo.value("when").toInt()).date();

                if(weightDate.daysTo(firstDayofWeek) < 14)
                {
                    weightMap.insert(weightDate,currWeight);
                    athleteMap.insert("weight",currWeight);
                    athleteMap.insert("boneskg",currBone);
                    athleteMap.insert("musclekg",currMuscle);
                    athleteMap.insert("bodyfat",currBodyfat);
                }
            }
        }

        qDebug() << weightMap;

        //Read Header values
        QDomDocument xmlDoc;
        QFile headerFile(gcInfo.value("schedule") + QDir::separator() + fileMap.value("headerfile"));

        xmlDoc.setContent(&headerFile);
        readMappingFile(&xmlDoc,&headerMap);
        xmlDoc.clear();
        headerFile.close();

        QFile xmlFile(gcInfo.value("schedule") + QDir::separator() + fileMap.value("xmlmapping"));
        xmlDoc.setContent(&xmlFile);
        readMappingFile(&xmlDoc,&xmlmapping);
        xmlFile.close();

        //Sport Value Settings
        if(gcInfo.value("schedule").isEmpty())
        {
            valueFilePath = QApplication::applicationDirPath() + QDir::separator() + valueFile;
        }
        else
        {
            valueFilePath = gcInfo.value("workouts") + QDir::separator() + valueFile;
        }

        if(QFile(valueFilePath).exists())
        {
            QSettings *myPref = new QSettings(gcInfo.value("confpath") + QDir::separator() + gcInfo.value("athletepref"),QSettings::IniFormat);
            athleteMap.insert("yob",myPref->value("dob").toDate().year());
            athleteMap.insert("height",myPref->value("height").toDouble());
            athleteMap.insert("sex",myPref->value("sex").toDouble());
            athleteMap.insert("riderfrg",(athleteMap.value("weight")+9.0)*9.81*0.45);
            athleteMap.insert("ridercw",0.2279+(athleteMap.value("weight")/(athleteMap.value("height")*750)));
            intMap.insert("ltsdays",myPref->value("LTSdays").toInt());
            intMap.insert("stsdays",myPref->value("STSdays").toInt());
            delete myPref;

            QStringList settingList,tempList;
            QString settingString;

            QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

            myvalues->beginGroup("Stressterm");
                if(intMap.value("ltsdays") == 0) intMap.insert("ltsdays",myvalues->value("ltsdays").toInt());
                if(intMap.value("stsdays") == 0) intMap.insert("ltsdays",myvalues->value("stsdays").toInt());
            myvalues->endGroup();

            myvalues->beginGroup("JsonFile");
                settingList << myvalues->value("actinfo").toString().split(splitter);
                listMap.insert("JsonFile",settingList);
                settingString = myvalues->value("intInfo").toString();
                fill_mapList(&intList,&settingString);
                settingString = myvalues->value("sampinfo").toString();
                fill_mapList(&sampList,&settingString);
                settingList.clear();
                settingList << myvalues->value("xdatainfo").toString().split(splitter);
                jsonTags.insert("xdata",settingList);
                settingList.clear();
            myvalues->endGroup();

            myvalues->beginGroup("Keylist");
                keyList << myvalues->value("keys").toString().split(splitter);
                extkeyList << myvalues->value("extkeys").toString().split(splitter);
            myvalues->endGroup();

            myvalues->beginGroup("Threshold");
                thresholdMap.insert("swimpower",myvalues->value("swimpower").toDouble());
                thresholdMap.insert("bikepower",myvalues->value("bikepower").toDouble());
                thresholdMap.insert("runpower",myvalues->value("runpower").toDouble());
                thresholdMap.insert("stgpower",myvalues->value("stgpower").toDouble());
                thresholdMap.insert("jumppower",(((athleteMap.value("weight") * 9.81)*myvalues->value("jumphigh").toDouble()) / myvalues->value("jumpsecond").toDouble()));
                thresholdMap.insert("ropefactor",myvalues->value("ropefactor").toDouble());
                thresholdMap.insert("swimfactor",myvalues->value("swimfactor").toDouble());
                thresholdMap.insert("bikefactor",myvalues->value("bikefactor").toDouble());
                thresholdMap.insert("wattfactor",myvalues->value("wattfactor").toDouble());
                thresholdMap.insert("athfactor",myvalues->value("athfactor").toDouble());
                thresholdMap.insert("runfactor",myvalues->value("runfactor").toDouble());
                thresholdMap.insert("stgstress",myvalues->value("stgstress").toDouble());
                thresholdMap.insert("athstress",myvalues->value("athstress").toDouble());
                thresholdMap.insert("runstress",myvalues->value("runstress").toDouble());
                thresholdMap.insert("swimpace",myvalues->value("swimpace").toDouble());
                thresholdMap.insert("bikepace",myvalues->value("bikepace").toDouble());
                thresholdMap.insert("bikespeed",myvalues->value("bikespeed").toDouble());
                thresholdMap.insert("runspeed",myvalues->value("runspeed").toDouble());
                thresholdMap.insert("swimlimit",myvalues->value("swimlimit").toDouble());
                thresholdMap.insert("bikelimit",myvalues->value("bikelimit").toDouble());
                thresholdMap.insert("runlimit",myvalues->value("runlimit").toDouble());
                thresholdMap.insert("runpace",myvalues->value("runpace").toDouble());
                thresholdMap.insert("swimpm",myvalues->value("swimpm").toDouble());
                thresholdMap.insert("bikepm",myvalues->value("bikepm").toDouble());
                thresholdMap.insert("runpm",myvalues->value("runpm").toDouble());
                thresholdMap.insert("hfthres",myvalues->value("hfthres").toDouble());
                thresholdMap.insert("hfmax",myvalues->value("hfmax").toDouble());
            myvalues->endGroup();

            myvalues->beginGroup("Level");
                settingList = myvalues->value("levels").toString().split(splitter);
                listMap.insert("Level",settingList);
                settingString = myvalues->value("color").toString();
                settings::fill_mapColor(&settingList,&settingString,true);
                settingList.clear();
            myvalues->endGroup();

            myvalues->beginGroup("Range");
                settingList = myvalues->allKeys();

                for(int i = 0; i < settingList.count(); ++i)
                {
                    rangeMap.insert(settingList.at(i),settings::set_rangeLevel(myvalues->value(settingList.at(i)).toString()));
                }
            myvalues->endGroup();

            myvalues->beginGroup("Phase");
                intMap.insert("usesaisons", myvalues->value("usesaisons").toInt());
                settingList = myvalues->value("phases").toString().split(splitter);
                listMap.insert("Phase",settingList);
                settingString = myvalues->value("color").toString();
                settings::fill_mapColor(&settingList,&settingString,false);
                settingList = myvalues->value("phaseweeks").toString().split(splitter);
                doubleVector.insert("Phaseweeks",set_doubleValues(&settingList));
            myvalues->endGroup();

            myvalues->beginGroup("Cycle");
                settingList = myvalues->value("cycles").toString().split(splitter);
                listMap.insert("Cycle",settingList);
                settingList.clear();
            myvalues->endGroup();

            myvalues->beginGroup("WorkoutCode");
                settingList = myvalues->value("codes").toString().split(splitter);
                listMap.insert("WorkoutCode",settingList);
                settingList.clear();
            myvalues->endGroup();

            myvalues->beginGroup("IntEditor");
                settingList = myvalues->value("parts").toString().split(splitter);
                listMap.insert("IntEditor",settingList);
                settingList.clear();
                settingList = myvalues->value("swimstyle").toString().split(splitter);
                listMap.insert("SwimStyle",settingList);
                settingList.clear();
                settingList = myvalues->value("swimMET").toString().split(splitter);
                listMap.insert("SwimMET",settingList);
                settingList.clear();
            myvalues->endGroup();

            myvalues->beginGroup("Foodplanner");
                settingList = myvalues->value("mode").toString().split(splitter);
                listMap.insert("Mode",settingList);
                settingList.clear();
                settingList = myvalues->value("modepercent").toString().split(splitter);
                for(int i = 0; i < listMap.value("Mode").count(); i++)
                {
                    settingString = settingList.at(i);
                    tempList = settingString.split("|");
                    doubleVector.insert(listMap.value("Mode").at(i),set_doubleValues(&tempList));
                }
                settingList = myvalues->value("modemacros").toString().split(splitter);

                for(int x = 0; x < settingList.count(); ++x)
                {
                    settingString = settingList.at(x);
                    tempList = settingString.split("|");

                }

                settingList = myvalues->value("modeborder").toString().split(splitter);
                settingString = myvalues->value("color").toString();
                settings::fill_mapColor(&settingList,&settingString,false);
                settingList.clear();
                settingList = myvalues->value("meals").toString().split(splitter);
                listMap.insert("Meals",settingList);
                settingList.clear();
                settingList = myvalues->value("mealdefault").toString().split(splitter);
                doubleVector.insert("Mealdefault",set_doubleValues(&settingList));
                settingList = myvalues->value("macros").toString().split(splitter);
                doubleVector.insert("Macros",set_doubleValues(&settingList));
                settingList = myvalues->value("macroheader").toString().split(splitter);
                listMap.insert("MacroHeader",settingList);
                settingString = myvalues->value("macrocolor").toString();
                settings::fill_mapColor(&settingList,&settingString,true);
                settingList.clear();

                settingList = myvalues->value("foodtags").toString().split(splitter);
                listMap.insert("foodtags",settingList);
                settingList.clear();
                doubleMap.insert("DayFiber",myvalues->value("fiber").toDouble());
                doubleMap.insert("DaySugar",myvalues->value("sugar").toDouble());
                doubleMap.insert("Macrorange",myvalues->value("macrorange").toDouble());
                doubleMap.insert("keephistory",myvalues->value("keephistory").toDouble());
                intMap.insert("savehistory",myvalues->value("savehistory").toInt());
                athleteMap.insert("BodyFatCal",myvalues->value("fatcal").toDouble());
                settingList = myvalues->value("palday").toString().split(splitter);
                doubleVector.insert("palday",set_doubleValues(&settingList));

            myvalues->endGroup();

            myvalues->beginGroup("Misc");
                settingList << myvalues->value("sum").toString();
                settingList << myvalues->value("empty").toString();
                settingList << myvalues->value("breakname").toString();
                settingList << myvalues->value("filecount").toString();
                settingList << myvalues->value("workfactor").toString();
                formatMap.insert("dateformat", myvalues->value("dateformat").toString());
                formatMap.insert("longtime", myvalues->value("longtime").toString());
                formatMap.insert("shorttime", myvalues->value("shorttime").toString());
                listMap.insert("Misc",settingList);
                listMap.insert("addworkout",myvalues->value("addworkout").toString().split(splitter));
                generalMap.insert("sum", settingList.at(0));
                colorMap.insert(settingList.at(0),settings::get_colorRGB(myvalues->value("sumcolor").toString(),false));
                generalMap.insert("empty", settingList.at(1));
                colorMap.insert(settingList.at(1),settings::get_colorRGB(myvalues->value("emptycolor").toString(),false));
                generalMap.insert("breakname",settingList.at(2));
                colorMap.insert(settingList.at(2),settings::get_colorRGB(myvalues->value("breakcolor").toString(),false));
                generalMap.insert("filecount",settingList.at(3));
                generalMap.insert("workfactor",settingList.at(4));
                settingList.clear();
                athleteMap.insert("currpal",myvalues->value("currpal").toDouble());
                athleteMap.insert("methode",myvalues->value("calmethode").toDouble());
                intMap.insert("maxworkouts",myvalues->value("maxworkouts").toInt());
                intMap.insert("weekrange",myvalues->value("weekrange").toInt());
                intMap.insert("weekdays",myvalues->value("weekdays").toInt());
            myvalues->endGroup();

            myvalues->beginGroup("Sport");
                settingList << myvalues->value("racesport").toString().split(splitter);
                fill_sportDistance(&settingList,myvalues);
                settingList.clear();
                settingList <<  myvalues->value("sports").toString().split(splitter);
                listMap.insert("Sport",myvalues->value("sports").toString().split(splitter));
                listMap.insert("Sportuse",myvalues->value("sportuse").toString().split(splitter));
                listMap.insert("Training",myvalues->value("training").toString().split(splitter));
                listMap.insert("Cardio",myvalues->value("Cardio").toString().split(splitter));
                listMap.insert("Muscular",myvalues->value("muscluar").toString().split(splitter));
                listMap.insert("Program",myvalues->value("powerprogram").toString().split(splitter));
                listMap.insert("Equipment",myvalues->value("equipment").toString().split(splitter));
                settingString = myvalues->value("color").toString();
                settings::fill_mapColor(&settingList,&settingString,false);
            myvalues->endGroup();

            for(int i = 0; i < settingList.count(); ++i)
            {
                if(settingList.at(i) == "Swim") SwimLabel = settingList.at(i);
                else if(settingList.at(i) == "Bike") BikeLabel = settingList.at(i);
                else if(settingList.at(i) == "Run") RunLabel = settingList.at(i);
                else if(settingList.at(i) == "Ropejump") JumpLabel = settingList.at(i);
                else if(settingList.at(i) == "Strength" || settingList.at(i) == "Power") StrengthLabel = settingList.at(i);
                else if(settingList.at(i) == "Alt" || settingList.at(i) == "Alternative") AltLabel = settingList.at(i);
                else if(settingList.at(i) == "Tria" || settingList.at(i) == "Triathlon") TriaLabel = settingList.at(i);
                else if(settingList.at(i) == "Ath" || settingList.at(i) == "Athletic") AthLabel = settingList.at(i);
                else if(settingList.at(i) == "Other") OtherLabel = settingList.at(i);
            }

            sportIcon.insert(generalMap.value("sum"),":/images/icons/Summery.png");
            sportIcon.insert(SwimLabel,":/images/icons/Swimming.png");
            sportIcon.insert(BikeLabel,":/images/icons/Biking.png");
            sportIcon.insert(RunLabel,":/images/icons/Running.png");
            sportIcon.insert(JumpLabel,":/images/icons/Ropejump.png");
            sportIcon.insert(StrengthLabel,":/images/icons/Strength.png");
            sportIcon.insert(AthLabel,":/images/icons/Athletic.png");
            sportIcon.insert(AltLabel,":/images/icons/SportIcon.png");
            sportIcon.insert(TriaLabel,":/images/icons/Triathlon.png");

            sportIcon.insert("Percent",":/images/icons/Percent.png");
            sportIcon.insert("TSS",":/images/icons/Lightning.png");
            sportIcon.insert("Duration",":/images/icons/Timewatch.png");
            sportIcon.insert("KJ",":/images/icons/Battery.png");
            sportIcon.insert("Distance",":/images/icons/Kilometer.png");
            sportIcon.insert("Workouts",":/images/icons/Count.png");

            delete myvalues;
        }
        else
        {
            return VALERROR;
        }

        QScreen *desktop = QGuiApplication::primaryScreen();
        screenSize.first = desktop->size().width();
        screenSize.second = desktop->size().height();

        //qDebug() << desktop->size();
        if(desktop->size().height() > 1280)
        {
            intMap.insert("weekRange",8);
            intMap.insert("fontBig",16);
            intMap.insert("fontMedium",14);
            intMap.insert("fontSmall",12);
        }
        else
        {
            intMap.insert("weekRange",6);
            intMap.insert("fontBig",14);
            intMap.insert("fontMedium",12);
            intMap.insert("fontSmall",10);
        }
        delete mysettings; 
    }
    else
    {
        return INITERROR;
    }
    return LOADED;
}

double settings::get_weightforDate(QDate actDate)
{
    if(actDate.daysTo(firstDayofWeek) <= 0)
    {
        if(actDate.daysTo(weightMap.lastKey()) < 0)
        {
            return weightMap.last();
        }
        else
        {
            return weightMap.first();
        }
    }
    else
    {
        return athleteMap.value("weight");
    }
}

void settings::writeListValues(QHash<QString,QStringList> *plist)
{
    for(QHash<QString,QStringList>::const_iterator it =  plist->cbegin(), end = plist->cend(); it != end; ++it)
    {
        listMap.insert(it.key(),it.value());
    }
    settings::saveSettings();
}

QString settings::setSettingString(QStringList list)
{
    QString setValue;

    for(int i = 0 ; i < list.count(); ++i)
    {
        setValue.append(list.at(i)+splitter);
    }
    setValue.remove(setValue.length()-1,1);
    return setValue;
}

QStringList settings::setRangeString(QHash<QString, QString> hash)
{
    QStringList rangeList;

    for(int i = 0; i < listMap.value("Level").count(); ++i)
    {
        rangeList.insert(i,hash.value(listMap.value("Level").at(i)));
    }
    return rangeList;
}

void settings::saveSettings()
{
    QStringList tempColor;
    QStringList settingList;
    QSettings *mysettings = new QSettings(settingFile,QSettings::IniFormat);

    mysettings->beginGroup("GoldenCheetah");
        mysettings->setValue("dir",gcInfo.value("dir"));
        mysettings->setValue("athlete",gcInfo.value("athlete"));
        mysettings->setValue("folder",gcInfo.value("folder"));
        mysettings->setValue("gcpath",gcInfo.value("gcpath"));
    mysettings->endGroup();

    mysettings->beginGroup("Filepath");
        mysettings->setValue("schedule",gcInfo.value("schedule"));
        mysettings->setValue("workouts",gcInfo.value("workouts"));
        mysettings->setValue("saisons",gcInfo.value("saisons"));
    mysettings->endGroup();

    QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

    myvalues->beginGroup("Stressterm");
        myvalues->setValue("ltsdays",intMap.value("ltsdays"));
        myvalues->setValue("stsdays",intMap.value("stsdays"));
    myvalues->endGroup();

    myvalues->beginGroup("Threshold");
        myvalues->setValue("swimpower",QString::number(thresholdMap.value("swimpower")));
        myvalues->setValue("swimpace",QString::number(thresholdMap.value("swimpace")));
        myvalues->setValue("swimlimit",QString::number(thresholdMap.value("swimlimit")));
        myvalues->setValue("swimpm",QString::number(thresholdMap.value("swimpm")));
        myvalues->setValue("swimfactor",QString::number(thresholdMap.value("swimfactor")));
        myvalues->setValue("bikepower",QString::number(thresholdMap.value("bikepower")));
        myvalues->setValue("bikespeed",QString::number(thresholdMap.value("bikespeed")));
        myvalues->setValue("bikepace",QString::number(thresholdMap.value("bikepace")));
        myvalues->setValue("bikefactor",QString::number(thresholdMap.value("bikefactor")));
        myvalues->setValue("bikelimit",QString::number(thresholdMap.value("bikelimit")));
        myvalues->setValue("bikepm",QString::number(thresholdMap.value("bikepm")));
        myvalues->setValue("runpower",QString::number(thresholdMap.value("runpower")));
        myvalues->setValue("runpace",QString::number(thresholdMap.value("runpace")));
        myvalues->setValue("runfactor",QString::number(thresholdMap.value("runfactor")));     
        myvalues->setValue("runlimit",QString::number(thresholdMap.value("runlimit")));
        myvalues->setValue("runpm",QString::number(thresholdMap.value("runpm")));
        myvalues->setValue("hfthres",QString::number(thresholdMap.value("hfthres")));
        myvalues->setValue("hfmax",QString::number(thresholdMap.value("hfmax")));
    myvalues->endGroup();

    myvalues->beginGroup("Sport");
        settingList = listMap.value("Sport");
        myvalues->setValue("sports",settings::setSettingString(settingList));
        myvalues->setValue("sportuse",settings::setSettingString(listMap.value("Sportuse")));
        tempColor = settings::get_colorStringList(&settingList);
        myvalues->setValue("color",settings::setSettingString(tempColor));
        tempColor.clear();
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Phase");
        settingList = listMap.value("Phase");
        myvalues->setValue("phases",settings::setSettingString(settingList));
        tempColor = settings::get_colorStringList(&settingList);
        myvalues->setValue("color",settings::setSettingString(tempColor));
        tempColor.clear();
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Level");
        settingList = listMap.value("Level");
        myvalues->setValue("levels",settings::setSettingString(settingList));
        tempColor = settings::get_colorStringList(&settingList);
        myvalues->setValue("color",settings::setSettingString(tempColor));
        tempColor.clear();
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Range");

    for(QHash<QString,QHash<QString,QString>>::const_iterator it = rangeMap.cbegin(); it != rangeMap.cend(); ++it)
    {
        myvalues->setValue(it.key(),settings::setSettingString(settings::setRangeString(it.value())));
    }

    myvalues->endGroup();

    myvalues->beginGroup("Cycle");
        settingList = listMap.value("Cycle");
        myvalues->setValue("cycles",settings::setSettingString(settingList));
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("WorkoutCode");
        settingList = listMap.value("WorkoutCode");
        myvalues->setValue("codes",settings::setSettingString(settingList));
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("JsonFile");
        settingList = listMap.value("JsonFile");
        myvalues->setValue("actinfo",settings::setSettingString(settingList));
    myvalues->endGroup();

    myvalues->beginGroup("IntEditor");
        settingList = listMap.value("IntEditor");
        myvalues->setValue("parts",settings::setSettingString(settingList));
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Foodplanner");
        settingList = listMap.value("Meals");
        myvalues->setValue("meals",settings::setSettingString(settingList));
        settingList.clear();
        settingList = listMap.value("Dish");
        myvalues->setValue("dish",settings::setSettingString(settingList));
        settingList.clear();
        for(int i = 0; i < listMap.value("Mode").count(); ++i)
        {
            for(int x = 0; x < 4; ++x)
            {
                settingList << QString::number(doubleVector.value(listMap.value("Mode").at(i)).at(x)) +"-";
            }
        }
        settingList.clear();

        for(int i = 0; i < doubleVector.value("palday").count(); ++i)
        {
            settingList << QString::number(doubleVector.value("palday").at(i));
        }
        myvalues->setValue("palday",settings::setSettingString(settingList));
        settingList.clear();

        for(int i = 0; i < doubleVector.value("Macros").count(); ++i)
        {
            settingList << QString::number(doubleVector.value("Macros").at(i));
        }
        myvalues->setValue("macros",settings::setSettingString(settingList));
        settingList.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Misc");
        settingList = listMap.value("Misc");
        myvalues->setValue("sum",settingList.at(0));
        myvalues->setValue("empty",settingList.at(1));
        myvalues->setValue("breakname",settingList.at(2));
        myvalues->setValue("sumcolor",settings::set_colorString(colorMap.value(settingList.at(0))));
        myvalues->setValue("emptycolor",settings::set_colorString(colorMap.value(settingList.at(1))));
        myvalues->setValue("breakcolor",settings::set_colorString(colorMap.value(settingList.at(2))));
        myvalues->setValue("currpal",athleteMap.value("currpal"));
        settingList.clear();
    myvalues->endGroup();

    delete myvalues;
    settingsUpdated = true;
}

QString settings::set_colorString(QColor color)
{
    return QString::number(color.red())+"-"+QString::number(color.green())+"-"+QString::number(color.blue());
}
