#!/usr/bin/python
from __future__ import annotations

import re
from typing import Callable

from mqt import syrec
from PyQt6 import QtCore, QtGui, QtWidgets


class CircuitLineItem(QtWidgets.QGraphicsItemGroup):
    def __init__(self, index, width, parent=None):
        QtWidgets.QGraphicsItemGroup.__init__(self, parent)

        # Tool Tip
        self.setToolTip(f'<b><font color="#606060">Line:</font></b> {index:d}')

        # Create sub-lines
        x = 0
        for i in range(0, width + 1):
            e_width = 15 if i in {0, width} else 30
            self.addToGroup(QtWidgets.QGraphicsLineItem(x, index * 30, x + e_width, index * 30))
            x += e_width


class GateItem(QtWidgets.QGraphicsItemGroup):
    def __init__(self, g, index, circ, parent=None):
        QtWidgets.QGraphicsItemGroup.__init__(self, parent)
        self.setFlag(QtWidgets.QGraphicsItem.GraphicsItemFlag.ItemIsSelectable)

        lines = list(g.controls)
        lines.extend(list(g.targets))
        lines.sort()

        self.gate = g
        annotations = [["Index", index]] + list(circ.annotations(g).items())
        self.setToolTip("\n".join([f'<b><font color="#606060">{k}:</font></b> {v}' for (k, v) in annotations]))

        if len(lines) > 1:
            circuitLine = QtWidgets.QGraphicsLineItem(0, lines[0] * 30, 0, lines[-1] * 30, self)
            self.addToGroup(circuitLine)

        for t in g.targets:
            if g.type == syrec.gate_type.toffoli:
                target = QtWidgets.QGraphicsEllipseItem(-10, t * 30 - 10, 20, 20, self)
                target_line = QtWidgets.QGraphicsLineItem(0, t * 30 - 10, 0, t * 30 + 10, self)
                target_line2 = QtWidgets.QGraphicsLineItem(-10, t * 30, 10, t * 30, self)
                self.addToGroup(target)
                self.addToGroup(target_line)
                self.addToGroup(target_line2)
            if g.type == syrec.gate_type.fredkin:
                crossTL_BR = QtWidgets.QGraphicsLineItem(-5, t * 30 - 5, 5, t * 30 + 5, self)
                crossTR_BL = QtWidgets.QGraphicsLineItem(5, t * 30 - 5, -5, t * 30 + 5, self)
                self.addToGroup(crossTL_BR)
                self.addToGroup(crossTR_BL)

        for c in g.controls:
            control = QtWidgets.QGraphicsEllipseItem(-5, c * 30 - 5, 10, 10, self)
            control.setBrush(QtGui.QColorConstants.Black)
            self.addToGroup(control)


class CircuitView(QtWidgets.QGraphicsView):
    def __init__(self, circ=None, parent=None):
        QtWidgets.QGraphicsView.__init__(self, parent)

        # Scene
        self.setScene(QtWidgets.QGraphicsScene(self))
        self.scene().setBackgroundBrush(QtGui.QColorConstants.White)

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
            self.inputs.append(
                self.add_line_label(
                    0, i * 30, circ.inputs[i], QtCore.Qt.AlignmentFlag.AlignRight, circ.constants[i] is not None
                )
            )
            self.outputs.append(
                self.add_line_label(width, i * 30, circ.outputs[i], QtCore.Qt.AlignmentFlag.AlignLeft, circ.garbage[i])
            )

        index = 0
        for g in circ:
            gate_item = GateItem(g, index, self.circ)
            gate_item.setPos(index * 30 + 15, 0)
            self.scene().addItem(gate_item)
            index += 1

    def add_line_label(self, x, y, text, align, color):
        text_item = self.scene().addText(text)
        text_item.setPlainText(text)

        if align == QtCore.Qt.AlignmentFlag.AlignRight:
            x -= text_item.boundingRect().width()

        text_item.setPos(x, y - 12)

        if color:
            text_item.setDefaultTextColor(QtGui.QColorConstants.Red)

        return text_item

    def wheelEvent(self, event):
        factor = 1.2
        if event.angleDelta().y() < 0 or event.angleDelta().x() < 0:
            factor = 1.0 / factor
        self.scale(factor, factor)

        return QtWidgets.QGraphicsView.wheelEvent(self, event)


class SyReCEditor(QtWidgets.QWidget):
    widget = None
    build_successful: Callable[[syrec.circuit], None] = None
    build_failed: Callable[[str], None] = None
    before_build: Callable[[], None] = None
    parser_failed: Callable[[str], None] = None

    cost_aware_synthesis = 0
    line_aware_synthesis = 0

    def __init__(self, parent=None):
        super().__init__()

        self.parent = parent
        self.setup_actions()

        self.filename = str

        self.title = "SyReC Simulation"
        self.left = 0
        self.top = 0
        self.width = 600
        self.height = 400

        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        self.layout = QtWidgets.QVBoxLayout()

        self.table = QtWidgets.QTableWidget()
        self.layout.addWidget(self.table)
        self.setLayout(self.layout)

    def setup_actions(self):
        self.open_action = QtGui.QAction(QtGui.QIcon.fromTheme("document-open"), "&Open...", self.parent)
        self.build_action = QtGui.QAction(QtGui.QIcon.fromTheme("media-playback-start"), "&Build...", self.parent)
        self.sim_action = QtGui.QAction(
            QtGui.QIcon.fromTheme("x-office-spreadsheet"), "&Sim...", self.parent
        )  # system-run
        self.stat_action = QtGui.QAction(QtGui.QIcon.fromTheme("applications-other"), "&Stats...", self.parent)

        self.butonCostAware = QtWidgets.QRadioButton("Cost-aware synthesis", self)
        self.butonCostAware.toggled.connect(self.item_selected)

        self.buttonLineAware = QtWidgets.QRadioButton("Line-aware synthesis", self)
        self.buttonLineAware.setChecked(True)
        self.line_aware_synthesis = 1
        self.buttonLineAware.toggled.connect(self.item_selected)

        self.sim_action.setDisabled(True)
        self.stat_action.setDisabled(True)

        self.open_action.triggered.connect(self.open)

        self.build_action.triggered.connect(self.build)

        self.sim_action.triggered.connect(self.sim)

        self.stat_action.triggered.connect(self.stat)

    def item_selected(self):

        # Disable sim and stat function
        self.sim_action.setDisabled(True)
        self.stat_action.setDisabled(True)

        # if first button is selected
        if self.sender() == self.butonCostAware:
            if self.butonCostAware.isChecked():
                self.cost_aware_synthesis = 1
                self.line_aware_synthesis = 0
                # making other check box to uncheck
                self.buttonLineAware.setChecked(False)
            else:
                self.cost_aware_synthesis = 0
                self.line_aware_synthesis = 1
                # making other check box to uncheck
                self.buttonLineAware.setChecked(True)

        # if second button is selected
        elif self.sender() == self.buttonLineAware:
            if self.buttonLineAware.isChecked():
                self.cost_aware_synthesis = 0
                self.line_aware_synthesis = 1
                # making other check box to uncheck
                self.butonCostAware.setChecked(False)
            else:
                self.cost_aware_synthesis = 1
                self.line_aware_synthesis = 0
                # making other check box to uncheck
                self.butonCostAware.setChecked(True)

    def writeEditorContentsToFile(self):
        data = QtCore.QFile("/tmp/out.src")
        if data.open(QtCore.QFile.OpenModeFlag.WriteOnly | QtCore.QFile.OpenModeFlag.Truncate):
            out = QtCore.QTextStream(data)
            out << self.getText()
        else:
            return

        data.close()

        return

    def open(self):
        filename = QtWidgets.QFileDialog.getOpenFileName(
            parent=self.parent, caption="Open Specification", filter="SyReC specification (*.src)"
        )
        self.load(filename)

    def load(self, filename):
        if filename:
            self.filename = filename

            f = QtCore.QFile(filename[0])
            if f.open(QtCore.QFile.OpenModeFlag.ReadOnly | QtCore.QFile.OpenModeFlag.Text):
                ts = QtCore.QTextStream(f)
                self.setText(ts.readAll())

    def build(self):

        if self.before_build is not None:
            self.before_build()

        self.writeEditorContentsToFile()

        self.prog = syrec.program()

        error_string = self.prog.read("/tmp/out.src")

        if error_string == "PARSE_STRING_FAILED":
            if self.parser_failed is not None:
                self.parser_failed("Editor is Empty")
            return

        if error_string != "":
            if self.build_failed is not None:
                self.build_failed(error_string)
            return

        self.circ = syrec.circuit()

        if self.cost_aware_synthesis:
            syrec.cost_aware_synthesis(self.circ, self.prog)
        else:
            syrec.line_aware_synthesis(self.circ, self.prog)

        self.sim_action.setDisabled(False)
        self.stat_action.setDisabled(False)

        num_consts = sum(c is not None for c in self.circ.constants)
        num_garbage = sum(self.circ.garbage)
        print("Number Of Gates         : ", self.circ.num_gates)
        print("Number Of Lines         : ", self.circ.lines)
        print("Number Of Inputs        : ", self.circ.lines - num_consts)
        print("Number Of Constants     : ", num_consts)
        print("Number of Outputs       : ", self.circ.lines - num_garbage)
        print("Number of Garbage Lines : ", num_garbage)

        if self.build_successful is not None:
            self.build_successful(self.circ)

    def stat(self):

        gates = self.circ.num_gates
        lines = self.circ.lines

        qc = self.circ.quantum_cost()

        tc = self.circ.transistor_cost()

        temp = "Gates:\t\t{}\nLines:\t\t{}\nQuantum Costs:\t{}\nTransistor Costs:\t{}\n"

        output = temp.format(gates, lines, qc, tc)

        msg = QtWidgets.QMessageBox()
        msg.setBaseSize(QtCore.QSize(300, 200))
        msg.setInformativeText(output)
        msg.setWindowTitle("Statistics")
        msg.setStandardButtons(QtWidgets.QMessageBox.StandardButton.Ok)
        msg.exec()

    def sim(self):

        bit_mask = 0
        bit_pos = 0
        bit1_mask = 0

        for i in self.circ.constants:

            if i is None:
                bit_mask = bit_mask + 2**bit_pos

            bit_pos += 1

        no_of_bits = len(self.circ.constants)

        input_list = [x & bit_mask for x in range(2**no_of_bits)]

        for i, constant in enumerate(self.circ.constants):
            if constant:
                bit1_mask = bit1_mask + 2**i

        input_list = [i + bit1_mask for i in input_list]

        input_list = list(set(input_list))

        input_list_len = len(input_list)

        combination_inp = []
        combination_out = []

        final_inp = []
        final_out = []

        settings = syrec.properties()

        for i in input_list:
            my_inp_bitset = syrec.bitset(self.circ.lines, i)
            my_out_bitset = syrec.bitset(self.circ.lines)

            syrec.simple_simulation(my_out_bitset, self.circ, my_inp_bitset, settings)
            combination_inp.append(str(my_inp_bitset))
            combination_out.append(str(my_out_bitset))

        sorted_ind = sorted(range(len(combination_inp)), key=lambda k: int(combination_inp[k], 2))

        for i in sorted_ind:
            final_inp.append(combination_inp[i])
            final_out.append(combination_out[i])

        num_inputs = len(self.circ.inputs)

        # Initiate table

        self.table.clear()
        self.table.setRowCount(0)
        self.table.setColumnCount(0)

        self.table.setRowCount(input_list_len + 2)
        self.table.setColumnCount(2 * num_inputs)

        self.table.setSpan(0, 0, 1, num_inputs)
        header1 = QtWidgets.QTableWidgetItem("INPUTS")
        header1.setTextAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        self.table.setItem(0, 0, header1)

        self.table.setSpan(0, num_inputs, 1, num_inputs)
        header2 = QtWidgets.QTableWidgetItem("OUTPUTS")
        header2.setTextAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
        self.table.setItem(0, num_inputs, header2)

        self.table.horizontalHeader().setVisible(False)
        self.table.verticalHeader().setVisible(False)

        # Fill Table
        for i in range(num_inputs):
            input_signal = QtWidgets.QTableWidgetItem(self.circ.inputs[i])
            input_signal.setTextAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
            self.table.setItem(1, i, QtWidgets.QTableWidgetItem(input_signal))

            output_signal = QtWidgets.QTableWidgetItem(self.circ.outputs[i])
            output_signal.setTextAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
            self.table.setItem(1, i + num_inputs, QtWidgets.QTableWidgetItem(output_signal))

        for i in range(input_list_len):
            for j in range(num_inputs):
                input_cell = QtWidgets.QTableWidgetItem(final_inp[i][j])
                input_cell.setTextAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
                self.table.setItem(i + 2, j, QtWidgets.QTableWidgetItem(input_cell))

                output_cell = QtWidgets.QTableWidgetItem(final_out[i][j])
                output_cell.setTextAlignment(QtCore.Qt.AlignmentFlag.AlignCenter)
                self.table.setItem(i + 2, j + num_inputs, QtWidgets.QTableWidgetItem(output_cell))

        self.table.horizontalHeader().setStretchLastSection(True)
        self.table.horizontalHeader().setSectionResizeMode(QtWidgets.QHeaderView.ResizeMode.Stretch)
        self.show()


class SyReCHighligher(QtGui.QSyntaxHighlighter):
    def __init__(self, parent):
        QtGui.QSyntaxHighlighter.__init__(self, parent)

        self.highlightingRules = []

        keywordFormat = QtGui.QTextCharFormat()
        keywordFormat.setForeground(QtGui.QColorConstants.DarkBlue)
        keywordFormat.setFontWeight(QtGui.QFont.Weight.Bold)
        keywords = [
            "module",
            "in",
            "out",
            "inout",
            "wire",
            "state",
            "if",
            "else",
            "then",
            "fi",
            "for",
            "step",
            "to",
            "do",
            "rof",
            "skip",
            "call",
            "uncall",
        ]

        for pattern in [f"\\b{keyword}\\b" for keyword in keywords]:
            self.highlightingRules.append([QtCore.QRegularExpression(pattern), keywordFormat])

        numberFormat = QtGui.QTextCharFormat()
        numberFormat.setForeground(QtGui.QColorConstants.DarkCyan)
        self.highlightingRules.append([QtCore.QRegularExpression("\\b[0-9]+\\b"), numberFormat])

        loopFormat = QtGui.QTextCharFormat()
        loopFormat.setForeground(QtGui.QColorConstants.DarkRed)
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
        self.widget.setFont(QtGui.QFont("Monospace", 10, QtGui.QFont.Weight.Normal))
        self.widget.highlighter = SyReCHighligher(self.widget.document())

    def setText(self, text):
        self.widget.setPlainText(text)

    def getText(self):
        return self.widget.toPlainText()


class LineNumberArea(QtWidgets.QWidget):
    def __init__(self, editor):
        QtWidgets.QWidget.__init__(self, editor)
        self.codeEditor = editor

    def sizeHint(self):
        return QtCore.QSize(self.codeEditor.lineNumberAreaWidth(), 0)

    def paintEvent(self, event):
        self.codeEditor.lineNumberAreaPaintEvent(event)


class CodeEditor(QtWidgets.QPlainTextEdit):
    def __init__(self, parent=None):
        QtWidgets.QPlainTextEdit.__init__(self, parent)

        self.lineNumberArea = LineNumberArea(self)

        self.blockCountChanged.connect(self.updateLineNumberAreaWidth)
        self.updateRequest.connect(self.updateLineNumberArea)
        self.cursorPositionChanged.connect(self.highlightCurrentLine)

        self.updateLineNumberAreaWidth()
        self.highlightCurrentLine()

    def load(self, filename):
        if len(filename) > 0:
            with open(filename, encoding="utf-8") as f:
                self.setPlainText(f.read())

    def lineNumberAreaPaintEvent(self, event):
        painter = QtGui.QPainter(self.lineNumberArea)
        painter.fillRect(event.rect(), QtGui.QColorConstants.LightGray)

        block = self.firstVisibleBlock()
        block_number = block.blockNumber()
        top = self.blockBoundingGeometry(block).translated(self.contentOffset()).top()
        bottom = top + self.blockBoundingGeometry(block).height()

        while block.isValid() and top <= event.rect().bottom():
            if block.isVisible() and bottom >= event.rect().top():
                number = str(block_number + 1)
                painter.setPen(QtGui.QColorConstants.Black)
                painter.drawText(
                    0,
                    round(top),
                    self.lineNumberArea.width(),
                    self.fontMetrics().height(),
                    QtCore.Qt.AlignmentFlag.AlignRight,
                    number,
                )

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

        space = 3 + self.fontMetrics().horizontalAdvance("9") * digits
        return space

    def resizeEvent(self, event):
        QtWidgets.QPlainTextEdit.resizeEvent(self, event)

        cr = self.contentsRect()
        self.lineNumberArea.setGeometry(QtCore.QRect(cr.left(), cr.top(), self.lineNumberAreaWidth(), cr.height()))

    def updateLineNumberAreaWidth(self):
        self.setViewportMargins(self.lineNumberAreaWidth(), 0, 0, 0)

    def highlightCurrentLine(self):
        extraSelections = []

        if not self.isReadOnly():
            selection = QtWidgets.QTextEdit.ExtraSelection()

            lineColor = QtGui.QColorConstants.Yellow.lighter(160)

            selection.format.setBackground(lineColor)
            selection.format.setProperty(QtGui.QTextFormat.Property.FullWidthSelection, True)
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
            self.updateLineNumberAreaWidth()


class LogWidget(QtWidgets.QTreeWidget):
    def __init__(self, parent=None):
        QtWidgets.QTreeWidget.__init__(self, parent)

        self.setRootIsDecorated(False)
        self.setHeaderLabels(["Message"])

    def addMessage(self, message):
        item = QtWidgets.QTreeWidgetItem([message])
        self.addTopLevelItem(item)


class MainWindow(QtWidgets.QMainWindow):
    def __init__(self, parent=None):
        QtWidgets.QWidget.__init__(self, parent)

        self.setWindowTitle("SyReC Editor")

        self.setup_widgets()
        self.setup_dock_widgets()
        self.setup_actions()
        self.setup_toolbar()

    def setup_widgets(self):
        self.editor = QtSyReCEditor(self)
        self.viewer = CircuitView(parent=self)

        splitter = QtWidgets.QSplitter(QtCore.Qt.Orientation.Vertical, self)
        splitter.addWidget(self.editor.widget)
        splitter.addWidget(self.viewer)

        self.setCentralWidget(splitter)

    def setup_dock_widgets(self):
        self.logWidget = LogWidget(self)
        self.logDockWidget = QtWidgets.QDockWidget("Log Messages", self)
        self.logDockWidget.setWidget(self.logWidget)
        self.addDockWidget(QtCore.Qt.DockWidgetArea.BottomDockWidgetArea, self.logDockWidget)

    def setup_actions(self):
        self.editor.before_build = self.logWidget.clear
        self.editor.build_successful = self.viewer.load
        self.editor.parser_failed = self.logWidget.addMessage
        self.editor.build_failed = lambda error_message: self.logWidget.addMessage(
            re.search("In line (.*): (.*)", error_message).group(2)
        )

    def setup_toolbar(self):
        toolbar = self.addToolBar("Main")
        toolbar.setIconSize(QtCore.QSize(32, 32))

        toolbar.addAction(self.editor.open_action)
        toolbar.addAction(self.editor.build_action)
        toolbar.addAction(self.editor.sim_action)
        toolbar.addAction(self.editor.stat_action)
        toolbar.addWidget(self.editor.butonCostAware)
        toolbar.addWidget(self.editor.buttonLineAware)


def main():
    a = QtWidgets.QApplication([])

    w = MainWindow()
    w.show()

    return a.exec()


if __name__ == "__main__":
    main()
