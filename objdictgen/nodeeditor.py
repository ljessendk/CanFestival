import wx

from node import OD_Subindex, OD_MultipleSubindexes, OD_IdenticalSubindexes, OD_IdenticalIndexes
from commondialogs import *

class NodeEditorTemplate:
    
    EDITMENU_ID = None
    
    def __init__(self, manager, frame, mode_solo):
        self.Manager = manager
        self.Frame = frame
        self.ModeSolo = mode_solo
        
        self.BusId = None
        self.Closing = False
    
    def SetBusId(self, bus_id):
        self.BusId = bus_id

    def GetBusId(self):
        return self.BusId

    def IsClosing(self):
        return self.Closing
    
    def OnAddSDOServerMenu(self, event):
        self.Manager.AddSDOServerToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        
    def OnAddSDOClientMenu(self, event):
        self.Manager.AddSDOClientToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        
    def OnAddPDOTransmitMenu(self, event):
        self.Manager.AddPDOTransmitToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        
    def OnAddPDOReceiveMenu(self, event):
        self.Manager.AddPDOReceiveToCurrent()
        self.RefreshBufferState()
        self.RefreshCurrentIndexList()
        
    def OnAddMapVariableMenu(self, event):
        self.AddMapVariable()
        
    def OnAddUserTypeMenu(self, event):
        self.AddUserType()
    
    def OnRefreshMenu(self, event):
        self.RefreshCurrentIndexList()
        
    def RefreshCurrentIndexList(self):
        pass
    
    def RefreshStatusBar(self):
        pass
    
    def SetStatusBarText(self, selection, manager):
        if selection:
            index, subIndex = selection
            if manager.IsCurrentEntry(index):
                self.Frame.HelpBar.SetStatusText(_("Index: 0x%04X")%index, 0)
                self.Frame.HelpBar.SetStatusText(_("Subindex: 0x%02X")%subIndex, 1)
                entryinfos = manager.GetEntryInfos(index)
                name = entryinfos["name"]
                category = _("Optional")
                if entryinfos["need"]:
                    category = _("Mandatory")
                struct = "VAR"
                number = ""
                if entryinfos["struct"] & OD_IdenticalIndexes:
                    number = _(" possibly defined %d times")%entryinfos["nbmax"]
                if entryinfos["struct"] & OD_IdenticalSubindexes:
                    struct = "REC"
                elif entryinfos["struct"] & OD_MultipleSubindexes:
                    struct = "ARRAY"
                text = _("%s: %s entry of struct %s%s.")%(name,category,struct,number)
                self.Frame.HelpBar.SetStatusText(text, 2)
            else:
                for i in xrange(3):
                    self.Frame.HelpBar.SetStatusText("", i)
        else:
            for i in xrange(3):
                self.Frame.HelpBar.SetStatusText("", i)
    
    def RefreshProfileMenu(self):
        if self.EDITMENU_ID is not None:
            profile = self.Manager.GetCurrentProfileName()
            edititem = self.Frame.EditMenu.FindItemById(self.EDITMENU_ID)
            if edititem:
                length = self.Frame.AddMenu.GetMenuItemCount()
                for i in xrange(length-6):
                    additem = self.Frame.AddMenu.FindItemByPosition(6)
                    self.Frame.AddMenu.Delete(additem.GetId())
                if profile not in ("None", "DS-301"):
                    edititem.SetText(_("%s Profile")%profile)
                    edititem.Enable(True)
                    self.Frame.AddMenu.AppendSeparator()
                    for text, indexes in self.Manager.GetCurrentSpecificMenu():
                        new_id = wx.NewId()
                        self.Frame.AddMenu.Append(help='', id=new_id, kind=wx.ITEM_NORMAL, text=text)
                        self.Frame.Bind(wx.EVT_MENU, self.GetProfileCallBack(text), id=new_id)
                else:
                    edititem.SetText(_("Other Profile"))
                    edititem.Enable(False)
        
#-------------------------------------------------------------------------------
#                            Buffer Functions
#-------------------------------------------------------------------------------

    def RefreshBufferState(self):
        pass

    def OnUndoMenu(self, event):
        self.Manager.LoadCurrentPrevious()
        self.RefreshCurrentIndexList()
        self.RefreshBufferState()
        
    def OnRedoMenu(self, event):
        self.Manager.LoadCurrentNext()
        self.RefreshCurrentIndexList()
        self.RefreshBufferState()

#-------------------------------------------------------------------------------
#                          Editing Profiles functions
#-------------------------------------------------------------------------------

    def OnCommunicationMenu(self, event):
        dictionary,current = self.Manager.GetCurrentCommunicationLists()
        self.EditProfile(_("Edit DS-301 Profile"), dictionary, current)
    
    def OnOtherCommunicationMenu(self, event):
        dictionary,current = self.Manager.GetCurrentDS302Lists()
        self.EditProfile(_("Edit DS-302 Profile"), dictionary, current)
    
    def OnEditProfileMenu(self, event):
        title = _("Edit %s Profile") % self.Manager.GetCurrentProfileName()
        dictionary,current = self.Manager.GetCurrentProfileLists()
        self.EditProfile(title, dictionary, current)
    
    def EditProfile(self, title, dictionary, current):
        dialog = CommunicationDialog(self.Frame)
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
            
        dialog.Destroy()

    def GetProfileCallBack(self, text):
        def ProfileCallBack(event):
            self.Manager.AddSpecificEntryToCurrent(text)
            self.RefreshBufferState()
            self.RefreshCurrentIndexList()
        return ProfileCallBack

#-------------------------------------------------------------------------------
#                         Edit Node informations function
#-------------------------------------------------------------------------------

    def OnNodeInfosMenu(self, event):
        dialog = NodeInfosDialog(self.Frame)
        name, id, type, description = self.Manager.GetCurrentNodeInfos()
        defaultstringsize = self.Manager.GetCurrentNodeDefaultStringSize()
        dialog.SetValues(name, id, type, description, defaultstringsize)
        if dialog.ShowModal() == wx.ID_OK:
            name, id, type, description, defaultstringsize = dialog.GetValues()
            self.Manager.SetCurrentNodeInfos(name, id, type, description)
            self.Manager.SetCurrentNodeDefaultStringSize(defaultstringsize)
            self.RefreshBufferState()
            self.RefreshCurrentIndexList()
            self.RefreshProfileMenu()

#-------------------------------------------------------------------------------
#                           Add User Types and Variables
#-------------------------------------------------------------------------------
        
    def AddMapVariable(self):
        index = self.Manager.GetCurrentNextMapIndex()
        if index:
            dialog = MapVariableDialog(self.Frame)
            dialog.SetIndex(index)
            if dialog.ShowModal() == wx.ID_OK:
                result = self.Manager.AddMapVariableToCurrent(*dialog.GetValues())
                if not isinstance(result, (StringType, UnicodeType)):
                    self.RefreshBufferState()
                    self.RefreshCurrentIndexList()
                else:
                    self.ShowErrorMessage(result)
            dialog.Destroy()
        else:
            self.ShowErrorMessage(_("No map variable index left!"))
        
    def AddUserType(self):
        dialog = UserTypeDialog(self)
        dialog.SetTypeList(self.Manager.GetCustomisableTypes())
        if dialog.ShowModal() == wx.ID_OK:
            result = self.Manager.AddUserTypeToCurrent(*dialog.GetValues())
            if not isinstance(result, (StringType, UnicodeType)):
                self.RefreshBufferState()
                self.RefreshCurrentIndexList()
            else:
                self.ShowErrorMessage(result)
        dialog.Destroy()

    def ShowErrorMessage(self, message):
        message = wx.MessageDialog(self.Frame, message, _("Error"), wx.OK|wx.ICON_ERROR)
        message.ShowModal()
        message.Destroy()
