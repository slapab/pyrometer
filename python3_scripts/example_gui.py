from __future__ import unicode_literals
import sys
import os
import random
import matplotlib
import tkinter as tk


from PyQt5 import QtCore, QtWidgets
# Make sure that we are using QT5
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QMenu, QLineEdit, QSpacerItem, QSizePolicy, QTableWidget, QTableWidgetItem, QListWidget, QListWidgetItem

from PyQt5.QtGui import QClipboard

matplotlib.use('Qt5Agg')


from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure


from embedded_actions import *



progname = os.path.basename(sys.argv[0])
progversion = "0.1"


class MyMplCanvas(FigureCanvas):
    """Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.)."""

    def __init__(self, parent=None, width=5, height=4, dpi=100):
        fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = fig.add_subplot(111)
        # We want the axes cleared every time plot() is called
        self.axes.hold(False)

        self.compute_initial_figure()

        #
        FigureCanvas.__init__(self, fig)
        self.setParent(parent)

        FigureCanvas.setSizePolicy(self,
                                   QSizePolicy.Expanding,
                                   QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)

    def compute_initial_figure(self):
        pass

class MyDynamicMplCanvas(MyMplCanvas):
    """A canvas that updates itself every second with a new plot."""

    def __init__(self, *args, **kwargs):
        MyMplCanvas.__init__(self, *args, **kwargs)
        timer = QtCore.QTimer(self)
        timer.timeout.connect(self.update_figure)
        timer.start(100)

    def compute_initial_figure(self):
        self.axes.plot([0, 1, 2, 3], [1, 2, 0, 4], 'r')

    def update_figure(self):
        # Build a list of 4 random integers between 0 and 10 (both inclusive)
        l = [random.randint(0, 10) for i in range(4)]

        self.axes.plot([0, 1, 2, 3], l, 'r')
        self.draw()


class ApplicationWindow(QtWidgets.QMainWindow):
    def __init__(self):
        QtWidgets.QMainWindow.__init__(self)
        self.setAttribute(QtCore.Qt.WA_DeleteOnClose)
        self.setWindowTitle("application main window")

        self.file_menu = QtWidgets.QMenu('&File', self)
        self.file_menu.addAction('&Quit', self.fileQuit,
                                 QtCore.Qt.CTRL + QtCore.Qt.Key_Q)
        self.menuBar().addMenu(self.file_menu)

        self.help_menu = QtWidgets.QMenu('&Help', self)
        self.menuBar().addSeparator()
        self.menuBar().addMenu(self.help_menu)

        self.help_menu.addAction('&About', self.about)

        self.main_widget = QtWidgets.QWidget(self)

        mainLayout = QtWidgets.QVBoxLayout(self.main_widget)

        # convas
        # dc = MyDynamicMplCanvas(self.main_widget, width=8, height=4, dpi=100)
        # mainLayout.addWidget(dc)


        hBoxLayout = QtWidgets.QHBoxLayout()

        # self.tableWidget = QTableWidget(self)
        # self.tableWidget.setRowCount(1)
        # self.tableWidget.setColumnCount(100)
        # self.tableWidget.resizeRowToContents(0)
        #self.tableWidget.setItem(0,0, QTableWidgetItem("aa"))
        #self.tableWidget.insertRow(1)

        self.listWidget = QListWidget(self)
        # self.listWidget.addItem(QListWidgetItem("3"))


        self.factorValue = QLineEdit(self)
        self.factorValue.setText("0.98")
        self.factorValue.setFixedSize(100, 30)

        self.factorButton = QPushButton(self)
        self.factorButton.setText('Change emissivity')
        self.factorButton.clicked.connect(self.factorButtonOnClicked)
        self.factorButton.setFixedSize(150, 30)

        self.stopButton = QPushButton(self)
        self.stopButton.setText('Stop')
        self.stopButton.clicked.connect(self.stopButtonOnClicked)
        self.stopButton.setFixedSize(100, 30)

        self.copyData = QPushButton(self)
        self.copyData.setText('Copy')
        self.copyData.clicked.connect(self.onCopyButtonCliced)


        self.oneMeasureButton = QPushButton(self)
        self.oneMeasureButton.setText('One Measure')
        self.oneMeasureButton.clicked.connect(self.onMeasureButtonOnClicked)


        # self.oneMeasureButton.setFixedSize(5,5)

        # self.factorMenu = QMenu(self)
        # self.factorMenu.frameSize()
        # self.factorSubMenu = self.factorMenu.addMenu("material")
        # self.factorSubMenu.addAction("new")
        # self.factorSubMenu.addAction("new2")


        spacer = QSpacerItem(500, 20, QSizePolicy.Expanding, QSizePolicy.Minimum)
        hBoxLayout.addItem(spacer)
        # hBoxLayout.addWidget(self.factorMenu)

        mainLayout.addWidget(self.listWidget)
        # mainLayout.addWidget(self.tableWidget)

        hBoxLayout.addWidget(self.copyData)
        hBoxLayout.addWidget(self.factorValue)
        hBoxLayout.addWidget(self.factorButton)
        hBoxLayout.addWidget(self.stopButton)
        hBoxLayout.addWidget(self.oneMeasureButton)

        mainLayout.addLayout(hBoxLayout)



        self.main_widget.setFocus()
        self.setCentralWidget(self.main_widget)

        #self.statusBar().showMessage("All hail matplotlib!", 2000)

    def stopButtonOnClicked(self):
        print("\tStop button clicked")
        connMan = PirConnManager()
        connMan.sendStopCommand()



    def factorButtonOnClicked(self):
        print("\tChange emissivity factor button clicked")

        e = float(self.factorValue.text())

        # save only when e is in range
        if e >= 0.1 and e <= 1.0:
            connMan = PirConnManager()
            if True == connMan.saveEmissivity(e):
                print("Emmisivity changed to ", e)
            else:
                print("Error whiel saving emmisivity")
        else:
            print("Aborting. Wrong emmisivity value!")


    def onMeasureButtonOnClicked(self):
        print("\tOne Measuere button clicked")
        connMan = PirConnManager()
        temp = connMan.readOneSample("object")
        self.listWidget.addItem(str(round(temp[0],2))+ " °C")
        self.listWidget.setCurrentRow( self.listWidget.count() - 1 )

        # copy to clipboard
        # cl =  QApplication.clipboard()
        # cl.setText("TestTest")

        print(temp)

    def onCopyButtonCliced(self):
        # copy to clipboard
        cl =  QApplication.clipboard()

        items = []
        for index in range( self.listWidget.count()):
            items.append( self.listWidget.item(index).text())
            # print(self.listWidget.item(index).text())

        cl.setText(str(items))


        #cl.setText("TestTest")

    def fileQuit(self):
        self.close()

    def closeEvent(self, ce):
        print("exiting")
        self.fileQuit()

    def about(self):
        QtWidgets.QMessageBox.about(self, "")



qApp = QtWidgets.QApplication(sys.argv)

aw = ApplicationWindow()
aw.setWindowTitle("%s" % progname)
aw.show()
sys.exit(qApp.exec_())
#qApp.exec_()