#pragma once

#include <QtCore/QHash>
#include <QtCore/QSet>
#include <QStandardItemModel>
#include <QtQml/QJSValue>

class QReadWriteLock;
class QQmlEngine;

constexpr inline int enumValueToFlag(const int enumValue, const int enumOffset = 0)
{
    return 1 << (enumValue - enumOffset);
}

class VenueModel : public QStandardItemModel
{
    Q_OBJECT

    Q_PROPERTY(VenueTypeFlags loadedVenueType READ loadedVenueType NOTIFY loadedVenueTypeChanged)

public:
    enum VenueVegCategory
    {
        Unknown                 = 0,
        // Defined by the GastroLocations.json format
        // which comes from the berlin-vegan.de backend
        Omnivore                = 1,
        OmnivoreVeganDeclared   = 2,
        Vegetarian              = 3,
        VegetarianVeganDeclared = 4,
        Vegan                   = 5,
    };
    Q_ENUM(VenueVegCategory)

    enum VenueType
    {
        Food,
        Shopping,
    };
    Q_ENUM(VenueType)

    enum VenueTypeFlag
    {
        FoodFlag     = enumValueToFlag(VenueType::Food),
        ShoppingFlag = enumValueToFlag(VenueType::Shopping)
    };
    Q_DECLARE_FLAGS(VenueTypeFlags, VenueTypeFlag)
    Q_FLAG(VenueTypeFlags)
    Q_ENUM(VenueTypeFlag)

    enum VenueModelRoles
    {
        ID = Qt::UserRole + 1,
        Name,

        // Model Category
        VenueTypeRole,
        Favorite,

        Street,
        Description,
        Website,
        Telephone,
        Pictures,

        // Coordinates
        LatCoord,
        LongCoord,

        // Properties
        FirstPropertyRole,
        Wlan = FirstPropertyRole,
        VegCategory,
        HandicappedAccessible,
        HandicappedAccessibleWc,
        Catering,
        Organic,
        GlutenFree,
        Delivery,
        SeatsOutdoor,
        SeatsIndoor,
        Dog,
        ChildChair,
        LastPropertyRole = ChildChair,

        // OpeningHours
        OtMon,
        OtTue,
        OtWed,
        OtThu,
        OtFri,
        OtSat,
        OtSun
    };

    VenueModel(QObject *parent = 0);

    Q_INVOKABLE void importFromJson(const QJSValue&, VenueType venueType);
    Q_INVOKABLE void setFavorite(const QString& id, bool favorite = true);
    QHash<int, QByteArray> roleNames() const override;

    VenueTypeFlags loadedVenueType() const;

signals:
    void loadedVenueTypeChanged();

private:
    QModelIndex indexFromID(const QString& id) const;
    QStandardItem* jsonItem2QStandardItem(const QJSValue& from);
    VenueTypeFlags m_loadedVenueType;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(VenueModel::VenueTypeFlags)
