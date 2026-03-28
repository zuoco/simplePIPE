#include "ui/PropertyModel.h"

#include "model/PipePoint.h"
#include "model/PipeSpec.h"

#include <QString>

#include <sstream>

namespace ui {

namespace {

std::string formatDouble(double value)
{
    std::ostringstream oss;
    oss.setf(std::ios::fixed);
    oss.precision(3);
    oss << value;
    return oss.str();
}

} // namespace

PropertyModel::PropertyModel(app::Document& document,
                             app::SelectionManager& selectionManager,
                             QObject* parent)
    : QAbstractListModel(parent)
    , document_(document)
    , selectionManager_(selectionManager)
{
    selectionManager_.addSelectionChangedCallback([this](const std::vector<foundation::UUID>&) {
        rebuild();
    });
    rebuild();
}

int PropertyModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(rows_.size());
}

QVariant PropertyModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || static_cast<std::size_t>(index.row()) >= rows_.size()) {
        return {};
    }

    const Row& row = rows_[static_cast<std::size_t>(index.row())];

    if (role == GroupRole) {
        return QString::fromStdString(row.group);
    }
    if (role == KeyRole) {
        return QString::fromStdString(row.key);
    }
    if (role == ValueRole || role == Qt::DisplayRole) {
        return QString::fromStdString(row.value);
    }
    if (role == EditableRole) {
        return row.editable;
    }
    return {};
}

QHash<int, QByteArray> PropertyModel::roleNames() const
{
    return {
        {GroupRole, "group"},
        {KeyRole, "key"},
        {ValueRole, "value"},
        {EditableRole, "editable"},
    };
}

void PropertyModel::refresh()
{
    rebuild();
}

void PropertyModel::rebuild()
{
    beginResetModel();
    rows_.clear();

    if (!selectionManager_.selected().empty()) {
        model::DocumentObject* selectedObject = document_.findObject(selectionManager_.selected().front());
        if (auto* point = dynamic_cast<model::PipePoint*>(selectedObject)) {
            pushPipePointRows(*point);
        } else if (auto* spec = dynamic_cast<model::PipeSpec*>(selectedObject)) {
            pushPipeSpecRows(*spec);
        }
    }

    endResetModel();
}

void PropertyModel::pushPipePointRows(const model::PipePoint& point)
{
    rows_.push_back({"Summary", "name", point.name(), true});
    rows_.push_back({"Summary", "type", pipePointTypeName(point.type()), false});

    rows_.push_back({"Coordinates", "x", formatDouble(point.position().X()), true});
    rows_.push_back({"Coordinates", "y", formatDouble(point.position().Y()), true});
    rows_.push_back({"Coordinates", "z", formatDouble(point.position().Z()), true});

    const std::string pipeSpecName = point.pipeSpec() ? point.pipeSpec()->name() : std::string{};
    rows_.push_back({"PipeSpec", "name", pipeSpecName, true});

    if (point.type() == model::PipePointType::Bend) {
        const std::string bendMultiplier = point.hasParam("bendMultiplier")
            ? formatDouble(foundation::variantToDouble(point.param("bendMultiplier")))
            : std::string{"1.500"};
        rows_.push_back({"Bend", "bendMultiplier", bendMultiplier, true});
    }

    for (const auto& entry : point.typeParams()) {
        if (entry.first == "bendMultiplier") {
            continue;
        }

        std::string value;
        if (std::holds_alternative<double>(entry.second)) {
            value = formatDouble(std::get<double>(entry.second));
        } else if (std::holds_alternative<int>(entry.second)) {
            value = std::to_string(std::get<int>(entry.second));
        } else {
            value = std::get<std::string>(entry.second);
        }

        rows_.push_back({"TypeParameters", entry.first, value, true});
    }
}

void PropertyModel::pushPipeSpecRows(const model::PipeSpec& spec)
{
    rows_.push_back({"PipeSpec", "name", spec.name(), true});

    for (const auto& [key, value] : spec.fields()) {
        std::string text;
        if (std::holds_alternative<double>(value)) {
            text = formatDouble(std::get<double>(value));
        } else if (std::holds_alternative<int>(value)) {
            text = std::to_string(std::get<int>(value));
        } else {
            text = std::get<std::string>(value);
        }

        rows_.push_back({"Fields", key, text, true});
    }
}

std::string PropertyModel::pipePointTypeName(model::PipePointType type)
{
    switch (type) {
    case model::PipePointType::Run:
        return "Run";
    case model::PipePointType::Bend:
        return "Bend";
    case model::PipePointType::Reducer:
        return "Reducer";
    case model::PipePointType::Tee:
        return "Tee";
    case model::PipePointType::Valve:
        return "Valve";
    case model::PipePointType::FlexJoint:
        return "FlexJoint";
    default:
        return "Unknown";
    }
}

} // namespace ui
