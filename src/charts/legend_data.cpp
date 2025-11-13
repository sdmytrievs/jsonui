#include <QPixmap>
#include <QJsonObject>
#include "markershapes.h"
#include "jsonqml/charts/legend_data.h"

#ifndef NO_JSONIO
#include "jsonio/jsonbase.h"
#endif

namespace jsonqml {

static QColor colorAt(const QColor &start, const QColor &end, qreal pos)
{
    Q_ASSERT(pos >= 0.0 && pos <= 1.0);
    qreal r = start.redF() + ((end.redF() - start.redF()) * pos);
    qreal g = start.greenF() + ((end.greenF() - start.greenF()) * pos);
    qreal b = start.blueF() + ((end.blueF() - start.blueF()) * pos);
    QColor c;
    c.setRgbF(r, g, b);
    return c;
}

void SeriesLineData::setChanges(int mrk_type, int mrk_size, int pn_size,
                                int pn_style, int usespline, const QColor &acolor)
{
    marker_shape = mrk_type;
    marker_size = mrk_size;
    pen_size = pn_size;
    pen_style = pn_style;
    use_spline  = usespline;
    red   = acolor.red();
    green = acolor.green();
    blue  = acolor.blue();
}

void SeriesLineData::setLineChanges(int pn_size, int pn_style, int usespline)
{
    pen_size = pn_size;
    pen_style = pn_style;
    use_spline  = usespline;
}

#ifndef NO_JSONIO
void SeriesLineData::toJsonNode(jsonio::JsonBase& object) const
{
    object.clear();
    object.set_value_via_path("gpt", marker_shape);
    object.set_value_via_path("gps", marker_size);
    object.set_value_via_path("gls", pen_size);
    object.set_value_via_path("glt", pen_style);
    object.set_value_via_path("gsp", use_spline);
    object.set_value_via_path("gndx", xcolumn);
    object.set_value_via_path("grd", red);
    object.set_value_via_path("ggr", green);
    object.set_value_via_path("gbl", blue);
    object.set_value_via_path("gnm", line_name.toStdString());
}

void SeriesLineData::fromJsonNode(const jsonio::JsonBase& object)
{
    object.get_value_via_path("gpt", marker_shape, 0);
    object.get_value_via_path("gps", marker_size, 4);
    object.get_value_via_path("gls", pen_size, 2);
    object.get_value_via_path("glt", pen_style, 0);
    object.get_value_via_path("gsp", use_spline, 0);
    object.get_value_via_path("gndx", xcolumn, -1);
    object.get_value_via_path("grd", red, 25);
    object.get_value_via_path("ggr", green, 0);
    object.get_value_via_path("gbl", blue, 150);
    std::string aname;
    object.get_value_via_path("gnm", aname, std::string(""));
    line_name = QString::fromStdString(aname);
}
#endif

void SeriesLineData::toJsonObject(QJsonObject& json) const
{
    json["gpt"] = marker_shape;
    json["gps"] = marker_size;
    json["gls"] = pen_size;
    json["glt"] = pen_style;
    json["gsp"] = use_spline;
    json["gndx"] = xcolumn;
    json["grd"] = red;
    json["ggr"] = green;
    json["gbl"] = blue;
    json["gnm"] = line_name;
}

void SeriesLineData::fromJsonObject(const QJsonObject& json)
{
    marker_shape = json["gpt"].toInt(0);
    marker_size = json["gps"].toInt(4);
    pen_size = json["gls"].toInt(2);
    pen_style = json["glt"].toInt(0);
    use_spline = json["gsp"].toInt(0);
    xcolumn = json["gndx"].toInt(-1);
    red = json["grd"].toInt(25);
    green = json["ggr"].toInt(0);
    blue = json["gbl"].toInt(150);
    line_name = json["gnm"].toString("");
}

QPixmap ChartImageProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    //qDebug() << " id " << id;
    QStringList data = id.split("/", Qt::KeepEmptyParts);
    SeriesLineData line_data("", data[0].toInt(), 8, data[1].toInt(), 1, 0, data[2]);
    if(size) {
        *size = QSize(32, 32);
    }
    QSize asize(requestedSize.width() > 0 ? requestedSize.width() : 32,
                   requestedSize.height() > 0 ? requestedSize.height() : 32);
    return markerShapePixmap(line_data, asize);
}

} // namespace jsonqml



