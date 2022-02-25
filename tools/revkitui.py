#!/usr/bin/python
import re, sys

from PyQt5 import * #QtCore, QtGui, Qsci
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
#from PyQt4.Qsci import *

from revkit import *
from revkitui import *



class LogWidget( QTreeWidget ):
    def __init__( self, parent = None ):
        QTreeWidget.__init__( self, parent )

        self.setRootIsDecorated( False )
        self.setHeaderLabels( [ "Line", "Message" ] )

    def addMessage( self, line, message ):
        item = QTreeWidgetItem( [ str( line ), message ] )
        self.addTopLevelItem( item )

class MainWindow( QMainWindow ):
    def __init__( self, parent = None ):
        QWidget.__init__( self, parent )

        self.setWindowTitle( "Text Editor" )

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
        self.editor.build_failed = lambda error_message: self.logWidget.addMessage( re.search ( 'In line (.*): (.*)', error_message ).group( 1 ), \
                                                                                    re.search ( 'In line (.*): (.*)', error_message ).group( 2 ) )

    def setupToolBar( self ):
        toolbar = self.addToolBar( 'Main' )
        toolbar.setIconSize( QSize( 32, 32 ) )

        toolbar.addAction( self.editor.openAction )
        toolbar.addAction( self.editor.buildAction )
        toolbar.addAction( self.editor.simAction )
        toolbar.addAction( self.editor.statAction )

if __name__ == "__main__":
    a = QApplication([])

    w = MainWindow()
    w.show()

    sys.exit( a.exec_() )