/**
 *
 *  This file is part of the Berlin-Vegan guide (SailfishOS app version),
 *  Copyright 2015-2016 (c) by micu <micuintus.de> (micuintus@gmx.de).
 *
 *      <https://github.com/micuintus/harbour-Berlin-vegan>.
 *
 *  The Berlin-Vegan guide is Free Software:
 *  you can redistribute it and/or modify it under the terms of the
 *  GNU General Public License as published by the Free Software Foundation,
 *  either version 2 of the License, or (at your option) any later version.
 *
 *  Berlin-Vegan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Berlin Vegan Guide.
 *
 *  If not, see <https://www.gnu.org/licenses/old-licenses/gpl-2.0.html>.
 *
**/

import "../components/distance.js" as Distance

import Sailfish.Silica 1.0
import QtPositioning 5.2
import QtQuick 2.2


CoverBackground {

    property var jsonModelCollection
    property var positionSource
    readonly property double listStretch: 1.15


    CoverActionList {
        id: actionlist

        iconBackground: true

        CoverAction {
            iconSource: "image://theme/icon-cover-refresh"
            onTriggered: {
                positionSource.update();
                jsonModelCollection.invalidate();
            }
        }
    }

    SilicaListView {
        id: listView
        model: jsonModelCollection

        height: parent.height * 0.6

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: Theme.paddingLarge
        }

        header: Label {
            text: qsTr("Berlin-Vegan")
            color: Theme.highlightColor
            height: contentHeight * listStretch
        }

        delegate: ListItem {
            id: delegate

            contentHeight: namelabel.height * listStretch
            contentWidth: parent.width

            Label {
                id: namelabel
                text: model.name

                anchors {
                    right: distance.left
                    left: parent.left
                    rightMargin: Theme.paddingSmall
                    verticalCenter: parent.verticalCenter
                }

                font.pixelSize: Theme.fontSizeSmall
                truncationMode: TruncationMode.Fade
            }

            Label {
                id: distance
                text: positionSource.supportedPositioningMethods !== PositionSource.NoPositioningMethods ?
                Distance.humanReadableDistanceString(positionSource.position.coordinate,
                                                     QtPositioning.coordinate(model.latCoord, model.longCoord)) : ""
                font.pixelSize: Theme.fontSizeExtraSmall
                horizontalAlignment: Text.AlignRight
                anchors {
                    right: parent.right
                    verticalCenter: parent.verticalCenter
                }
            }
        }
    }
}