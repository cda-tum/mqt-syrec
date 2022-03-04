#!/usr/bin/python
import re, sys

from PyQt5 import * #QtCore, QtGui, Qsci
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *


from .revkitui import *



class LogWidget( QTreeWidget ):
    def __init__( self, parent = None ):
        QTreeWidget.__init__( self, parent )

        self.setRootIsDecorated( False )
        #self.setHeaderLabels( [ "Line", "Message" ] )
        self.setHeaderLabels( [ "Message" ] )

    def addMessage(self, message):
        item = QTreeWidgetItem( [ message ] )
        self.addTopLevelItem( item )

    #def addLineAndMessage( self, line, message ):
    #    item = QTreeWidgetItem( [ str( int(line) + 1 ), message ] )
    #    self.addTopLevelItem( item )

class MainWindow( QMainWindow ):
    def __init__( self, parent = None ):
        QWidget.__init__( self, parent )

        self.setWindowTitle( "SyReC Editor" )

        self.setupWidgets()
        self.setupDockWidgets()
        self.setupActions()
        self.setupToolBar()

    def setupWidgets( self ):
        self.editor = QtSyReCEditor( self )
        self.viewer = CircuitView( None, self )

        splitter = QSplitter( Qt.Vertical, self )
        splitter.addWidget( self.editor.widget )
        splitter.addWidget( self.viewer )

        self.setCentralWidget( splitter )


    def setupDockWidgets( self ):
        self.logWidget = LogWidget( self )
        self.logDockWidget = QDockWidget( "Log Messages", self )
        self.logDockWidget.setWidget( self.logWidget )
        self.addDockWidget( Qt.BottomDockWidgetArea, self.logDockWidget )

    def setupActions( self ):
        self.editor.before_build = self.logWidget.clear
        self.editor.build_successful = lambda circ: self.viewer.load( circ )
        self.editor.parser_failed = lambda error_message: self.logWidget.addMessage(error_message)
        self.editor.build_failed = lambda error_message: self.logWidget.addMessage(re.search( 'In line (.*): (.*)', error_message ).group( 2 ))

        #self.editor.build_failed = lambda error_message: self.logWidget.addLineAndMessage( re.search ( 'In line (.*): (.*)', error_message ).group( 1 ), \
        #                                                                                   re.search ( 'In line (.*): (.*)', error_message ).group( 2 ) )

    def setupToolBar( self ):
        toolbar = self.addToolBar( 'Main' )
        toolbar.setIconSize( QSize( 32, 32 ) )

        toolbar.addAction( self.editor.openAction )
        toolbar.addAction( self.editor.buildAction )
        toolbar.addAction( self.editor.simAction )
        toolbar.addAction( self.editor.statAction )


def main():
    a = QApplication([])

    w = MainWindow()
    w.show()

    sys.exit(a.exec_())
