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

import wx

from types import *
import os, re, platform, sys, time, traceback, getopt

__version__ = "$Revision$"

from node import OD_Subindex, OD_MultipleSubindexes, OD_IdenticalSubindexes, OD_IdenticalIndexes

from nodemanager import *
from subindextable import *
from commondialogs import *
from doc_index.DS301_index import *

try:
    import wx.html

    EVT_HTML_URL_CLICK = wx.NewId()

    class HtmlWindowUrlClick(wx.PyEvent):
        def __init__(self, linkinfo):
            wx.PyEvent.__init__(self)
            self.SetEventType(EVT_HTML_URL_CLICK)
            self.linkinfo = (linkinfo.GetHref(), linkinfo.GetTarget())
            
    class UrlClickHtmlWindow(wx.html.HtmlWindow):
        """ HTML window that generates and OnLinkClicked event.

        Use this to avoid having to override HTMLWindow
        """
        def OnLinkClicked(self, linkinfo):
            wx.PostEvent(self, HtmlWindowUrlClick(linkinfo))
        
        def Bind(self, event, handler, source=None, id=wx.ID_ANY, id2=wx.ID_ANY):
            if event == HtmlWindowUrlClick:
                self.Connect(-1, -1, EVT_HTML_URL_CLICK, handler)
            else:
                wx.html.HtmlWindow.Bind(event, handler, source=source, id=id, id2=id2)
    
#-------------------------------------------------------------------------------
#                                Html Frame
#-------------------------------------------------------------------------------

    [ID_HTMLFRAME, ID_HTMLFRAMEHTMLCONTENT] = [wx.NewId() for _init_ctrls in range(2)]

    class HtmlFrame(wx.Frame):
        def _init_ctrls(self, prnt):
            wx.Frame.__init__(self, id=ID_HTMLFRAME, name='HtmlFrame',
                  parent=prnt, pos=wx.Point(320, 231), size=wx.Size(853, 616),
                  style=wx.DEFAULT_FRAME_STYLE, title='')
            self.Bind(wx.EVT_CLOSE, self.OnCloseFrame, id=ID_HTMLFRAME)
            
            self.HtmlContent = UrlClickHtmlWindow(id=ID_HTMLFRAMEHTMLCONTENT,
                  name='HtmlContent', parent=self, pos=wx.Point(0, 0),
                  size=wx.Size(-1, -1), style=wx.html.HW_SCROLLBAR_AUTO|wx.html.HW_NO_SELECTION)
            self.HtmlContent.Bind(HtmlWindowUrlClick, self.OnLinkClick)

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
                wx.MessageBox('Please point your browser at: %s' % url)
            else:
                webbrowser.open(url)

    Html_Window = True
except:
    Html_Window = False

def create(parent):
    return objdictedit(parent)

def usage():
    print "\nUsage of objdictedit.py :"
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
ScriptDirectory = os.path.split(__file__)[0]


[ID_OBJDICTEDIT, ID_OBJDICTEDITFILEOPENED, 
 ID_OBJDICTEDITHELPBAR,
] = [wx.NewId() for _init_ctrls in range(3)]

[ID_OBJDICTEDITADDMENUITEMS0, ID_OBJDICTEDITADDMENUITEMS1, 
 ID_OBJDICTEDITADDMENUITEMS2, ID_OBJDICTEDITADDMENUITEMS3, 
 ID_OBJDICTEDITADDMENUITEMS4, ID_OBJDICTEDITADDMENUITEMS5, 
] = [wx.NewId() for _init_coll_AddMenu_Items in range(6)]

[ID_OBJDICTEDITFILEMENUITEMS0, ID_OBJDICTEDITFILEMENUITEMS1, 
 ID_OBJDICTEDITFILEMENUITEMS2, ID_OBJDICTEDITFILEMENUITEMS4, 
 ID_OBJDICTEDITFILEMENUITEMS5, ID_OBJDICTEDITFILEMENUITEMS6, 
 ID_OBJDICTEDITFILEMENUITEMS7, ID_OBJDICTEDITFILEMENUITEMS8,
 ID_OBJDICTEDITFILEMENUITEMS9,
] = [wx.NewId() for _init_coll_FileMenu_Items in range(9)]

[ID_OBJDICTEDITEDITMENUITEMS0, ID_OBJDICTEDITEDITMENUITEMS1, 
 ID_OBJDICTEDITEDITMENUITEMS2, ID_OBJDICTEDITEDITMENUITEMS4, 
 ID_OBJDICTEDITEDITMENUITEMS6, ID_OBJDICTEDITEDITMENUITEMS7, 
 ID_OBJDICTEDITEDITMENUITEMS8, 
] = [wx.NewId() for _init_coll_EditMenu_Items in range(7)]

[ID_OBJDICTEDITHELPMENUITEMS0, ID_OBJDICTEDITHELPMENUITEMS1,
 ID_OBJDICTEDITHELPMENUITEMS2,
] = [wx.NewId() for _init_coll_HelpMenu_Items in range(3)]

class objdictedit(wx.Frame):
    def _init_coll_menuBar1_Menus(self, parent):
        parent.Append(menu=self.FileMenu, title='File')
        parent.Append(menu=self.EditMenu, title='Edit')
        parent.Append(menu=self.AddMenu, title='Add')
        parent.Append(menu=self.HelpMenu, title='Help')

    def _init_coll_EditMenu_Items(self, parent):
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS4,
              kind=wx.ITEM_NORMAL, text='Refresh\tCTRL+R')
        parent.AppendSeparator()
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='Undo\tCTRL+Z')
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='Redo\tCTRL+Y')
        parent.AppendSeparator()
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS6,
              kind=wx.ITEM_NORMAL, text='Node infos')
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='DS-301 Profile')
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS8,
              kind=wx.ITEM_NORMAL, text='DS-302 Profile')
        parent.Append(help='', id=ID_OBJDICTEDITEDITMENUITEMS7,
              kind=wx.ITEM_NORMAL, text='Other Profile')
        self.Bind(wx.EVT_MENU, self.OnUndoMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnRedoMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnCommunicationMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS2)
        self.Bind(wx.EVT_MENU, self.OnRefreshMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS4)
        self.Bind(wx.EVT_MENU, self.OnNodeInfosMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS6)
        self.Bind(wx.EVT_MENU, self.OnEditProfileMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS7)
        self.Bind(wx.EVT_MENU, self.OnOtherCommunicationMenu,
              id=ID_OBJDICTEDITEDITMENUITEMS8)

    def _init_coll_HelpMenu_Items(self, parent):
        parent.Append(help='', id=ID_OBJDICTEDITHELPMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='DS-301 Standard\tF1')
        self.Bind(wx.EVT_MENU, self.OnHelpDS301Menu,
              id=ID_OBJDICTEDITHELPMENUITEMS0)
        parent.Append(help='', id=ID_OBJDICTEDITHELPMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='CAN Festival Docs\tF2')
        self.Bind(wx.EVT_MENU, self.OnHelpCANFestivalMenu,
              id=ID_OBJDICTEDITHELPMENUITEMS1)
        if Html_Window:
            parent.Append(help='', id=ID_OBJDICTEDITHELPMENUITEMS2,
                  kind=wx.ITEM_NORMAL, text='About')
            self.Bind(wx.EVT_MENU, self.OnAboutMenu,
                  id=ID_OBJDICTEDITHELPMENUITEMS2)

    def _init_coll_FileMenu_Items(self, parent):
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS5,
              kind=wx.ITEM_NORMAL, text='New\tCTRL+N')
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='Open\tCTRL+O')
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='Save\tCTRL+S')
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS6,
              kind=wx.ITEM_NORMAL, text='Save As...\tALT+S')
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='Close\tCTRL+W')
        parent.AppendSeparator()
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS7,
              kind=wx.ITEM_NORMAL, text='Import EDS file')
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS9,
              kind=wx.ITEM_NORMAL, text='Export to EDS file')
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS8,
              kind=wx.ITEM_NORMAL, text='Build Dictionary\tCTRL+B')
        parent.AppendSeparator()
        parent.Append(help='', id=ID_OBJDICTEDITFILEMENUITEMS4,
              kind=wx.ITEM_NORMAL, text='Exit')
        self.Bind(wx.EVT_MENU, self.OnOpenMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnSaveMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnCloseMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS2)
        self.Bind(wx.EVT_MENU, self.OnQuitMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS4)
        self.Bind(wx.EVT_MENU, self.OnNewMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS5)
        self.Bind(wx.EVT_MENU, self.OnSaveAsMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS6)
        self.Bind(wx.EVT_MENU, self.OnImportEDSMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS7)
        self.Bind(wx.EVT_MENU, self.OnExportCMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS8)
        self.Bind(wx.EVT_MENU, self.OnExportEDSMenu,
              id=ID_OBJDICTEDITFILEMENUITEMS9)

    def _init_coll_AddMenu_Items(self, parent):
        parent.Append(help='', id=ID_OBJDICTEDITADDMENUITEMS0,
              kind=wx.ITEM_NORMAL, text='SDO Server')
        parent.Append(help='', id=ID_OBJDICTEDITADDMENUITEMS1,
              kind=wx.ITEM_NORMAL, text='SDO Client')
        parent.Append(help='', id=ID_OBJDICTEDITADDMENUITEMS2,
              kind=wx.ITEM_NORMAL, text='PDO Transmit')
        parent.Append(help='', id=ID_OBJDICTEDITADDMENUITEMS3,
              kind=wx.ITEM_NORMAL, text='PDO Receive')
        parent.Append(help='', id=ID_OBJDICTEDITADDMENUITEMS4,
              kind=wx.ITEM_NORMAL, text='Map Variable')
        parent.Append(help='', id=ID_OBJDICTEDITADDMENUITEMS5,
              kind=wx.ITEM_NORMAL, text='User Type')
        self.Bind(wx.EVT_MENU, self.OnAddSDOServerMenu,
              id=ID_OBJDICTEDITADDMENUITEMS0)
        self.Bind(wx.EVT_MENU, self.OnAddSDOClientMenu,
              id=ID_OBJDICTEDITADDMENUITEMS1)
        self.Bind(wx.EVT_MENU, self.OnAddPDOTransmitMenu,
              id=ID_OBJDICTEDITADDMENUITEMS2)
        self.Bind(wx.EVT_MENU, self.OnAddPDOReceiveMenu,
              id=ID_OBJDICTEDITADDMENUITEMS3)
        self.Bind(wx.EVT_MENU, self.OnAddMapVariableMenu,
              id=ID_OBJDICTEDITADDMENUITEMS4)
        self.Bind(wx.EVT_MENU, self.OnAddUserTypeMenu,
              id=ID_OBJDICTEDITADDMENUITEMS5)

    def _init_coll_HelpBar_Fields(self, parent):
        parent.SetFieldsCount(3)

        parent.SetStatusText(number=0, text='')
        parent.SetStatusText(number=1, text='')
        parent.SetStatusText(number=2, text='')

        parent.SetStatusWidths([100, 110, -1])

    def _init_utils(self):
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
        wx.Frame.__init__(self, id=ID_OBJDICTEDIT, name='objdictedit',
              parent=prnt, pos=wx.Point(149, 178), size=wx.Size(1000, 700),
              style=wx.DEFAULT_FRAME_STYLE, title='Objdictedit')
        self._init_utils()
        self.SetClientSize(wx.Size(1000, 700))
        self.SetMenuBar(self.menuBar1)
        self.Bind(wx.EVT_CLOSE, self.OnCloseFrame, id=ID_OBJDICTEDIT)

        self.FileOpened = wx.Notebook(id=ID_OBJDICTEDITFILEOPENED,
              name='FileOpened', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=0)
        self.FileOpened.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED,
              self.OnFileSelectedChanged, id=ID_OBJDICTEDITFILEOPENED)

        self.HelpBar = wx.StatusBar(id=ID_OBJDICTEDITHELPBAR, name='HelpBar',
              parent=self, style=wx.ST_SIZEGRIP)
        self._init_coll_HelpBar_Fields(self.HelpBar)
        self.SetStatusBar(self.HelpBar)

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.HtmlFrameOpened = []
        self.ModeSolo = True
        
        self.Manager = NodeManager()
        for filepath in filesOpen:
            result = self.Manager.OpenFileInCurrent(filepath)
            if type(result) == IntType:
                new_editingpanel = EditingPanel(self, self.Manager)
                new_editingpanel.SetIndex(result)
                self.FileOpened.AddPage(new_editingpanel, "")
            window = self.FileOpened.GetPage(0)
            if window:
                self.Manager.ChangeCurrentNode(window.GetIndex())
                self.FileOpened.SetSelection(0)
        if self.Manager.CurrentDS302Defined():
            self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS8, True)
        else:
            self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS8, False)
        self.RefreshEditMenu()
        self.RefreshBufferState()
        self.RefreshProfileMenu()
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
        if selected >= 0:
            window = self.FileOpened.GetPage(selected)
            if window:
                self.Manager.ChangeCurrentNode(window.GetIndex())
                self.RefreshBufferState()
                self.RefreshStatusBar()
                self.RefreshProfileMenu()
        event.Skip()

    def OnHelpDS301Menu(self, event):
        find_index = False
        selected = self.FileOpened.GetSelection()
        if selected >= 0:
            window = self.FileOpened.GetPage(selected)
            result = window.GetSelection()
            if result:
                find_index = True
                index, subIndex = result
                result = OpenPDFDocIndex(index, ScriptDirectory)
                if type(result) == StringType:
                    message = wx.MessageDialog(self, result, "ERROR", wx.OK|wx.ICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
        if not find_index:
            result = OpenPDFDocIndex(None, ScriptDirectory)
            if type(result) == StringType:
                message = wx.MessageDialog(self, result, "ERROR", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
        event.Skip()
        
    def OnHelpCANFestivalMenu(self, event):
        #self.OpenHtmlFrame("CAN Festival Reference", os.path.join(ScriptDirectory, "doc/canfestival.html"), wx.Size(1000, 600))
        os.system("xpdf -remote CANFESTIVAL %s %d &"%(os.path.join(ScriptDirectory, "doc/manual_en.pdf"),16))
        event.Skip()

    def OnAboutMenu(self, event):
        self.OpenHtmlFrame("About CAN Festival", os.path.join(ScriptDirectory, "doc/about.html"), wx.Size(500, 450))
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
            dialog = wx.MessageDialog(self, "There are changes, do you want to save?",  "Close Application", wx.YES_NO|wx.CANCEL|wx.ICON_QUESTION)
            answer = dialog.ShowModal()
            dialog.Destroy()
            if answer == wx.ID_YES:
                self.Manager.ChangeCurrentNode(0)
                for i in xrange(self.FileOpened.GetPageCount()):
                    window = self.FileOpened.GetPage(i)
                    self.Manager.ChangeCurrentNode(window.GetIndex())
                    if self.Manager.CurrentIsSaved():
                        self.Manager.CloseCurrent()
                    else:
                        self.Save()
                        self.Manager.CloseCurrent(True)
                event.Skip()
            elif answer == wx.ID_NO:
                for i in xrange(self.FileOpened.GetPageCount()):
                    self.Manager.CloseCurrent(True)
                wx.CallAfter(self.Close)
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
        if self.HelpBar:
            selected = self.FileOpened.GetSelection()
            if selected >= 0:
                window = self.FileOpened.GetPage(selected)
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
        if self.FileMenu:
            if self.FileOpened.GetPageCount() > 0:
                self.menuBar1.EnableTop(1, True)
                self.menuBar1.EnableTop(2, True)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS1, True)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS2, True)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS6, True)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS8, True)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS9, True)
            else:
                self.menuBar1.EnableTop(1, False)
                self.menuBar1.EnableTop(2, False)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS1, False)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS2, False)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS6, False)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS8, False)
                self.FileMenu.Enable(ID_OBJDICTEDITFILEMENUITEMS9, False)

    def RefreshEditMenu(self):
        if self.FileMenu:
            if self.FileOpened.GetPageCount() > 0:
                undo, redo = self.Manager.GetCurrentBufferState()
                self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS1, undo)
                self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS0, redo)
            else:
                self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS1, False)
                self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS0, False)

    def RefreshProfileMenu(self):
        if self.EditMenu:
            profile = self.Manager.GetCurrentProfileName()
            edititem = self.EditMenu.FindItemById(ID_OBJDICTEDITEDITMENUITEMS7)
            if edititem:
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
        if dialog.ShowModal() == wx.ID_OK:
            name, id, nodetype, description = dialog.GetValues()
            profile, filepath = dialog.GetProfile()
            NMT = dialog.GetNMTManagement()
            options = dialog.GetOptions()
            result = self.Manager.CreateNewNode(name, id, nodetype, description, profile, filepath, NMT, options)
            if type(result) == IntType:
                new_editingpanel = EditingPanel(self, self.Manager)
                new_editingpanel.SetIndex(result)
                self.FileOpened.AddPage(new_editingpanel, "")
                self.FileOpened.SetSelection(self.FileOpened.GetPageCount() - 1)
                self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS8, False)
                if "DS302" in options:
                    self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS8, True)
                self.RefreshBufferState()
                self.RefreshProfileMenu()
                self.RefreshMainMenu()
            else:
                message = wx.MessageDialog(self, result, "ERROR", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
        event.Skip()

    def OnOpenMenu(self, event):
        filepath = self.Manager.GetCurrentFilePath()
        if filepath != "":
            directory = os.path.dirname(filepath)
        else:
            directory = os.getcwd()
        dialog = wx.FileDialog(self, "Choose a file", directory, "",  "OD files (*.od)|*.od|All files|*.*", wx.OPEN|wx.CHANGE_DIR)
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if os.path.isfile(filepath):
                result = self.Manager.OpenFileInCurrent(filepath)
                if type(result) == IntType:
                    new_editingpanel = EditingPanel(self, self.Manager)
                    new_editingpanel.SetIndex(result)
                    self.FileOpened.AddPage(new_editingpanel, "")
                    self.FileOpened.SetSelection(self.FileOpened.GetPageCount() - 1)
                    if self.Manager.CurrentDS302Defined(): 
                        self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS8, True)
                    else:
                        self.EditMenu.Enable(ID_OBJDICTEDITEDITMENUITEMS8, False)
                    self.RefreshEditMenu()
                    self.RefreshBufferState()
                    self.RefreshProfileMenu()
                    self.RefreshMainMenu()
                else:
                    message = wx.MessageDialog(self, e.args[0], "Error", wx.OK|wx.ICON_ERROR)
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
            message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
            message.ShowModal()
            message.Destroy()

    def SaveAs(self):
        filepath = self.Manager.GetCurrentFilePath()
        if filepath != "":
            directory, filename = os.path.split(filepath)
        else:
            directory, filename = os.getcwd(), "%s.od"%self.Manager.GetCurrentNodeInfos()[0]
        dialog = wx.FileDialog(self, "Choose a file", directory, filename,  "OD files (*.od)|*.od|All files|*.*", wx.SAVE|wx.OVERWRITE_PROMPT|wx.CHANGE_DIR)
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if os.path.isdir(os.path.dirname(filepath)):
                result = self.Manager.SaveCurrentInFile(filepath)
                if type(result) != StringType:
                    self.RefreshBufferState()
                else:
                    message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wx.MessageDialog(self, "%s is not a valid folder!"%os.path.dirname(filepath), "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()

    def OnCloseMenu(self, event):
        answer = wx.ID_YES
        result = self.Manager.CloseCurrent()
        if not result:
            dialog = wx.MessageDialog(self, "There are changes, do you want to save?",  "Close File", wx.YES_NO|wx.CANCEL|wx.ICON_QUESTION)
            answer = dialog.ShowModal()
            dialog.Destroy()
            if answer == wx.ID_YES:
                self.OnSaveMenu(event)
                if self.Manager.CurrentIsSaved():
                    self.Manager.CloseCurrent()
            elif answer == wx.ID_NO:
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

    def OnImportEDSMenu(self, event):
        dialog = wx.FileDialog(self, "Choose a file", os.getcwd(), "",  "EDS files (*.eds)|*.eds|All files|*.*", wx.OPEN|wx.CHANGE_DIR)
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if os.path.isfile(filepath):
                result = self.Manager.ImportCurrentFromEDSFile(filepath)
                if type(result) == IntType:
                    new_editingpanel = EditingPanel(self, self.Manager)
                    new_editingpanel.SetIndex(result)
                    self.FileOpened.AddPage(new_editingpanel, "")
                    self.FileOpened.SetSelection(self.FileOpened.GetPageCount() - 1)
                    self.RefreshBufferState()
                    self.RefreshCurrentIndexList()
                    self.RefreshProfileMenu()
                    self.RefreshMainMenu()
                    message = wx.MessageDialog(self, "Import successful", "Information", wx.OK|wx.ICON_INFORMATION)
                    message.ShowModal()
                    message.Destroy()
                else:
                    message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wx.MessageDialog(self, "\"%s\" is not a valid file!"%filepath, "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()
        event.Skip()


    def OnExportEDSMenu(self, event):
        dialog = wx.FileDialog(self, "Choose a file", os.getcwd(), self.Manager.GetCurrentNodeInfos()[0], "EDS files (*.eds)|*.eds|All files|*.*", wx.SAVE|wx.OVERWRITE_PROMPT|wx.CHANGE_DIR)
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if os.path.isdir(os.path.dirname(filepath)):
                path, extend = os.path.splitext(filepath)
                if extend in ("", "."):
                    filepath = path + ".eds"
                result = self.Manager.ExportCurrentToEDSFile(filepath)
                if not result:
                    message = wx.MessageDialog(self, "Export successful", "Information", wx.OK|wx.ICON_INFORMATION)
                    message.ShowModal()
                    message.Destroy()
                else:
                    message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wx.MessageDialog(self, "\"%s\" is not a valid folder!"%os.path.dirname(filepath), "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()
        event.Skip()

    def OnExportCMenu(self, event):
        dialog = wx.FileDialog(self, "Choose a file", os.getcwd(), self.Manager.GetCurrentNodeInfos()[0],  "CANFestival C files (*.c)|*.c|All files|*.*", wx.SAVE|wx.OVERWRITE_PROMPT|wx.CHANGE_DIR)
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if os.path.isdir(os.path.dirname(filepath)):
                path, extend = os.path.splitext(filepath)
                if extend in ("", "."):
                    filepath = path + ".c"
                result = self.Manager.ExportCurrentToCFile(filepath)
                if not result:
                    message = wx.MessageDialog(self, "Export successful", "Information", wx.OK|wx.ICON_INFORMATION)
                    message.ShowModal()
                    message.Destroy()
                else:
                    message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            else:
                message = wx.MessageDialog(self, "\"%s\" is not a valid folder!"%os.path.dirname(filepath), "Error", wx.OK|wx.ICON_ERROR)
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
        if dialog.ShowModal() == wx.ID_OK:
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
        name, id, type, description = self.Manager.GetCurrentNodeInfos()
        dialog.SetValues(name, id, type, description)
        if dialog.ShowModal() == wx.ID_OK:
            name, id, type, description = dialog.GetValues()
            self.Manager.SetCurrentNodeInfos(name, id, type, description)
            self.RefreshBufferState()
            self.RefreshProfileMenu()
            selected = self.FileOpened.GetSelection()
            if selected >= 0:
                window = self.FileOpened.GetPage(selected)
                window.RefreshTable()
        event.Skip()


#-------------------------------------------------------------------------------
#                           Add User Types and Variables
#-------------------------------------------------------------------------------
        
    def AddMapVariable(self):
        index = self.Manager.GetCurrentNextMapIndex()
        if index:
            dialog = MapVariableDialog(self)
            dialog.SetIndex(index)
            if dialog.ShowModal() == wx.ID_OK:
                index, name, struct, number = dialog.GetValues()
                result = self.Manager.AddMapVariableToCurrent(index, name, struct, number)
                if type(result) != StringType:
                    self.RefreshBufferState()
                    self.RefreshCurrentIndexList()
                else:
                    message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
            dialog.Destroy()
        else:
            message = wx.MessageDialog(self, result, "No map variable index left!", wx.OK|wx.ICON_ERROR)
            message.ShowModal()
            message.Destroy()
        
    def AddUserType(self):
        dialog = UserTypeDialog(self)
        dialog.SetTypeList(self.Manager.GetCustomisableTypes())
        if dialog.ShowModal() == wx.ID_OK:
            type, min, max, length = dialog.GetValues()
            result = self.Manager.AddUserTypeToCurrent(type, min, max, length)
            if not result:
                self.RefreshBufferState()
                self.RefreshCurrentIndexList()
            else:
                message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
        dialog.Destroy()
    

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
    message = wx.MessageDialog(None, str(e_value), "Error", wx.OK|wx.ICON_ERROR)
    message.ShowModal()
    message.Destroy()

def get_last_traceback(tb):
    while tb.tb_next:
        tb = tb.tb_next
    return tb


def format_namespace(d, indent='    '):
    return '\n'.join(['%s%s: %s' % (indent, k, repr(v)[:10000]) for k, v in d.iteritems()])


ignored_exceptions = [] # a problem with a line in a module is only reported once per session

def AddExceptHook(path, app_version='[No version]'):#, ignored_exceptions=[]):
    
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
    app = wx.PySimpleApp()
    wx.InitAllImageHandlers()
    
    # Install a exception handle for bug reports
    AddExceptHook(os.getcwd(),__version__)
    
    frame = objdictedit(None)

    frame.Show()
    app.MainLoop()
