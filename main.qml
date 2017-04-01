import QtQuick 2.7
import QtQuick.Controls 2.1
import QtQuick.Layouts 1.3
import TrackingController 1.0

ApplicationWindow {
    id : root
    visible: true
    width: 640
    minimumWidth: mainToolBar.implicitWidth
    height: 480
    title: qsTr("Bumblebee")

    function frameUpdate() {
        controller.update();
        camA.source="image://bumblebee/A/"+timeStepSlider.value+"/"+imageTypeGroup.checkedButton.text
        camB.source="image://bumblebee/B/"+timeStepSlider.value+"/"+imageTypeGroup.checkedButton.text
        timeStepSlider.to = controller.frameMax
    }

    header : ToolBar {
        id : mainToolBar
        RowLayout {
            anchors.fill: parent
            Button {
                text: "Load File"
                onClicked: root.color = "khaki"
            }
            ToolButton {
                text: "Save trajectories"
            }

            Slider {
                id : timeStepSlider
                Layout.fillWidth: true
                implicitWidth: 150
                from : 1
                to : 10000
                stepSize: 1
                snapMode: "SnapAlways"
                onValueChanged: frameLabel.text=this.value, frameUpdate()
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

        //Rectangle {
        //    anchors.top: parent.top
        //    id: modeMenuContainer
        //    width: 150

            ButtonGroup {
                id : imageTypeGroup
                onCheckedButtonChanged : frameUpdate()
            }
            Column{
                id : imageType

                RadioButton {
                    id : rawButton
                    text : "Raw"
                    checked : true
                    ButtonGroup.group : imageTypeGroup
                    //exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Lab"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                    //exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "ColorFiltered"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                }

                RadioButton {
                    text : "Smoothed"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                    //exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Binary"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                    //exclusiveGroup : imageMode
                }
                RadioButton {
                    text : "Raw + Identifiers"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                    //exclusiveGroup : imageMode
                }


                TextField {
                    id : thresholdField
                    text : "120"
                    onEditingFinished: controller.threshold = parseInt(text), frameUpdate();
                }

                RangeSlider {
                    from: 1
                    to: 100
                    first.value: 25
                    second.value: 75
                }

                Button {
                    id : updateButton
                    text : "Update"
                    onClicked : frameUpdate()
                }
            }

           // }





        /*
        Rectangle{
            color : "gray"
            Layout.alignment: Qt.AlignCenter
            //Layout.fillWidth: true
            //Layout.fillHeight: true
            anchors.fill : parent
        */
            Column {
                //anchors.fill: parent
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing : 10

                Rectangle{
                    width : parent.width
                    height : parent.height/2
                    color : "transparent"
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
                    color : "transparent"
                    //Layout.fillWidth:true
                    //Layout.fillHeight: true
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

