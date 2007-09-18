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

import os

ScriptDirectory = os.path.split(__file__)[0]

#-------------------------------------------------------------------------------
#                          Editing Communication Dialog
#-------------------------------------------------------------------------------


[ID_COMMUNICATIONDIALOG, ID_COMMUNICATIONDIALOGPOSSIBLEINDEXES, 
 ID_COMMUNICATIONDIALOGCURRENTINDEXES, ID_COMMUNICATIONDIALOGSELECT, 
 ID_COMMUNICATIONDIALOGUNSELECT, ID_COMMUNICATIONDIALOGSTATICTEXT1, 
 ID_COMMUNICATIONDIALOGSTATICTEXT2
] = [wx.NewId() for _init_ctrls in range(7)]

class CommunicationDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        parent.AddSizer(self.MainSizer, 0, border=20, flag=wx.GROW|wx.TOP|wx.LEFT|wx.RIGHT)
        parent.AddSizer(self.ButtonSizer, 0, border=20, flag=wx.ALIGN_RIGHT|wx.BOTTOM|wx.LEFT|wx.RIGHT)

    def _init_coll_flexGridSizer1_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_MainSizer_Items(self, parent):
        parent.AddSizer(self.LeftGridSizer, 0, border=0, flag=wx.GROW)
        parent.AddSizer(self.MiddleGridSizer, 0, border=0, flag=wx.GROW)
        parent.AddSizer(self.RightGridSizer, 0, border=0, flag=wx.GROW)

    def _init_coll_MainSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableCol(2)
        parent.AddGrowableRow(0)

    def _init_coll_LeftGridSizer_Items(self, parent):
        parent.AddWindow(self.staticText1, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.PossibleIndexes, 0, border=0, flag=wx.GROW)

    def _init_coll_LeftGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(1)

    def _init_coll_MiddleGridSizer_Items(self, parent):
        parent.AddWindow(self.Select, 0, border=0, flag=wx.ALIGN_BOTTOM)
        parent.AddWindow(self.Unselect, 0, border=0, flag=wx.ALIGN_TOP)

    def _init_coll_MiddleGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)
        parent.AddGrowableRow(1)

    def _init_coll_RightGridSizer_Items(self, parent):
        parent.AddWindow(self.staticText2, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.CurrentIndexes, 0, border=0, flag=wx.GROW)

    def _init_coll_RightGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(1)

    def _init_sizers(self):
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=10)
        self.MainSizer = wx.FlexGridSizer(cols=3, hgap=15, rows=1, vgap=0)
        self.LeftGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=5)
        self.MiddleGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=40)
        self.RightGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=5)
        
        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)
        self._init_coll_flexGridSizer1_Growables(self.flexGridSizer1)
        self._init_coll_MainSizer_Items(self.MainSizer)
        self._init_coll_MainSizer_Growables(self.MainSizer)
        self._init_coll_LeftGridSizer_Items(self.LeftGridSizer)
        self._init_coll_LeftGridSizer_Growables(self.LeftGridSizer)
        self._init_coll_MiddleGridSizer_Items(self.MiddleGridSizer)
        self._init_coll_MiddleGridSizer_Growables(self.MiddleGridSizer)
        self._init_coll_RightGridSizer_Items(self.RightGridSizer)
        self._init_coll_RightGridSizer_Growables(self.RightGridSizer)
        
        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        wx.Dialog.__init__(self, id=ID_COMMUNICATIONDIALOG,
              name='CommunicationDialog', parent=prnt, pos=wx.Point(234, 216),
              size=wx.Size(726, 437), style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER,
              title='Edit Communication Profile')
        self.SetClientSize(wx.Size(726, 437))

        self.staticText1 = wx.StaticText(id=ID_COMMUNICATIONDIALOGSTATICTEXT1,
              label='Possible Profile Indexes:', name='staticText1',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0,
              17), style=0)

        self.PossibleIndexes = wx.ListBox(choices=[],
              id=ID_COMMUNICATIONDIALOGPOSSIBLEINDEXES,
              name='PossibleIndexes', parent=self, pos=wx.Point(0, 0), 
              size=wx.Size(0, 0), style=wx.LB_EXTENDED)
        self.Bind(wx.EVT_LISTBOX_DCLICK, self.OnPossibleIndexesDClick,
              id=ID_COMMUNICATIONDIALOGPOSSIBLEINDEXES)

        self.Select = wx.Button(id=ID_COMMUNICATIONDIALOGSELECT, label='>>',
              name='Select', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(32, 32), style=0)
        self.Select.Bind(wx.EVT_BUTTON, self.OnSelectButton,
              id=ID_COMMUNICATIONDIALOGSELECT)

        self.Unselect = wx.Button(id=ID_COMMUNICATIONDIALOGUNSELECT,
              label='<<', name='Unselect', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(32, 32), style=0)
        self.Unselect.Bind(wx.EVT_BUTTON, self.OnUnselectButton,
              id=ID_COMMUNICATIONDIALOGUNSELECT)

        self.staticText2 = wx.StaticText(id=ID_COMMUNICATIONDIALOGSTATICTEXT2,
              label='Current Profile Indexes:', name='staticText2',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0,
              17), style=0)

        self.CurrentIndexes = wx.ListBox(choices=[],
              id=ID_COMMUNICATIONDIALOGCURRENTINDEXES, name='CurrentIndexes',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 0), 
              style=wx.LB_EXTENDED)
        self.Bind(wx.EVT_LISTBOX_DCLICK, self.OnCurrentIndexesDClick,
              id=ID_COMMUNICATIONDIALOGCURRENTINDEXES)

        self.ButtonSizer = self.CreateButtonSizer(wx.OK|wx.CANCEL)
        
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
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


[ID_MAPVARIABLEDIALOG, ID_MAPVARIABLEDIALOGINDEX, 
 ID_MAPVARIABLEDIALOGINDEXNAME, ID_MAPVARIABLEDIALOGSPACER, 
 ID_MAPVARIABLEDIALOGSPACER2, ID_MAPVARIABLEDIALOGNUMBER, 
 ID_MAPVARIABLEDIALOGRADIOBUTTON1, ID_MAPVARIABLEDIALOGRADIOBUTTON2, 
 ID_MAPVARIABLEDIALOGRADIOBUTTON3, ID_MAPVARIABLEDIALOGSTATICTEXT1, 
 ID_MAPVARIABLEDIALOGSTATICTEXT2, ID_MAPVARIABLEDIALOGSTATICTEXT3, 
 ID_MAPVARIABLEDIALOGSTATICTEXT4, 
] = [wx.NewId() for _init_ctrls in range(13)]

class MapVariableDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        parent.AddSizer(self.MainSizer, 0, border=20, flag=wx.GROW|wx.TOP|wx.LEFT|wx.RIGHT)
        parent.AddSizer(self.ButtonSizer, 0, border=20, flag=wx.ALIGN_RIGHT|wx.BOTTOM|wx.LEFT|wx.RIGHT)

    def _init_coll_flexGridSizer1_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_MainSizer_Items(self, parent):
        parent.AddWindow(self.staticText1, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText2, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Spacer, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Index, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.radioButton1, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Spacer2, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText3, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.radioButton2, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText4, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.IndexName, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.radioButton3, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Number, 0, border=0, flag=wx.GROW)
        
    def _init_coll_MainSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableCol(2)

    def _init_sizers(self):
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=10)
        self.MainSizer = wx.FlexGridSizer(cols=3, hgap=30, rows=4, vgap=5)
        
        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)
        self._init_coll_flexGridSizer1_Growables(self.flexGridSizer1)
        self._init_coll_MainSizer_Items(self.MainSizer)
        self._init_coll_MainSizer_Growables(self.MainSizer)
        
        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        wx.Dialog.__init__(self, id=ID_MAPVARIABLEDIALOG,
              name='CommunicationDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(444, 186), style=wx.DEFAULT_DIALOG_STYLE,
              title='Add Map Variable')
        self.SetClientSize(wx.Size(444, 186))

        self.staticText1 = wx.StaticText(id=ID_MAPVARIABLEDIALOGSTATICTEXT1,
              label='Index:', name='staticText1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.staticText2 = wx.StaticText(id=ID_MAPVARIABLEDIALOGSTATICTEXT2,
              label='Type:', name='staticText2', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.staticText3 = wx.StaticText(id=ID_MAPVARIABLEDIALOGSTATICTEXT3,
              label='Name:', name='staticText3', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.staticText4 = wx.StaticText(id=ID_MAPVARIABLEDIALOGSTATICTEXT4,
              label='Number:', name='staticText4', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 16), style=0)

        self.radioButton1 = wx.RadioButton(id=ID_MAPVARIABLEDIALOGRADIOBUTTON1,
              label='VAR', name='radioButton1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(80, 24), style=wx.RB_GROUP)
        self.radioButton1.SetValue(True)
        self.radioButton1.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton1Click,
              id=ID_MAPVARIABLEDIALOGRADIOBUTTON1)

        self.radioButton2 = wx.RadioButton(id=ID_MAPVARIABLEDIALOGRADIOBUTTON2,
              label='ARRAY', name='radioButton2', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(80, 24), style=wx.RB_SINGLE)
        self.radioButton2.SetValue(False)
        self.radioButton2.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton2Click,
              id=ID_MAPVARIABLEDIALOGRADIOBUTTON2)

        self.radioButton3 = wx.RadioButton(id=ID_MAPVARIABLEDIALOGRADIOBUTTON3,
              label='REC', name='radioButton3', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(80, 24), style=wx.RB_SINGLE)
        self.radioButton3.SetValue(False)
        self.radioButton3.Bind(wx.EVT_RADIOBUTTON, self.OnRadioButton3Click,
              id=ID_MAPVARIABLEDIALOGRADIOBUTTON3)

        self.Index = wx.TextCtrl(id=ID_MAPVARIABLEDIALOGINDEX, name='Index',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 25), 
              style=0, value='0x2000')
        
        self.IndexName = wx.TextCtrl(id=ID_MAPVARIABLEDIALOGINDEXNAME,
              name='IndexName', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 24), style=0, value='Undefined')
        
        self.Number = wx.TextCtrl(id=ID_MAPVARIABLEDIALOGNUMBER,
              name='Number', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 24), style=wx.TE_RIGHT, value='0')
        
        self.Spacer = wx.Panel(id=ID_MAPVARIABLEDIALOGSPACER,
              name='Spacer', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=wx.TAB_TRAVERSAL)
        
        self.Spacer2 = wx.Panel(id=ID_MAPVARIABLEDIALOGSPACER2,
              name='Spacer2', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=wx.TAB_TRAVERSAL)
        
        self.ButtonSizer = self.CreateButtonSizer(wx.OK|wx.CANCEL)
        self.Bind(wx.EVT_BUTTON, self.OnOK, id=self.ButtonSizer.GetAffirmativeButton().GetId())
        
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        self.staticText4.Enable(False)
        self.Number.Enable(False)
        
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
            message = wx.MessageDialog(self, "Form isn't valid. %s must be integer%s!"%(text,suffix), "Error", wx.OK|wx.ICON_ERROR)
            message.ShowModal()
            message.Destroy()
        else:
            self.EndModal(wx.ID_OK)

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


[ID_USERTYPEDIALOG, ID_USERTYPEDIALOGSPACER, 
 ID_USERTYPEDIALOGTYPE, ID_USERTYPEDIALOGMAX, 
 ID_USERTYPEDIALOGMIN, ID_USERTYPEDIALOGLENGTH, 
 ID_USERTYPEDIALOGSTATICBOX1, ID_USERTYPEDIALOGSTATICTEXT1, 
 ID_USERTYPEDIALOGSTATICTEXT2, ID_USERTYPEDIALOGSTATICTEXT3, 
 ID_USERTYPEDIALOGSTATICTEXT4,  
] = [wx.NewId() for _init_ctrls in range(11)]

class UserTypeDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        parent.AddSizer(self.MainSizer, 0, border=20, flag=wx.GROW|wx.TOP|wx.LEFT|wx.RIGHT)
        parent.AddSizer(self.ButtonSizer, 0, border=20, flag=wx.ALIGN_RIGHT|wx.BOTTOM|wx.LEFT|wx.RIGHT)

    def _init_coll_flexGridSizer1_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_MainSizer_Items(self, parent):
        parent.AddSizer(self.LeftGridSizer, 2, border=5, flag=wx.GROW|wx.RIGHT)
        parent.AddSizer(self.RightBoxSizer, 3, border=5, flag=wx.GROW|wx.LEFT)

    def _init_coll_LeftGridSizer_Items(self, parent):
        parent.AddWindow(self.staticText1, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Type, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Spacer, 0, border=0, flag=wx.GROW)

    def _init_coll_LeftGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(2)

    def _init_coll_RightBoxSizer_Items(self, parent):
        parent.AddSizer(self.RightBoxGridSizer, 0, border=10, flag=wx.GROW|wx.ALL)

    def _init_coll_RightBoxGridSizer_Items(self, parent):
        parent.AddWindow(self.staticText2, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Min, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText3, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Max, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText4, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Length, 0, border=0, flag=wx.GROW)
        
    def _init_coll_RightBoxGridSizer_Growables(self, parent):
        parent.AddGrowableCol(1)
        
    def _init_sizers(self):
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=10)
        self.MainSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.LeftGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=3, vgap=5)
        self.RightBoxSizer = wx.StaticBoxSizer(self.staticBox1, wx.VERTICAL)
        self.RightBoxGridSizer = wx.FlexGridSizer(cols=2, hgap=5, rows=3, vgap=10)
        
        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)
        self._init_coll_flexGridSizer1_Growables(self.flexGridSizer1)
        self._init_coll_MainSizer_Items(self.MainSizer)
        self._init_coll_LeftGridSizer_Items(self.LeftGridSizer)
        self._init_coll_LeftGridSizer_Growables(self.LeftGridSizer)
        self._init_coll_RightBoxSizer_Items(self.RightBoxSizer)
        self._init_coll_RightBoxGridSizer_Items(self.RightBoxGridSizer)
        self._init_coll_RightBoxGridSizer_Growables(self.RightBoxGridSizer)
        
        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        wx.Dialog.__init__(self, id=ID_USERTYPEDIALOG, name='UserTypeDialog',
              parent=prnt, pos=wx.Point(376, 223), size=wx.Size(444, 210),
              style=wx.DEFAULT_DIALOG_STYLE, title='Add User Type')
        self.SetClientSize(wx.Size(444, 210))

        self.staticText1 = wx.StaticText(id=ID_USERTYPEDIALOGSTATICTEXT1,
              label='Type:', name='staticText1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.Type = wx.Choice(choices=[], id=ID_USERTYPEDIALOGTYPE,
              name='Type', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 24), style=0)
        self.Type.Bind(wx.EVT_CHOICE, self.OnTypeChoice,
              id=ID_USERTYPEDIALOGTYPE)

        self.Spacer = wx.Panel(id=ID_MAPVARIABLEDIALOGSPACER,
              name='Spacer', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=wx.TAB_TRAVERSAL)

        self.staticBox1 = wx.StaticBox(id=ID_USERTYPEDIALOGSTATICBOX1,
              label='Values', name='staticBox1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 0), style=0)

        self.staticText2 = wx.StaticText(id=ID_USERTYPEDIALOGSTATICTEXT2,
              label='Minimum:', name='staticText2', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(80, 17), style=0)

        self.Min = wx.TextCtrl(id=ID_USERTYPEDIALOGMIN, name='Min',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), 
              style=wx.TE_RIGHT, value='0')

        self.staticText3 = wx.StaticText(id=ID_USERTYPEDIALOGSTATICTEXT3,
              label='Maximum:', name='staticText3', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(80, 17), style=0)

        self.Max = wx.TextCtrl(id=ID_USERTYPEDIALOGMAX, name='Max',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), 
              style=wx.TE_RIGHT, value='0')

        self.staticText4 = wx.StaticText(id=ID_USERTYPEDIALOGSTATICTEXT4,
              label='Length:', name='staticText4', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(80, 17), style=0)

        self.Length = wx.TextCtrl(id=ID_USERTYPEDIALOGLENGTH, name='Length',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), 
              style=wx.TE_RIGHT, value='0')

        self.ButtonSizer = self.CreateButtonSizer(wx.OK|wx.CANCEL)
        self.Bind(wx.EVT_BUTTON, self.OnOK, id=self.ButtonSizer.GetAffirmativeButton().GetId())
        
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        
        self.TypeDictionary = {}

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
            message = wx.MessageDialog(self, "Form isn't valid%s%s%s!"%(firstmessage,secondmessage), "Error", wx.OK|wx.ICON_ERROR)
            message.ShowModal()
            message.Destroy()
        else:
            self.EndModal(wx.ID_OK)

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


[ID_NODEINFOSDIALOG, ID_NODEINFOSDIALOGNAME, 
 ID_NODEINFOSDIALOGNODEID, ID_NODEINFOSDIALOGDESCRIPTION, 
 ID_NODEINFOSDIALOGTYPE, ID_NODEINFOSDIALOGSTATICTEXT1, 
 ID_NODEINFOSDIALOGSTATICTEXT2, ID_NODEINFOSDIALOGSTATICTEXT3, 
 ID_NODEINFOSDIALOGSTATICTEXT4,  
] = [wx.NewId() for _init_ctrls in range(9)]

class NodeInfosDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        parent.AddSizer(self.MainSizer, 0, border=20, flag=wx.GROW|wx.TOP|wx.LEFT|wx.RIGHT)
        parent.AddSizer(self.ButtonSizer, 0, border=20, flag=wx.ALIGN_RIGHT|wx.BOTTOM|wx.LEFT|wx.RIGHT)

    def _init_coll_flexGridSizer1_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_MainSizer_Items(self, parent):
        parent.AddWindow(self.staticText1, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.NodeName, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText2, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.NodeID, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText3, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Type, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText4, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Description, 0, border=0, flag=wx.GROW)

    def _init_coll_MainSizer_Growables(self, parent):
        parent.AddGrowableCol(0)

    def _init_sizers(self):
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=10)
        self.MainSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=8, vgap=5)
        
        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)
        self._init_coll_flexGridSizer1_Growables(self.flexGridSizer1)
        self._init_coll_MainSizer_Items(self.MainSizer)
        self._init_coll_MainSizer_Growables(self.MainSizer)
        
        self.SetSizer(self.flexGridSizer1)
        
    def _init_ctrls(self, prnt):
        wx.Dialog.__init__(self, id=ID_NODEINFOSDIALOG,
              name='NodeInfosDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(300, 280), style=wx.DEFAULT_DIALOG_STYLE,
              title='Node Infos')
        self.SetClientSize(wx.Size(300, 280))

        self.staticText1 = wx.StaticText(id=ID_NODEINFOSDIALOGSTATICTEXT1,
              label='Name:', name='staticText1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.NodeName = wx.TextCtrl(id=ID_NODEINFOSDIALOGNAME, name='NodeName',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), 
              style=0, value='')

        self.staticText2 = wx.StaticText(id=ID_NODEINFOSDIALOGSTATICTEXT2,
              label='Node ID:', name='staticText2', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.NodeID = wx.TextCtrl(id=ID_NODEINFOSDIALOGNODEID, name='NodeID',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 25), 
              style=wx.TE_RIGHT, value='')

        self.staticText3 = wx.StaticText(id=ID_NODEINFOSDIALOGSTATICTEXT3,
              label='Type:', name='staticText3', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.Type = wx.Choice(choices=[], id=ID_NODEINFOSDIALOGTYPE,
              name='Type', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 25), style=0)

        self.staticText4 = wx.StaticText(id=ID_NODEINFOSDIALOGSTATICTEXT4,
              label='Description:', name='staticText4', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.Description = wx.TextCtrl(id=ID_NODEINFOSDIALOGDESCRIPTION, 
              name='Description', parent=self, pos=wx.Point(0, 0), 
              size=wx.Size(0, 24), style=0, value='')

        self.ButtonSizer = self.CreateButtonSizer(wx.OK|wx.CANCEL)
        self.Bind(wx.EVT_BUTTON, self.OnOK, id=self.ButtonSizer.GetAffirmativeButton().GetId())
        
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        
        self.Type.Append("master")
        self.Type.Append("slave")

    def OnOK(self, event):
        name = self.NodeName.GetValue()
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
            message = wx.MessageDialog(self, message, "ERROR", wx.OK|wx.ICON_ERROR)
            message.ShowModal()
            message.Destroy()
            self.NodeName.SetFocus()
        else:
            self.EndModal(wx.ID_OK)
    
    def SetValues(self, name, id, type, description):
        self.NodeName.SetValue(name)
        self.NodeID.SetValue("0x%02X"%id)
        self.Type.SetStringSelection(type)
        self.Description.SetValue(description)

    def GetValues(self):
        name = self.NodeName.GetValue()
        nodeid = int(self.NodeID.GetValue(), 16)
        type = self.Type.GetStringSelection()
        description = self.Description.GetValue()
        return name, nodeid, type, description



#-------------------------------------------------------------------------------
#                          Create New Node Dialog
#-------------------------------------------------------------------------------


[ID_CREATENODEDIALOG, ID_CREATENODEDIALOGEMERGENCY, 
 ID_CREATENODEDIALOGGENSYNC, ID_CREATENODEDIALOGSPACER, 
 ID_CREATENODEDIALOGNAME, ID_CREATENODEDIALOGNMT_HEARTBEAT, 
 ID_CREATENODEDIALOGNMT_NODEGUARDING, ID_CREATENODEDIALOGNMT_NONE, 
 ID_CREATENODEDIALOGNODEID, ID_CREATENODEDIALOGPROFILE, 
 ID_CREATENODEDIALOGSAVECONFIG, ID_CREATENODEDIALOGSTATICTEXT1, 
 ID_CREATENODEDIALOGSTATICTEXT2, ID_CREATENODEDIALOGSTATICTEXT3, 
 ID_CREATENODEDIALOGSTATICTEXT4, ID_CREATENODEDIALOGSTATICTEXT5, 
 ID_CREATENODEDIALOGSTATICTEXT6, ID_CREATENODEDIALOGSTATICTEXT7,
 ID_CREATENODEDIALOGSTOREEDS, ID_CREATENODEDIALOGDESCRIPTION,
 ID_CREATENODEDIALOGTYPE, 
] = [wx.NewId() for _init_ctrls in range(21)]

class CreateNodeDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        parent.AddSizer(self.MainSizer, 0, border=20, flag=wx.GROW|wx.TOP|wx.LEFT|wx.RIGHT)
        parent.AddSizer(self.ButtonSizer, 0, border=20, flag=wx.ALIGN_RIGHT|wx.BOTTOM|wx.LEFT|wx.RIGHT)

    def _init_coll_flexGridSizer1_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_MainSizer_Items(self, parent):
        parent.AddSizer(self.TopBoxSizer, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.staticText7, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Description, 0, border=0, flag=wx.GROW)

    def _init_coll_MainSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableCol(0)
    
    def _init_coll_TopBoxSizer_Items(self, parent):
        parent.AddSizer(self.LeftBoxSizer, 1, border=10, flag=wx.GROW|wx.RIGHT)
        parent.AddSizer(self.RightGridSizer, 1, border=10, flag=wx.GROW|wx.LEFT)
    
    def _init_coll_LeftBoxSizer_Items(self, parent):
        parent.AddWindow(self.staticText1, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.Type, 0, border=10, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.staticText2, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.NodeName, 0, border=10, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.staticText3, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.NodeID, 0, border=10, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.staticText4, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.Profile, 0, border=20, flag=wx.GROW|wx.BOTTOM)

    def _init_coll_RightGridSizer_Items(self, parent):
        parent.AddSizer(self.RightTopGridSizer, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Spacer, 0, border=0, flag=wx.GROW)
        parent.AddSizer(self.RightBottomGridSizer, 0, border=0, flag=wx.GROW)

    def _init_coll_RightGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(1)

    def _init_coll_RightTopGridSizer_Items(self, parent):
        parent.AddWindow(self.staticText5, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.NMT_None, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.NMT_NodeGuarding, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.NMT_Heartbeat, 0, border=0, flag=wx.GROW)
        
    def _init_coll_RightTopGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        
    def _init_coll_RightBottomGridSizer_Items(self, parent):
        parent.AddWindow(self.staticText6, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.DS302, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.GenSYNC, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.Emergency, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.SaveConfig, 0, border=0, flag=wx.GROW)
        parent.AddWindow(self.StoreEDS, 0, border=0, flag=wx.GROW)
        
    def _init_coll_RightBottomGridSizer_Growables(self, parent):
        parent.AddGrowableCol(0)

    def _init_sizers(self):
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=10)
        self.MainSizer = wx.FlexGridSizer(cols=1, hgap=5, rows=3, vgap=0)
        self.TopBoxSizer = wx.BoxSizer(wx.HORIZONTAL)
        self.LeftBoxSizer = wx.BoxSizer(wx.VERTICAL)
        self.RightGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=3, vgap=5)
        self.RightTopGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=4, vgap=0)
        self.RightBottomGridSizer = wx.FlexGridSizer(cols=1, hgap=0, rows=6, vgap=0)
        
        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)
        self._init_coll_flexGridSizer1_Growables(self.flexGridSizer1)
        self._init_coll_MainSizer_Items(self.MainSizer)
        self._init_coll_MainSizer_Growables(self.MainSizer)
        self._init_coll_TopBoxSizer_Items(self.TopBoxSizer)
        self._init_coll_LeftBoxSizer_Items(self.LeftBoxSizer)
        self._init_coll_RightGridSizer_Items(self.RightGridSizer)
        self._init_coll_RightGridSizer_Growables(self.RightGridSizer)
        self._init_coll_RightTopGridSizer_Items(self.RightTopGridSizer)
        self._init_coll_RightTopGridSizer_Growables(self.RightTopGridSizer)
        self._init_coll_RightBottomGridSizer_Items(self.RightBottomGridSizer)
        self._init_coll_RightBottomGridSizer_Growables(self.RightBottomGridSizer)
        
        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        wx.Dialog.__init__(self, id=ID_CREATENODEDIALOG,
              name='CreateNodeDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(450, 350), style=wx.DEFAULT_DIALOG_STYLE,
              title='Create a new Node')
        self.SetClientSize(wx.Size(450, 350))

        self.staticText1 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT1,
              label='Type:', name='staticText1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.staticText2 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT2,
              label='Name:', name='staticText2', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.staticText3 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT3,
              label='Node ID:', name='staticText3', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.staticText4 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT4,
              label='Profile:', name='staticText4', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.Type = wx.Choice(choices=[], id=ID_CREATENODEDIALOGTYPE,
              name='Type', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 25), style=0)

        self.NodeName = wx.TextCtrl(id=ID_CREATENODEDIALOGNAME, name='NodeName',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), 
              style=0, value='')

        self.NodeID = wx.TextCtrl(id=ID_CREATENODEDIALOGNODEID, name='NodeID',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), 
              style=wx.TE_RIGHT, value='')

        self.Profile = wx.Choice(choices=[], id=ID_CREATENODEDIALOGPROFILE,
              name='Profile', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 24), style=0)
        self.Profile.Bind(wx.EVT_CHOICE, self.OnProfileChoice,
              id=ID_CREATENODEDIALOGPROFILE)

        self.staticText5 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT5,
              label='Network Management:', name='staticText5',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.NMT_None = wx.RadioButton(id=ID_CREATENODEDIALOGNMT_NONE,
              label='None', name='NMT_None', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.NMT_None.SetValue(True)

        self.NMT_NodeGuarding = wx.RadioButton(id=ID_CREATENODEDIALOGNMT_NODEGUARDING,
              label='Node Guarding', name='NMT_NodeGuarding', parent=self, 
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.NMT_NodeGuarding.SetValue(False)

        self.NMT_Heartbeat = wx.RadioButton(id=ID_CREATENODEDIALOGNMT_HEARTBEAT,
              label='Heartbeat', name='NMT_Heartbeat', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.NMT_Heartbeat.SetValue(False)

        self.staticText6 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT6,
              label='Options:', name='staticText6', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.DS302 = wx.CheckBox(id=ID_CREATENODEDIALOGGENSYNC,
              label='DS-302 Profile', name='DS302', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.DS302.SetValue(False)
        #self.DS302.Enable(False)

        self.GenSYNC = wx.CheckBox(id=ID_CREATENODEDIALOGGENSYNC,
              label='Generate SYNC', name='GenSYNC', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.GenSYNC.SetValue(False)

        self.Emergency = wx.CheckBox(id=ID_CREATENODEDIALOGEMERGENCY,
              label='Emergency support', name='Emergency',
              parent=self, pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.Emergency.SetValue(False)
        self.Emergency.Enable(False)

        self.SaveConfig = wx.CheckBox(id=ID_CREATENODEDIALOGSAVECONFIG,
              label='Save Configuration', name='SaveConfig', parent=self, 
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.SaveConfig.SetValue(False)
        self.SaveConfig.Enable(False)

        self.StoreEDS = wx.CheckBox(id=ID_CREATENODEDIALOGSTOREEDS,
              label='Store EDS', name='StoreEDS', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 24), style=0)
        self.StoreEDS.SetValue(False)
        self.StoreEDS.Hide()

        self.staticText7 = wx.StaticText(id=ID_CREATENODEDIALOGSTATICTEXT7,
              label='Description:', name='staticText7', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.Description = wx.TextCtrl(id=ID_CREATENODEDIALOGDESCRIPTION, 
              name='Description', parent=self, pos=wx.Point(0, 0), 
              size=wx.Size(0, 24), style=0, value='')

        self.Spacer = wx.Panel(id=ID_CREATENODEDIALOGSPACER,
              name='Spacer', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=wx.TAB_TRAVERSAL)

        self.ButtonSizer = self.CreateButtonSizer(wx.OK|wx.CANCEL)
        self.Bind(wx.EVT_BUTTON, self.OnOK, id=self.ButtonSizer.GetAffirmativeButton().GetId())
        
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        
        self.NodeID.SetValue("0x00")
        self.Type.Append("master")
        self.Type.Append("slave")
        self.Type.SetStringSelection("slave")
        self.Description.SetValue("")
        self.ListProfile = {"None" : ""}
        self.Profile.Append("None")
        self.Directory = os.path.join(ScriptDirectory, "config")
        listfiles = os.listdir(self.Directory)
        listfiles.sort()
        for item in listfiles:
            name, extend = os.path.splitext(item)
            if os.path.isfile(os.path.join(self.Directory, item)) and extend == ".prf" and name != "DS-302":
                self.ListProfile[name] = os.path.join(self.Directory, item)
                self.Profile.Append(name)
        self.Profile.Append("Other")
        self.Profile.SetStringSelection("None")
        self.NodeName.SetFocus()
        
    def OnOK(self, event):
        name = self.NodeName.GetValue()
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
            message = wx.MessageDialog(self, message, "ERROR", wx.OK|wx.ICON_ERROR)
            message.ShowModal()
            message.Destroy()
            self.NodeName.SetFocus()
        else:
            self.EndModal(wx.ID_OK)

    def GetValues(self):
        name = self.NodeName.GetValue()
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
        if self.StoreEDS.GetValue():
            options.append("StoreEDS")
        return options

    def OnProfileChoice(self, event):
        if self.Profile.GetStringSelection() == "Other":
            dialog = wx.FileDialog(self, "Choose a file", self.Directory, "",  "OD Profile files (*.prf)|*.prf|All files|*.*", wx.OPEN|wx.CHANGE_DIR)
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

[ID_ADDSLAVEDIALOG, ID_ADDSLAVEDIALOGSLAVENAME, 
 ID_ADDSLAVEDIALOGSLAVENODEID, ID_ADDSLAVEDIALOGEDSFILE, 
 ID_ADDSLAVEDIALOGIMPORTEDS, ID_ADDSLAVEDIALOGSTATICTEXT1, 
 ID_ADDSLAVEDIALOGSTATICTEXT2, ID_ADDSLAVEDIALOGSTATICTEXT3, 
] = [wx.NewId() for _init_ctrls in range(8)]

class AddSlaveDialog(wx.Dialog):
    def _init_coll_flexGridSizer1_Items(self, parent):
        parent.AddSizer(self.MainSizer, 0, border=20, flag=wx.GROW|wx.TOP|wx.LEFT|wx.RIGHT)
        parent.AddSizer(self.ButtonSizer, 0, border=20, flag=wx.ALIGN_RIGHT|wx.BOTTOM|wx.LEFT|wx.RIGHT)

    def _init_coll_flexGridSizer1_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_coll_MainSizer_Items(self, parent):
        parent.AddWindow(self.staticText1, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.SlaveName, 0, border=10, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.staticText2, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.SlaveNodeID, 0, border=10, flag=wx.GROW|wx.BOTTOM)
        parent.AddWindow(self.staticText3, 0, border=5, flag=wx.GROW|wx.BOTTOM)
        parent.AddSizer(self.BottomSizer, 0, border=0, flag=wx.GROW)

    def _init_coll_BottomSizer_Items(self, parent):
        parent.AddWindow(self.EDSFile, 0, border=4, flag=wx.GROW|wx.TOP|wx.BOTTOM)
        parent.AddWindow(self.ImportEDS, 0, border=0, flag=0)

    def _init_coll_BottomSizer_Growables(self, parent):
        parent.AddGrowableCol(0)
        parent.AddGrowableRow(0)

    def _init_sizers(self):
        self.flexGridSizer1 = wx.FlexGridSizer(cols=1, hgap=0, rows=2, vgap=10)
        self.MainSizer = wx.BoxSizer(wx.VERTICAL)
        self.BottomSizer = wx.FlexGridSizer(cols=2, hgap=5, rows=1, vgap=0)
        
        self._init_coll_flexGridSizer1_Items(self.flexGridSizer1)
        self._init_coll_flexGridSizer1_Growables(self.flexGridSizer1)
        self._init_coll_MainSizer_Items(self.MainSizer)
        self._init_coll_BottomSizer_Items(self.BottomSizer)
        self._init_coll_BottomSizer_Growables(self.BottomSizer)
        
        self.SetSizer(self.flexGridSizer1)

    def _init_ctrls(self, prnt):
        wx.Dialog.__init__(self, id=ID_ADDSLAVEDIALOG,
              name='AddSlaveDialog', parent=prnt, pos=wx.Point(376, 223),
              size=wx.Size(300, 250), style=wx.DEFAULT_DIALOG_STYLE|wx.RESIZE_BORDER,
              title='Add a slave to nodelist')
        self.SetClientSize(wx.Size(300, 250))

        self.staticText1 = wx.StaticText(id=ID_ADDSLAVEDIALOGSTATICTEXT1,
              label='Slave Name:', name='staticText1', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.SlaveName = wx.TextCtrl(id=ID_ADDSLAVEDIALOGSLAVENAME,
              name='SlaveName', parent=self, pos=wx.Point(0, 0), 
              size=wx.Size(0, 24), style=0)

        self.staticText2 = wx.StaticText(id=ID_ADDSLAVEDIALOGSTATICTEXT2,
              label='Slave Node ID:', name='staticText2', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.SlaveNodeID = wx.TextCtrl(id=ID_ADDSLAVEDIALOGSLAVENODEID,
              name='SlaveName', parent=self, pos=wx.Point(0, 0), 
              size=wx.Size(0, 24), style=wx.ALIGN_RIGHT)

        self.staticText3 = wx.StaticText(id=ID_ADDSLAVEDIALOGSTATICTEXT3,
              label='EDS File:', name='staticText3', parent=self,
              pos=wx.Point(0, 0), size=wx.Size(0, 17), style=0)

        self.EDSFile = wx.Choice(id=ID_ADDSLAVEDIALOGEDSFILE,
              name='EDSFile', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(0, 24), style=0)
        
        self.ImportEDS = wx.Button(id=ID_ADDSLAVEDIALOGIMPORTEDS, label='Import EDS',
              name='ImportEDS', parent=self, pos=wx.Point(0, 0),
              size=wx.Size(100, 32), style=0)
        self.ImportEDS.Bind(wx.EVT_BUTTON, self.OnImportEDSButton,
              id=ID_ADDSLAVEDIALOGIMPORTEDS)
        
        self.ButtonSizer = self.CreateButtonSizer(wx.OK|wx.CANCEL|wx.CENTRE)
        self.Bind(wx.EVT_BUTTON, self.OnOK, id=self.ButtonSizer.GetAffirmativeButton().GetId())
    
        self._init_sizers()

    def __init__(self, parent):
        self._init_ctrls(parent)
        
        self.SlaveNodeID.SetValue("0x00")
            
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
            message = wx.MessageDialog(self, "Form isn't complete. %s must be filled!"%text, "Error", wx.OK|wx.ICON_ERROR)
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
                message = wx.MessageDialog(self, "Slave Node ID must be a value in decimal or hexadecimal!", "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
                return
            if not 0 <= nodeid <= 127:
                message = wx.MessageDialog(self, "Slave Node ID must be between 0 and 127!", "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
            elif nodeid == 0 or nodeid in self.NodeList.SlaveNodes.keys():
                message = wx.MessageDialog(self, "A Node with this ID already exist in the network!", "Error", wx.OK|wx.ICON_ERROR)
                message.ShowModal()
                message.Destroy()
            else:
                self.EndModal(wx.ID_OK)

    def OnImportEDSButton(self, event):
        dialog = wx.FileDialog(self, "Choose an EDS file", os.getcwd(), "",  "EDS files (*.eds)|*.eds|All files|*.*", wx.OPEN|wx.CHANGE_DIR)
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if os.path.isfile(filepath):
                result = self.NodeList.ImportEDSFile(filepath)
                if result:
                    message = wx.MessageDialog(self, result, "Error", wx.OK|wx.ICON_ERROR)
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
        if self.EDSFile.FindString(selection) != wx.NOT_FOUND:
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
