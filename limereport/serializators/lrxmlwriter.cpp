/***************************************************************************
 *   This file is part of the Lime Report project                          *
 *   Copyright (C) 2015 by Alexander Arin                                  *
 *   arin_a@bk.ru                                                          *
 *                                                                         *
 **                   GNU General Public License Usage                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation, either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
 *                                                                         *
 **                  GNU Lesser General Public License                    **
 *                                                                         *
 *   This library is free software: you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation, either version 3 of the    *
 *   License, or (at your option) any later version.                       *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library.                                      *
 *   If not, see <http://www.gnu.org/licenses/>.                           *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 ****************************************************************************/
#include "lrxmlwriter.h"
#include "lrbasedesignintf.h"
#include "serializators/lrxmlserializatorsfactory.h"
#include "lrcollection.h"
#include <QDebug>

namespace LimeReport{

XMLWriter::XMLWriter() : m_doc(new QDomDocument)
{
    m_rootElement=m_doc->createElement("Report");
    m_doc->appendChild(m_rootElement);
}

XMLWriter::XMLWriter(QSharedPointer<QDomDocument> doc) : m_doc(doc){
    m_rootElement=m_doc->createElement("Report");
    m_doc->appendChild(m_rootElement);
}

void XMLWriter::putItem(QObject *item)
{
    QDomElement node=putQObjectItem(item->metaObject()->className(),item);
    if (!replaceNode(node,item)) m_rootElement.appendChild(node);
}

QString XMLWriter::extractClassName(QObject *item)
{
    BaseDesignIntf* baseItem = dynamic_cast<BaseDesignIntf*>(item);
    if(baseItem) return baseItem->storageTypeName();
    else return item->metaObject()->className();
}

void XMLWriter::putChildQObjectItem(QString name, QObject *item, QDomElement *parentNode)
{   
    QDomElement itemNode = m_doc->createElement(name);
    itemNode.setAttribute("ClassName",extractClassName(item));
    itemNode.setAttribute("Type","Object");
    if (parentNode) parentNode->appendChild(itemNode);
    saveProperties(item,&itemNode);
}

bool XMLWriter::setContent(QString fileName)
{
    QFile xmlFile(fileName);
    if (xmlFile.open(QFile::ReadOnly)){
        return m_doc->setContent(&xmlFile);
    }
    return false;
}

bool XMLWriter::saveToFile(QString fileName)
{
    if ((m_doc->childNodes().count()==0)||fileName.isEmpty()) return false;
    QFile xmlFile(fileName);
    if (xmlFile.open(QFile::WriteOnly)) {
        QTextStream buffer(&xmlFile);
        m_doc->save(buffer,2);
        xmlFile.close();
        return true;
    }
    return false;
}

QString XMLWriter::saveToString()
{
    QString res;
    QTextStream buffer(&res);
    m_doc->save(buffer,2);
    return res;
}

QByteArray XMLWriter::saveToByteArray()
{
    QByteArray res;
    QTextStream buffer(&res);
    m_doc->save(buffer,2);
    return res;
}

QDomElement XMLWriter::putQObjectItem(QString name, QObject *item)
{
    Q_UNUSED(name)
    QDomElement itemNode = m_doc->createElement("object");
    itemNode.setAttribute("ClassName",extractClassName(item));
    itemNode.setAttribute("Type","Object");
    saveProperties(item,&itemNode);
    return itemNode;
}

void XMLWriter::saveProperty(QString name, QObject* item, QDomElement *node)
{
    QString typeName;
    if (name.compare("itemIndexMethod")==0)
        typeName = item->metaObject()->property(item->metaObject()->indexOfProperty(name.toLatin1())).typeName();
    else
        typeName = item->property(name.toLatin1()).typeName();

    CreateSerializator creator=0;
    if (isCollection(name,item)) { saveCollection(name,item,node); return;}
    if (isQObject(name,item)) {
        if (qvariant_cast<QObject *>(item->property(name.toLatin1())))
            putQObjectProperty(name,qvariant_cast<QObject *>(item->property(name.toLatin1())),node);
        else {
            qDebug()<<"Warnig property can`t be casted to QObject"<<name;
        }
        return;
    }

    if (enumOrFlag(name,item))
        creator=XMLAbstractSerializatorFactory::instance().objectCreator(
                    "enumAndFlags"
                );
    else
    try {
        creator=XMLAbstractSerializatorFactory::instance().objectCreator(typeName);
    } catch (LimeReport::ReportError &exception){
        qDebug()<<"class name ="<<item->metaObject()->className()
               <<"property name="<<name<<" property type="<<typeName
               <<exception.what();

    }

    if (creator) {
        QScopedPointer<SerializatorIntf> serializator(creator(m_doc.data(),node));
        serializator->save(
            item->property(name.toLatin1()),
            name
        );
    }
}

void XMLWriter::saveProperties(QObject *item, QDomElement *node)
{
    for (int i=0;i<item->metaObject()->propertyCount();i++){
        saveProperty(item->metaObject()->property(i).name(),item,node);
    }
}

bool XMLWriter::enumOrFlag(QString name, QObject *item)
{
    return item->metaObject()->property(item->metaObject()->indexOfProperty(name.toLatin1())).isFlagType() ||
           item->metaObject()->property(item->metaObject()->indexOfProperty(name.toLatin1())).isEnumType();
}

bool XMLWriter::isCollection(QString propertyName, QObject* item)
{
    QMetaProperty prop=item->metaObject()->property(item->metaObject()->indexOfProperty(propertyName.toLatin1()));
    return QMetaType::type(prop.typeName())==COLLECTION_TYPE_ID;
}

void XMLWriter::saveCollection(QString propertyName, QObject *item, QDomElement *node)
{
    ICollectionContainer * collection=dynamic_cast<ICollectionContainer*>(item);
    QDomElement collectionNode=m_doc->createElement(propertyName);
    collectionNode.setAttribute("Type","Collection");

    for(int i=0;i<collection->elementsCount(propertyName);i++){
        putCollectionItem(collection->elementAt(propertyName,i),&collectionNode);
    }

    node->appendChild(collectionNode);
}

bool XMLWriter::isQObject(QString propertyName, QObject *item)
{
    QMetaProperty prop=item->metaObject()->property(item->metaObject()->indexOfProperty(propertyName.toLatin1()));
    return QMetaType::type(prop.typeName())==QMetaType::QObjectStar;
}

bool XMLWriter::replaceNode(QDomElement node, QObject* item)
{
    QDomElement element = m_rootElement.firstChildElement(item->metaObject()->className());
    while (!element.isNull()){
        QDomElement objectName = element.firstChildElement(QLatin1String("objectName"));
        if (!objectName.isNull()&&(objectName.text()==item->objectName())){
            QDomElement removeElement=element;
            element=element.nextSiblingElement(item->metaObject()->className());
            m_rootElement.replaceChild(node,removeElement);
            return true;
        }
        else element=element.nextSiblingElement(item->metaObject()->className());
    }
    return false;
}

void XMLWriter::putCollectionItem(QObject *item, QDomElement *parentNode)
{
    putChildQObjectItem("item",item,parentNode);
}

void XMLWriter::putQObjectProperty(QString propertyName, QObject* item, QDomElement* parentNode)
{
    putChildQObjectItem(propertyName,item,parentNode);
}

}








