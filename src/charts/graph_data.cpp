
#include <QJsonArray>
#include "jsonqml/charts/graph_data.h"
#ifndef NO_JSONIO
#include "jsonio/jsonbase.h"
#endif

namespace jsonqml {

jsonqml::ChartData::~ChartData()
{}

void ChartData::setGraphType(int newtype)
{
    graph_type = newtype;
    auto model_type = static_cast<GRAPHTYPES>(graph_type);
    for( auto& model: modelsdata) {
        model->setGraphType(model_type);
    }
}

size_t ChartData::seriesNumber() const
{
    size_t nmb = 0;
    for(const auto& model: modelsdata) {
        nmb += model->getSeriesNumber();
    }
    return nmb;
}

int ChartData::plot(size_t line, size_t *modelline) const
{
    size_t sizecnt=0;
    for(size_t ii=0 ; ii<modelsdata.size(); ii++) {
        sizecnt += modelsdata[ii]->getSeriesNumber();
        if(line < sizecnt)  {
            if(modelline) {
                *modelline = line - sizecnt + modelsdata[ii]->getSeriesNumber();
            }
            return static_cast<int>(ii);
        }
    }
    return -1;
}

void ChartData::model_update_y_xcolumns()
{
    for(size_t nlines = 0, ii=0; ii<modelsdata.size(); ii++)  {
        modelsdata[ii]->clearXColumn();
        for(size_t jj=0; jj<modelsdata[ii]->getSeriesNumber(); jj++, nlines++)  {
            modelsdata[ii]->addXColumn(linesdata[nlines].xColumn());
        }
    }
}

void ChartData::setLines(const std::vector<SeriesLineData> &new_lines)
{
    size_t length = std::min(new_lines.size(), linesdata.size());
    std::copy_n(new_lines.begin(), length, linesdata.begin());
    model_update_y_xcolumns();
}

void ChartData::updateXSelections()
{
    size_t defined_lines = linesdata.size();
    size_t nlines = 0;

    for(size_t ii=0; ii<modelsdata.size(); ii++)  {
        auto numx_colms = modelsdata[ii]->getAbscissaNumber();
        auto nlin =  modelsdata[ii]->getSeriesNumber();
        for(size_t jj=0; jj<nlin; jj++, nlines++)  {
            if(nlines >= defined_lines) {
                jsonio::JSONIO_THROW("ChartData", 10, "error into graph data..");
            }
            if(linesdata[nlines].xColumn() >= numx_colms) {
                linesdata[nlines].setXColumn(-1);
            }
        }
    }
    model_update_y_xcolumns();
    emit changedModelSelections();
}

void ChartData::updateYSelections(bool update_names)
{
    size_t defined_lines = linesdata.size();
    size_t nlines = 0;

    for(size_t ii=0; ii<modelsdata.size(); ii++)  {
        auto nlin =  modelsdata[ii]->getSeriesNumber();
        for(size_t jj=0; jj<nlin; jj++, nlines++)  {
            if(nlines >= defined_lines) {
                linesdata.push_back(SeriesLineData(jj, nlin, modelsdata[ii]->getName(jj)));
            }
            else if(update_names) {
                linesdata[nlines].setName(modelsdata[ii]->getName(jj));
            }
        }
    }
    linesdata.resize(nlines);
    model_update_y_xcolumns();
    emit changedModelSelections();
}

#ifndef NO_JSONIO
void ChartData::toJsonNode(jsonio::JsonBase& object) const
{
    object.clear();
    object.set_value_via_path("title", title.toStdString());
    object.set_value_via_path("graphType", graph_type);

    // define grid of plot
    object.set_value_via_path("axisTypeX", axis_typeX);
    object.set_value_via_path("axisTypeY", axis_typeY);
    object.set_value_via_path("axisFont", axis_font.toString().toStdString());
    object.set_value_via_path("xName", xname.toStdString());
    object.set_value_via_path("yName", yname.toStdString());

    object.set_value_via_path("region", region);
    object.set_value_via_path("part", part);
    object.set_value_via_path("b_color", b_color);

    // define curves
    decltype(object)& arr1 = object.add_array_via_path("lines");
    for(size_t ii=0; ii<linesdata.size(); ii++){
        auto& obj = arr1.add_object_via_path(std::to_string(ii));
        linesdata[ii].toJsonNode(obj);
    }

    decltype(object)& arr2 = object.add_array_via_path("models");
    for(size_t ii=0; ii<modelsdata.size(); ii++) {
        auto& obj = arr2.add_object_via_path(std::to_string(ii));
        modelsdata[ii]->toJsonNode(obj);
    }
}

void ChartData::fromJsonNode(const jsonio::JsonBase& object)
{
    size_t ii;
    std::string str_buf;
    std::vector<double> reg_part;
    std::vector<int> b_col_vec;

    object.get_value_via_path<std::string>("title", str_buf, "title");
    title = QString::fromStdString(str_buf);
    object.get_value_via_path<int>("graphType", graph_type, LineChart);

    // define grid of plot
    object.get_value_via_path("axisTypeX", axis_typeX, 5);
    object.get_value_via_path("axisTypeY", axis_typeY, 5);

    std::string fnt_name;
    if(object.get_value_via_path<std::string>("axisFont", fnt_name, "")) {
        axis_font.fromString(fnt_name.c_str());
    }

    object.get_value_via_path<std::string>("xName", str_buf, "x");
    xname = QString::fromStdString(str_buf);
    object.get_value_via_path<std::string>("yName", str_buf, "y");
    yname = QString::fromStdString(str_buf);

    if(object.get_value_via_path("region", reg_part, {}) && reg_part.size() >= 4) {
        for(ii=0; ii<4; ii++) {
            region[ii] = reg_part[ii];
        }
    }

    if(object.get_value_via_path("part", reg_part, {}) && reg_part.size() >= 4) {
        for(ii=0; ii<4; ii++) {
            part[ii] = reg_part[ii];
        }
    }

    b_color = { 255, 255, 255 };
    if(object.get_value_via_path("b_color", b_col_vec, {}) && b_col_vec.size() >= 3 ) {
        for(ii=0; ii<3; ii++) {
            b_color[ii] = b_col_vec[ii];
        }
    }

    linesdata.clear();
    auto arr = object.field("lines");
    if(arr != nullptr) {
        SeriesLineData linebuf;
        for(ii=0; ii< arr->size(); ii++)  {
            linebuf.fromJsonNode(arr->child(ii));
            linesdata.push_back(linebuf);
        }
    }
    arr  = object.field("models");
    if(arr != nullptr) {
        for(ii=0; ii<arr->size(); ii++) {
            if( ii >= modelsdata.size()) {
                break;
            }
            modelsdata[ii]->fromJsonNode(arr->child(ii));
        }
    }
    // refresh model type
    setGraphType(graph_type);

    emit dataChanged();
}
#endif

void ChartData::toJsonObject(QJsonObject& json) const
{
    json["title"] =  title;
    json["graphType"] = graph_type;
    json["axisTypeX"] = axis_typeX;
    json["axisTypeY"] = axis_typeY;
    json["axisFont"] = axis_font.toString();
    json["xName"] =  xname;
    json["yName"] = yname;

    QJsonArray regArray;
    QJsonArray partArray;
    for(size_t ii=0; ii<4; ii++) {
        regArray.append(region[ii]);
        partArray.append(part[ii]);
    }
    json["region"] = regArray;
    json["part"] = partArray;

    QJsonArray colorArray;
    for(size_t ii=0; ii<3; ii++) {
        colorArray.append(b_color[ii]);
    }
    json["b_color"] = colorArray;

    QJsonArray linesArray;
    for(size_t ii=0; ii<linesdata.size(); ii++) {
        QJsonObject lnObject;
        linesdata[ii].toJsonObject(lnObject);
        linesArray.append(lnObject);
    }
    json["lines"] = linesArray;

    QJsonArray modelArray;
    for(size_t ii=0; ii<modelsdata.size(); ii++) {
        QJsonObject lnObject;
        modelsdata[ii]->toJsonObject(lnObject);
        modelArray.append(lnObject);
    }
    json["models"] = modelArray;
}

void ChartData::fromJsonObject(const QJsonObject& json)
{
    size_t ii;
    title = json["title"].toString("Graph title");
    graph_type = json["graphType"].toInt(LineChart);
    axis_typeX = json["axisTypeX"].toInt(5);
    axis_typeY = json["axisTypeY"].toInt(5);
    QString fntname = json["axisFont"].toString("Sans Serif");
    axis_font.fromString(fntname);
    xname = json["xName"].toString("x");
    yname = json["yName"].toString("y");

    QJsonArray regArray = json["region"].toArray();
    QJsonArray partArray = json["part"].toArray();
    if(regArray.size() > 3 && partArray.size() > 3) {
        for(ii=0; ii<4; ii++)  {
            region[ii] = regArray[ii].toDouble();
            part[ii] = partArray[ii].toDouble();
        }
    }
    QJsonArray colorArray = json["b_color"].toArray();
    if(colorArray.size() > 2) {
        for(ii=0; ii<3; ii++) {
            b_color[ii] = colorArray[ii].toInt();
        }
    }
    linesdata.clear();
    SeriesLineData linebuf;
    QJsonArray linesArray = json["lines"].toArray();
    for(ii=0; ii<linesArray.size(); ii++)  {
        QJsonObject lnObject = linesArray[ii].toObject();
        linebuf.fromJsonObject(lnObject);
        linesdata.push_back(linebuf);
    }

    linesArray = json["models"].toArray();
    for(ii=0; ii<linesArray.size(); ii++) {
        if(ii >= modelsdata.size()) {
            break;
        }
        QJsonObject lnObject = linesArray[ii].toObject();
        modelsdata[ii]->fromJsonObject(lnObject);
    }
    // refresh model type
    setGraphType(graph_type);

    emit dataChanged();
}

void ChartData::setMinMaxRegion(double reg[4])
{
    region[0] = reg[0];
    region[1] = reg[1];
    region[2] = reg[2];
    region[3] = reg[3];
    part[0] = reg[0]+(reg[1]-reg[0])/3;
    part[1] = reg[1]-(reg[1]-reg[0])/3;
    part[2] = reg[2]+(reg[3]-reg[2])/3;
    part[3] = reg[3]-(reg[3]-reg[2])/3;
}

double ChartData::xMin() const
{
    return region[0];
}
void ChartData::setxMin(double val)
{
    region[0] = val;
}

double ChartData::xMax() const
{
    return region[1];
}
void ChartData::setxMax(double val)
{
    region[1] = val;
}

double ChartData::yMin() const
{
    return region[2];
}
void ChartData::setyMin(double val)
{
    region[2] = val;
}

double ChartData::yMax() const
{
    return region[3];
}
void ChartData::setyMax(double val)
{
    region[3] = val;
}

double ChartData::fxMin() const
{
    return part[0];
}
void ChartData::setfxMin(double val)
{
    part[0] = val;
}

double ChartData::fxMax() const
{
    return part[1];
}
void ChartData::setfxMax(double val)
{
    part[1] = val;
}

double ChartData::fyMin() const
{
    return part[2];
}
void ChartData::setfyMin(double val)
{
    part[2] = val;
}

double ChartData::fyMax() const
{
    return part[3];
}
void ChartData::setfyMax(double val)
{
    part[3] = val;
}

QColor ChartData::backgroundColor() const
{
    return QColor(b_color[0], b_color[1], b_color[2]);
}

void ChartData::setBackgroundColor(const QColor &acolor)
{
    b_color[0] = acolor.red();
    b_color[1] = acolor.green();
    b_color[2] = acolor.blue();
}

QFont ChartData::titleFont() const
{
    auto title_font = axis_font;
    title_font.setPointSize(title_font.pointSize()+4);
    return title_font;
}

QFont ChartData::axisFont() const
{
    return  axis_font;
}

void ChartData::connect_data_changed()
{
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::graphTypeChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::titleChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::axisXChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::axisYChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::xNameChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::yNameChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::xMinChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::xMaxChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::yMinChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::yMaxChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::fxMinChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::fxMaxChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::fyMinChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::fyMaxChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::backColorChanged);
    QObject::connect(this, &ChartData::dataChanged, this, &ChartData::axisFontChanged);
}

} // namespace jsonqml



