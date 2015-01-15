#include "polygonentity.h"
#include <cmath>
#include <QWidget>
#include <QStyleOptionGraphicsItem>
#include <QPainter>

PolygonEntity::PolygonEntity(QGraphicsItem *parent)
    :MapEntity(parent), m_area(0.0)
{
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
}

PolygonEntity::PolygonEntity(const QString & name, QGraphicsItem *parent)
    : MapEntity(parent), m_area(0.0)
{
    setObjectName(name);
    setFlags(ItemIsSelectable);
    setAcceptHoverEvents(true);
}

PolygonEntity::PolygonEntity(const QString & name, int id) :  m_area(0.0)
{
    setObjectName(name);
    m_id = id;
    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}

PolygonEntity::PolygonEntity(const QString & name, int id, QPolygon poly, double area)
    :m_outline(poly), m_area(area)
{
    setObjectName(name);
    m_id = id;
    setFlags(ItemIsSelectable | ItemIsMovable);
    setAcceptHoverEvents(true);
}


void PolygonEntity::copy(PolygonEntity &polygon)
{
    setObjectName(polygon.objectName());
    m_enName = polygon.enName();
    m_id = polygon.id();
    m_center = polygon.center();
    m_area = polygon.area();
    setOutline(polygon.outline());
}

void PolygonEntity::setOutline(const QVector<QPoint> & points)
{
   m_outline = QPolygon(points);
   update();
}

const QPolygon & PolygonEntity::outline() const
{
    return m_outline;
}

bool PolygonEntity::load(const QJsonObject &jsonObject)
{

    MapEntity::load(jsonObject);
    m_area = jsonObject["Area"].toDouble();

    const QJsonArray & jsonArray = jsonObject["Outline"].toArray()[0].toArray()[0].toArray();
    for(int i = 0; i < jsonArray.size() - 1; i+=2){
        m_outline.append(QPoint(jsonArray[i].toInt(), -jsonArray[i+1].toInt()));
    }

    if(m_area == 0){
        computeArea();
    }
    if(m_center.isNull()){
        computeCenter();
    }
    return true;
}

bool PolygonEntity::save(QJsonObject &jsonObject) const
{
    MapEntity::save(jsonObject);
    jsonObject["Area"] = m_area;
    QJsonArray jsonArray;
    for(int i = 0; i < m_outline.size(); i++){
        jsonArray.append(m_outline[i].x());
        jsonArray.append(-m_outline[i].y());
    }
    QJsonArray array0,array1;
    array0.append(jsonArray);
    array1.append(array0);

    jsonObject["Outline"] = array1;
    return true;
}

double PolygonEntity::area(){
    if(m_area < 0){
        computeArea();
    }
    return m_area;
}

void PolygonEntity::setArea(const double area)
{
    m_area = area;
}

void PolygonEntity::addPoint(const QPoint & p)
{
    m_outline.append(p);
}

void PolygonEntity::movePoint(const int id, const QPoint & vector)
{
    Q_ASSERT(id >= 0 && id < m_outline.size());
    m_outline[id] += vector;
}

void PolygonEntity::movePointTo(const int id, const QPoint & point){
    Q_ASSERT(id >= 0 && id < m_outline.size());
    m_outline[id] = point;
}

void PolygonEntity::insertPoint(const int id, const QPoint &p)
{
    m_outline.insert(id, p);
}

void PolygonEntity::removePoint(const int id)
{
    m_outline.removeAt(id);
}

int PolygonEntity::PointNum() const
{
    return m_outline.size();
}

QRectF PolygonEntity::boundingRect() const
{
    const int margin = 5;
    return m_outline.boundingRect().adjusted(-margin, -margin, margin, margin);
}

QPainterPath PolygonEntity::shape() const
{
    QPainterPath path;
    path.addPolygon(m_outline);
    return path;
}

void PolygonEntity::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);

    //if selected
    QColor fillColor = (option->state & QStyle::State_Selected) ? m_color.darker(150) : m_color;

    QColor borderColor = ((option->state & QStyle::State_Selected) || (option->state & QStyle::State_MouseOver) ) ? QColor(0, 160, 233) : m_color.darker(250);

    //setZValue(100);

    painter->setBrush(fillColor);
    painter->setPen(QPen(borderColor, 1));
    painter->drawPolygon(m_outline);
}

double PolygonEntity::computeArea()
{
    //Area = 1/2 \sum_{i}(x_{i}y_{i+1} - x_{i+1}y_{i})
    //refer to: http://www.efg2.com/Lab/Graphics/PolygonArea.htm
    if(m_outline.size() < 3){
        m_area = 0;
    }else{
        double sum = 0;
        QPoint p0, p1;
        for(int i = 0; i < m_outline.size()-1; i++){
            p0 = m_outline.at(i);
            p1 = m_outline.at(i+1);
            sum += p0.x() * p1.y() - p1.x() * p0.y();
        }

        //the last point
        p0 = m_outline.at(m_outline.size()-1);
        p1 = m_outline.at(0);
        sum += p0.x() * p1.y() - p1.x() * p0.y();

        m_area = fabs(0.5 * sum) / 100.0; //from dm to m
    }
    return m_area;
}

const QPointF & PolygonEntity::computeCenter(){
    QPointF point(0,0);
    int count = m_outline.size();
    for(int i = 0; i < count; i++){
        point += m_outline[i];
    }
    point /= qreal(count);
    m_center = point;
    return m_center;
    emit centerChanged(m_center);
}
