import wx

from nodeeditortemplate import NodeEditorTemplate
from subindextable import *
from commondialogs import *

[ID_NETWORKEDITNETWORKNODES,
] = [wx.NewId() for _init_ctrls in range(1)]

class NetworkEditorTemplate(NodeEditorTemplate):
    
    def _init_ctrls(self, prnt):
        self.NetworkNodes = wx.Notebook(id=ID_NETWORKEDITNETWORKNODES,
              name='NetworkNodes', parent=prnt, pos=wx.Point(0, 0),
              size=wx.Size(0, 0), style=wx.NB_LEFT)
        self.NetworkNodes.Bind(wx.EVT_NOTEBOOK_PAGE_CHANGED,
              self.OnNodeSelectedChanged, id=ID_NETWORKEDITNETWORKNODES)
    
    def __init__(self, manager, frame, mode_solo):
        self.NodeList = manager
        NodeEditorTemplate.__init__(self, self.NodeList.GetManager(), frame, mode_solo)
    
    def GetCurrentNodeId(self):
        selected = self.NetworkNodes.GetSelection()
        # At init selected = -1
        if selected > 0:
            window = self.NetworkNodes.GetPage(selected)
            return window.GetIndex()
        else:
            return 0
    
    def RefreshCurrentIndexList(self):
        selected = self.NetworkNodes.GetSelection()
        if selected == 0:
            window = self.NetworkNodes.GetPage(selected)
            window.RefreshIndexList()
        else:
            pass

    def RefreshNetworkNodes(self):
        if self.NetworkNodes.GetPageCount() > 0:
            self.NetworkNodes.DeleteAllPages()
        if self.NodeList:
            new_editingpanel = EditingPanel(self.NetworkNodes, self, self.Manager)
            new_editingpanel.SetIndex(self.Manager.GetCurrentNodeID())
            self.NetworkNodes.AddPage(new_editingpanel, "")
            for idx in self.NodeList.GetSlaveIDs():
                new_editingpanel = EditingPanel(self.NetworkNodes, self, self.NodeList, False)
                new_editingpanel.SetIndex(idx)
                self.NetworkNodes.AddPage(new_editingpanel, "")
    
    def OnNodeSelectedChanged(self, event):
        if not self.Closing:
            selected = event.GetSelection()
            # At init selected = -1
            if selected >= 0:
                window = self.NetworkNodes.GetPage(selected)
                self.NodeList.SetCurrentSelected(window.GetIndex())
            wx.CallAfter(self.RefreshMainMenu)
            wx.CallAfter(self.RefreshStatusBar)
        event.Skip()

#-------------------------------------------------------------------------------
#                              Buffer Functions
#-------------------------------------------------------------------------------

    def RefreshBufferState(self):
        if self.NodeList is not None:
            nodeID = self.Manager.GetCurrentNodeID()
            if nodeID != None:
                nodename = "0x%2.2X %s"%(nodeID, self.Manager.GetCurrentNodeName())
            else:
                nodename = self.Manager.GetCurrentNodeName()
            self.NetworkNodes.SetPageText(0, nodename)
            for idx, name in enumerate(self.NodeList.GetSlaveNames()):
                self.NetworkNodes.SetPageText(idx + 1, name)

#-------------------------------------------------------------------------------
#                             Slave Nodes Management
#-------------------------------------------------------------------------------

    def OnAddSlaveMenu(self, event):
        dialog = AddSlaveDialog(self.Frame)
        dialog.SetNodeList(self.NodeList)
        if dialog.ShowModal() == wx.ID_OK:
            values = dialog.GetValues()
            result = self.NodeList.AddSlaveNode(values["slaveName"], values["slaveNodeID"], values["edsFile"])
            if not result:
                new_editingpanel = EditingPanel(self.NetworkNodes, self, self.NodeList, False)
                new_editingpanel.SetIndex(values["slaveNodeID"])
                idx = self.NodeList.GetOrderNumber(values["slaveNodeID"])
                self.NetworkNodes.InsertPage(idx, new_editingpanel, "")
                self.NodeList.SetCurrentSelected(idx)
                self.NetworkNodes.SetSelection(idx)
                self.RefreshBufferState()
            else:
                self.ShowErrorMessage(result)
        dialog.Destroy()
        
    def OnRemoveSlaveMenu(self, event):
        slavenames = self.NodeList.GetSlaveNames()
        slaveids = self.NodeList.GetSlaveIDs()
        dialog = wx.SingleChoiceDialog(self.Frame, _("Choose a slave to remove"), _("Remove slave"), slavenames)
        if dialog.ShowModal() == wx.ID_OK:
            choice = dialog.GetSelection()
            result = self.NodeList.RemoveSlaveNode(slaveids[choice])
            if not result:
                slaveids.pop(choice)
                current = self.NetworkNodes.GetSelection()
                self.NetworkNodes.DeletePage(choice + 1)
                if self.NetworkNodes.GetPageCount() > 0:
                    new_selection = min(current, self.NetworkNodes.GetPageCount() - 1)
                    self.NetworkNodes.SetSelection(new_selection)
                    if new_selection > 0:
                        self.NodeList.SetCurrentSelected(slaveids[new_selection - 1])
                self.RefreshBufferState()
            else:
                self.ShowErrorMessage(result)
        dialog.Destroy()

    def OpenMasterDCFDialog(self, node_id):
        self.NetworkNodes.SetSelection(0)
        self.NetworkNodes.GetPage(0).OpenDCFDialog(node_id)
