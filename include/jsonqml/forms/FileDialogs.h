#pragma once

#include <QWidget>
#include <QString>


namespace jsonqml {

/// Open&Save Image File filters
QString imageFilters();

/// Open&Save text File filters
extern const QString textFilters;

/// Open&Save JSON or YAML or XML File filters
extern const QString domFilters;
/// Open&Save JSON File filters
extern const QString jsonFilters;

#ifndef IMPEX_OFF

/// Open&Save YAML File filters
extern const QString yamlFilters;
/// Open&Save XML File filters
extern const QString xmlFilters;

#endif

extern const QString csvFilters;

/// Function returns true and an existing file selected by the user into param path_.
/// If the user presses Cancel, it returns false
bool ChooseFileOpen(QWidget* par, QString& file_path, const QString& title, const QString& filter);

/// Function returns true and an existing file selected by the user into param path_.
/// If the user presses Cancel, it returns false
/// The file does not have to exist.
bool ChooseFileSave(QWidget* par, QString& file_path, const QString& title, const QString& filter, const QString& schemafilter = "");

} // namespace jsonui

