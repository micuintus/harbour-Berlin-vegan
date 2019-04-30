import QtQuick 2.7
import Felgo 3.0
import BerlinVegan.components.platform 1.0 as BVApp

AppCheckBox {
    id: checkbox
    x: parent.width/6

    height: BVApp.Theme.iconSizeLarge
    spacing: BVApp.Theme.paddingLarge

    labelColorOff: BVApp.Theme.primaryColor
    labelFontSize: BVApp.Theme.fontSizeExtraSmall
    checkBoxSize: labelFontSize * 1.3
}
