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
from wxPython.grid import *
import wx

import os

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
              label='Possible Profile Indexes:', name='staticText1',
              parent=self.MainPanel, pos=wx.Point(40, 24), size=wx.Size(156,
              17), style=0)

        self.staticText2 = wx.StaticText(id=wxID_COMMUNICATIONDIALOGSTATICTEXT2,
              label='Current Profile Indexes:', name='staticText2',
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
              label='Index:', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.Index = wx.TextCtrl(id=wxID_MAPVARIABLEDIALOGINDEX, name='Index',
              parent=self.MainPanel, pos=wx.Point(24, 48), size=wx.Size(152,
              25), style=0, value='0x2000')

        self.staticText3 = wx.StaticText(id=wxID_MAPVARIABLEDIALOGSTATICTEXT3,
              label='Name:', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(47, 17), style=0)

        self.IndexName = wx.TextCtrl(id=wxID_MAPVARIABLEDIALOGINDEXNAME,
              name='IndexName', parent=self.MainPanel, pos=wx.Point(24, 104),
              size=wx.Size(152, 24), style=0, value='Undefined')

        self.staticText2 = wx.StaticText(id=wxID_MAPVARIABLEDIALOGSTATICTEXT2,
              label='Type:', name='staticText2', parent=self.MainPanel,
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
              label='Number:', name='staticText4', parent=self.MainPanel,
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
        
        EVT_BUTTON(self, self.ButtonSizer.GetAffirmativeButton().GetId(), self.OnOK)

    def SetIndex(self, index):
        self.Index.SetValue("0x%04X"%index)

    def OnOK(self, event):
        error = []
        try:
            int(self.Index.GetValue(), 16)
        except:
            error.append("Index")
        if self.radioButton2.GetValue() or self.radioButton3.GetValue():
            try:
                int(self.Number.GetValue())
            except:
                error.append("Number")
        if len(error) > 0:
            text = ""
            if len(error) > 1:
                suffix = "s"
            else:
                suffix = ""
            for i, item in enumerate(error):
                if i == 0:
                    text += item
                elif i == len(error) - 1:
                    text += " and %s"%item
                else:
                    text += ", %s"%item
            message = wxMessageDialog(self, "Form isn't valid. %s must be integer%s!"%(text,suffix), "Error", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()
        else:
            self.EndModal(wxID_OK)

    def GetValues(self):
        name = self.IndexName.GetValue()
        index = int(self.Index.GetValue(), 16)
        if self.radioButton1.GetValue():
            struct = 1
            number = None
        elif self.radioButton2.GetValue():
            struct = 3
            number = int(self.Number.GetValue())
        elif self.radioButton3.GetValue():
            struct = 7
            number = int(self.Number.GetValue())
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
              label='Type:', name='staticText1', parent=self.MainPanel,
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
              label='Minimum:', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(216, 48), size=wx.Size(67, 17), style=0)

        self.Min = wx.TextCtrl(id=wxID_USERTYPEDIALOGMIN, name='Min',
              parent=self.MainPanel, pos=wx.Point(296, 48), size=wx.Size(112,
              24), style=wx.TE_RIGHT, value='0')

        self.staticText3 = wx.StaticText(id=wxID_USERTYPEDIALOGSTATICTEXT3,
              label='Maximum:', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(216, 88), size=wx.Size(71, 17), style=0)

        self.Max = wx.TextCtrl(id=wxID_USERTYPEDIALOGMAX, name='Max',
              parent=self.MainPanel, pos=wx.Point(296, 88), size=wx.Size(112,
              25), style=wx.TE_RIGHT, value='0')

        self.staticText4 = wx.StaticText(id=wxID_USERTYPEDIALOGSTATICTEXT4,
              label='Length:', name='staticText4', parent=self.MainPanel,
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

        EVT_BUTTON(self, self.ButtonSizer.GetAffirmativeButton().GetId(), self.OnOK)

    def OnOK(self, event):
        error = []
        good = True
        firstmessage = ""
        secondmessage = ""
        name = self.Type.GetStringSelection()
        if name != "":
            valuetype = self.TypeDictionary[name][1]
            if valuetype == 0:
                try:
                    int(self.Min.GetValue(), 16)
                except:
                    error.append("Minimum")
                    good = False
                try:
                    int(self.Max.GetValue(), 16)
                except:
                    error.append("Maximum")
                    good = False
            elif valuetype == 1:
                try:
                    int(self.Length.GetValue(), 16)
                except:
                    error.append("Length")
                    good = False
            if len(error) > 0:
                secondmessage = ". "
                for i, item in enumerate(error):
                    if i == 0:
                        secondmessage += item
                    elif i == len(error) - 1:
                        secondmessage += " and %s"%item
                    else:
                        secondmessage += ", %s"%item
                secondmessage += " must be integer"
                if len(error) > 1:
                    secondmessage += "s"
        else:
            firstmessage = ". A type must be selected"
            good = False
        if not good:
            message = wxMessageDialog(self, "Form isn't valid%s%s%s!"%(firstmessage,secondmessage), "Error", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()
            self.Name.SetFocus()
        else:
            self.EndModal(wxID_OK)

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
        min = int(self.Min.GetValue())
        max = int(self.Max.GetValue())
        length = int(self.Length.GetValue())
        return type, min, max, length



#-------------------------------------------------------------------------------
#                          Editing Node Infos Dialog
#-------------------------------------------------------------------------------


[wxID_NODEINFOSDIALOG, wxID_NODEINFOSDIALOGMAINPANEL, 
 wxID_NODEINFOSDIALOGNAME, wxID_NODEINFOSDIALOGNODEID, 
 wxID_NODEINFOSDIALOGDESCRIPTION, wxID_NODEINFOSDIALOGSTATICTEXT1, 
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
              size=wx.Size(300, 300), style=wx.DEFAULT_DIALOG_STYLE,
              title='Node Infos')
        self.SetClientSize(wx.Size(300, 300))

        self.MainPanel = wx.Panel(id=wxID_NODEINFOSDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(280, 264), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT1,
              label='Name:', 
              name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.Name = wx.TextCtrl(id=wxID_NODEINFOSDIALOGNAME, name='Name',
              parent=self.MainPanel, pos=wx.Point(24, 48), size=wx.Size(250,
              25), style=0, value='')

        self.staticText2 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT2,
              label='Node ID:', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(67, 17), style=0)

        self.NodeID = wx.TextCtrl(id=wxID_NODEINFOSDIALOGNODEID, name='NodeID',
              parent=self.MainPanel, pos=wx.Point(24, 104), size=wx.Size(250,
              25), style=wx.TE_RIGHT, value='')

        self.staticText3 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT3,
              label='Type:', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(24, 136), size=wx.Size(71, 17), style=0)

        self.Type = wx.Choice(choices=[], id=wxID_NODEINFOSDIALOGTYPE,
              name='Type', parent=self.MainPanel, pos=wx.Point(24, 160),
              size=wx.Size(250, 25), style=0)

        self.staticText4 = wx.StaticText(id=wxID_NODEINFOSDIALOGSTATICTEXT4,
              label='Description:', name='staticText4', parent=self.MainPanel,
              pos=wx.Point(24, 192), size=wx.Size(71, 17), style=0)

        self.Description = wx.TextCtrl(id=wxID_NODEINFOSDIALOGDESCRIPTION, 
              name='Description', parent=self.MainPanel, pos=wx.Point(24, 216), 
              size=wx.Size(250, 25), style=0, value='')

        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.Type.Append("master")
        self.Type.Append("slave")

        EVT_BUTTON(self, self.ButtonSizer.GetAffirmativeButton().GetId(), self.OnOK)

    def OnOK(self, event):
        name = self.Name.GetValue()
        message = ""
        if name != "":
            good = not name[0].isdigit()
            for item in name.split("_"):
                good &= item.isalnum()
            if not good:
                message = "Node name can't be undefined or start with a digit and must be composed of alphanumerical characters or underscore!"
        if message != "":
            try:
                nodeid = int(self.NodeID.GetValue(), 16)
            except:
                message = "Node ID must be integer!"
        if message != "":
            message = wxMessageDialog(self, message, "ERROR", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()
            self.Name.SetFocus()
        else:
            self.EndModal(wxID_OK)
    
    def SetValues(self, name, id, type, description):
        self.Name.SetValue(name)
        self.NodeID.SetValue("0x%02X"%id)
        self.Type.SetStringSelection(type)
        self.Description.SetValue(description)

    def GetValues(self):
        name = self.Name.GetValue()
        nodeid = int(self.NodeID.GetValue(), 16)
        type = self.Type.GetStringSelection()
        description = self.Description.GetValue()
        return name, nodeid, type, description



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
 wxID_CREATENODEDIALOGSTATICTEXT6, wxID_CREATENODEDIALOGSTATICTEXT7,
 wxID_CREATENODEDIALOGSTOREEDS, wxID_CREATENODEDIALOGDESCRIPTION,
 wxID_CREATENODEDIALOGTYPE, 
] = [wx.NewId() for _init_ctrls in range(21)]

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
              size=wx.Size(451, 376), style=wx.DEFAULT_DIALOG_STYLE,
              title='Create a new Node')
        self.SetClientSize(wx.Size(451, 376))

        self.MainPanel = wx.Panel(id=wxID_CREATENODEDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(440, 278), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT1,
              label='Name:', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(156, 17), style=0)

        self.staticText2 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT2,
              label='Node ID:', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(67, 17), style=0)

        self.staticText3 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT3,
              label='Type:', name='staticText3', parent=self.MainPanel,
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
              label='Profile:', name='staticText4', parent=self.MainPanel,
              pos=wx.Point(24, 192), size=wx.Size(47, 17), style=0)

        self.Profile = wx.Choice(choices=[], id=wxID_CREATENODEDIALOGPROFILE,
              name='Profile', parent=self.MainPanel, pos=wx.Point(24, 216),
              size=wx.Size(200, 24), style=0)
        self.Profile.Bind(wx.EVT_CHOICE, self.OnProfileChoice,
              id=wxID_CREATENODEDIALOGPROFILE)

        self.staticText5 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT5,
              label='Network Management:', name='staticText5',
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
              label='Options:', name='staticText6', parent=self.MainPanel,
              pos=wx.Point(256, 128), size=wx.Size(72, 17), style=0)

        self.DS302 = wx.CheckBox(id=wxID_CREATENODEDIALOGGENSYNC,
              label='DS-302 Profile', name='DS302', parent=self.MainPanel,
              pos=wx.Point(256, 144), size=wx.Size(128, 24), style=0)
        self.DS302.SetValue(False)
        #self.DS302.Enable(False)

        self.GenSYNC = wx.CheckBox(id=wxID_CREATENODEDIALOGGENSYNC,
              label='Generate SYNC', name='GenSYNC', parent=self.MainPanel,
              pos=wx.Point(256, 168), size=wx.Size(128, 24), style=0)
        self.GenSYNC.SetValue(False)

        self.Emergency = wx.CheckBox(id=wxID_CREATENODEDIALOGEMERGENCY,
              label='Emergency support', name='Emergency',
              parent=self.MainPanel, pos=wx.Point(256, 192), size=wx.Size(152,
              24), style=0)
        self.Emergency.SetValue(False)
        self.Emergency.Enable(False)

        self.SaveConfig = wx.CheckBox(id=wxID_CREATENODEDIALOGSAVECONFIG,
              label='Save Configuration', name='SaveConfig',
              parent=self.MainPanel, pos=wx.Point(256, 216), size=wx.Size(152,
              24), style=0)
        self.SaveConfig.SetValue(False)
        self.SaveConfig.Enable(False)

#        self.StoreEDS = wx.CheckBox(id=wxID_CREATENODEDIALOGSTOREEDS,
#              label='Store EDS', name='StoreEDS', parent=self.MainPanel,
#              pos=wx.Point(256, 240), size=wx.Size(144, 24), style=0)
#        self.StoreEDS.SetValue(False)

        self.staticText7 = wx.StaticText(id=wxID_CREATENODEDIALOGSTATICTEXT7,
              label='Description:', name='staticText7', parent=self.MainPanel,
              pos=wx.Point(24, 248), size=wx.Size(71, 17), style=0)

        self.Description = wx.TextCtrl(id=wxID_CREATENODEDIALOGDESCRIPTION, 
              name='Description', parent=self.MainPanel, pos=wx.Point(24, 272), 
              size=wx.Size(400, 25), style=0, value='')

        self._init_sizers()

    def __init__(self, parent, cwd):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_CENTER)
        self.NodeID.SetValue("0x00")
        self.Type.Append("master")
        self.Type.Append("slave")
        self.Type.SetStringSelection("slave")
        self.Description.SetValue("")
        self.ListProfile = {"None" : ""}
        self.Profile.Append("None")
        self.Directory = os.path.join(cwd, "config")
        listfiles = os.listdir(self.Directory)
        listfiles.sort()
        for item in listfiles:
            name, extend = os.path.splitext(item)
            if os.path.isfile(os.path.join(self.Directory, item)) and extend == ".prf" and name != "DS-302":
                self.ListProfile[name] = os.path.join(self.Directory, item)
                self.Profile.Append(name)
        self.Profile.Append("Other")
        self.Profile.SetStringSelection("None")
        self.Name.SetFocus()
        
        EVT_BUTTON(self, self.ButtonSizer.GetAffirmativeButton().GetId(), self.OnOK)

    def OnOK(self, event):
        name = self.Name.GetValue()
        message = ""
        if name != "":
            good = not name[0].isdigit()
            for item in name.split("_"):
                good &= item.isalnum()
            if not good:
                message = "Node name can't be undefined or start with a digit and must be composed of alphanumerical characters or underscore!"
        if message != "":
            try:
                nodeid = int(self.NodeID.GetValue(), 16)
            except:
                message = "Node ID must be an integer!"
        if message != "":
            message = wxMessageDialog(self, message, "ERROR", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()
            self.Name.SetFocus()
        else:
            self.EndModal(wxID_OK)

    def GetValues(self):
        name = self.Name.GetValue()
        nodeid = 0
        if self.NodeID.GetValue() != "":
            nodeid = int(self.NodeID.GetValue(), 16)
        type = self.Type.GetStringSelection()
        description = self.Description.GetValue()
        return name, nodeid, type, description

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
#        if self.StoreEDS.GetValue():
#            options.append("StoreEDS")
        return options

    def OnProfileChoice(self, event):
        if self.Profile.GetStringSelection() == "Other":
            dialog = wxFileDialog(self, "Choose a file", self.Directory, "",  "OD Profile files (*.prf)|*.prf|All files|*.*", wxOPEN|wxCHANGE_DIR)
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
#                            ADD Slave to NodeList Dialog
#-------------------------------------------------------------------------------

[wxID_ADDSLAVEDIALOG, wxID_ADDSLAVEDIALOGMAINPANEL, wxID_ADDSLAVEDIALOGSLAVENAME, 
 wxID_ADDSLAVEDIALOGSLAVENODEID, wxID_ADDSLAVEDIALOGEDSFILE, 
 wxID_ADDSLAVEDIALOGIMPORTEDS, wxID_ADDSLAVEDIALOGSTATICTEXT1, 
 wxID_ADDSLAVEDIALOGSTATICTEXT2, wxID_ADDSLAVEDIALOGSTATICTEXT3, 
] = [wx.NewId() for _init_ctrls in range(9)]

class AddSlaveDialog(wx.Dialog):
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
        wx.Dialog.__init__(self, id=wxID_ADDSLAVEDIALOG,
              name='AddSlaveDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(300, 250), style=wx.DEFAULT_DIALOG_STYLE,
              title='Add a slave to nodelist')
        self.SetClientSize(wx.Size(300, 250))

        self.MainPanel = wx.Panel(id=wxID_ADDSLAVEDIALOGMAINPANEL,
              name='MainPanel', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(350, 250), style=wx.TAB_TRAVERSAL)
        self.MainPanel.SetAutoLayout(True)

        self.staticText1 = wx.StaticText(id=wxID_ADDSLAVEDIALOGSTATICTEXT1,
              label='Slave Name:', name='staticText1', parent=self.MainPanel,
              pos=wx.Point(24, 24), size=wx.Size(150, 17), style=0)

        self.SlaveName = wx.TextCtrl(id=wxID_ADDSLAVEDIALOGSLAVENAME,
              name='SlaveName', parent=self.MainPanel, pos=wx.Point(24, 48), 
              size=wx.Size(250, 24), style=0)

        self.staticText2 = wx.StaticText(id=wxID_ADDSLAVEDIALOGSTATICTEXT2,
              label='Slave Node ID:', name='staticText2', parent=self.MainPanel,
              pos=wx.Point(24, 80), size=wx.Size(150, 17), style=0)

        self.SlaveNodeID = wx.TextCtrl(id=wxID_ADDSLAVEDIALOGSLAVENODEID,
              name='SlaveName', parent=self.MainPanel, pos=wx.Point(24, 104), 
              size=wx.Size(250, 24), style=wxALIGN_RIGHT)

        self.staticText3 = wx.StaticText(id=wxID_ADDSLAVEDIALOGSTATICTEXT3,
              label='EDS File:', name='staticText3', parent=self.MainPanel,
              pos=wx.Point(24, 136), size=wx.Size(155, 17), style=0)

        self.EDSFile = wx.Choice(id=wxID_ADDSLAVEDIALOGEDSFILE,
              name='EDSFile', parent=self.MainPanel, pos=wx.Point(24, 160),
              size=wx.Size(145, 24), style=0)
        
        self.ImportEDS = wxButton(id=wxID_ADDSLAVEDIALOGIMPORTEDS, label='Import EDS',
              name='ImportEDS', parent=self.MainPanel, pos=wx.Point(174, 160),
              size=wx.Size(100, 32), style=0)
        self.ImportEDS.Bind(wx.EVT_BUTTON, self.OnImportEDSButton,
              id=wxID_ADDSLAVEDIALOGIMPORTEDS)
        
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.ButtonSizer = self.CreateButtonSizer(wxOK|wxCANCEL|wxCENTRE)
        self.flexGridSizer1.Add(self.ButtonSizer, 1, wxALIGN_RIGHT)
        
        self.SlaveNodeID.SetValue("0x00")
        
        EVT_BUTTON(self, self.ButtonSizer.GetAffirmativeButton().GetId(), self.OnOK)
    
    def OnOK(self, event):
        error = []
        if self.SlaveName.GetValue() == "":
            error.append("Slave Name")
        if self.SlaveNodeID.GetValue() == "":
            error.append("Slave Node ID")
        if self.EDSFile.GetStringSelection() == "":
            error.append("EDS File")
        if len(error) > 0:
            text = ""
            for i, item in enumerate(error):
                if i == 0:
                    text += item
                elif i == len(error) - 1:
                    text += " and %s"%item
                else:
                    text += ", %s"%item 
            message = wxMessageDialog(self, "Form isn't complete. %s must be filled!"%text, "Error", wxOK|wxICON_ERROR)
            message.ShowModal()
            message.Destroy()
        else:
            try:
                nodeid = self.SlaveNodeID.GetValue()
                if nodeid.find("x") != -1:
                    nodeid = int(nodeid, 16)
                else:
                    nodeid = int(nodeid)
            except:
                message = wxMessageDialog(self, "Slave Node ID must be a value in decimal or hexadecimal!", "Error", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
                return
            if not 0 <= nodeid <= 127:
                message = wxMessageDialog(self, "Slave Node ID must be between 0 and 127!", "Error", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
            elif nodeid == 0 or nodeid in self.NodeList.SlaveNodes.keys():
                message = wxMessageDialog(self, "A Node with this ID already exist in the network!", "Error", wxOK|wxICON_ERROR)
                message.ShowModal()
                message.Destroy()
            else:
                self.EndModal(wxID_OK)

    def OnImportEDSButton(self, event):
        dialog = wxFileDialog(self, "Choose an EDS file", os.getcwd(), "",  "EDS files (*.eds)|*.eds|All files|*.*", wxOPEN|wxCHANGE_DIR)
        if dialog.ShowModal() == wxID_OK:
            filepath = dialog.GetPath()
            if os.path.isfile(filepath):
                result = self.NodeList.ImportEDSFile(filepath)
                if result:
                    message = wxMessageDialog(self, result, "Error", wxOK|wxICON_ERROR)
                    message.ShowModal()
                    message.Destroy()
        dialog.Destroy()
        self.RefreshEDSFile()
        event.Skip()

    def RefreshEDSFile(self):
        selection = self.EDSFile.GetStringSelection()
        self.EDSFile.Clear()
        for option in self.NodeList.EDSNodes.keys():
            self.EDSFile.Append(option)
        if self.EDSFile.FindString(selection) != wxNOT_FOUND:
            self.EDSFile.SetStringSelection(selection)
    
    def SetNodeList(self, nodelist):
        self.NodeList = nodelist
        self.RefreshEDSFile()
    
    def GetValues(self):
        values = {}
        values["slaveName"] = self.SlaveName.GetValue()
        nodeid = self.SlaveNodeID.GetValue()
        if nodeid.find("x") != -1:
            values["slaveNodeID"] = int(nodeid, 16)
        else:
            values["slaveNodeID"] = int(nodeid)
        values["edsFile"] = self.EDSFile.GetStringSelection()
        return values
