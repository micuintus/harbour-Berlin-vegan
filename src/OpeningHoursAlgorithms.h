#pragma once

#include "VenueModel.h"

#include <QStandardItem>
#include <QVariant>
#include <QtQml/QJSValue>

#include <QDateTime>
#include <QtMath>

#define DAYS_PER_WEEK 7
#define HOURS_PER_DAY 24
#define MICROSECONDS_PER_SECOND 1000
#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR 60
#define MINUTES_PER_DAY (HOURS_PER_DAY * MINUTES_PER_HOUR)
#define MINUTES_CLOSES_SOON 30

#define MONDAY_INDEX    static_cast<unsigned char>(0)
#define TUESDAY_INDEX   static_cast<unsigned char>(1)
#define WEDNESDAY_INDEX static_cast<unsigned char>(2)
#define THURSDAY_INDEX  static_cast<unsigned char>(3)
#define FRIDAY_INDEX    static_cast<unsigned char>(4)
#define SATURDAY_INDEX  static_cast<unsigned char>(5)
#define SUNDAY_INDEX    static_cast<unsigned char>(6)


// Condense opening hours part --->

inline QVariantMap mergeElements(const QVariantList& openingHours,
                          const int from,
                          const int until,
                          const int todayIndex)
{
    QVariantMap result;

    if (from == until)
    {
        result = openingHours[from].toMap();
    }
    else
    {
        const auto& fromDay  = openingHours[from]. toMap()["day"].toString();
        const auto& untilDay = openingHours[until].toMap()["day"].toString();

        result["day"]      = fromDay + " - " + untilDay;
        result["hours"]    = openingHours[from].toMap()["hours"];
    }

    result["current"]  = from <= todayIndex && todayIndex <= until;

    return result;
}

inline QVariantList condenseOpeningHours(const QVariantList& uncondensedOpeningHours, const int todayIndex = -1)
{
    QVariantList condensedOpeningHours;

    int curr = 0;
    const int numElements = uncondensedOpeningHours.size();

    while (curr < numElements)
    {
        int next = curr + 1;

        while (next < numElements
               &&    uncondensedOpeningHours[curr].toMap()["hours"]
                  == uncondensedOpeningHours[next].toMap()["hours"]
               && uncondensedOpeningHours[next].toMap()["day"]
                  != qtTrId("id-sunday")) // Do not merge sundays
        {
            next++;
        }

        condensedOpeningHours.append(mergeElements(uncondensedOpeningHours, curr, next - 1, todayIndex));

        curr = next;
    }

    return condensedOpeningHours;
}

inline QString hoursString(const QJSValue& from, const QString& property)
{
    const QString& hoursString = from.property(property).toVariant().toString();

    if (hoursString.isEmpty())
    {
                  //% "closed"
        return qtTrId("id-closed");
    }
    else
    {
        return hoursString;
    }
}

inline QVariantList extractOpenHoursData(const QJSValue& from)
{
    QVariantList uncondensedOpeningHours
    {
                                 //% "Monday"
        QVariantMap {{ "day", qtTrId("id-monday")},    { "hours", hoursString(from, "otMon") }},
                                //% "Tuesday"
        QVariantMap {{ "day", qtTrId("id-tuesday")},   { "hours", hoursString(from, "otTue") }},
                                 //% "Wednesday"
        QVariantMap {{ "day", qtTrId("id-wednesday")}, { "hours", hoursString(from, "otWed") }},
                                 //% "Thursday"
        QVariantMap {{ "day", qtTrId("id-thursday")},  { "hours", hoursString(from, "otThu") }},
                                 //% "Friday"
        QVariantMap {{ "day", qtTrId("id-friday")},    { "hours", hoursString(from, "otFri") }},
                                 //% "Saturday"
        QVariantMap {{ "day", qtTrId("id-saturday")},  { "hours", hoursString(from, "otSat") }},
                                 //% "Sunday / Holiday"
        QVariantMap {{ "day", qtTrId("id-sunday")},    { "hours", hoursString(from, "otSun") }}
    };

    return uncondensedOpeningHours;
}

// <--- Condense opening hours part

// Extract machine readable hours part --->

inline int minute(const QString& time)
{
    if (time == nullptr || time.isEmpty())
    {
        return 0;
    }

    int hour   = 0;
    int minute = 0;

    if (time.contains(":"))
    {
        const QStringList parts = time.split(":");
        hour   = parts[0].trimmed().toInt();
        minute = parts[1].trimmed().toInt();
    }
    else
    {
        hour = time.trimmed().toInt();
    }

    return hour * MINUTES_PER_HOUR + minute;
}

inline QVariantMap parseOpeningMinutes(const QString& openingString)
{
    int startMinute = 0;
    int endMinute   = 0;

    if (openingString.contains("-"))
    {
        const QStringList parts  = openingString.split("-");
        const QString& startTime = parts[0];

        startMinute = minute(startTime);

        if (parts.size() > 1)
        {
            const QString& endTime = parts[1];
            endMinute = minute(endTime);

            if (startMinute != 0 && endMinute == 0)
            {
                endMinute = MINUTES_PER_DAY;
            }
        }
        else if (startMinute != 0)
        {
            endMinute = MINUTES_PER_DAY;
        }
    }

    if (endMinute < startMinute) // closing time is after midnight
    {
        endMinute += MINUTES_PER_DAY;
    }

    return QVariantMap
    {
        { "startMinute", startMinute },
        { "endMinute",   endMinute   }
    };
}

inline QVariantList extractOpeningMinutes(const QVariantList& openingHours)
{
    QVariantList openingMinutes;
    std::transform(openingHours.begin(), openingHours.end(), std::back_inserter(openingMinutes),
                   [](const QVariant& openingLine)
    {
        const QString& openingString = openingLine.toMap()["hours"].toString();
        return parseOpeningMinutes(openingString);
    });

    return openingMinutes;
}

// <--- Extract machine readable hours part

// Opening state calculations --->

inline bool isShortAfterMidnight(const QDateTime& dateTime)
{
    return dateTime.time() < QTime(6, 0, 0);
}

inline QDate easterSunday(int year)
{
    // calulate easter date
    // https://stackoverflow.com/a/1284335
    const auto C = qFloor(year/100);
    const auto N = year - 19*qFloor(year/19);
    const auto K = qFloor((C - 17)/25);
    auto I = C - qFloor(C/4) - qFloor((C - K)/3) + 19*N + 15;
    I = I - 30*qFloor((I/30));
    I = I - qFloor(I/28)*(1 - qFloor(I/28)*qFloor(29/(I + 1))*qFloor((21 - N)/11));
    auto J = year + qFloor(year/4) + I + 2 - C + qFloor(C/4);
    J = J - 7*qFloor(J/7);
    const auto L = I - J;
    const auto month = 3 + qFloor((L + 40)/44);
    const auto day = L + 28 - 31*qFloor(month/4);

    // easter sunday
    return QDate{year, month, day};
}

inline bool isPublicHoliday(const QDate &date)
{
    const auto year = date.year();
    auto const es = easterSunday(year);

    const QDate newYearsDay(year, 1, 1);
    const QDate internationalWomensDay(year, 3, 8);
    const QDate goodFriday(es.addDays(-2));
    const QDate easterMonday(es.addDays(1));
    const QDate labourDay{year, 05, 01};
    const QDate ascensionDay(es.addDays(39));
    const QDate whitMonday(es.addDays(50));
    const QDate dayOfGermanUnity(year, 10, 3);
    const QDate firstChristmasDay(year, 12, 25);
    const QDate secondChristmasDay(year, 12, 26);

    // C++ doesn't allow for non-integral types in switch statements
    if (date == newYearsDay)
        return true;
    else if (date == internationalWomensDay)
        return true;
    else if (date == goodFriday)
        return true;
    else if (date == easterMonday)
        return true;
    else if (date == labourDay)
        return true;
    else if (date == ascensionDay)
        return true;
    else if (date == whitMonday)
        return true;
    else if (date == dayOfGermanUnity)
        return true;
    else if (date == firstChristmasDay)
        return true;
    else if (date == secondChristmasDay)
        return true;
    return false;
}

inline std::pair<unsigned char, unsigned> extractDayIndexAndMinute(QDateTime dateTime)
{
    const int currentHour = dateTime.time().hour();
    int currentMinute = currentHour * MINUTES_PER_HOUR + dateTime.time().minute();

    unsigned char dayIndex = static_cast<unsigned char>(dateTime.date().dayOfWeek() - 1);

    if (isShortAfterMidnight(dateTime))            // If that is the case, we treat this time as if from the day before:
    {
        currentMinute += MINUTES_PER_DAY;          // We count the minutes starting from the day before
        dayIndex = (dayIndex + 6) % DAYS_PER_WEEK; // and we use the opening hour from the day before.
        dateTime = dateTime.addDays(-1);
    }

    if (isPublicHoliday(dateTime.date()))
    {
        dayIndex = SUNDAY_INDEX;
    }

    return { dayIndex, currentMinute };
}

inline bool isInRange(const QVariantMap& openingMinutes, const unsigned currentMinute)
{
    return currentMinute >= openingMinutes["startMinute"].toUInt()
        && currentMinute <= openingMinutes["endMinute"].toUInt();
}


// <--- Opening state calculations


inline void extractAndProcessOpenHoursData(QStandardItem& to, const QJSValue& from)
{
    auto const openingHours = extractOpenHoursData(from);
    to.setData(openingHours, VenueModel::OpeningHours);

    auto const openingMinutes = extractOpeningMinutes(openingHours);
    to.setData(openingMinutes, VenueModel::OpeningMinutes);
}
