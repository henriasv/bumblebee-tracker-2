import QtQuick 2.7
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import TrackingController 1.0

ApplicationWindow {
    id : root
    visible: true
    width: 640
    minimumWidth: mainToolBar.implicitWidth
    height: 480
    title: qsTr("Bumblebee")

    function frameUpdate() {
        camA.source="image://bumblebee/A/"+timeStepSlider.value+"/"+imageMode.current.text;
        camB.source="image://bumblebee/B/"+timeStepSlider.value+"/"+imageMode.current.text
    }

    toolBar : ToolBar {
        id : mainToolBar
        RowLayout {
            anchors.fill: parent
            Button {
                text: "Load File"
                onClicked: root.color = "khaki"
            }
            Button {
                text: "Save trajectories"
                onClicked: gui.setSource("file:/users/henriksveinsson/Dropbox/diverse/henrik.jpg")
            }

            Slider {
                id : timeStepSlider
                Layout.fillWidth: true
                implicitWidth: 150
                stepSize: 1
                minimumValue: 1
                maximumValue: 10000
                onValueChanged: frameLabel.text=this.value, frameUpdate()
                updateValueWhileDragging: false
            }
            Label {
                id : frameLabel
                text : "Frame"

            }
        }
    }


    RowLayout
    {
        spacing : 10
        anchors.fill : parent
        Rectangle {
            anchors.top: parent.top
            id: modeMenuContainer
            width: 150

            ColumnLayout {
                spacing : 10

                ExclusiveGroup {
                    id : imageMode
                    onCurrentChanged: frameUpdate()
                }
                RadioButton {
                    id : rawButton
                    text : "Raw"
                    checked : true
                    exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Lab"
                    checked : false
                    exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Smoothed"
                    checked : false
                    exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Binary"
                    checked : false
                    exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Raw + Identifiers"
                    checked : false
                    exclusiveGroup : imageMode
                }

            }
        }



        Rectangle{
            color : "gray"
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
            Layout.fillHeight: true

            Column {
                anchors.fill: parent
                spacing : 10

                Rectangle{
                    width : parent.width
                    height : parent.height/2
                    color : parent.parent.color
                    id : camArec

                    Image {
                        id: camA
                        anchors.fill : parent
                        source: "file:/Users/henriksveinsson/Desktop/plantegning.png"
                        fillMode: Image.PreserveAspectFit
                    }

                }

                Rectangle{
                    id : camBrec
                    color : parent.parent.color
                    width : parent.width
                    height : parent.height/2

                    Image {
                        id: camB
                        anchors.fill : parent
                        source: "file:/Users/henriksveinsson/Desktop/plantegning.png"
                        fillMode: Image.PreserveAspectFit
                    }

                }
            }
        }
    }
}
