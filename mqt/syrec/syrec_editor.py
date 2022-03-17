#!/usr/bin/python
import re

from mqt.syrec.pysyrec import *

from PyQt6 import QtCore
from PyQt6.QtCore import *
from PyQt6.QtGui import *
from PyQt6.QtWidgets import *


def error_msg(statistics):
    """Returns the error message contained in statistics"""

    return "Error: " + statistics.get_string("error", "")


def read_program(prog, filename, default_bitwidth=32):
    """Reads a SyReC program from a file"""

    settings = read_program_settings()
    settings.default_bitwidth = default_bitwidth

    error = py_read_program(prog, filename, settings)

    return error


def syrec_synthesis(circ, prog, variable_name_format="%1$s%3$s.%2$d", main_module="", if_realization=0, efficient_controls=True, modules_hierarchy=False):
    settings = properties()

    settings.set_string("variable_name_format", variable_name_format)
    settings.set_string("main_module", main_module)
    settings.set_unsigned("if_realization", if_realization)
    settings.set_bool("efficient_controls", efficient_controls)
    settings.set_bool("modules_hierarchy", modules_hierarchy)

    statistics = properties()

    if py_syrec_synthesis(circ, prog, settings, statistics):
        return dict(runtime=statistics.get_double("runtime"))
    else:
        return error_msg(statistics)


class CircuitLineItem(QGraphicsItemGroup):
    def __init__(self, index, width, parent=None):
        QGraphicsItemGroup.__init__(self, parent)

        # Tool Tip
        self.setToolTip("<b><font color=\"#606060\">Line:</font></b> %d" % index)

        # Create sub-lines
        x = 0
        for i in range(0, width + 1):
            e_width = 15 if i == 0 or i == width else 30
            self.addToGroup(QGraphicsLineItem(x, index * 30, x + e_width, index * 30))
            x += e_width


class GateItem(QGraphicsItemGroup):
    def __init__(self, g, index, circ, parent=None):
        QGraphicsItemGroup.__init__(self, parent)
        self.setFlag(QGraphicsItem.GraphicsItemFlag.ItemIsSelectable)

        l = list(g.controls)
        l.extend(list(g.targets))
        l.sort()

        self.gate = g
        annotations = [["Index", index]] + list(circ.annotations(g).items())
        self.setToolTip('\n'.join(["<b><font color=\"#606060\">%s:</font></b> %s" % (k, v) for (k, v) in annotations]))

        if len(l) > 1:
            circuitLine = QGraphicsLineItem(0, l[0] * 30, 0, l[-1] * 30, self)
            self.addToGroup(circuitLine)

        for t in g.targets:
            if g.type == gate_type.toffoli:
                target = QGraphicsEllipseItem(-10, t * 30 - 10, 20, 20, self)
                target_line = QGraphicsLineItem(0, t * 30 - 10, 0, t * 30 + 10, self)
                target_line2 = QGraphicsLineItem(-10, t * 30, 10, t * 30, self)
                self.addToGroup(target)
                self.addToGroup(target_line)
                self.addToGroup(target_line2)
            if g.type == gate_type.fredkin:
                crossTL_BR = QGraphicsLineItem(-5, t * 30 - 5, 5, t * 30 + 5, self)
                crossTR_BL = QGraphicsLineItem(5, t * 30 - 5, -5, t * 30 + 5, self)
                self.addToGroup(crossTL_BR)
                self.addToGroup(crossTR_BL)

        for c in g.controls:
            control = QGraphicsEllipseItem(-5, c * 30 - 5, 10, 10, self)
            control.setBrush(QColorConstants.Black)
            self.addToGroup(control)


class CircuitView(QGraphicsView):

    def __init__(self, circ=None, parent=None):
        QGraphicsView.__init__(self, parent)

        # Scene
        self.setScene(QGraphicsScene(self))
        self.scene().setBackgroundBrush(QColorConstants.White)

        # Load circuit
        self.circ = None
        self.lines = []
        self.inputs = []
        self.outputs = []
        if circ is not None:
            self.load(circ)

    def load(self, circ):
        for item in self.scene().items():
            self.scene().removeItem(item)

        self.circ = circ
        self.lines = []
        self.inputs = []
        self.outputs = []

        width = 30 * circ.num_gates

        for i in range(0, circ.lines):
            line = CircuitLineItem(i, circ.num_gates)
            self.lines.append(line)
            self.scene().addItem(line)
            self.inputs.append(self.add_line_label(0, i * 30, circ.inputs[i], Qt.AlignmentFlag.AlignRight, circ.constants[i] is not None))
            self.outputs.append(self.add_line_label(width, i * 30, circ.outputs[i], Qt.AlignmentFlag.AlignLeft, circ.garbage[i]))

        index = 0
        for g in circ:
            gate_item = GateItem(g, index, self.circ)
            gate_item.setPos(index * 30 + 15, 0)
            self.scene().addItem(gate_item)
            index += 1

    def add_line_label(self, x, y, text, align, color):
        text_item = self.scene().addText(text)
        text_item.setPlainText(text)

        if align == Qt.AlignmentFlag.AlignRight:
            x -= text_item.boundingRect().width()

        text_item.setPos(x, y - 12)

        if color:
            text_item.setDefaultTextColor(QColorConstants.Red)

        return text_item

    def wheelEvent(self, event):
        factor = 1.2
        if event.angleDelta().y() < 0 or event.angleDelta().x() < 0:
            factor = 1.0 / factor
        self.scale(factor, factor)

        return QGraphicsView.wheelEvent(self, event)


class SyReCEditor(QWidget):
    widget = None
    build_successful = None
    build_failed = None
    before_build = None
    parser_failed = None

    def __init__(self, parent=None):
        super().__init__()

        self.parent = parent
        self.setup_actions()

        self.filename = type("")

        self.title = 'SyReC Simulation'
        self.left = 0
        self.top = 0
        self.width = 600
        self.height = 400

        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        self.layout = QVBoxLayout()

        self.table = QTableWidget()
        self.layout.addWidget(self.table)
        self.setLayout(self.layout)

    def setup_actions(self):
        self.open_action = QAction(QIcon.fromTheme('document-open'), '&Open...', self.parent)
        self.build_action = QAction(QIcon.fromTheme('media-playback-start'), '&Build...', self.parent)
        self.sim_action = QAction(QIcon.fromTheme('x-office-spreadsheet'), '&Sim...', self.parent)  # system-run
        self.stat_action = QAction(QIcon.fromTheme('applications-other'), '&Stats...', self.parent)

        self.open_action.triggered.connect(self.open)
        self.build_action.triggered.connect(self.build)
        self.sim_action.triggered.connect(self.sim)
        self.stat_action.triggered.connect(self.stat)

    def writeEditorContentsToFile(self):
        data = QFile("/tmp/out.src")
        if data.open(QFile.OpenModeFlag.WriteOnly | QFile.OpenModeFlag.Truncate):
            out = QTextStream(data)
            out << self.getText()
        else:
            return

        data.close()
        return

    def open(self):
        filename = QFileDialog.getOpenFileName(parent=self.parent, caption='Open Specification', filter='SyReC specification (*.src)')
        self.load(filename)

    def load(self, filename):
        if filename:
            self.filename = filename

            f = QFile(filename[0])
            if f.open(QFile.OpenModeFlag.ReadOnly | QFile.OpenModeFlag.Text):
                ts = QTextStream(f)
                self.setText(ts.readAll())

        return

    def build(self):
        if self.before_build is not None:
            self.before_build()

        self.writeEditorContentsToFile()

        prog = syrec_program()

        error_string = read_program(prog, "/tmp/out.src")

        if error_string == "PARSE_STRING_FAILED":
            if self.parser_failed is not None:
                self.parser_failed("Editor is Empty")
            return

        elif error_string != "":
            if self.build_failed is not None:
                self.build_failed(error_string)
            return

        circ = circuit()

        syrec_synthesis(circ, prog)

        print("Number Of Gates         : ", circ.num_gates)
        print("Number Of Lines         : ", circ.lines)
        print("Number Of Inputs        : ", circ.inputs)
        print("Number Of Constants     : ", circ.constants)
        print("Number of Garbage Lines : ", circ.garbage)

        if self.build_successful is not None:
            self.build_successful(circ)

        return

    def stat(self):
        if self.before_build is not None:
            self.before_build()

        self.writeEditorContentsToFile()

        prog = syrec_program()

        error_string = read_program(prog, "/tmp/out.src")
        if error_string == "PARSE_STRING_FAILED":
            if self.parser_failed is not None:
                self.parser_failed("Editor is Empty")
            return

        elif error_string != "":
            if self.build_failed is not None:
                self.build_failed(error_string)
            return

        circ = circuit()
        syrec_synthesis(circ, prog)

        gates = circ.num_gates
        lines = circ.lines

        qc = quantum_costs(circ, circ.lines)

        tc = transistor_costs(circ)

        temp = "Gates:\t\t{}\nLines:\t\t{}\nQuantum Costs:\t{}\nTransistor Costs:\t{}\n"

        output = temp.format(gates, lines, qc, tc)

        msg = QMessageBox()
        msg.setBaseSize(QSize(300, 200))
        msg.setInformativeText(output)
        msg.setWindowTitle("Statistics")
        msg.setStandardButtons(QMessageBox.StandardButton.Ok)
        msg.exec()

        return

    def sim(self):
        if self.before_build is not None:
            self.before_build()

        self.writeEditorContentsToFile()

        prog = syrec_program()

        error_string = read_program(prog, "/tmp/out.src")
        if error_string == "PARSE_STRING_FAILED":
            if self.parser_failed is not None:
                self.parser_failed("Editor is Empty")
            return
        elif error_string != "":
            if self.build_failed is not None:
                self.build_failed(error_string)
            return

        circ = circuit()

        syrec_synthesis(circ, prog)

        bit_mask = 0
        bit_pos = 0
        bit1_mask = 0

        for i in range(len(circ.inputs)):
            print(circ.inputs[i])

        for i in circ.constants:

            if i is None:
                bit_mask = bit_mask + 2 ** bit_pos

            bit_pos += 1

        no_of_bits = len(circ.constants)

        input_list = [x & bit_mask for x in range(2 ** no_of_bits)]

        for i in range(len(circ.constants)):
            if circ.constants[i]:
                bit1_mask = bit1_mask + 2 ** i

        input_list = [i + bit1_mask for i in input_list]

        print(input_list)
        input_list = list(set(input_list))

        input_list_len = len(input_list)

        combination_inp = list()
        combination_out = list()

        final_inp = list()
        final_out = list()

        settings = properties()

        for i in input_list:
            my_inp_bitset = bitset(circ.lines, i)
            my_out_bitset = bitset(circ.lines)

            py_simple_simulation(my_out_bitset, circ, my_inp_bitset, settings)
            combination_inp.append(str(my_inp_bitset))
            combination_out.append(str(my_out_bitset))

        print(combination_inp)
        print(combination_out)

        # sorted_inp_str=sorted(combination_inp,key=lambda x: int(x,2))

        sorted_ind = sorted(range(len(combination_inp)), key=lambda k: int(combination_inp[k], 2))

        print(sorted_ind)

        for i in sorted_ind:
            final_inp.append(combination_inp[i])
            final_out.append(combination_out[i])

        print(final_inp)
        print(final_out)

        num_inputs = len(circ.inputs)

        # Initiate table

        self.table.clear()
        self.table.setRowCount(0)
        self.table.setColumnCount(0)

        self.table.setRowCount(input_list_len + 2)
        self.table.setColumnCount(2 * num_inputs)

        self.table.setSpan(0, 0, 1, num_inputs)
        header1 = QTableWidgetItem("INPUTS")
        header1.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
        self.table.setItem(0, 0, header1)

        self.table.setSpan(0, num_inputs, 1, num_inputs)
        header2 = QTableWidgetItem("OUTPUTS")
        header2.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
        self.table.setItem(0, num_inputs, header2)

        self.table.horizontalHeader().setVisible(False)
        self.table.verticalHeader().setVisible(False)

        # Fill Table
        for i in range(num_inputs):
            input_signal = QTableWidgetItem(circ.inputs[i])
            input_signal.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            self.table.setItem(1, i, QTableWidgetItem(input_signal))

            output_signal = QTableWidgetItem(circ.outputs[i])
            output_signal.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
            self.table.setItem(1, i + num_inputs, QTableWidgetItem(output_signal))

        for i in range(input_list_len):
            for j in range(num_inputs):
                input_cell = QTableWidgetItem(final_inp[i][j])
                input_cell.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                self.table.setItem(i + 2, j, QTableWidgetItem(input_cell))

                output_cell = QTableWidgetItem(final_out[i][j])
                output_cell.setTextAlignment(Qt.AlignmentFlag.AlignCenter)
                self.table.setItem(i + 2, j + num_inputs, QTableWidgetItem(output_cell))

        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.ResizeMode.Stretch)

        # Add Table to QWidget

        # show table
        self.show()


class SyReCHighligher(QSyntaxHighlighter):
    def __init__(self, parent):
        QSyntaxHighlighter.__init__(self, parent)

        self.highlightingRules = []

        keywordFormat = QTextCharFormat()
        keywordFormat.setForeground(QColorConstants.DarkBlue)
        keywordFormat.setFontWeight(QFont.Weight.Bold)
        keywords = ["module", "in", "out", "inout", "wire", "state", "if", "else", "then", "fi", "for", "step", "to", "do", "rof", "skip", "call", "uncall"]

        for pattern in ["\\b%s\\b" % keyword for keyword in keywords]:
            self.highlightingRules.append([QtCore.QRegularExpression(pattern), keywordFormat])

        numberFormat = QTextCharFormat()
        numberFormat.setForeground(QColorConstants.DarkCyan)
        self.highlightingRules.append([QtCore.QRegularExpression("\\b[0-9]+\\b"), numberFormat])

        loopFormat = QTextCharFormat()
        loopFormat.setForeground(QColorConstants.DarkRed)
        self.highlightingRules.append([QtCore.QRegularExpression("\\$[A-Za-z_0-9]+"), loopFormat])

    def highlightBlock(self, text):
        for rule in self.highlightingRules:
            expression = rule[0]
            match = expression.match(text)
            while match.hasMatch():
                index = match.capturedStart()
                length = match.capturedLength()
                self.setFormat(index, length, rule[1])
                match = expression.match(text, offset=index + length)


class QtSyReCEditor(SyReCEditor):
    def __init__(self, parent=None):
        SyReCEditor.__init__(self, parent)

        self.widget = CodeEditor(parent)
        self.widget.setFont(QFont("Monospace", 10, QFont.Weight.Normal))
        self.widget.highlighter = SyReCHighligher(self.widget.document())

    def setText(self, text):
        self.widget.setPlainText(text)

    def getText(self):
        return self.widget.toPlainText()


class LineNumberArea(QWidget):
    def __init__(self, editor):
        QWidget.__init__(self, editor)
        self.codeEditor = editor

    def sizeHint(self):
        return QSize(self.codeEditor.lineNumberAreaWidth(), 0)

    def paintEvent(self, event):
        self.codeEditor.lineNumberAreaPaintEvent(event)


class CodeEditor(QPlainTextEdit):
    def __init__(self, parent=None):
        QPlainTextEdit.__init__(self, parent)

        self.lineNumberArea = LineNumberArea(self)

        self.blockCountChanged.connect(self.updateLineNumberAreaWidth)
        self.updateRequest.connect(self.updateLineNumberArea)
        self.cursorPositionChanged.connect(self.highlightCurrentLine)

        self.updateLineNumberAreaWidth(0)
        self.highlightCurrentLine()

    def load(self, filename):
        if len(filename) > 0:
            self.setPlainText(open(filename, 'r').read())

    def lineNumberAreaPaintEvent(self, event):
        painter = QPainter(self.lineNumberArea)
        painter.fillRect(event.rect(), QColorConstants.LightGray)

        block = self.firstVisibleBlock()
        block_number = block.blockNumber()
        top = self.blockBoundingGeometry(block).translated(self.contentOffset()).top()
        bottom = top + self.blockBoundingGeometry(block).height()

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(block_number + 1)
                painter.setPen(QColorConstants.Black)
                painter.drawText(0, round(top), self.lineNumberArea.width(), self.fontMetrics().height(), Qt.AlignmentFlag.AlignRight, number)

            block = block.next()
            top = bottom
            bottom = top + self.blockBoundingGeometry(block).height()
            block_number += 1

    def lineNumberAreaWidth(self):
        digits = 1
        max_ = max(1, self.blockCount())
        while max_ >= 10:
            max_ /= 10
            digits += 1

        space = 3 + self.fontMetrics().horizontalAdvance('9') * digits
        return space

    def resizeEvent(self, event):
        QPlainTextEdit.resizeEvent(self, event)

        cr = self.contentsRect()
        self.lineNumberArea.setGeometry(QRect(cr.left(), cr.top(), self.lineNumberAreaWidth(), cr.height()))

    def updateLineNumberAreaWidth(self, newBlockCount):
        self.setViewportMargins(self.lineNumberAreaWidth(), 0, 0, 0)

    def highlightCurrentLine(self):
        extraSelections = []

        if not self.isReadOnly():
            selection = QTextEdit.ExtraSelection()

            lineColor = QColorConstants.Yellow.lighter(160)

            selection.format.setBackground(lineColor)
            selection.format.setProperty(QTextFormat.Property.FullWidthSelection, True)
            selection.cursor = self.textCursor()
            selection.cursor.clearSelection()
            extraSelections.append(selection)

        self.setExtraSelections(extraSelections)

    def updateLineNumberArea(self, rect, dy):
        if dy != 0:
            self.lineNumberArea.scroll(0, dy)
        else:
            self.lineNumberArea.update(0, rect.y(), self.lineNumberArea.width(), rect.height())

        if rect.contains(self.viewport().rect()):
            self.updateLineNumberAreaWidth(0)


class RevLibHighlighter(QSyntaxHighlighter):
    def __init__(self, parent):
        QSyntaxHighlighter.__init__(self, parent)

        self.highlightingRules = []

        keywordFormat = QTextCharFormat()
        keywordFormat.setForeground(QColorConstants.DarkRed)
        keywords = ["version", "numvars", "variables", "inputs", "outputs", "inputbus", "outputbus", "state", "constants", "garbage", "module", "begin", "end"]

        for pattern in ["\\.%s" % keyword for keyword in keywords]:
            self.highlightingRules.append([QtCore.QRegularExpression(pattern), keywordFormat])

        gateFormat = QTextCharFormat()
        gateFormat.setForeground(QColorConstants.DarkBlue)
        gateFormat.setFontWeight(QFont.Weight.Bold)
        gates = ["t", "f", "p", "v"]

        for pattern in ["\\b%s\\d*\\b" % gate for gate in gates]:
            self.highlightingRules.append([QtCore.QRegularExpression(pattern), gateFormat])
        self.highlightingRules.append([QtCore.QRegularExpression("v\\+"), gateFormat])

        numberFormat = QTextCharFormat()
        numberFormat.setForeground(QColorConstants.DarkCyan)
        self.highlightingRules.append([QtCore.QRegularExpression("\\b[0-9]+\\b"), numberFormat])

        commentFormat = QTextCharFormat()
        commentFormat.setForeground(QColorConstants.DarkGray)
        commentFormat.setFontItalic(True)
        self.highlightingRules.append([QtCore.QRegularExpression("#.*$"), commentFormat])

    def highlightBlock(self, text):
        for rule in self.highlightingRules:
            expression = rule[0]
            match = expression.match(text)
            while match.hasMatch():
                index = match.capturedStart()
                length = match.capturedLength()
                self.setFormat(index, length, rule[1])
                match = expression.match(text, offset=index + length)


class RevLibEditor(CodeEditor):
    def __init__(self, parent=None):
        CodeEditor.__init__(self, parent)

        self.highlighter = RevLibHighlighter(self.document())
        self.setFont(QFont("Monospace"))


class LogWidget(QTreeWidget):
    def __init__(self, parent=None):
        QTreeWidget.__init__(self, parent)

        self.setRootIsDecorated(False)
        self.setHeaderLabels(["Message"])

    def addMessage(self, message):
        item = QTreeWidgetItem([message])
        self.addTopLevelItem(item)

    # def addLineAndMessage( self, line, message ):
    #    item = QTreeWidgetItem( [ str( int(line) + 1 ), message ] )
    #    self.addTopLevelItem( item )


class MainWindow(QMainWindow):
    def __init__(self, parent=None):
        QWidget.__init__(self, parent)

        self.setWindowTitle("SyReC Editor")

        self.setup_widgets()
        self.setup_dock_widgets()
        self.setup_actions()
        self.setup_toolbar()

    def setup_widgets(self):
        self.editor = QtSyReCEditor(self)
        self.viewer = CircuitView(parent=self)

        splitter = QSplitter(Qt.Orientation.Vertical, self)
        splitter.addWidget(self.editor.widget)
        splitter.addWidget(self.viewer)

        self.setCentralWidget(splitter)

    def setup_dock_widgets(self):
        self.logWidget = LogWidget(self)
        self.logDockWidget = QDockWidget("Log Messages", self)
        self.logDockWidget.setWidget(self.logWidget)
        self.addDockWidget(Qt.DockWidgetArea.BottomDockWidgetArea, self.logDockWidget)

    def setup_actions(self):
        self.editor.before_build = self.logWidget.clear
        self.editor.build_successful = lambda circ: self.viewer.load(circ)
        self.editor.parser_failed = lambda error_message: self.logWidget.addMessage(error_message)
        self.editor.build_failed = lambda error_message: self.logWidget.addMessage(re.search('In line (.*): (.*)', error_message).group(2))

    def setup_toolbar(self):
        toolbar = self.addToolBar('Main')
        toolbar.setIconSize(QSize(32, 32))

        toolbar.addAction(self.editor.open_action)
        toolbar.addAction(self.editor.build_action)
        toolbar.addAction(self.editor.sim_action)
        toolbar.addAction(self.editor.stat_action)


def main():
    a = QApplication([])

    w = MainWindow()
    w.show()

    return a.exec()


if __name__ == "__main__":
    main()
