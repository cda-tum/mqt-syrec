# RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
# Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os

from .revkit import *
from .pysyrec import *

from PyQt5 import QtCore

from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *

# QString=type("")

__all__ = ['CircuitLineItem', 'GateItem', 'CircuitView', 'SyReCEditor', 'SyReCHighligher', 'QtSyReCEditor', 'RevLibEditor', 'RevLibHighlighter', 'CodeEditor', 'LineNumberArea']


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
        self.setFlag(QGraphicsItem.ItemIsSelectable)

        l = control_lines(g)
        l.extend(target_lines(g))
        l.sort()

        self.gate = g
        annotations = [["Index", index]] + list(circ.annotations(g).items())
        self.setToolTip('\n'.join(["<b><font color=\"#606060\">%s:</font></b> %s" % (k, v) for (k, v) in annotations]))

        if len(l) > 1:
            circuitLine = QGraphicsLineItem(0, l[0] * 30, 0, l[-1] * 30, self)
            self.addToGroup(circuitLine)

        for t in target_lines(g):
            if g.type == gate_type.toffoli:
                print("toffoli")
                target = QGraphicsEllipseItem(-10, t * 30 - 10, 20, 20, self)
                targetLine = QGraphicsLineItem(0, t * 30 - 10, 0, t * 30 + 10, self)
                targetLine2 = QGraphicsLineItem(-10, t * 30, 10, t * 30, self)
                self.addToGroup(target)
                self.addToGroup(targetLine)
                self.addToGroup(targetLine2)
            if g.type == gate_type.fredkin:
                print("fredkin")
                crossTL_BR = QGraphicsLineItem(-5, t * 30 - 5, 5, t * 30 + 5, self)
                crossTR_BL = QGraphicsLineItem(5, t * 30 - 5, -5, t * 30 + 5, self)
                self.addToGroup(crossTL_BR)
                self.addToGroup(crossTR_BL)


        for c in control_lines(g):
            control = QGraphicsEllipseItem(-5, c * 30 - 5, 10, 10, self)
            control.setBrush(Qt.black)
            self.addToGroup(control)

class CircuitView(QGraphicsView):

    def __init__(self, circ=None, parent=None):
        QGraphicsView.__init__(self, parent)

        # Scene
        self.setScene(QGraphicsScene(self))
        self.scene().setBackgroundBrush(Qt.white)

        # Load circuit
        self.circ = None
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
            self.inputs.append(self.addLineLabel(0, i * 30, circ.inputs[i], Qt.AlignRight, circ.constants[i] != None))
            self.outputs.append(self.addLineLabel(width, i * 30, circ.outputs[i], Qt.AlignLeft, circ.garbage[i]))

        index = 0
        for g in circ.gates():
            gateItem = GateItem(g, index, self.circ)
            gateItem.setPos(index * 30 + 15, 0)
            self.scene().addItem(gateItem)
            index += 1

    def addLineLabel(self, x, y, text, align, color):
        textItem = self.scene().addText(text)
        textItem.setPlainText(text)

        if align == Qt.AlignRight:
            x -= textItem.boundingRect().width()

        textItem.setPos(x, y - 12)

        if color:
            textItem.setDefaultTextColor(Qt.red)

        return textItem

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
        self.setupActions()

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

    def setupActions(self):

        self.openAction = QAction(QIcon.fromTheme('document-open'), '&Open...', self.parent)
        self.buildAction = QAction(QIcon.fromTheme('media-playback-start'), '&Build...', self.parent)
        self.simAction = QAction(QIcon.fromTheme('x-office-spreadsheet'), '&Sim...', self.parent)  # system-run
        self.statAction = QAction(QIcon.fromTheme('applications-other'), '&Stats...', self.parent)

        self.openAction.triggered.connect(self.open)
        self.buildAction.triggered.connect(self.build)
        self.simAction.triggered.connect(self.sim)
        self.statAction.triggered.connect(self.stat)

    def writeEditorContentsToFile(self):

        data = QFile("/tmp/out.src")
        if data.open(QFile.WriteOnly | QFile.Truncate):
            out = QTextStream(data)
            out << self.getText()
        else:
            return

        data.close()
        return

    def open(self):
        filename = QFileDialog.getOpenFileName(self.parent, 'Open Realization', '', 'SyReC specification (*.src)')
        self.load(filename)

    def load(self, filename):
        if filename:
            self.filename = filename

            f = QFile(filename[0])
            if f.open(QFile.ReadOnly | QFile.Text):
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

        tc = transistor_costs(circ, circ.lines)

        temp = "Gates:\t\t{}\nLines:\t\t{}\nQuantum Costs:\t{}\nTransistor Costs:\t{}\n"

        output = temp.format(gates, lines, qc, tc)

        msg = QMessageBox()
        msg.setBaseSize(QSize(6, 120))
        msg.setInformativeText(output)
        msg.setWindowTitle("Statistics")
        msg.setStandardButtons(QMessageBox.Ok)
        msg.exec_()

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

            bit_pos = bit_pos + 1

        no_of_bits = len(circ.constants)

        input_list = [x & bit_mask for x in range(2 ** no_of_bits)]

        for i in range(len(circ.constants)):
            if circ.constants[i] == True:
                bit1_mask = bit1_mask + 2 ** (i)

        input_list = [i + bit1_mask for i in input_list]

        print(input_list)
        input_list = list(set(input_list))

        input_list_len = len(input_list)

        combination_inp = list()
        combination_out = list()

        final_inp = list()
        final_out = list()

        p1 = properties()
        p2 = properties()

        for i in input_list:
            my_inp_bitset = bitset(circ.lines, i)
            my_out_bitset = bitset(circ.lines)

            py_simple_simulation(my_out_bitset, circ, my_inp_bitset, p1, p2)
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
        header1.setTextAlignment(QtCore.Qt.AlignCenter)
        self.table.setItem(0, 0, header1)

        self.table.setSpan(0, num_inputs, 1, num_inputs)
        header2 = QTableWidgetItem("OUTPUTS")
        header2.setTextAlignment(QtCore.Qt.AlignCenter)
        self.table.setItem(0, num_inputs, header2)

        self.table.horizontalHeader().setVisible(False)
        self.table.verticalHeader().setVisible(False)

        # Fill Table
        for i in range(num_inputs):
            input_signal = QTableWidgetItem(circ.inputs[i])
            input_signal.setTextAlignment(QtCore.Qt.AlignCenter)
            self.table.setItem(1, i, QTableWidgetItem(input_signal))

            output_signal = QTableWidgetItem(circ.outputs[i])
            output_signal.setTextAlignment(QtCore.Qt.AlignCenter)
            self.table.setItem(1, i + num_inputs, QTableWidgetItem(output_signal))

        for i in range(input_list_len):
            for j in range(num_inputs):
                input_cell = QTableWidgetItem(final_inp[i][j])
                input_cell.setTextAlignment(QtCore.Qt.AlignCenter)
                self.table.setItem(i + 2, j, QTableWidgetItem(input_cell))

                output_cell = QTableWidgetItem(final_out[i][j])
                output_cell.setTextAlignment(QtCore.Qt.AlignCenter)
                self.table.setItem(i + 2, j + num_inputs, QTableWidgetItem(output_cell))

        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)

        # Add Table to QWidget

        # show table
        self.show()


class SyReCHighligher(QSyntaxHighlighter):
    def __init__(self, parent):
        QSyntaxHighlighter.__init__(self, parent)

        self.highlightingRules = []

        keywordFormat = QTextCharFormat()
        keywordFormat.setForeground(Qt.darkBlue)
        keywordFormat.setFontWeight(QFont.Bold)
        keywords = ["module", "in", "out", "inout", "wire", "state", "if", "else", "then", "fi", "for", "step", "to", "do", "rof", "skip", "call", "uncall"]

        for pattern in ["\\b%s\\b" % keyword for keyword in keywords]:
            self.highlightingRules.append([QRegExp(pattern), keywordFormat])

        numberFormat = QTextCharFormat()
        numberFormat.setForeground(Qt.darkCyan)
        self.highlightingRules.append([QRegExp("\\b[0-9]+\\b"), numberFormat])

        loopFormat = QTextCharFormat()
        loopFormat.setForeground(Qt.darkRed)
        self.highlightingRules.append([QRegExp("\\$[A-Za-z_0-9]+"), loopFormat])

    def highlightBlock(self, text):
        for rule in self.highlightingRules:
            expression = rule[0]
            index = expression.indexIn(text)
            while index >= 0:
                length = expression.matchedLength()
                self.setFormat(index, length, rule[1])
                index = expression.indexIn(text, index + length)


class QtSyReCEditor(SyReCEditor):
    def __init__(self, parent=None):
        SyReCEditor.__init__(self, parent)

        self.widget = CodeEditor(parent)
        self.widget.setFont(QFont("Monospace", 8, QFont.Normal, False))
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
        painter.fillRect(event.rect(), Qt.lightGray)

        block = self.firstVisibleBlock()
        blockNumber = block.blockNumber()
        top = self.blockBoundingGeometry(block).translated(self.contentOffset()).top()
        bottom = top + self.blockBoundingGeometry(block).height()

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                # number = QString.number( blockNumber + 1 )
                number = str(blockNumber + 1)
                painter.setPen(Qt.black)
                painter.drawText(0, top, self.lineNumberArea.width(), self.fontMetrics().height(), Qt.AlignRight, number)

            block = block.next()
            top = bottom
            bottom = top + self.blockBoundingGeometry(block).height()
            blockNumber += 1

    def lineNumberAreaWidth(self):
        digits = 1
        max_ = max(1, self.blockCount())
        while (max_ >= 10):
            max_ /= 10
            digits += 1

        space = 3 + self.fontMetrics().width('9') * digits
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

            lineColor = QColor(Qt.yellow).lighter(160)

            selection.format.setBackground(lineColor)
            selection.format.setProperty(QTextFormat.FullWidthSelection, True)
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
        keywordFormat.setForeground(Qt.darkRed)
        keywords = ["version", "numvars", "variables", "inputs", "outputs", "inputbus", "outputbus", "state", "constants", "garbage", "module", "begin", "end"]

        for pattern in ["\\.%s" % keyword for keyword in keywords]:
            self.highlightingRules.append([QRegExp(pattern), keywordFormat])

        gateFormat = QTextCharFormat()
        gateFormat.setForeground(Qt.darkBlue)
        gateFormat.setFontWeight(QFont.Bold)
        gates = ["t", "f", "p", "v"]

        for pattern in ["\\b%s\\d*\\b" % gate for gate in gates]:
            self.highlightingRules.append([QRegExp(pattern), gateFormat])
        self.highlightingRules.append([QRegExp("v\\+"), gateFormat])

        numberFormat = QTextCharFormat()
        numberFormat.setForeground(Qt.darkCyan)
        self.highlightingRules.append([QRegExp("\\b[0-9]+\\b"), numberFormat])

        commentFormat = QTextCharFormat()
        commentFormat.setForeground(Qt.darkGray)
        commentFormat.setFontItalic(True)
        self.highlightingRules.append([QRegExp("#.*$"), commentFormat])

    def highlightBlock(self, text):
        for rule in self.highlightingRules:
            expression = rule[0]
            index = expression.indexIn(text)
            while index >= 0:
                length = expression.matchedLength()
                self.setFormat(index, length, rule[1])
                index = expression.indexIn(text, index + length)


class RevLibEditor(CodeEditor):
    def __init__(self, parent=None):
        CodeEditor.__init__(self, parent)

        self.highlighter = RevLibHighlighter(self.document())
        self.setFont(QFont("Monospace"))
