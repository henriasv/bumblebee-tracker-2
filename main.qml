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

    Component.onCompleted: {
        controller.framesUpdated.connect(function() {
            timer.start()
            camA.source="image://bumblebee/A/"+timeStepSlider.value+"/"+imageTypeGroup.checkedButton.text
            camB.source="image://bumblebee/B/"+timeStepSlider.value+"/"+imageTypeGroup.checkedButton.text
            camStereo.source="image://bumblebee/Stereo/"+timeStepSlider.value+"/"+imageTypeGroup.checkedButton.text

        })
    }


    function frameUpdate() {
        controller.requestFrameUpdate(timeStepSlider.value+"/"+imageTypeGroup.checkedButton.text, parseInt(thresholdField.text), stereoButton.checked);
        timeStepSlider.to = controller.frameMax
    }



    Timer {
        id : timer
        repeat: false
        interval : 5
        onTriggered: if (playButton.checked) {timeStepSlider.value += 1}
    }


    function togglePlay() {
        if (playButton.checked)
        {
            timer.stop()
            timer.start()
        }
        else
        {
            timer.stop()
        }

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
            Button {
                id : playButton
                text: "play"
                checkable: true
                onClicked : togglePlay()
            }

            Slider {
                id : timeStepSlider
                Layout.fillWidth: true
                implicitWidth: 150
                from : 1
                to : 10000
                stepSize: 1
                snapMode: "SnapAlways"
                onValueChanged: frameLabel.text=parseInt(this.value), frameUpdate()
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
                    text : "ColorFiltered"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                }

                RadioButton{
                    text : "DOG"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
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
                RadioButton {
                    text : "Features"
                    checked : false
                    ButtonGroup.group : imageTypeGroup
                    //exclusiveGroup : imageMode
                }


                Button {
                    id : stereoButton
                    checkable : true
                    checked: false
                    text : "Stereo"
                }

                Label {
                    text : "Threshold"
                }

                TextField {
                    id : thresholdField
                    text : "100"
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                Label {
                    text : "Lower Gaussian width"
                }

                TextField {
                    id : dogField1
                    text : "15"
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                Label {
                    text : "Upper Gaussian width"
                }

                TextField {
                    id : dogField2
                    text : "31"
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                Label {
                    text : "Minimum Area"
                }

                TextField {
                    id : minimumAreaField
                    text : "100"
                }

                Label {
                    text : "Maximum Area"
                }

                TextField {
                    id : maximumAreaField
                    text : "2000"
                }

                Button {
                    id : dogButton
                    text: "Invoke settings"
                    onPressed: controller.setParameters(parseInt(dogField1.text), parseInt(dogField2.text), parseInt(minimumAreaField.text), parseInt(maximumAreaField.text))
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
            Row {
                //anchors.fill: parent
                Layout.fillWidth: true
                Layout.fillHeight: true
                spacing : 10

                Rectangle{
                    width : parent.width/3
                    height : parent.height
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
                    width : parent.width/3
                    height : parent.height

                    Image {
                        id: camB
                        anchors.fill : parent
                        source: "file:/Users/henriksveinsson/Desktop/plantegning.png"
                        fillMode: Image.PreserveAspectFit
                    }

                }
                Rectangle{
                    id : camStereoRec
                    color : "transparent"
                    //Layout.fillWidth:true
                    //Layout.fillHeight: true
                    width : parent.width/3
                    height : parent.height

                    Image {
                        id: camStereo
                        anchors.fill : parent
                        source: "file:/Users/henriksveinsson/Desktop/plantegning.png"
                        fillMode: Image.PreserveAspectFit
                    }

                }
            }
        }
    }

