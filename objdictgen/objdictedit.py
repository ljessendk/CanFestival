#Boa:Frame:objdictedit
#!/usr/bin/env python
# -*- coding: utf-8 -*-

#This file is part of CanFestival, a library implementing CanOpen Stack. 
#
#Copyright (C): Edouard TISSERANT, Francis DUPIN and Laurent BESSARD
#
#See COPYING file for copyrights details.
#
#This library is free software; you can redistribute it and/or
#modify it under the terms of the GNU Lesser General Public
#License as published by the Free Software Foundation; either
#version 2.1 of the License, or (at your option) any later version.
#
#This library is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#Lesser General Public License for more details.
#
#You should have received a copy of the GNU Lesser General Public
#License along with this library; if not, write to the Free Software
#Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

from wxPython.wx import *
from wxPython.html import *
from wxPython.grid import *
import wx
from wx.lib.anchors import LayoutAnchors
import wx.grid

from types import *
import os, re, platform, sys, time, traceback, getopt

__version__ = "$Revision$"

from nodemanager import *
from node import OD_Subindex,OD_MultipleSubindexes,OD_IdenticalSubindexes,OD_IdenticalIndexes
from doc_index.DS301_index import *

wxEVT_HTML_URL_CLICK = wxNewId()

def EVT_HTML_URL_CLICK(win, func):
    win.Connect(-1, -1, wxEVT_HTML_URL_CLICK, func)

class wxHtmlWindowUrlClick(wxPyEvent):
    def __init__(self, linkinfo):
        wxPyEvent.__init__(self)
        self.SetEventType(wxEVT_HTML_URL_CLICK)
        self.linkinfo = (linkinfo.GetHref(), linkinfo.GetTarget())

class wxUrlClickHtmlWindow(wxHtmlWindow):
    """ HTML window that generates and OnLinkClicked event.

    Use this to avoid having to override HTMLWindow
    """
    def OnLinkClicked(self, linkinfo):
        wxPostEvent(self, wxHtmlWindowUrlClick(linkinfo))

def create(parent):
    return objdictedit(parent)

def usage():
    print "\nUsage of objectdict.py :"
    print "\n   %s [Filepath, ...]\n"%sys.argv[0]

try:
    opts, args = getopt.getopt(sys.argv[1:], "h", ["help"])
except getopt.GetoptError:
    # print help information and exit:
    usage()
    sys.exit(2)

for o, a in opts:
    if o in ("-h", "--help"):
        usage()
        sys.exit()

filesOpen = args

ColSizes = [75, 250, 150, 125, 100, 60, 250]
ColAlignements = [wxALIGN_CENTER, wxALIGN_LEFT, wxALIGN_CENTER, wxALIGN_RIGHT, wxALIGN_CENTER, wxALIGN_CENTER, wxALIGN_LEFT]
AccessList = "Read Only,Write Only,Read/Write"
BoolList = "True,False"
OptionList = "Yes,No"

DictionaryOrganisation = [
    {"minIndex" : 0x0001, "maxIndex" : 0x0FFF, "name" : "Data Type Definitions"},
    {"minIndex" : 0x1000, "maxIndex" : 0x1029, "name" : "Communication Parameters"},
    {"minIndex" : 0x1200, "maxIndex" : 0x12FF, "name" : "SDO Parameters"},
    {"minIndex" : 0x1400, "maxIndex" : 0x15FF, "name" : "Receive PDO Parameters"},
    {"minIndex" : 0x1600, "maxIndex" : 0x17FF, "name" : "Receive PDO Mapping"},
    {"minIndex" : 0x1800, "maxIndex" : 0x19FF, "name" : "Transmit PDO Parameters"},
    {"minIndex" : 0x1A00, "maxIndex" : 0x1BFF, "name" : "Transmit PDO Mapping"},
    {"minIndex" : 0x1C00, "maxIndex" : 0x1FFF, "name" : "Other Communication Parameters"},
    {"minIndex" : 0x2000, "maxIndex" : 0x5FFF, "name" : "Manufacturer Specific"},
    {"minIndex" : 0x6000, "maxIndex" : 0x9FFF, "name" : "Standardized Device Profile"},
    {"minIndex" : 0xA000, "maxIndex" : 0xBFFF, "name" : "Standardized Interface Profile"}]

class SubindexTable(wxPyGridTableBase):
    
    """
    A custom wxGrid Table using user supplied data
    """
    def __init__(self, parent, data, editors, colnames):
        # The base class must be initialized *first*
        wxPyGridTableBase.__init__(self)
        self.data = data
        self.editors = editors
        self.CurrentIndex = 0
        self.colnames = colnames
        self.Parent = parent
        # XXX
        # we need to store the row length and collength to
        # see if the table has changed size
        self._rows = self.GetNumberRows()
        self._cols = self.GetNumberCols()
    
    def GetNumberCols(self):
        return len(self.colnames)
        
    def GetNumberRows(self):
        return len(self.data)

    def GetColLabelValue(self, col):
        if col < len(self.colnames):
            return self.colnames[col]

    def GetRowLabelValues(self, row):
        return row

    def GetValue(self, row, col):
        if row < self.GetNumberRows():
            name = str(self.data[row].get(self.GetColLabelValue(col), ""))
            return name
    
    def GetEditor(self, row, col):
        if row < self.GetNumberRows():
            return self.editors[row].get(self.GetColLabelValue(col), "")
    
    def GetValueByName(self, row, colname):
        return self.data[row].get(colname)

    def SetValue(self, row, col, value):
        if col < len(self.colnames):
            self.data[row][self.GetColLabelValue(col)] = value
        
    def ResetView(self, grid):
        """
        (wxGrid) -> Reset the grid view.   Call this to
        update the grid if rows and columns have been added or deleted
        """
        grid.BeginBatch()
        for current, new, delmsg, addmsg in [
            (self._rows, self.GetNumberRows(), wxGRIDTABLE_NOTIFY_ROWS_DELETED, wxGRIDTABLE_NOTIFY_ROWS_APPENDED),
            (self._cols, self.GetNumberCols(), wxGRIDTABLE_NOTIFY_COLS_DELETED, wxGRIDTABLE_NOTIFY_COLS_APPENDED),
        ]:
            if new < current:
                msg = wxGridTableMessage(self,delmsg,new,current-new)
                grid.ProcessTableMessage(msg)
            elif new > current:
                msg = wxGridTableMessage(self,addmsg,new-current)
                grid.ProcessTableMessage(msg)
                self.UpdateValues(grid)
        grid.EndBatch()

        self._rows = self.GetNumberRows()
        self._cols = self.GetNumberCols()
        # update the column rendering scheme
        self._updateColAttrs(grid)

        # update the scrollbars and the displayed part of the grid
        grid.AdjustScrollbars()
        grid.ForceRefresh()


    def UpdateValues(self, grid):
        """Update all displayed values"""
        # This sends an event to the grid table to update all of the values
        msg = wxGridTableMessage(self, wxGRIDTABLE_REQUEST_VIEW_GET_VALUES)
        grid.ProcessTableMessage(msg)

    def _updateColAttrs(self, grid):
        """
        wxGrid -> update the column attributes to add the
        appropriate renderer given the column name.

        Otherwise default to the default renderer.
        """
        
        for col in range(self.GetNumberCols()):
            attr = wxGridCellAttr()
            attr.SetAlignment(ColAlignements[col], wxALIGN_CENTRE)
            grid.SetColAttr(col, attr)
            grid.SetColSize(col, ColSizes[col])
        
        typelist = None
        accesslist = None
        for row in range(self.GetNumberRows()):
            editors = self.editors[row]
            for col in range(self.GetNumberCols()):
                editor = None
                renderer = None
                
                colname = self.GetColLabelValue(col)
                editortype = editors[colname]
                if editortype:
                    grid.SetReadOnly(row, col, False)
                    if editortype == "string":
                        editor = wxGridCellTextEditor()
                        renderer = wxGridCellStringRenderer()
                        if colname == "value" and "length" in editors:
                            editor.SetParameters(editors["length"]) 
                    elif editortype == "number":
                        editor = wxGridCellNumberEditor()
                        renderer = wxGridCellNumberRenderer()
                        if colname == "value" and "min" in editors and "max" in editors:
                            editor.SetParameters("%s,%s"%(editors["min"],editors["max"]))
                    elif editortype == "real":
                        editor = wxGridCellFloatEditor()
                        renderer = wxGridCellFloatRenderer()
                        if colname == "value" and "min" in editors and "max" in editors:
                            editor.SetParameters("%s,%s"%(editors["min"],editors["max"]))
                    elif editortype == "bool":
                        editor = wxGridCellChoiceEditor()
                        editor.SetParameters(BoolList)
                    elif editortype == "access":
                        editor = wxGridCellChoiceEditor()
                        editor.SetParameters(AccessList)
                    elif editortype == "option":
                        editor = wxGridCellChoiceEditor()
                        editor.SetParameters(OptionList)
                    elif editortype == "type":
                        editor = wxGridCellChoiceEditor()
                        editor.SetParameters(self.Parent.Manager.GetCurrentTypeList())
                    elif editortype == "map":
                        editor = wxGridCellChoiceEditor()
                        editor.SetParameters(self.Parent.Manager.GetCurrentMapList())
                else:
                    grid.SetReadOnly(row, col, True)
                    
                grid.SetCellEditor(row, col, editor)
                grid.SetCellRenderer(row, col, renderer)
                
                grid.SetCellBackgroundColour(row, col, wxWHITE)
    
    def SetData(self, data):
        self.data = data
        
    def SetEditors(self, editors):
        self.editors = editors
    
    def GetCurrentIndex(self):
        return self.CurrentIndex
    
    def SetCurrentIndex(self, index):
        self.CurrentIndex = index
    
    def AppendRow(self, row_content):
        self.data.append(row_content)

    def Empty(self):
        self.data = []
        self.editors = []

[wxID_EDITINGPANEL, wxID_EDITINGPANELADDBUTTON, wxID_EDITINGPANELINDEXCHOICE, 
 wxID_EDITINGPANELINDEXLIST, wxID_EDITINGPANELINDEXLISTPANEL, wxID_EDITINGPANELPARTLIST, 
 wxID_EDITINGPANELSECONDSPLITTER, wxID_EDITINGPANELSUBINDEXGRID,
 wxID_EDITINGPANELSUBINDEXGRIDPANEL, wxID_EDITINGPANELCALLBACKCHECK,
] = [wx.NewId() for _init_ctrls in range(10)]

[wxID_EDITINGPANELINDEXLISTMENUITEMS0, wxID_EDITINGPANELINDEXLISTMENUITEMS1, 
 wxID_EDITINGPANELINDEXLISTMENUITEMS2, 
] = [wx.NewId() for _init_coll_IndexListMenu_Items in range(3)]

[wxID_EDITINGPANELMENU1ITEMS0, wxID_EDITINGPANELMENU1ITEMS1, 
] = [wx.NewId() for _init_coll_SubindexGridMenu_Items in range(2)]

class EditingPanel(wx.SplitterWindow):
    def _init_coll_AddToListSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.AddButton, 0, border=0, flag=0)
        parent.AddWindow(self.IndexChoice, 0, border=0, flag=wxGROW)

    def _init_coll_SubindexGridSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.CallbackCheck, 0, border=0, flag=0)
        parent.AddWindow(self.SubindexGrid, 0, border=0, flag=wxGROW)

    def _init_coll_IndexListSizer_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.IndexList, 0, border=0, flag=wxGROW)
        parent.AddSizer(self.AddToListSizer, 0, border=0, flag=wxGROW)

    def _init_coll_AddToListSizer_Growables(self, parent):
        # generated method, don't edit

        parent.AddGrowableCol(1)

    def _init_coll_SubindexGridSizer_Growables(self, parent):
        # generated method, don't edit

        parent.AddGrowableCol(0)
        parent.AddGrowableRow(1)

    def _init_coll_IndexListSizer_Growables(self, parent):
        # generated method, don't edit

        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_SubindexGridMenu_Items(self, parent):
        # generated method, don't edit

        parent.Append(help='', id=wxID_EDITINGPANELMENU1ITEMS0,
              kind=wx.ITEM_NORMAL, text='Add')
        parent.Append(help='', id=wxID_EDITINGPANELMENU1ITEMS1,
              kind=wx.ITEM_NORMAL, text='Delete')
        self.Bind(wx.EVT_MENU, self.OnAddSubindexMenu,
              id=wxID_EDITINGPANELMENU1ITEMS0)
        self.Bind(wx.EVT_MENU, self.OnDeleteSubindexMenu,
              id=wxID_EDITINGPANELMENU1ITEMS1)

    def _init_coll_IndexListMenu_Items(self, parent):
        # generated method, don't edit

        parent.Append(help='', id=wxID_EDITINGPANELINDEXLISTMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='Rename')
        parent.Append(help='', id=wxID_EDITINGPANELINDEXLISTMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='Modify')
        parent.Append(help='', id=wxID_EDITINGPANELINDEXLISTMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='Delete')
        self.Bind(wx.EVT_MENU, self.OnRenameIndexMenu,
              id=wxID_EDITINGPANELINDEXLISTMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnDeleteIndexMenu,
              id=wxID_EDITINGPANELINDEXLISTMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnModifyIndexMenu,
              id=wxID_EDITINGPANELINDEXLISTMENUITEMS2)

    def _init_utils(self):
        # generated method, don't edit
        self.IndexListMenu = wx.Menu(title='')

        self.SubindexGridMenu = wx.Menu(title='')

        self._init_coll_IndexListMenu_Items(self.IndexListMenu)
        self._init_coll_SubindexGridMenu_Items(self.SubindexGridMenu)

    def _init_sizers(self):
        # generated method, don't edit
        self.IndexListSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self.SubindexGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self.AddToListSizer = wx.FlexGridSizer(cols=2, hgap=0, rows=1, vgap=0)

        self._init_coll_IndexListSizer_Growables(self.IndexListSizer)
        self._init_coll_IndexListSizer_Items(self.IndexListSizer)
        self._init_coll_SubindexGridSizer_Growables(self.SubindexGridSizer)
        self._init_coll_SubindexGridSizer_Items(self.SubindexGridSizer)
        self._init_coll_AddToListSizer_Growables(self.AddToListSizer)
        self._init_coll_AddToListSizer_Items(self.AddToListSizer)

        self.SubindexGridPanel.SetSizer(self.SubindexGridSizer)
        self.IndexListPanel.SetSizer(self.IndexListSizer)
        
    def _init_ctrls(self, prnt):
        wx.SplitterWindow.__init__(self, id=wxID_EDITINGPANEL,
              name='MainSplitter', parent=prnt, point=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=wx.SP_3D)
        self._init_utils()
        self.SetNeedUpdating(True)
        self.SetMinimumPaneSize(1)

        self.PartList = wx.ListBox(choices=[], id=wxID_EDITINGPANELPARTLIST,
              name='PartList', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=0)
        self.PartList.Bind(wx.EVT_LISTBOX, self.OnPartListBoxClick,
              id=wxID_EDITINGPANELPARTLIST)

        self.SecondSplitter = wx.SplitterWindow(id=wxID_EDITINGPANELSECONDSPLITTER,
              name='SecondSplitter', parent=self, point=wx.Point(0,
              0), size=wx.Size(-1, -1), style=wx.SP_3D)
        self.SecondSplitter.SetMinimumPaneSize(1)
        self.SplitHorizontally(self.PartList, self.SecondSplitter,
              110)

        self.SubindexGridPanel = wx.Panel(id=wxID_EDITINGPANELSUBINDEXGRIDPANEL,
              name='SubindexGridPanel', parent=self.SecondSplitter, pos=wx.Point(0,
              0), size=wx.Size(-1, -1), style=wx.TAB_TRAVERSAL)

        self.IndexListPanel = wx.Panel(id=wxID_EDITINGPANELINDEXLISTPANEL,
              name='IndexListPanel', parent=self.SecondSplitter, pos=wx.Point(0,
              0), size=wx.Size(-1, -1), style=wx.TAB_TRAVERSAL)
        self.SecondSplitter.SplitVertically(self.IndexListPanel,
              self.SubindexGridPanel, 280)

        self.SubindexGrid = wx.grid.Grid(id=wxID_EDITINGPANELSUBINDEXGRID,
              name='SubindexGrid', parent=self.SubindexGridPanel, pos=wx.Point(0,
              0), size=wx.Size(-1, -1), style=0)
        self.SubindexGrid.SetFont(wx.Font(12, 77, wx.NORMAL, wx.NORMAL, False,
              'Sans'))
        self.SubindexGrid.SetLabelFont(wx.Font(10, 77, wx.NORMAL, wx.NORMAL,
              False, 'Sans'))
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_CELL_CHANGE,
              self.OnSubindexGridCellChange)
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_CELL_RIGHT_CLICK,
              self.OnSubindexGridRightClick)
        self.SubindexGrid.Bind(wx.grid.EVT_GRID_SELECT_CELL,
              self.OnSubindexGridSelectCell)

        self.CallbackCheck = wx.CheckBox(id=wxID_EDITINGPANELCALLBACKCHECK,
              label='Have Callbacks', name='CallbackCheck',
              parent=self.SubindexGridPanel, pos=wx.Point(0, 0), size=wx.Size(152,
              24), style=0)
        self.CallbackCheck.Bind(wx.EVT_CHECKBOX, self.OnCallbackCheck,
              id=wxID_EDITINGPANELCALLBACKCHECK)

        self.IndexList = wx.ListBox(choices=[], id=wxID_EDITINGPANELINDEXLIST,
              name='IndexList', parent=self.IndexListPanel, pos=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=0)
        self.IndexList.Bind(wx.EVT_LISTBOX, self.OnIndexListClick,
              id=wxID_EDITINGPANELINDEXLIST)
        self.IndexList.Bind(wx.EVT_RIGHT_UP, self.OnIndexListRightUp)

        self.AddButton = wx.Button(id=wxID_EDITINGPANELADDBUTTON, label='Add',
              name='AddButton', parent=self.IndexListPanel, pos=wx.Point(0, 0),
              size=wx.Size(50, 30), style=0)
        self.AddButton.Bind(wx.EVT_BUTTON, self.OnAddButtonClick,
              id=wxID_EDITINGPANELADDBUTTON)

        self.IndexChoice = wx.Choice(choices=[], id=wxID_EDITINGPANELINDEXCHOICE,
              name='IndexChoice', parent=self.IndexListPanel, pos=wx.Point(50,
              0), size=wx.Size(-1, 30), style=0)

        self._init_sizers()

    def __init__(self, parent, manager):
        self._init_ctrls(parent.GetNoteBook())
        self.Parent = parent
        self.Manager = manager
        self.ListIndex = []
        self.ChoiceIndex = []
        self.FirstCall = False
        
        for values in DictionaryOrganisation:
            text = "   0x%04X-0x%04X      %s"%(values["minIndex"],values["maxIndex"],values["name"])
            self.PartList.Append(text)
        self.Table = SubindexTable(self, [], [], ["subindex", "name", "type", "value", "access", "save", "comment"])
        self.SubindexGrid.SetTable(self.Table)
        self.SubindexGrid.SetRowLabelSize(0)
        self.CallbackCheck.Disable()
        self.Table.ResetView(self.SubindexGrid)

    def GetSelection(self):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            subIndex = self.SubindexGrid.GetGridCursorRow()
            return index, subIndex
        return None

    def OnAddButtonClick(self, event):
        selected = self.IndexChoice.GetStringSelection()
        if selected != "":
            if selected == "User Type":
                self.Parent.AddUserType()
            elif selected == "SDO Server":
                self.Manager.AddSDOServerToCurrent()
            elif selected == "SDO Client":
                self.Manager.AddSDOClientToCurrent()
            elif selected == "PDO Receive":
                self.Manager.AddPDOReceiveToCurrent()
            elif selected == "PDO Transmit":
                self.Manager.AddPDOTransmitToCurrent()
            elif selected == "Map Variable":
                self.Parent.AddMapVariable()
            elif selected in [menu for menu, indexes in self.Manager.GetCurrentSpecificMenu()]:
                self.Manager.AddSpecificEntryToCurrent(selected)
            else:
                index = self.ChoiceIndex[self.IndexChoice.GetSelection()]
                self.Manager.ManageEntriesOfCurrent([index], [])
            self.Parent.RefreshBufferState()
            self.RefreshIndexList()
        event.Skip()

    def OnPartListBoxClick(self, event):
        self.SubindexGrid.SetGridCursor(0, 0)
        self.RefreshIndexList()
        event.Skip()

    def OnIndexListClick(self, event):
        self.SubindexGrid.SetGridCursor(0, 0)
        self.RefreshTable()
        event.Skip()

    def OnSubindexGridSelectCell(self, event):
        wxCallAfter(self.Parent.RefreshStatusBar)
        event.Skip()

#-------------------------------------------------------------------------------
#                             Refresh Functions
#-------------------------------------------------------------------------------

    def RefreshIndexList(self):
        selected = self.IndexList.GetSelection()
        choice = self.IndexChoice.GetStringSelection()
        choiceindex = self.IndexChoice.GetSelection()
        if selected != wxNOT_FOUND:
            selectedindex = self.ListIndex[selected]
        self.IndexList.Clear()
        self.IndexChoice.Clear()
        i = self.PartList.GetSelection()
        if i < len(DictionaryOrganisation):
            values = DictionaryOrganisation[i]
            self.ListIndex = []
            for name, index in self.Manager.GetCurrentValidIndexes(values["minIndex"], values["maxIndex"]):
                self.IndexList.Append("0x%04X   %s"%(index, name))
                self.ListIndex.append(index)
            self.ChoiceIndex = []
            if i == 0:
                self.IndexChoice.Append("User Type")
                self.IndexChoice.SetStringSelection("User Type")
            elif i == 2:
                self.IndexChoice.Append("SDO Server")
                self.IndexChoice.Append("SDO Client")
                if choiceindex != wxNOT_FOUND and choice == self.IndexChoice.GetString(choiceindex):
                     self.IndexChoice.SetStringSelection(choice)
            elif i in (3, 4):
                self.IndexChoice.Append("PDO Receive")
                self.IndexChoice.SetStringSelection("PDO Receive")
            elif i in (5, 6):
                self.IndexChoice.Append("PDO Transmit")
                self.IndexChoice.SetStringSelection("PDO Transmit")
            elif i == 8:
                self.IndexChoice.Append("Map Variable")
                self.IndexChoice.SetStringSelection("Map Variable")
            else:
                for name, index in self.Manager.GetCurrentValidChoices(values["minIndex"], values["maxIndex"]):
                    if index:
                        self.IndexChoice.Append("0x%04X   %s"%(index, name))
                    else:
                        self.IndexChoice.Append(name)
                    self.ChoiceIndex.append(index)
            if choiceindex != wxNOT_FOUND and choice == self.IndexChoice.GetString(choiceindex):
                self.IndexChoice.SetStringSelection(choice)
        self.IndexChoice.Enable(self.IndexChoice.GetCount() != 0)
        self.AddButton.Enable(self.IndexChoice.GetCount() != 0)
        if selected == wxNOT_FOUND or selected >= len(self.ListIndex) or selectedindex != self.ListIndex[selected]:
            self.Table.Empty()
            self.CallbackCheck.SetValue(False)
            self.CallbackCheck.Disable()
            self.Table.ResetView(self.SubindexGrid)
            self.Parent.RefreshStatusBar()
        else:
            self.IndexList.SetSelection(selected)
            self.RefreshTable()

    def RefreshTable(self):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if index > 0x260:
                self.CallbackCheck.Enable()
                self.CallbackCheck.SetValue(self.Manager.HasCurrentEntryCallbacks(index))
            result = self.Manager.GetCurrentEntryValues(index)
            if result != None:
                self.Table.SetCurrentIndex(index)
                data, editors = result
                self.Table.SetData(data)
                self.Table.SetEditors(editors)
                self.Table.ResetView(self.SubindexGrid)
        self.Parent.RefreshStatusBar()

#-------------------------------------------------------------------------------
#                        Editing Table value function
#-------------------------------------------------------------------------------

    def OnSubindexGridCellChange(self, event):
        index = self.Table.GetCurrentIndex()
        subIndex = event.GetRow()
        col = event.GetCol()
        name = self.Table.GetColLabelValue(col)
        value = self.Table.GetValue(subIndex, col)
        editor = self.Table.GetEditor(subIndex, col)
        self.Manager.SetCurrentEntry(index, subIndex, value, name, editor)
        self.Parent.RefreshBufferState()
        wxCallAfter(self.RefreshTable)
        event.Skip()

    def OnCallbackCheck(self, event):
        index = self.Table.GetCurrentIndex()
        self.Manager.SetCurrentEntryCallbacks(index, self.CallbackCheck.GetValue())
        self.Parent.RefreshBufferState()
        wxCallAfter(self.RefreshTable)
        event.Skip()

#-------------------------------------------------------------------------------
#                          Contextual Menu functions
#-------------------------------------------------------------------------------

    def OnIndexListRightUp(self, event):
        if not self.FirstCall:
            self.FirstCall = True
            selected = self.IndexList.GetSelection()
            if selected != wxNOT_FOUND:
                index = self.ListIndex[selected]
                if index < 0x260:
                    self.IndexListMenu.FindItemByPosition(0).Enable(False)
                    self.IndexListMenu.FindItemByPosition(1).Enable(True)
                    self.PopupMenu(self.IndexListMenu)
                elif 0x1000 <= index <= 0x1BFF:
                    self.IndexListMenu.FindItemByPosition(0).Enable(False)
                    self.IndexListMenu.FindItemByPosition(1).Enable(False)
                    self.PopupMenu(self.IndexListMenu)
                elif 0x2000 <= index <= 0x5FFF:
                    self.IndexListMenu.FindItemByPosition(0).Enable(True)
                    self.IndexListMenu.FindItemByPosition(1).Enable(False)
                    self.PopupMenu(self.IndexListMenu)
                elif index >= 0x6000:
                    self.IndexListMenu.FindItemByPosition(0).Enable(False)
                    self.IndexListMenu.FindItemByPosition(1).Enable(False)
                    self.PopupMenu(self.IndexListMenu)
        else:
            self.FirstCall = False
        event.Skip()

    def OnSubindexGridRightClick(self, event):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if self.Manager.IsCurrentEntry(index):
                infos = self.Manager.GetEntryInfos(index)
                if index >= 0x2000 and infos["struct"] & OD_MultipleSubindexes or infos["struct"] & OD_IdenticalSubindexes:
                    self.PopupMenu(self.SubindexGridMenu)
        event.Skip()

    def OnRenameIndexMenu(self, event):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if self.Manager.IsCurrentEntry(index):
                infos = self.Manager.GetEntryInfos(index)
                dialog = wxTextEntryDialog(self, "Give a new name for index 0x%04X"%index,
                             "Rename an index", infos["name"], wxOK|wxCANCEL)
                if dialog.ShowModal() == wxID_OK:
                    self.Manager.SetCurrentEntryName(index, dialog.GetValue())
                    self.Parent.RefreshBufferState()
                    self.RefreshIndexList()
                dialog.Destroy()
        event.Skip()

    def OnModifyIndexMenu(self, event):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if self.Manager.IsCurrentEntry(index) and index < 0x260:
                values, valuetype = self.Manager.GetCustomisedTypeValues(index)
                dialog = UserTypeDialog(self)
                dialog.SetTypeList(self.Manager.GetCustomisableTypes(), values[1])
                if valuetype == 0:
                    dialog.SetValues(min = values[2], max = values[3])
                elif valuetype == 1:
                    dialog.SetValues(length = values[2])
                if dialog.ShowModal() == wxID_OK:
                    type, min, max, length = dialog.GetValues()
                    self.Manager.SetCurrentUserType(index, type, min, max, length)
                    self.Parent.RefreshBufferState()
                    self.RefreshIndexList()
        event.Skip()
        
    def OnDeleteIndexMenu(self, event):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if self.Manager.IsCurrentEntry(index):
                self.Manager.ManageEntriesOfCurrent([],[index])
                self.Parent.RefreshBufferState()
                self.RefreshIndexList()
        event.Skip()

    def OnAddSubindexMenu(self, event):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if self.Manager.IsCurrentEntry(index):
                dialog = wxTextEntryDialog(self, "Number of subindexes to add:",
                             "Add subindexes", "1", wxOK|wxCANCEL)
                if dialog.ShowModal() == wxID_OK:
                    number = eval(dialog.GetValue())
                    if type(number) == IntType:
                        self.Manager.AddSubentriesToCurrent(index, number)
                        self.Parent.RefreshBufferState()
                        self.RefreshIndexList()
                    else:
                        message = wxMessageDialog(self, "An integer is required!", "ERROR", wxOK|wxICON_ERROR)
                        message.ShowModal()
                        message.Destroy()
                dialog.Destroy()
        event.Skip()

    def OnDeleteSubindexMenu(self, event):
        selected = self.IndexList.GetSelection()
        if selected != wxNOT_FOUND:
            index = self.ListIndex[selected]
            if self.Manager.IsCurrentEntry(index):
                dialog = wxTextEntryDialog(self, "Number of subindexes to delete:",
                             "Delete subindexes", "1", wxOK|wxCANCEL)
                if dialog.ShowModal() == wxID_OK:
                    number = eval(dialog.GetValue())
                    if type(number) == IntType:
                        self.Manager.RemoveSubentriesFromCurrent(index, number)
                        self.Parent.RefreshBufferState()
                        self.RefreshIndexList()
                    else:
                        message = wxMessageDialog(self, "An integer is required!", "ERROR", wxOK|wxICON_ERROR)
                        message.ShowModal()
                        message.Destroy()
                dialog.Destroy()
        event.Skip()

[wxID_OBJDICTEDIT, wxID_OBJDICTEDITFILEOPENED, 
 wxID_OBJDICTEDITHELPBAR,
] = [wx.NewId() for _init_ctrls in range(3)]

[wxID_OBJDICTEDITADDMENUITEMS0, wxID_OBJDICTEDITADDMENUITEMS1, 
 wxID_OBJDICTEDITADDMENUITEMS2, wxID_OBJDICTEDITADDMENUITEMS3, 
 wxID_OBJDICTEDITADDMENUITEMS4, wxID_OBJDICTEDITADDMENUITEMS5, 
] = [wx.NewId() for _init_coll_AddMenu_Items in range(6)]

[wxID_OBJDICTEDITFILEMENUITEMS0, wxID_OBJDICTEDITFILEMENUITEMS1, 
 wxID_OBJDICTEDITFILEMENUITEMS2, wxID_OBJDICTEDITFILEMENUITEMS4, 
 wxID_OBJDICTEDITFILEMENUITEMS5, wxID_OBJDICTEDITFILEMENUITEMS6, 
 wxID_OBJDICTEDITFILEMENUITEMS7, wxID_OBJDICTEDITFILEMENUITEMS8,
] = [wx.NewId() for _init_coll_FileMenu_Items in range(8)]

[wxID_OBJDICTEDITEDITMENUITEMS0, wxID_OBJDICTEDITEDITMENUITEMS1, 
 wxID_OBJDICTEDITEDITMENUITEMS2, wxID_OBJDICTEDITEDITMENUITEMS4, 
 wxID_OBJDICTEDITEDITMENUITEMS6, wxID_OBJDICTEDITEDITMENUITEMS7, 
 wxID_OBJDICTEDITEDITMENUITEMS8, 
] = [wx.NewId() for _init_coll_EditMenu_Items in range(7)]

[wxID_OBJDICTEDITHELPMENUITEMS0, wxID_OBJDICTEDITHELPMENUITEMS1,
 wxID_OBJDICTEDITHELPMENUITEMS2,
] = [wx.NewId() for _init_coll_HelpMenu_Items in range(3)]

class objdictedit(wx.Frame):
    def _init_coll_menuBar1_Menus(self, parent):
        # generated method, don't edit

        parent.Append(menu=self.FileMenu, title='File')
        parent.Append(menu=self.EditMenu, title='Edit')
        parent.Append(menu=self.AddMenu, title='Add')
        parent.Append(menu=self.HelpMenu, title='Help')

    def _init_coll_EditMenu_Items(self, parent):
        # generated method, don't edit

        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS4,
              kind=wx.ITEM_NORMAL, text='Refresh\tCTRL+R')
        parent.AppendSeparator()
        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='Undo\tCTRL+Z')
        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='Redo\tCTRL+Y')
        parent.AppendSeparator()
        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS6,
              kind=wx.ITEM_NORMAL, text='Node infos')
        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='DS-301 Profile')
        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS8,
              kind=wx.ITEM_NORMAL, text='DS-302 Profile')
        parent.Append(help='', id=wxID_OBJDICTEDITEDITMENUITEMS7,
              kind=wx.ITEM_NORMAL, text='Other Profile')
        self.Bind(wx.EVT_MENU, self.OnUndoMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnRedoMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnCommunicationMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS2)
        self.Bind(wx.EVT_MENU, self.OnRefreshMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS4)
        self.Bind(wx.EVT_MENU, self.OnNodeInfosMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS6)
        self.Bind(wx.EVT_MENU, self.OnEditProfileMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS7)
        self.Bind(wx.EVT_MENU, self.OnOtherCommunicationMenu,
              id=wxID_OBJDICTEDITEDITMENUITEMS8)

    def _init_coll_HelpMenu_Items(self, parent):
        # generated method, don't edit

        parent.Append(help='', id=wxID_OBJDICTEDITHELPMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='DS-301 Standard\tF1')
        parent.Append(help='', id=wxID_OBJDICTEDITHELPMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='CAN Festival Docs\tF2')
        parent.Append(help='', id=wxID_OBJDICTEDITHELPMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='About')
        self.Bind(wx.EVT_MENU, self.OnHelpDS301Menu,
              id=wxID_OBJDICTEDITHELPMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnHelpCANFestivalMenu,
              id=wxID_OBJDICTEDITHELPMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnAboutMenu,
              id=wxID_OBJDICTEDITHELPMENUITEMS2)

    def _init_coll_FileMenu_Items(self, parent):
        # generated method, don't edit

        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS5,
              kind=wx.ITEM_NORMAL, text='New\tCTRL+N')
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='Open\tCTRL+O')
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='Save\tCTRL+S')
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS6,
              kind=wx.ITEM_NORMAL, text='Save As...\tALT+S')
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='Close\tCTRL+W')
        parent.AppendSeparator()
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS7,
              kind=wx.ITEM_NORMAL, text='Import XML file')
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS8,
              kind=wx.ITEM_NORMAL, text='Build Dictionary\tCTRL+B')
        parent.AppendSeparator()
        parent.Append(help='', id=wxID_OBJDICTEDITFILEMENUITEMS4,
              kind=wx.ITEM_NORMAL, text='Exit')
        self.Bind(wx.EVT_MENU, self.OnOpenMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnSaveMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnCloseMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS2)
        self.Bind(wx.EVT_MENU, self.OnQuitMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS4)
        self.Bind(wx.EVT_MENU, self.OnNewMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS5)
        self.Bind(wx.EVT_MENU, self.OnSaveAsMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS6)
        self.Bind(wx.EVT_MENU, self.OnImportMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS7)
        self.Bind(wx.EVT_MENU, self.OnExportMenu,
              id=wxID_OBJDICTEDITFILEMENUITEMS8)

    def _init_coll_AddMenu_Items(self, parent):
        # generated method, don't edit

        parent.Append(help='', id=wxID_OBJDICTEDITADDMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='SDO Server')
        parent.Append(help='', id=wxID_OBJDICTEDITADDMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='SDO Client')
        parent.Append(help='', id=wxID_OBJDICTEDITADDMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='PDO Transmit')
        parent.Append(help='', id=wxID_OBJDICTEDITADDMENUITEMS3,
              kind=wx.ITEM_NORMAL, text='PDO Receive')
        parent.Append(help='', id=wxID_OBJDICTEDITADDMENUITEMS4,
              kind=wx.ITEM_NORMAL, text='Map Variable')
        parent.Append(help='', id=wxID_OBJDICTEDITADDMENUITEMS5,
              kind=wx.ITEM_NORMAL, text='User Type')
        self.Bind(wx.EVT_MENU, self.OnAddSDOServerMenu,
              id=wxID_OBJDICTEDITADDMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnAddSDOClientMenu,
              id=wxID_OBJDICTEDITADDMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnAddPDOTransmitMenu,
              id=wxID_OBJDICTEDITADDMENUITEMS2)
        self.Bind(wx.EVT_MENU, self.OnAddPDOReceiveMenu,
              id=wxID_OBJDICTEDITADDMENUITEMS3)
        self.Bind(wx.EVT_MENU, self.OnAddMapVariableMenu,
              id=wxID_OBJDICTEDITADDMENUITEMS4)
        self.Bind(wx.EVT_MENU, self.OnAddUserTypeMenu,
              id=wxID_OBJDICTEDITADDMENUITEMS5)

    def _init_coll_HelpBar_Fields(self, parent):
        # generated method, don't edit
        parent.SetFieldsCount(3)

        parent.SetStatusText(number=0, text='')
        parent.SetStatusText(number=1, text='')
        parent.SetStatusText(number=2, text='')

        parent.SetStatusWidths([100, 110, -1])

    def _init_utils(self):
        # generated method, don't edit
        self.menuBar1 = wx.MenuBar()
        self.menuBar1.SetEvtHandlerEnabled(True)

        self.FileMenu = wx.Menu(title='')

        self.EditMenu = wx.Menu(title='')

        self.AddMenu = wx.Menu(title='')

        self.HelpMenu = wx.Menu(title='')

        self._init_coll_menuBar1_Menus(self.menuBar1)
        self._init_coll_FileMenu_Items(self.FileMenu)
        self._init_coll_EditMenu_Items(self.EditMenu)
        self._init_coll_AddMenu_Items(self.AddMenu)
        self._init_coll_HelpMenu_Items(self.HelpMenu)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_OBJDICTEDIT, name='objdictedit',
              parent=prnt, pos=wx.Point(149, 178), size=wx.Size(1000, 700),
              style=wx.DEFAULT_FRAME_STYLE, title='Objdictedit')
        self._init_utils()
        self.SetClientSize(wx.Size(1000, 700))
        self.SetMenuBar(self.menuBar1)
        self.Bind(wx.EVT_CLOSE, self.OnCloseFrame, id=wxID_OBJDICTEDIT)

        self.FileOpened = wx.Notebook(id=wxID_OBJDICTEDITFILEOPENED,
              name='FileOpened', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=0)
        self.FileOpened.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED,
              self.OnFileSelectedChanged, id=wxID_OBJDICTEDITFILEOPENED)

        self.HelpBar = wx.StatusBar(id=wxID_OBJDICTEDITHELPBAR, name='HelpBar',
              parent=self, style=wxST_SIZEGRIP)
        self._init_coll_HelpBar_Fields(self.HelpBar)
        self.SetStatusBar(self.HelpBar)

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.HtmlFrameOpened = []
        
        self.Manager = NodeManager()
        for filepath in filesOpen:
            self.Manager.OpenFileInCurrent(filepath)
            new_editingpanel = EditingPanel(self, self.Manager)
            self.FileOpened.AddPage(new_editingpanel, "")
            self.FileOpened.SetSelection(self.Manager.GetCurrentNodeIndex())
        if self.Manager.CurrentDS302Defined(): 
            self.EditMenu.Enable(wxID_OBJDICTEDITEDITMENUITEMS8, True)
        else:
            self.EditMenu.Enable(wxID_OBJDICTEDITEDITMENUITEMS8, False)
        self.RefreshEditMenu()
        self.RefreshBufferState()
        self.RefreshProfileMenu()
        self.RefreshMainMenu()

        self.RefreshBufferState()
        self.RefreshTitle()
        self.RefreshMainMenu()

    def GetNoteBook(self):
        return self.FileOpened

    def OnAddSDOServerMenu(self, event):
        self.Manager.AddSDOServerToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        event.Skip()
    
    def OnAddSDOClientMenu(self, event):
        self.Manager.AddSDOClientToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        event.Skip()

    def OnAddPDOTransmitMenu(self, event):
        self.Manager.AddPDOTransmitToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        event.Skip()

    def OnAddPDOReceiveMenu(self, event):
        self.Manager.AddPDOReceiveToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        event.Skip()

    def OnAddMapVariableMenu(self, event):
        self.AddMapVariable()
        event.Skip()

    def OnAddUserTypeMenu(self, event):
        self.AddUserType()
        event.Skip()

    def OnFileSelectedChanged(self, event):
        selected = event.GetSelection()
        # At init selected = -1
        print selected
        if selected >= 0:
        	self.Manager.ChangeCurrentNode(selected)
        	self.RefreshBufferState()
        	self.RefreshProfileMenu()
        event.Skip()

    def OnHelpDS301Menu(self, event):
        selected = self.FileOpened.GetSelection()
        if selected >= 0:
            window = self.FileOpened.GetPage(selected)
            result = window.GetSelection()
            if result:
                index, subIndex = result
                result = OpenPDFDocIndex(index)
                if type(result) == StringType:
                    message = wxMessageDialog(self, result, "ERROR", wxOK|wxICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
        event.Skip()
        
    def OnHelpCANFestivalMenu(self, event):
        self.OpenHtmlFrame("CAN Festival Reference", "../doc/canfestival.html", wx.Size(1000, 600))
        event.Skip()

    def OnAboutMenu(self, event):
        self.OpenHtmlFrame("About CAN Festival", "../doc/about.html", wx.Size(500, 450))
        event.Skip()

    def OpenHtmlFrame(self, title, file, size):
        if title not in self.HtmlFrameOpened:
            self.HtmlFrameOpened.append(title)
            window = HtmlFrame(self, self.HtmlFrameOpened)
            window.SetTitle(title)
            window.SetHtmlPage(file)
            window.SetClientSize(size)
            window.Show()

    def OnQuitMenu(self, event):
        self.Close()
        event.Skip()
    
    def OnCloseFrame(self, event):
        if self.Manager.OneFileHasChanged():
            dialog = wxMessageDialog(self, "There are changes, do you want to save?",  "Close Application", wxYES_NO|wxCANCEL|wxICON_QUESTION)
            answer = dialog.ShowModal()
            dialog.Destroy()
            if answer == wxID_YES:
                self.Manager.ChangeCurrentNode(0)
                for i in xrange(self.FileOpened.GetPageCount()):
                    if self.Manager.CurrentIsSaved():
                        self.Manager.CloseCurrent()
                    else:
                        self.Save()
                        self.Manager.CloseCurrent(True)
                event.Skip()
            elif answer == wxID_NO:
                for i in xrange(self.FileOpened.GetPageCount()):
                    self.Manager.CloseCurrent(True)
                wxCallAfter(self.Close)
                event.Skip()
        else:
            event.Skip()

#-------------------------------------------------------------------------------
#                             Refresh Functions
#-------------------------------------------------------------------------------

    def RefreshTitle(self):
        if self.FileOpened.GetPageCount() > 0:
            self.SetTitle("Objdictedit - %s"%self.Manager.GetCurrentFilename())
        else:
            self.SetTitle("Objdictedit")

    def OnRefreshMenu(self, event):
        self.RefreshCurrentIndexList()
        event.Skip()

    def RefreshCurrentIndexList(self):
        selected = self.FileOpened.GetSelection()
        window = self.FileOpened.GetPage(selected)
        window.RefreshIndexList()

    def RefreshStatusBar(self):
        window = self.FileOpened.GetPage(self.FileOpened.GetSelection())
        selection = window.GetSelection()
        if selection:
            index, subIndex = selection
            if self.Manager.IsCurrentEntry(index):
                self.HelpBar.SetStatusText("Index: 0x%04X"%index, 0)
                self.HelpBar.SetStatusText("Subindex: 0x%02X"%subIndex, 1)
                entryinfos = self.Manager.GetEntryInfos(index)
                name = entryinfos["name"]
                category = "Optional"
                if entryinfos["need"]:
                    category = "Mandatory"
                struct = "VAR"
                number = ""
                if entryinfos["struct"] & OD_IdenticalIndexes:
                    number = " possibly defined %d times"%entryinfos["nbmax"]
                if entryinfos["struct"] & OD_IdenticalSubindexes:
                    struct = "REC"
                elif entryinfos["struct"] & OD_MultipleSubindexes:
                    struct = "ARRAY"
                text = "%s: %s entry of struct %s%s."%(name,category,struct,number)
                self.HelpBar.SetStatusText(text, 2)
            else:
                for i in xrange(3):
                    self.HelpBar.SetStatusText("", i)
        else:
            for i in xrange(3):
                self.HelpBar.SetStatusText("", i)

    def RefreshMainMenu(self):
        if self.FileOpened.GetPageCount() > 0:
            self.menuBar1.EnableTop(1, True)
            self.menuBar1.EnableTop(2, True)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS1, True)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS2, True)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS6, True)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS8, True)
        else:
            self.menuBar1.EnableTop(1, False)      
            self.menuBar1.EnableTop(2, False)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS1, False)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS2, False)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS6, False)
            self.FileMenu.Enable(wxID_OBJDICTEDITFILEMENUITEMS8, False)

    def RefreshEditMenu(self):
        if self.FileOpened.GetPageCount() > 0:
            undo, redo = self.Manager.GetCurrentBufferState()
            self.EditMenu.FindItemByPosition(2).Enable(undo)
            self.EditMenu.FindItemByPosition(3).Enable(redo)
        else:
            self.EditMenu.FindItemByPosition(2).Enable(False)
            self.EditMenu.FindItemByPosition(3).Enable(False)

    def RefreshProfileMenu(self):
        profile = self.Manager.GetCurrentProfileName()
        edititem = self.EditMenu.FindItemByPosition(8)
        length = self.AddMenu.GetMenuItemCount()
        for i in xrange(length-6):
            additem = self.AddMenu.FindItemByPosition(6)
            self.AddMenu.Delete(additem.GetId())
        if profile not in ("None", "DS-301"):
            edititem.SetText("%s Profile"%profile)
            edititem.Enable(True)
            self.AddMenu.AppendSeparator()
            for text, indexes in self.Manager.GetCurrentSpecificMenu():
                new_id = wx.NewId()
                self.AddMenu.Append(help='', id=new_id, kind=wx.ITEM_NORMAL, text=text)
                self.Bind(wx.EVT_MENU, self.GetProfileCallBack(text), id=new_id)
        else:
            edititem.SetText("Other Profile")
            edititem.Enable(False)
        

#-------------------------------------------------------------------------------
#                            Buffer Functions
#-------------------------------------------------------------------------------

    def RefreshBufferState(self):
        fileopened = self.Manager.GetAllFilenames()
        for idx, filename in enumerate(fileopened):
            self.FileOpened.SetPageText(idx, filename)
        self.RefreshEditMenu()
        self.RefreshTitle()

    def OnUndoMenu(self, event):
        self.Manager.LoadCurrentPrevious()
        self.RefreshCurrentIndexList()
        self.RefreshBufferState()
        event.Skip()

    def OnRedoMenu(self, event):
        self.Manager.LoadCurrentNext()
        self.RefreshCurrentIndexList()
        self.RefreshBufferState()
        event.Skip()


#-------------------------------------------------------------------------------
#                         Load and Save Funtions
#-------------------------------------------------------------------------------

    def OnNewMenu(self, event):
        self.FilePath = ""
        dialog = CreateNodeDialog(self)
        if dialog.ShowModal() == wxID_OK:
            name, id, type = dialog.GetValues()
            if name != "":
                good = not name[0].isdigit()
                for item in name.split("_"):
                    good &= item.isalnum()
            else:
                good = False
            if good:
                profile,filepath = dialog.GetProfile()
                NMT = dialog.GetNMTManagement()
                options = dialog.GetOptions()
                result = self.Manager.CreateNewNode(name, id, type, profile, filepath, NMT, options)
                if not IsOfType(result, StringType):
                    new_editingpanel = EditingPanel(self, self.Manager)
                    self.FileOpened.AddPage(new_editingpanel, "")
                    self.FileOpened.SetSelection(self.Manager.GetCurrentNodeIndex())
                    self.EditMenu.Enable(wxID_OBJDICTEDITEDITMENUITEMS8, False)
                    if "DS302" in options:
                        self.EditMenu.Enable(wxID_OBJDICTEDITEDITMENUITEMS8, True)
                    self.RefreshBufferState()
                    self.RefreshProfileMenu()
                    self.RefreshMainMenu()
                else:
                    message = wxMessageDialog(self, result, "ERROR", wxOK|wxICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wxMessageDialog(self, "Node name can't be undefined or start with a digit and must be composed of alphanumerical characters or underscore!", "ERROR", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
        event.Skip()

    def OnOpenMenu(self, event):
        filepath = self.Manager.GetCurrentFilePath()
        if filepath != "":
            directory = os.path.dirname(filepath)
        else:
            directory = os.getcwd()
        dialog = wxFileDialog(self, "Choose a file", directory, "",  "OD files (*.od)|*.od|All files|*.*", wxOPEN)
        if dialog.ShowModal() == wxID_OK:
            filepath = dialog.GetPath()
            if os.path.isfile(filepath):
                result = self.Manager.OpenFileInCurrent(filepath)
                if type(result) != StringType:
                    new_editingpanel = EditingPanel(self, self.Manager)
                    self.FileOpened.AddPage(new_editingpanel, "")
                    self.FileOpened.SetSelection(self.Manager.GetCurrentNodeIndex())
                    if self.Manager.CurrentDS302Defined(): 
                        self.EditMenu.Enable(wxID_OBJDICTEDITEDITMENUITEMS8, True)
                    else:
                        self.EditMenu.Enable(wxID_OBJDICTEDITEDITMENUITEMS8, False)
                    self.RefreshEditMenu()
                    self.RefreshBufferState()
                    self.RefreshProfileMenu()
                    self.RefreshMainMenu()
                else:
                    message = wxMessageDialog(self, e.args[0], "Error", wxOK|wxICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
        dialog.Destroy()
        event.Skip()

    def OnSaveMenu(self, event):
        self.Save()
        event.Skip()
    
    def OnSaveAsMenu(self, event):
        self.SaveAs()
        event.Skip()
        
    def Save(self):
        result = self.Manager.SaveCurrentInFile()
        if not result:
            self.SaveAs()
        elif type(result) != StringType:
            self.RefreshBufferState()
        else:
            message = wxMessageDialog(self, result, "Error", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()

    def SaveAs(self):
        filepath = self.Manager.GetCurrentFilePath()
        if filepath != "":
            directory, filename = os.path.split(filepath)
        else:
            directory, filename = os.getcwd(), "%s.od"%self.Manager.GetCurrentNodeInfos()[0]
        dialog = wxFileDialog(self, "Choose a file", directory, filename,  "OD files (*.od)|*.od|All files|*.*", wxSAVE|wxOVERWRITE_PROMPT)
        if dialog.ShowModal() == wxID_OK:
            filepath = dialog.GetPath()
            if os.path.isdir(os.path.dirname(filepath)):
                result = self.Manager.SaveCurrentInFile(filepath)
                if type(result) != StringType:
                    self.RefreshBufferState()
                else:
                    message = wxMessageDialog(self, result, "Error", wxOK|wxICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wxMessageDialog(self, "%s is not a valid folder!"%os.path.dirname(filepath), "Error", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()

    def OnCloseMenu(self, event):
        answer = wxID_YES
        result = self.Manager.CloseCurrent()
        if not result:
            dialog = wxMessageDialog(self, "There are changes, do you want to save?",  "Close File", wxYES_NO|wxCANCEL|wxICON_QUESTION)
            answer = dialog.ShowModal()
            dialog.Destroy()
            if answer == wxID_YES:
                self.OnSaveMenu(event)
                if self.Manager.CurrentIsSaved():
                    self.Manager.CloseCurrent()
            elif answer == wxID_NO:
                self.Manager.CloseCurrent(True)
        if self.FileOpened.GetPageCount() > self.Manager.GetBufferNumber():
            current = self.FileOpened.GetSelection()
            self.FileOpened.DeletePage(current)
            if self.FileOpened.GetPageCount() > 0:
                self.FileOpened.SetSelection(min(current, self.FileOpened.GetPageCount() - 1))
            self.RefreshBufferState()
            self.RefreshMainMenu()
        event.Skip()
        

#-------------------------------------------------------------------------------
#                         Import and Export Functions
#-------------------------------------------------------------------------------

    def OnImportMenu(self, event):
        dialog = wxFileDialog(self, "Choose a file", os.getcwd(), "",  "XML OD files (*.xml)|*.xml|All files|*.*", wxOPEN)
        if dialog.ShowModal() == wxID_OK:
            filepath = dialog.GetPath()
            if os.path.isfile(filepath):
                result = self.Manager.ImportCurrentFromFile(filepath)
                if result:
                    if self.FileOpened.GetPageCount() == 0:
                        new_editingpanel = EditingPanel(self, self.Manager)
                        self.FileOpened.AddPage(new_editingpanel, "")
                        self.FileOpened.SetSelection(self.Manager.GetCurrentNodeIndex())
                    self.RefreshBufferState()
                    self.RefreshCurrentIndexList()
                    self.RefreshProfileMenu()
                    self.RefreshMainMenu()
                    message = wxMessageDialog(self, "Import successful", "Information", wxOK|wxICON_INFORMATION)
                    message.ShowModal()
                    message.Destroy()
        dialog.Destroy()
        event.Skip()

    def OnExportMenu(self, event):
        dialog = wxFileDialog(self, "Choose a file", os.getcwd(), self.Manager.GetCurrentNodeInfos()[0],  "CANFestival OD files (*.c)|*.c|All files|*.*", wxSAVE|wxOVERWRITE_PROMPT)
        if dialog.ShowModal() == wxID_OK:
            filepath = dialog.GetPath()
            if os.path.isdir(os.path.dirname(filepath)):
                path, extend = os.path.splitext(filepath)
                if extend in ("", "."):
                    filepath = path + ".c"
                result = self.Manager.ExportCurrentToFile(filepath)
                if result:
                    message = wxMessageDialog(self, "Export successful", "Information", wxOK|wxICON_INFORMATION)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wxMessageDialog(self, "%s is not a valid folder!"%os.path.dirname(filepath), "Error", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()
        event.Skip()

#-------------------------------------------------------------------------------
#                          Editing Profiles functions
#-------------------------------------------------------------------------------

    def OnCommunicationMenu(self, event):
        dictionary,current = self.Manager.GetCurrentCommunicationLists()
        self.EditProfile("Edit DS-301 Profile", dictionary, current)
        event.Skip()
    
    def OnOtherCommunicationMenu(self, event):
        dictionary,current = self.Manager.GetCurrentDS302Lists()
        self.EditProfile("Edit DS-301 Profile", dictionary, current)
        event.Skip()
    
    def OnEditProfileMenu(self, event):
        title = "Edit %s Profile"%self.Manager.GetCurrentProfileName()
        dictionary,current = self.Manager.GetCurrentProfileLists()
        self.EditProfile(title, dictionary, current)
        event.Skip()
    
    def EditProfile(self, title, dictionary, current):
        dialog = CommunicationDialog(self)
        dialog.SetTitle(title)
        dialog.SetIndexDictionary(dictionary)
        dialog.SetCurrentList(current)
        dialog.RefreshLists()
        if dialog.ShowModal() == wxID_OK:
            new_profile = dialog.GetCurrentList()
            addinglist = []
            removinglist = []
            for index in new_profile:
                if index not in current:
                    addinglist.append(index)
            for index in current:
                if index not in new_profile:
                    removinglist.append(index)
            self.Manager.ManageEntriesOfCurrent(addinglist, removinglist)
            self.Manager.GenerateMapList()
            self.Manager.BufferCurrentNode()
            self.RefreshBufferState()
            self.RefreshCurrentIndexList()
        dialog.Destroy()

    def GetProfileCallBack(self, text):
        def ProfileCallBack(event):
            self.Manager.AddSpecificEntryToCurrent(text)
            self.RefreshBufferState()
            self.RefreshCurrentIndexList()
            event.Skip()
        return ProfileCallBack

#-------------------------------------------------------------------------------
#                         Edit Node informations function
#-------------------------------------------------------------------------------

    def OnNodeInfosMenu(self, event):
        dialog = NodeInfosDialog(self)
        name,id,type = self.Manager.GetCurrentNodeInfos()
        profile = self.Manager.GetCurrentProfileName()
        dialog.SetProfiles([profile])
        dialog.SetValues(name, id, type, profile)
        if dialog.ShowModal() == wxID_OK:
            name,id,type,profile = dialog.GetValues()
            self.Manager.SetCurrentNodeInfos(name, id, type)
            self.RefreshBufferState()
            self.RefreshProfileMenu()
        event.Skip()


#-------------------------------------------------------------------------------
#                           Add User Types and Variables
#-------------------------------------------------------------------------------
        
    def AddMapVariable(self):
        index = self.Manager.GetCurrentNextMapIndex()
        if index:
            dialog = MapVariableDialog(self)
            dialog.SetIndex(index)
            if dialog.ShowModal() == wxID_OK:
                index, name, struct, number = dialog.GetValues()
                result = self.Manager.AddMapVariableToCurrent(index, name, struct, number)
                if type(result) != StringType:
                    self.RefreshBufferState()
                    self.RefreshCurrentIndexList()
                else:
                    message = wxMessageDialog(self, result, "Error", wxOK|wxICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            dialog.Destroy()
        else:
            message = wxMessageDialog(self, result, "No map variable index left!", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()
        
    def AddUserType(self):
        dialog = UserTypeDialog(self)
        dialog.SetTypeList(self.Manager.GetCustomisableTypes())
        if dialog.ShowModal() == wxID_OK:
            type, min, max, length = dialog.GetValues()
            result = self.Manager.AddUserTypeToCurrent(type, min, max, length)
            if not IsOfType(result, StringType):
                self.RefreshBufferState()
                self.RefreshCurrentIndexList()
            else:
                message = wxMessageDialog(self, result, "Error", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()



#-------------------------------------------------------------------------------
#                          Editing Communication Dialog
#-------------------------------------------------------------------------------


[wxID_COMMUNICATIONDIALOG, wxID_COMMUNICATIONDIALOGMAINPANEL,
 wxID_COMMUNICATIONDIALOGPOSSIBLEINDEXES, wxID_COMMUNICATIONDIALOGCURRENTINDEXES,
 wxID_COMMUNICATIONDIALOGSELECT, wxID_COMMUNICATIONDIALOGUNSELECT, 
 wxID_COMMUNICATIONDIALOGSTATICTEXT1, wxID_COMMUNICATIONDIALOGSTATICTEXT2
] = [wx.NewId() for _init_ctrls in range(8)]

class CommunicationDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.MainPanel, 0, border=0, flag=0)

    def _init_sizers(self):
        # generated method, don't edit
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)

        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Dialog.__init__(self, id=wxID_COMMUNICATIONDIALOG,
              name='CommunicationDialog', parent=prnt, pos=wx.Point(234, 216),
              size=wx.Size(726, 437), style=wx.DEFAULT_DIALOG_STYLE,
              title='Edit Communication Profile')
        self.SetClientSize(wx.Size(726, 437))

        self.MainPanel = wx.Panel(id=wxID_COMMUNICATIONDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(688, 382), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.PossibleIndexes = wx.ListBox(choices=[],
              id=wxID_COMMUNICATIONDIALOGPOSSIBLEINDEXES,
              name='PossibleIndexes', parent=self.MainPanel, pos=wx.Point(40,
              48), size=wx.Size(280, 320), style=wxLB_EXTENDED)
        self.PossibleIndexes.Bind(wx.EVT_LEFT_DCLICK, self.OnPossibleIndexesDClick,
              id=wxID_COMMUNICATIONDIALOGPOSSIBLEINDEXES)

        self.CurrentIndexes = wx.ListBox(choices=[],
              id=wxID_COMMUNICATIONDIALOGCURRENTINDEXES, name='CurrentIndexes',
              parent=self.MainPanel, pos=wx.Point(400, 48), size=wx.Size(280,
              320), style=wxLB_EXTENDED)
        self.CurrentIndexes.Bind(wx.EVT_LEFT_DCLICK, self.OnCurrentIndexesDClick,
              id=wxID_COMMUNICATIONDIALOGCURRENTINDEXES)

        self.Select = wx.Button(id=wxID_COMMUNICATIONDIALOGSELECT, label='>>',
              name='Select', parent=self.MainPanel, pos=wx.Point(345, 136),
              size=wx.Size(32, 32), style=0)
        self.Select.Bind(wx.EVT_BUTTON, self.OnSelectButton,
              id=wxID_COMMUNICATIONDIALOGSELECT)

        self.Unselect = wx.Button(id=wxID_COMMUNICATIONDIALOGUNSELECT,
              label='<<', name='Unselect', parent=self.MainPanel,
              pos=wx.Point(345, 216), size=wx.Size(32, 30), style=0)
        self.Unselect.Bind(wx.EVT_BUTTON, self.OnUnselectButton,
              id=wxID_COMMUNICATIONDIALOGUNSELECT)

        self.staticText1 = wx.StaticText(id=wxID_COMMUNICATIONDIALOGSTATICTEXT1,
              label='Possible Profile Indexes :', name='staticText1',
              parent=self.MainPanel, pos=wx.Point(40, 24), size=wx.Size(156,
              17), style=0)

        self.staticText2 = wx.StaticText(id=wxID_COMMUNICATIONDIALOGSTATICTEXT2,
              label='Current Profile Indexes :', name='staticText2',
              parent=self.MainPanel, pos=wx.Point(400, 24), size=wx.Size(152,
              17), style=0)

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.AllList = []
        self.CurrentList = []
        self.IndexDictionary = {}

    def SetIndexDictionary(self, dictionary):
        self.IndexDictionary = dictionary
        
    def SetCurrentList(self, list):
        self.CurrentList = []
        self.CurrentList.extend(list)
        self.CurrentList.sort()
        
    def GetCurrentList(self):
        return self.CurrentList
        
    def RefreshLists(self):
        self.PossibleIndexes.Clear()
        self.CurrentIndexes.Clear()
        self.AllList = []
        for index in self.IndexDictionary.iterkeys():
            if index not in self.CurrentList:
                self.AllList.append(index)
        self.AllList.sort()
        for index in self.AllList:
            self.PossibleIndexes.Append("0x%04X   %s"%(index, self.IndexDictionary[index][0]))
        for index in self.CurrentList:
            if index in self.IndexDictionary:
                self.CurrentIndexes.Append("0x%04X   %s"%(index, self.IndexDictionary[index][0]))

    def OnPossibleIndexesDClick(self, event):
        self.SelectPossible()
        event.Skip()

    def OnCurrentIndexesDClick(self, event):
        self.UnselectCurrent()
        event.Skip()

    def OnSelectButton(self, event):
        self.SelectPossible()
        event.Skip()

    def OnUnselectButton(self, event):
        self.UnselectCurrent()
        event.Skip()

    def SelectPossible(self):
        selected = self.PossibleIndexes.GetSelections()
        for i in selected:
            self.CurrentList.append(self.AllList[i])
        self.CurrentList.sort()
        self.RefreshLists()

    def UnselectCurrent(self):
        selected = self.CurrentIndexes.GetSelections()
        for i in selected:
            if not self.IndexDictionary[self.CurrentList[i]][1]:
                self.CurrentList.pop(i)
        self.CurrentList.sort()
        self.RefreshLists()



#-------------------------------------------------------------------------------
#                          Create Map Variable Dialog
#-------------------------------------------------------------------------------


[wxID_MAPVARIABLEDIALOG, wxID_MAPVARIABLEDIALOGINDEX, 
 wxID_MAPVARIABLEDIALOGINDEXNAME, wxID_MAPVARIABLEDIALOGMAINPANEL, 
 wxID_MAPVARIABLEDIALOGNUMBER, wxID_MAPVARIABLEDIALOGRADIOBUTTON1, 
 wxID_MAPVARIABLEDIALOGRADIOBUTTON2, wxID_MAPVARIABLEDIALOGRADIOBUTTON3, 
 wxID_MAPVARIABLEDIALOGSTATICTEXT1, wxID_MAPVARIABLEDIALOGSTATICTEXT2, 
 wxID_MAPVARIABLEDIALOGSTATICTEXT3, wxID_MAPVARIABLEDIALOGSTATICTEXT4, 
] = [wx.NewId() for _init_ctrls in range(12)]

class MapVariableDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.MainPanel, 0, border=0, flag=0)

    def _init_sizers(self):
        # generated method, don't edit
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)

        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Dialog.__init__(self, id=wxID_MAPVARIABLEDIALOG,
              name='CommunicationDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(444, 186), style=wx.DEFAULT_DIALOG_STYLE,
              title='Add Map Variable')
        self.SetClientSize(wx.Size(444, 186))

        self.MainPanel = wx.Panel(id=wxID_MAPVARIABLEDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(431, 142), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_MAPVARIABLEDIALOGSTATICTEXT1,
              label='Index :', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.Index = wx.TextCtrl(id=wxID_MAPVARIABLEDIALOGINDEX, name='Index',
              parent=self.MainPanel, pos=wx.Point(24, 48), size=wx.Size(152,
              25), style=0, value='0x2000')

        self.staticText3 = wx.StaticText(id=wxID_MAPVARIABLEDIALOGSTATICTEXT3,
              label='Name :', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(47, 17), style=0)

        self.IndexName = wx.TextCtrl(id=wxID_MAPVARIABLEDIALOGINDEXNAME,
              name='IndexName', parent=self.MainPanel, pos=wx.Point(24, 104),
              size=wx.Size(152, 24), style=0, value='Undefined')

        self.staticText2 = wx.StaticText(id=wxID_MAPVARIABLEDIALOGSTATICTEXT2,
              label='Type :', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(208, 24), size=wx.Size(38, 17), style=0)

        self.radioButton1 = wx.RadioButton(id=wxID_MAPVARIABLEDIALOGRADIOBUTTON1,
              label='VAR', name='radioButton1', parent=self.MainPanel,
              pos=wx.Point(208, 48), size=wx.Size(72, 24), style=wxRB_GROUP)
        self.radioButton1.SetValue(True)
        self.radioButton1.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton1Click,
              id=wxID_MAPVARIABLEDIALOGRADIOBUTTON1)

        self.radioButton2 = wx.RadioButton(id=wxID_MAPVARIABLEDIALOGRADIOBUTTON2,
              label='ARRAY', name='radioButton2', parent=self.MainPanel,
              pos=wx.Point(208, 72), size=wx.Size(80, 24), style=wxRB_SINGLE)
        self.radioButton2.SetValue(False)
        self.radioButton2.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton2Click,
              id=wxID_MAPVARIABLEDIALOGRADIOBUTTON2)

        self.radioButton3 = wx.RadioButton(id=wxID_MAPVARIABLEDIALOGRADIOBUTTON3,
              label='REC', name='radioButton3', parent=self.MainPanel,
              pos=wx.Point(208, 96), size=wx.Size(96, 24), style=wxRB_SINGLE)
        self.radioButton3.SetValue(False)
        self.radioButton3.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton3Click,
              id=wxID_MAPVARIABLEDIALOGRADIOBUTTON3)

        self.staticText4 = wx.StaticText(id=wxID_MAPVARIABLEDIALOGSTATICTEXT4,
              label='Number :', name='staticText4', parent=self.MainPanel,
              pos=wx.Point(312, 80), size=wx.Size(88, 16), style=0)

        self.Number = wx.TextCtrl(id=wxID_MAPVARIABLEDIALOGNUMBER,
              name='Number', parent=self.MainPanel, pos=wx.Point(312, 104),
              size=wx.Size(112, 24), style=wx.TE_RIGHT, value='0')

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.staticText4.Enable(False)
        self.Number.Enable(False)

    def SetIndex(self, index):
        self.Index.SetValue("0x%04X"%index)

    def GetValues(self):
        if self.radioButton1.GetValue():
            struct = 1
        elif self.radioButton2.GetValue():
            struct = 3
        elif self.radioButton3.GetValue():
            struct = 7
        name = self.IndexName.GetValue()
        index = eval(self.Index.GetValue())
        number = eval(self.Number.GetValue())
        return index, name, struct, number

    def OnRadioButton1Click(self, event):
        self.EnableNumberTyping(False)
        event.Skip()

    def OnRadioButton2Click(self, event):
        self.EnableNumberTyping(True)
        event.Skip()

    def OnRadioButton3Click(self, event):
        self.EnableNumberTyping(True)
        event.Skip()

    def EnableNumberTyping(self, enable):
        self.staticText4.Enable(enable)
        self.Number.Enable(enable)


#-------------------------------------------------------------------------------
#                          Create User Type Dialog
#-------------------------------------------------------------------------------


[wxID_USERTYPEDIALOG, wxID_USERTYPEDIALOGLENGTH, wxID_USERTYPEDIALOGMAINPANEL, 
 wxID_USERTYPEDIALOGMAX, wxID_USERTYPEDIALOGMIN, 
 wxID_USERTYPEDIALOGSTATICBOX1, wxID_USERTYPEDIALOGSTATICTEXT1, 
 wxID_USERTYPEDIALOGSTATICTEXT2, wxID_USERTYPEDIALOGSTATICTEXT3, 
 wxID_USERTYPEDIALOGSTATICTEXT4, wxID_USERTYPEDIALOGTYPE, 
] = [wx.NewId() for _init_ctrls in range(11)]

class UserTypeDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.MainPanel, 0, border=0, flag=0)

    def _init_sizers(self):
        # generated method, don't edit
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)

        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Dialog.__init__(self, id=wxID_USERTYPEDIALOG, name='UserTypeDialog',
              parent=prnt, pos=wx.Point(376, 223), size=wx.Size(444, 228),
              style=wx.DEFAULT_DIALOG_STYLE, title='Add User Type')
        self.SetClientSize(wx.Size(444, 228))

        self.MainPanel = wx.Panel(id=wxID_USERTYPEDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(431, 182), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_USERTYPEDIALOGSTATICTEXT1,
              label='Type :', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.Type = wx.Choice(choices=[], id=wxID_USERTYPEDIALOGTYPE,
              name='Type', parent=self.MainPanel, pos=wx.Point(24, 48),
              size=wx.Size(160, 24), style=0)
        self.Type.Bind(wx.EVT_CHOICE, self.OnTypeChoice,
              id=wxID_USERTYPEDIALOGTYPE)

        self.staticBox1 = wx.StaticBox(id=wxID_USERTYPEDIALOGSTATICBOX1,
              label='Values', name='staticBox1', parent=self.MainPanel,
              pos=wx.Point(200, 24), size=wx.Size(224, 144), style=0)

        self.staticText2 = wx.StaticText(id=wxID_USERTYPEDIALOGSTATICTEXT2,
              label='Minimum :', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(216, 48), size=wx.Size(67, 17), style=0)

        self.Min = wx.TextCtrl(id=wxID_USERTYPEDIALOGMIN, name='Min',
              parent=self.MainPanel, pos=wx.Point(296, 48), size=wx.Size(112,
              24), style=wx.TE_RIGHT, value='0')

        self.staticText3 = wx.StaticText(id=wxID_USERTYPEDIALOGSTATICTEXT3,
              label='Maximum :', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(216, 88), size=wx.Size(71, 17), style=0)

        self.Max = wx.TextCtrl(id=wxID_USERTYPEDIALOGMAX, name='Max',
              parent=self.MainPanel, pos=wx.Point(296, 88), size=wx.Size(112,
              25), style=wx.TE_RIGHT, value='0')

        self.staticText4 = wx.StaticText(id=wxID_USERTYPEDIALOGSTATICTEXT4,
              label='Length :', name='staticText4', parent=self.MainPanel,
              pos=wx.Point(216, 128), size=wx.Size(52, 17), style=0)

        self.Length = wx.TextCtrl(id=wxID_USERTYPEDIALOGLENGTH, name='Length',
              parent=self.MainPanel, pos=wx.Point(296, 128), size=wx.Size(112,
              25), style=wx.TE_RIGHT, value='0')

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.TypeDictionary = {}
        
    def SetValues(self, min = None, max = None, length = None):
        if min != None:
            self.Min.SetValue(str(min))
        if max != None:
            self.Max.SetValue(str(max))
        if length != None:
            self.Length.SetValue(str(length))

    def SetTypeList(self, typedic, type = None):
        self.Type.Clear()
        list = []
        for index, (name, valuetype) in typedic.iteritems():
            self.TypeDictionary[name] = (index, valuetype)
            list.append((index, name))
        list.sort()
        for index, name in list:
            self.Type.Append(name)
        if type != None:
            self.Type.SetStringSelection(typedic[type][0])
        self.RefreshValues()

    def OnTypeChoice(self, event):
        self.RefreshValues()
        event.Skip()
    
    def RefreshValues(self):
        name = self.Type.GetStringSelection()
        if name != "":
            valuetype = self.TypeDictionary[name][1]
            if valuetype == 0:
                self.staticText2.Enable(True)
                self.staticText3.Enable(True)
                self.staticText4.Enable(False)
                self.Min.Enable(True)
                self.Max.Enable(True)
                self.Length.Enable(False)
            elif valuetype == 1:
                self.staticText2.Enable(False)
                self.staticText3.Enable(False)
                self.staticText4.Enable(True)
                self.Min.Enable(False)
                self.Max.Enable(False)
                self.Length.Enable(True)
        else:
            self.staticText2.Enable(False)
            self.staticText3.Enable(False)
            self.staticText4.Enable(False)
            self.Min.Enable(False)
            self.Max.Enable(False)
            self.Length.Enable(False)

    def GetValues(self):
        name = self.Type.GetStringSelection()
        type = self.TypeDictionary[name][0]
        min = eval(self.Min.GetValue())
        max = eval(self.Max.GetValue())
        length = eval(self.Length.GetValue())
        return type, min, max, length



#-------------------------------------------------------------------------------
#                          Editing Node Infos Dialog
#-------------------------------------------------------------------------------


[wxID_NODEINFOSDIALOG, wxID_NODEINFOSDIALOGMAINPANEL, 
 wxID_NODEINFOSDIALOGNAME, wxID_NODEINFOSDIALOGNODEID, 
 wxID_NODEINFOSDIALOGPROFILE, wxID_NODEINFOSDIALOGSTATICTEXT1, 
 wxID_NODEINFOSDIALOGSTATICTEXT2, wxID_NODEINFOSDIALOGSTATICTEXT3, 
 wxID_NODEINFOSDIALOGSTATICTEXT4, wxID_NODEINFOSDIALOGTYPE, 
] = [wx.NewId() for _init_ctrls in range(10)]

class NodeInfosDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.MainPanel, 0, border=0, flag=0)

    def _init_sizers(self):
        # generated method, don't edit
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)

        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Dialog.__init__(self, id=wxID_NODEINFOSDIALOG,
              name='NodeInfosDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(249, 250), style=wx.DEFAULT_DIALOG_STYLE,
              title='Node Infos')
        self.SetClientSize(wx.Size(249, 250))

        self.MainPanel = wx.Panel(id=wxID_NODEINFOSDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(231, 264), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT1,
              label='Name :', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.Name = wx.TextCtrl(id=wxID_NODEINFOSDIALOGNAME, name='Name',
              parent=self.MainPanel, pos=wx.Point(24, 48), size=wx.Size(200,
              25), style=0, value='')

        self.staticText2 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT2,
              label='Node ID :', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(67, 17), style=0)

        self.NodeID = wx.TextCtrl(id=wxID_NODEINFOSDIALOGNODEID, name='NodeID',
              parent=self.MainPanel, pos=wx.Point(24, 104), size=wx.Size(200,
              25), style=wx.TE_RIGHT, value='')

        self.staticText3 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT3,
              label='Type :', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(24, 136), size=wx.Size(71, 17), style=0)

        self.Type = wx.Choice(choices=[], id=wxID_NODEINFOSDIALOGTYPE,
              name='Type', parent=self.MainPanel, pos=wx.Point(24, 160),
              size=wx.Size(200, 25), style=0)

        self.staticText4 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT4,
              label='Profile :', name='staticText4', parent=self.MainPanel,
              pos=wx.Point(24, 192), size=wx.Size(47, 17), style=0)

        self.Profile = wx.Choice(choices=[], id=wxID_NODEINFOSDIALOGPROFILE,
              name='Profile', parent=self.MainPanel, pos=wx.Point(24, 216),
              size=wx.Size(200, 25), style=0)

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.Type.Append("master")
        self.Type.Append("slave")
        self.staticText4.Hide()
        self.Profile.Hide()
    
    def SetProfiles(self, profiles):
        for profile in profiles:
            self.Profile.Append(profile)
    
    def SetValues(self, name, id, type, profile):
        self.Name.SetValue(name)
        self.NodeID.SetValue("0x%02X"%id)
        self.Type.SetStringSelection(type)
        self.Profile.SetStringSelection(profile)

    def GetValues(self):
        name = self.Name.GetValue()
        nodeid = eval(self.NodeID.GetValue())
        type = self.Type.GetStringSelection()
        profile = self.Profile.GetStringSelection()
        return name, nodeid, type, profile



#-------------------------------------------------------------------------------
#                          Create New Node Dialog
#-------------------------------------------------------------------------------


[wxID_CREATENODEDIALOG, wxID_CREATENODEDIALOGEMERGENCY, 
 wxID_CREATENODEDIALOGGENSYNC, wxID_CREATENODEDIALOGMAINPANEL, 
 wxID_CREATENODEDIALOGNAME, wxID_CREATENODEDIALOGNMT_HEARTBEAT, 
 wxID_CREATENODEDIALOGNMT_NODEGUARDING, wxID_CREATENODEDIALOGNMT_NONE, 
 wxID_CREATENODEDIALOGNODEID, wxID_CREATENODEDIALOGPROFILE, 
 wxID_CREATENODEDIALOGSAVECONFIG, wxID_CREATENODEDIALOGSTATICTEXT1, 
 wxID_CREATENODEDIALOGSTATICTEXT2, wxID_CREATENODEDIALOGSTATICTEXT3, 
 wxID_CREATENODEDIALOGSTATICTEXT4, wxID_CREATENODEDIALOGSTATICTEXT5, 
 wxID_CREATENODEDIALOGSTATICTEXT6, wxID_CREATENODEDIALOGSTOREEDS, 
 wxID_CREATENODEDIALOGTYPE, 
] = [wx.NewId() for _init_ctrls in range(19)]

class CreateNodeDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        # generated method, don't edit

        parent.AddWindow(self.MainPanel, 0, border=0, flag=0)

    def _init_sizers(self):
        # generated method, don't edit
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=0)

        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)

        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Dialog.__init__(self, id=wxID_CREATENODEDIALOG,
              name='CreateNodeDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(451, 316), style=wx.DEFAULT_DIALOG_STYLE,
              title='Create a new Node')
        self.SetClientSize(wx.Size(451, 316))

        self.MainPanel = wx.Panel(id=wxID_CREATENODEDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(440, 278), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT1,
              label='Name :', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.staticText2 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT2,
              label='Node ID :', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(67, 17), style=0)

        self.staticText3 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT3,
              label='Type :', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(24, 136), size=wx.Size(71, 17), style=0)

        self.Type = wx.Choice(choices=[], id=wxID_CREATENODEDIALOGTYPE,
              name='Type', parent=self.MainPanel, pos=wx.Point(24, 160),
              size=wx.Size(200, 24), style=0)

        self.Name = wx.TextCtrl(id=wxID_CREATENODEDIALOGNAME, name='Name',
              parent=self.MainPanel, pos=wx.Point(24, 48), size=wx.Size(200,
              25), style=0, value='')

        self.NodeID = wx.TextCtrl(id=wxID_CREATENODEDIALOGNODEID, name='NodeID',
              parent=self.MainPanel, pos=wx.Point(24, 104), size=wx.Size(200,
              25), style=wx.TE_RIGHT, value='')

        self.staticText4 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT4,
              label='Profile :', name='staticText4', parent=self.MainPanel,
              pos=wx.Point(24, 192), size=wx.Size(47, 17), style=0)

        self.Profile = wx.Choice(choices=[], id=wxID_CREATENODEDIALOGPROFILE,
              name='Profile', parent=self.MainPanel, pos=wx.Point(24, 216),
              size=wx.Size(200, 24), style=0)
        self.Profile.Bind(wx.EVT_CHOICE, self.OnProfileChoice,
              id=wxID_CREATENODEDIALOGPROFILE)

        self.staticText5 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT5,
              label='Network Management :', name='staticText5',
              parent=self.MainPanel, pos=wx.Point(256, 24), size=wx.Size(152,
              16), style=0)

        self.NMT_None = wx.RadioButton(id=wxID_CREATENODEDIALOGNMT_NONE,
              label='None', name='NMT_None', parent=self.MainPanel,
              pos=wx.Point(256, 40), size=wx.Size(114, 24), style=0)
        self.NMT_None.SetValue(True)

        self.NMT_NodeGuarding = wx.RadioButton(id=wxID_CREATENODEDIALOGNMT_NODEGUARDING,
              label='Node Guarding', name='NMT_NodeGuarding',
              parent=self.MainPanel, pos=wx.Point(256, 64), size=wx.Size(128,
              24), style=0)
        self.NMT_NodeGuarding.SetValue(False)

        self.NMT_Heartbeat = wx.RadioButton(id=wxID_CREATENODEDIALOGNMT_HEARTBEAT,
              label='Heartbeat', name='NMT_Heartbeat', parent=self.MainPanel,
              pos=wx.Point(256, 88), size=wx.Size(114, 24), style=0)
        self.NMT_Heartbeat.SetValue(False)

        self.staticText6 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT6,
              label='Options :', name='staticText6', parent=self.MainPanel,
              pos=wx.Point(256, 128), size=wx.Size(72, 17), style=0)

        self.DS302 = wx.CheckBox(id=wxID_CREATENODEDIALOGGENSYNC,
              label='DS-302 Profile', name='DS302', parent=self.MainPanel,
              pos=wx.Point(256, 144), size=wx.Size(128, 24), style=0)
        self.DS302.SetValue(False)

        self.GenSYNC = wx.CheckBox(id=wxID_CREATENODEDIALOGGENSYNC,
              label='Generate SYNC', name='GenSYNC', parent=self.MainPanel,
              pos=wx.Point(256, 168), size=wx.Size(128, 24), style=0)
        self.GenSYNC.SetValue(False)

        self.Emergency = wx.CheckBox(id=wxID_CREATENODEDIALOGEMERGENCY,
              label='Emergency support', name='Emergency',
              parent=self.MainPanel, pos=wx.Point(256, 192), size=wx.Size(152,
              24), style=0)
        self.Emergency.SetValue(False)

        self.SaveConfig = wx.CheckBox(id=wxID_CREATENODEDIALOGSAVECONFIG,
              label='Save Configuration', name='SaveConfig',
              parent=self.MainPanel, pos=wx.Point(256, 216), size=wx.Size(152,
              24), style=0)
        self.SaveConfig.SetValue(False)

        self.StoreEDS = wx.CheckBox(id=wxID_CREATENODEDIALOGSTOREEDS,
              label='Store EDS', name='StoreEDS', parent=self.MainPanel,
              pos=wx.Point(256, 240), size=wx.Size(144, 24), style=0)
        self.StoreEDS.SetValue(False)

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.Type.Append("master")
        self.Type.Append("slave")
        self.Type.SetStringSelection("slave")
        self.ListProfile = {"None" : ""}
        self.Profile.Append("None")
        self.Directory = os.path.join(os.getcwd(), "config")
        listfiles = os.listdir(self.Directory)
        listfiles.sort()
        for item in listfiles:
            name, extend = os.path.splitext(item)
            if os.path.isfile(os.path.join(self.Directory, item)) and extend == ".prf" and name != "DS-302":
                self.ListProfile[name] = os.path.join(self.Directory, item)
                self.Profile.Append(name)
        self.Profile.Append("Other")
        self.Profile.SetStringSelection("None")

    def GetValues(self):
        name = self.Name.GetValue()
        nodeid = 0
        if self.NodeID.GetValue() != "":
            nodeid = eval(self.NodeID.GetValue())
        type = self.Type.GetStringSelection()
        return name, nodeid, type

    def GetProfile(self):
        name = self.Profile.GetStringSelection()
        return name, self.ListProfile[name]

    def GetNMTManagement(self):
        if self.NMT_None.GetValue():
            return "None"
        elif self.NMT_NodeGuarding.GetValue():
            return "NodeGuarding"
        elif self.NMT_Heartbeat.GetValue():
            return "Heartbeat"
        return None
    
    def GetOptions(self):
        options = []
        if self.DS302.GetValue():
            options.append("DS302")
        if self.GenSYNC.GetValue():
            options.append("GenSYNC")
        if self.Emergency.GetValue():
            options.append("Emergency")
        if self.SaveConfig.GetValue():
            options.append("SaveConfig")
        if self.StoreEDS.GetValue():
            options.append("StoreEDS")
        return options

    def OnProfileChoice(self, event):
        if self.Profile.GetStringSelection() == "Other":
            dialog = wxFileDialog(self, "Choose a file", self.Directory, "",  "OD Profile files (*.prf)|*.prf|All files|*.*", wxOPEN)
            dialog.ShowModal()
            filepath = dialog.GetPath()
            dialog.Destroy()
            if os.path.isfile(filepath):
                name = os.path.splitext(os.path.basename(filepath))[0]
                self.ListProfile[name] = filepath
                length = self.Profile.GetCount()
                self.Profile.Insert(name, length - 2)
                self.Profile.SetStringSelection(name)
            else:
                self.Profile.SetStringSelection("None")
        event.Skip()



#-------------------------------------------------------------------------------
#                                Html Frame
#-------------------------------------------------------------------------------


[wxID_HTMLFRAME, wxID_HTMLFRAMEHTMLCONTENT, 
] = [wx.NewId() for _init_ctrls in range(2)]

class HtmlFrame(wx.Frame):
    def _init_ctrls(self, prnt):
        # generated method, don't edit
        wx.Frame.__init__(self, id=wxID_HTMLFRAME, name='HtmlFrame',
              parent=prnt, pos=wx.Point(320, 231), size=wx.Size(853, 616),
              style=wx.DEFAULT_FRAME_STYLE, title='')
        self.Bind(wx.EVT_CLOSE, self.OnCloseFrame, id=wxID_HTMLFRAME)
        
        self.HtmlContent = wxUrlClickHtmlWindow(id=wxID_HTMLFRAMEHTMLCONTENT,
              name='HtmlContent', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(-1, -1), style=wxHW_SCROLLBAR_AUTO|wxHW_NO_SELECTION)
        EVT_HTML_URL_CLICK(self.HtmlContent, self.OnLinkClick)

    def __init__(self, parent, opened):
        self._init_ctrls(parent)
        self.HtmlFrameOpened = opened
    
    def SetHtmlCode(self, htmlcode):
        self.HtmlContent.SetPage(htmlcode)
        
    def SetHtmlPage(self, htmlpage):
        self.HtmlContent.LoadPage(htmlpage)
        
    def OnCloseFrame(self, event):
        self.HtmlFrameOpened.remove(self.GetTitle())
        event.Skip()
    
    def OnLinkClick(self, event):
        url = event.linkinfo[0]
        try:
            import webbrowser
        except ImportError:
            wxMessageBox('Please point your browser at: %s' % url)
        else:
            webbrowser.open(url)
    

#-------------------------------------------------------------------------------
#                               Exception Handler
#-------------------------------------------------------------------------------

Max_Traceback_List_Size = 20

def Display_Exception_Dialog(e_type,e_value,e_tb):
    trcbck_lst = []
    for i,line in enumerate(traceback.extract_tb(e_tb)):
        trcbck = " " + str(i+1) + ". "
        if line[0].find(os.getcwd()) == -1:
            trcbck += "file : " + str(line[0]) + ",   "
        else:
            trcbck += "file : " + str(line[0][len(os.getcwd()):]) + ",   "
        trcbck += "line : " + str(line[1]) + ",   " + "function : " + str(line[2])
        trcbck_lst.append(trcbck)
        
    # Allow clicking....
    cap = wx.Window_GetCapture()
    if cap:
        cap.ReleaseMouse()

    dlg = wx.SingleChoiceDialog(None, 
        """
An error happens.

Click on OK for saving an error report.

Please contact LOLITech at:
+33 (0)3 29 52 95 67
bugs_objdictedit@lolitech.fr


Error:
""" +
        str(e_type) + " : " + str(e_value), 
        "Error",
        trcbck_lst)
    try:
        res = (dlg.ShowModal() == wx.ID_OK)
    finally:
        dlg.Destroy()

    return res

def Display_Error_Dialog(e_value):
    message = wxMessageDialog(None, str(e_value), "Error", wxOK|wxICON_ERROR)
    message.ShowModal()
    message.Destroy()

def get_last_traceback(tb):
    while tb.tb_next:
        tb = tb.tb_next
    return tb


def format_namespace(d, indent='    '):
    return '\n'.join(['%s%s: %s' % (indent, k, repr(v)[:10000]) for k, v in d.iteritems()])


ignored_exceptions = [] # a problem with a line in a module is only reported once per session

def wxAddExceptHook(path, app_version='[No version]'):#, ignored_exceptions=[]):
    
    def handle_exception(e_type, e_value, e_traceback):
        traceback.print_exception(e_type, e_value, e_traceback) # this is very helpful when there's an exception in the rest of this func
        last_tb = get_last_traceback(e_traceback)
        ex = (last_tb.tb_frame.f_code.co_filename, last_tb.tb_frame.f_lineno)
        if str(e_value).startswith("!!!"):
            Display_Error_Dialog(e_value)
        elif ex not in ignored_exceptions:
            ignored_exceptions.append(ex)
            result = Display_Exception_Dialog(e_type,e_value,e_traceback)
            if result:
                info = {
                    'app-title' : wx.GetApp().GetAppName(), # app_title
                    'app-version' : app_version,
                    'wx-version' : wx.VERSION_STRING,
                    'wx-platform' : wx.Platform,
                    'python-version' : platform.python_version(), #sys.version.split()[0],
                    'platform' : platform.platform(),
                    'e-type' : e_type,
                    'e-value' : e_value,
                    'date' : time.ctime(),
                    'cwd' : os.getcwd(),
                    }
                if e_traceback:
                    info['traceback'] = ''.join(traceback.format_tb(e_traceback)) + '%s: %s' % (e_type, e_value)
                    last_tb = get_last_traceback(e_traceback)
                    exception_locals = last_tb.tb_frame.f_locals # the locals at the level of the stack trace where the exception actually occurred
                    info['locals'] = format_namespace(exception_locals)
                    if 'self' in exception_locals:
                        info['self'] = format_namespace(exception_locals['self'].__dict__)
                
                output = open(path+os.sep+"bug_report_"+info['date'].replace(':','-').replace(' ','_')+".txt",'w')
                lst = info.keys()
                lst.sort()
                for a in lst:
                    output.write(a+":\n"+str(info[a])+"\n\n")

    #sys.excepthook = lambda *args: wx.CallAfter(handle_exception, *args)
    sys.excepthook = handle_exception

if __name__ == '__main__':
    app = wxPySimpleApp()
    wxInitAllImageHandlers()
    
    # Install a exception handle for bug reports
    wxAddExceptHook(os.getcwd(),__version__)
    
    frame = objdictedit(None)

    frame.Show()
    app.MainLoop()
