
#include <QPen>
#include <QPainter>
#include <QImageWriter>
#include <QtCore/QtMath>
#include "markershapes.h"
#include "jsonqml/charts/legend_data.h"

const double Pi = 3.14159;

namespace jsonqml {

MarkerShapes &shapes()
{
    static MarkerShapes  msh;
    return msh;
}

QStringList allImageFilters();
QStringList allImageFilters()
{
    static  QStringList filter_line;

    if(filter_line.isEmpty())  {
        const QList<QByteArray> image_formats = QImageWriter::supportedImageFormats();

        QStringList filter;
        filter += "PDF Documents (*.pdf)";
        filter += "SVG Documents (*.svg)";

        if(image_formats.size() > 0) {
            QString image_filter;
            for(int i=0; i<image_formats.size(); i++) {
                image_filter = image_formats[i].toUpper();
                image_filter += "   Image (*.";
                image_filter +=  image_formats[i];
                image_filter += ")";
                filter += image_filter;
            }
        }
        filter_line = filter;
    }
    return  filter_line;
}

QImage markerShapeImage(const SeriesLineData& linedata)
{
    QPainterPath image_path = shapes().shape(static_cast<size_t>(linedata.markerShape()));
    QImage image(32, 32, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    QPen pen = QPen(linedata.color(), 3);
    pen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(pen);

    if(static_cast<size_t>(linedata.markerShape()) < shapes().size()) {
        painter.setBrush(linedata.color());
    }
    painter.drawPath(image_path);
    return image;
}


QImage textImage(const QFont& font, const QString& text)
{
    QFontMetrics fm(font);
    int pixelsWide = fm.horizontalAdvance(text)+2;
    int pixelsHigh = fm.height();
    int size, ascent = 0;
    if(pixelsWide > pixelsHigh) {
        size  =  pixelsWide;
        ascent = fm.ascent()+(pixelsWide-pixelsHigh)/2;
    }
    else  {
        size  =  pixelsHigh;
        ascent = pixelsHigh-fm.descent();
    }
    QPainterPath textPath;
    textPath.addText(1, ascent, font, text);

    QImage image(size, size, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(QPen( QColor(Qt::darkGray), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(QColor(Qt::darkGray));
    painter.drawPath(textPath);

    //image.save("TextImage.png");
    return image;
}


QPixmap markerShapePixmap(const SeriesLineData& linedata, const QSize& dsize)
{
    int size = dsize.height()/2;
    QPixmap pic(dsize);
    pic.fill(QColor(0, 0, 0, 0));
    QPainter painter(&pic);

    QRect rect(0,0,dsize.width(), dsize.height());
    painter.setPen(QPen(linedata.color(), 2));
    painter.drawImage(QRectF(QPointF(size/2, size/2), QSizeF(size,size)),
                      markerShapeImage(linedata));

    if(linedata.penSize() > 0) {
        QPoint center = rect.center();
        painter.drawLine( rect.x(), center.y(), center.x()-size/2, center.y());
        painter.drawLine( center.x()+size/2, center.y(), rect.width(), center.y());
    }
    return pic;
}

QIcon markerShapeIcon(const SeriesLineData& linedata)
{
    QIcon icon;
    icon.addPixmap(markerShapePixmap(linedata, QSize(32,32)));
    return icon;
}

// If you know size then you can simply use:
// QPixmap pixmap = icon.pixmap(requiredImageSize);
// QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
void getLinePen(QPen& pen, const SeriesLineData& linedata)
{
    pen.setColor(linedata.color() );
    pen.setWidth(linedata.penSize());
    Qt::PenStyle style = static_cast<Qt::PenStyle>(linedata.penStyle());
    pen.setStyle(style);
}

MarkerShapes::MarkerShapes()
{
    QFont timesFont("Times", 40);
    timesFont.setStyleStrategy(QFont::ForceOutline);

    // MsCircle,   Circle
    QPainterPath circlePath;
    circlePath.addEllipse(2, 2, 28, 28);
    circlePath.closeSubpath();
    markers.push_back(circlePath);

    // MsRectangle,  Rectangle
    QPainterPath rectPath;
    rectPath.addRect(2,2,28,28);
    rectPath.closeSubpath();
    markers.push_back(rectPath);

    // MsDiamond,   Diamond
    QPainterPath diamondPath;
    diamondPath.moveTo(2, 16.0);
    diamondPath.lineTo(16.0, 30.0);
    diamondPath.lineTo(30.0, 16.0);
    diamondPath.lineTo(16.0, 2.0);
    diamondPath.closeSubpath();
    markers.push_back(diamondPath);

    //MsDTriangle,   Triangle pointing downwards
    QPainterPath dTrianglePath;
    dTrianglePath.addPolygon( triangle_polygon( TriangleDown, QSize(30,30) ));
    dTrianglePath.closeSubpath();
    markers.push_back( dTrianglePath );
    //MsUTriangle,   Triangle pointing upwards
    QPainterPath uTrianglePath;
    uTrianglePath.addPolygon( triangle_polygon( TriangleUp, QSize(30,30) ));
    uTrianglePath.closeSubpath();
    markers.push_back( uTrianglePath );
    //MsLTriangle,   Triangle pointing left
    QPainterPath lTrianglePath;
    lTrianglePath.addPolygon( triangle_polygon( TriangleLeft, QSize(30,30) ));
    lTrianglePath.closeSubpath();
    markers.push_back( lTrianglePath );
    //MsRTriangle,   Triangle pointing right
    QPainterPath rTrianglePath;
    rTrianglePath.addPolygon( triangle_polygon( TriangleRight, QSize(30,30) ));
    rTrianglePath.closeSubpath();
    markers.push_back( rTrianglePath );

    // MsStar,    Five-pointed star
    QPainterPath starPath;
    starPath.moveTo(30, 15);
    for (int i = 1; i < 5; ++i) {
        starPath.lineTo(15 + 16 * qCos(0.8 * i * Pi),
                        15 + 16 * qSin(0.8 * i * Pi));
    }
    starPath.closeSubpath();
    markers.push_back( starPath );

    //MsCross, Cross (+)
    QPainterPath crossPath;
    crossPath.addText(0, 30, timesFont, "+" );
    markers.push_back( crossPath );

    //MsXCross,  Diagonal cross (X)
    QPainterPath xcrossPath;
    xcrossPath.addText(0, 30, timesFont, "x" );
    markers.push_back( xcrossPath );

    // MsHLine,  Horizontal line
    QPainterPath hlinePath;
    hlinePath.moveTo(0, 15.0);
    hlinePath.lineTo(30.0, 15.0);
    hlinePath.closeSubpath();
    markers.push_back( hlinePath );

    // MsVLine,   Vertical line
    QPainterPath vlinePath;
    vlinePath.moveTo(15, 0.0);
    vlinePath.lineTo(15, 30.0);
    vlinePath.closeSubpath();
    markers.push_back( vlinePath );

    // MsLineStar,  Symbol star (*)
    QPainterPath textPath;
    textPath.addText(0, 40, timesFont, "*" );
    markers.push_back( textPath );
}

QPolygonF MarkerShapes::triangle_polygon(TriangleType type, const QSize& size)
{
    QPolygonF triangle(3);
    QPointF *trianglePoints = triangle.data();
    double sw2 = 0.5 * size.width();
    double sh2 = 0.5 * size.height();
    double x = size.width()/2.;
    double y = size.height()/2.;
    const double x1 = x - sw2+2;
    const double x2 = x1 + size.width()-2;
    const double y1 = y - sh2+2;
    const double y2 = y1 + size.height()-2;

    switch(type) {
    case TriangleLeft: {
        trianglePoints[0].rx() = x2;
        trianglePoints[0].ry() = y1;
        trianglePoints[1].rx() = x1;
        trianglePoints[1].ry() = y;
        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y2;
        break;
    }
    case TriangleRight: {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y1;
        trianglePoints[1].rx() = x2;
        trianglePoints[1].ry() = y;
        trianglePoints[2].rx() = x1;
        trianglePoints[2].ry() = y2;
        break;
    }
    case TriangleUp: {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y2;
        trianglePoints[1].rx() = x;
        trianglePoints[1].ry() = y1;
        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y2;
        break;
    }
    case TriangleDown: {
        trianglePoints[0].rx() = x1;
        trianglePoints[0].ry() = y1;
        trianglePoints[1].rx() = x;
        trianglePoints[1].ry() = y2;
        trianglePoints[2].rx() = x2;
        trianglePoints[2].ry() = y1;
        break;
    }
    }
    return triangle;
}

} // namespace jsonqml

