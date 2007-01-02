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

import cPickle
from types import *

"""
Dictionary of translation between access symbol and their signification
"""
AccessType = {"ro" : "Read Only", "wo" : "Write Only", "rw" : "Read/Write"}

BoolType = {True : "True", False : "False"} 
OptionType = {True : "Yes", False : "No"}

CustomisableTypes = [(0x02, 0), (0x03, 0), (0x04, 0), (0x05, 0), (0x06, 0), (0x07, 0),
    (0x08, 0), (0x09, 1), (0x0A, 1), (0x0B, 1), (0x10, 0), (0x11, 0), (0x12, 0),
    (0x13, 0), (0x14, 0), (0x15, 0), (0x16, 0), (0x18, 0), (0x19, 0), (0x1A, 0),
    (0x1B, 0)]

DefaultParams = {"comment" : "", "save" : False}

#-------------------------------------------------------------------------------
#                      Dictionary Mapping and Organisation
#-------------------------------------------------------------------------------

"""
Properties of entry structure in the Object Dictionary
"""
OD_Subindex = 1             # Entry has at least one subindex
OD_MultipleSubindexes = 2   # Entry has more than one subindex
OD_IdenticalSubindexes = 4  # Subindexes of entry have the same description
OD_IdenticalIndexes = 8     # Entry has the same description on multiple indexes

"""
Structures of entry in the Object Dictionary, sum of the properties described above
for all sorts of entries use in CAN Open specification
"""
nosub = 0 # Entry without subindex (only for type declaration)
var = 1
array = 3
rec = 7
# Entries identical on multiple indexes
plurivar = 9
pluriarray = 11 # Example : PDO Parameters
plurirec = 15   # Example : PDO Mapping

"""
MappingDictionary is the structure used for writing a good organised Object
Dictionary. It follows the specifications of the CANOpen standard.
Change the informations within it if there is a mistake. But don't modify the
organisation of this object, it will involve in a malfunction of the application.
"""

MappingDictionary = {
    0x0001 : {"name" : "BOOLEAN", "struct" : nosub, "size" : 1, "default" : False, "values" : []},
    0x0002 : {"name" : "INTEGER8", "struct" : nosub, "size" : 8, "default" : 0, "values" : []},
    0x0003 : {"name" : "INTEGER16", "struct" : nosub, "size" : 16, "default" : 0, "values" : []},
    0x0004 : {"name" : "INTEGER32", "struct" : nosub, "size" : 32, "default" : 0, "values" : []},
    0x0005 : {"name" : "UNSIGNED8", "struct" : nosub, "size" : 8, "default" : 0, "values" : []},
    0x0006 : {"name" : "UNSIGNED16", "struct" : nosub, "size" : 16, "default" : 0, "values" : []},
    0x0007 : {"name" : "UNSIGNED32", "struct" : nosub, "size" : 32, "default" : 0, "values" : []},
    0x0008 : {"name" : "REAL32", "struct" : nosub, "size" : 32, "default" : 0.0, "values" : []},
    0x0009 : {"name" : "VISIBLE_STRING", "struct" : nosub, "size" : 8, "default" : "", "values" : []},
    0x000A : {"name" : "OCTET_STRING", "struct" : nosub, "size" : 8, "default" : "", "values" : []},
    0x000B : {"name" : "UNICODE_STRING", "struct" : nosub, "size" : 16, "default" : "", "values" : []},
    0x000C : {"name" : "TIME_OF_DAY", "struct" : nosub, "size" : 48, "default" : 0, "values" : []},
    0x000D : {"name" : "TIME_DIFFERENCE", "struct" : nosub, "size" : 48, "default" : 0, "values" : []},
#    0x000F : {"name" : "DOMAIN", "struct" : nosub, "size" : 0, "default" : "", "values" : []},
    0x0010 : {"name" : "INTEGER24", "struct" : nosub, "size" : 24, "default" : 0, "values" : []},
    0x0011 : {"name" : "REAL64", "struct" : nosub, "size" : 64, "default" : 0.0, "values" : []},
    0x0012 : {"name" : "INTEGER40", "struct" : nosub, "size" : 40, "default" : 0, "values" : []},
    0x0013 : {"name" : "INTEGER48", "struct" : nosub, "size" : 48, "default" : 0, "values" : []},
    0x0014 : {"name" : "INTEGER56", "struct" : nosub, "size" : 56, "default" : 0, "values" : []},
    0x0015 : {"name" : "INTEGER64", "struct" : nosub, "size" : 64, "default" : 0, "values" : []},
    0x0016 : {"name" : "UNSIGNED24", "struct" : nosub, "size" : 24, "default" : 0, "values" : []},
    0x0018 : {"name" : "UNSIGNED40", "struct" : nosub, "size" : 40, "default" : 0, "values" : []},
    0x0019 : {"name" : "UNSIGNED48", "struct" : nosub, "size" : 48, "default" : 0, "values" : []},
    0x001A : {"name" : "UNSIGNED56", "struct" : nosub, "size" : 56, "default" : 0, "values" : []},
    0x001B : {"name" : "UNSIGNED64", "struct" : nosub, "size" : 64, "default" : 0, "values" : []},
    0x1000 : {"name" : "Device Type", "struct" : var, "need" : True, "values" : 
                [{"name" : "Device Type", "type" : 0x04, "access" : 'ro', "pdo" : False}]},
    0x1001 : {"name" : "Error Register", "struct" : var,  "need" : True, "values" : 
                [{"name" : "Error Register", "type" : 0x02, "access": 'ro', "pdo" : True}]},
    0x1002 : {"name" : "Manufacturer Status Register", "struct" : var, "need" : False,  "values" :
                [{"name" : "Manufacturer Status Register", "type" : 0x04, "access" : 'ro', "pdo" : True}]},
    0x1003 : {"name" : "Pre-defined Error Field", "struct" : rec, "need" : False,  "values" :
                [{"name" : "Number of Errors", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Standard Error Field", "type" : 0x04, "access" : 'ro', "pdo" : False, "nbmax" : 0xFE}]},
    0x1005 : {"name" : "SYNC COB ID", "struct" : var, "need" : True, "callback" : True, "values" :
                [{"name" : "SYNC COB ID", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1006 : {"name" : "Communication / Cycle Period", "struct" : var, "need" : False, "callback" : True, "values" :
                [{"name" : "Communication Cycle Period", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1007 : {"name" : "Synchronous Window Length", "struct" : var, "need" : False, "values" :
                [{"name" : "Synchronous Window Length", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1008 : {"name" : "Manufacturer Device Name", "struct" : var, "need" : False, "values" :
                [{"name" : "Manufacturer Device Name", "type" : 0x09, "access" : 'ro', "pdo" : False}]},
    0x1009 : {"name" : "Manufacturer Hardware Version", "struct" : var, "need" : False, "values" :
                [{"name" : "Manufacturer Hardware Version", "type" : 0x09, "access" : 'ro', "pdo" : False}]},
    0x100A : {"name" : "Manufacturer Software Version", "struct" : var, "need" : False, "values" :
                [{"name" : "Manufacturer Software Version", "type" : 0x09, "access" : 'ro', "pdo" : False}]},
    0x100C : {"name" : "Guard Time", "struct" : var, "need" : False, "values" :
                [{"name" : "Guard Time", "type" : 0x03, "access" : 'rw', "pdo" : False}]},
    0x100D : {"name" : "Life Time Factor", "struct" : var, "need" : False, "values" :
                [{"name" : "Life Time Factor", "type" : 0x02, "access" : 'rw', "pdo" : False}]},
    0x1010 : {"name" : "Store parameters", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Save All Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Save Communication Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Save Application Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Save Manufacturer Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False, "nbmax" : 0x7C}]},
    0x1011 : {"name" : "Restore Default Parameters", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Restore All Default Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Restore Communication Default Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Restore Application Default Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Restore Manufacturer Default Parameters", "type" : 0x04, "access" : 'rw', "pdo" : False, "nbmax" : 0x7C}]},
    0x1012 : {"name" : "TIME COB ID", "struct" : var, "need" : False, "values" :
                [{"name" : "TIME COB ID", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1013 : {"name" : "High Resolution Timestamp", "struct" : var, "need" : False, "values" :
                [{"name" : "High Resolution Time Stamp", "type" : 0x04, "access" : 'rw', "pdo" : True}]},
    0x1014 : {"name" : "Emergency COB ID", "struct" : var, "need" : False, "values" :
                [{"name" : "Emergency COB ID", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1015 : {"name" : "Inhibit Time Emergency", "struct" : var, "need" : False, "values" :
                [{"name" : "Inhibit Time Emergency", "type" : 0x03, "access" : 'rw', "pdo" : False}]},
    0x1016 : {"name" : "Consumer Heartbeat Time", "struct" : rec, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Consumer Heartbeat Time", "type" : 0x04, "access" : 'rw', "pdo" : False, "nbmax" : 0x7F}]},
    0x1017 : {"name" : "Producer Heartbeat Time", "struct" : var, "need" : False, "values" :
                [{"name" : "Producer Heartbeat Time", "type" : 0x03, "access" : 'rw', "pdo" : False}]},
    0x1018 : {"name" : "Identity", "struct" : array, "need" : True, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Vendor ID", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "Product Code", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "Revision Number", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "Serial Number", "type" : 0x04, "access" : 'ro', "pdo" : False}]},
    0x1020 : {"name" : "Verify Configuration", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Configuration Date", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "Configuration Time", "type" : 0x04, "access" : 'ro', "pdo" : False}]},
#    0x1021 : {"name" : "Store EDS", "struct" : var, "need" : False, "values" :
#                [{"name" : "Store EDS", "type" : 0x0F, "access" : 'rw', "pdo" : False}]},
#    0x1022 : {"name" : "Storage Format", "struct" : var, "need" : False, "values" :
#                [{"name" : "Storage Format", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1023 : {"name" : "OS Command", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Command", "type" : 0x0A, "access" : 'rw', "pdo" : False},
                 {"name" : "Status", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Reply", "type" : 0x0A, "access" : 'ro', "pdo" : False}]},
    0x1024 : {"name" : "OS Command Mode", "struct" : var, "need" : False, "values" :
                [{"name" : "OS Command Mode", "type" : 0x02, "access" : 'wo', "pdo" : False}]},
    0x1025 : {"name" : "OS Debugger Interface", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Command", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Status", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "Reply", "type" : 0x04, "access" : 'ro', "pdo" : False}]},
    0x1026 : {"name" : "OS Prompt", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "StdIn", "type" : 0x02, "access" : 'wo', "pdo" : True},
                 {"name" : "StdOut", "type" : 0x02, "access" : 'ro', "pdo" : True},
                 {"name" : "StdErr", "type" : 0x02, "access" : 'ro', "pdo" : True}]},
    0x1027 : {"name" : "Module List", "struct" : rec, "need" : False, "values" :
                [{"name" : "Number of Connected Modules", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Module %d[(sub)]", "type" : 0x03, "access" : 'ro', "pdo" : False, "nbmax" : 0xFE}]},
    0x1028 : {"name" : "Emergency Consumer", "struct" : rec, "need" : False, "values" :
                [{"name" : "Number of Consumed Emergency Objects", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Emergency Consumer", "type" : 0x04, "access" : 'rw', "pdo" : False, "nbmax" : 0x7E}]},
    0x1029 : {"name" : "Error Behavior", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Error Classes", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "Communication Error", "type" : 0x02, "access" : 'rw', "pdo" : False},
                 {"name" : "Device Profile", "type" : 0x02, "access" : 'rw', "pdo" : False, "nbmax" : 0xFE}]},
    0x1200 : {"name" : "Server SDO Parameter", "struct" : array, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID Client to Server (Receive SDO)", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID Server to Client (Transmit SDO)", "type" : 0x04, "access" : 'ro', "pdo" : False}]},
    0x1201 : {"name" : "Additional Server SDO %d Parameter[(idx)]", "struct" : pluriarray, "incr" : 1, "nbmax" : 0x7F, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID Client to Server (Receive SDO)", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID Server to Client (Transmit SDO)", "type" : 0x04, "access" : 'ro', "pdo" : False},
                 {"name" : "Node ID of the SDO Client", "type" : 0x04, "access" : 'ro', "pdo" : False}]},
    0x1280 : {"name" : "Client SDO %d Parameter[(idx)]", "struct" : pluriarray, "incr" : 1, "nbmax" : 0x100, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID Client to Server (Transmit SDO)", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "COB ID Server to Client (Receive SDO)", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Node ID of the SDO Server", "type" : 0x04, "access" : 'rw', "pdo" : False}]},
    0x1400 : {"name" : "Receive PDO %d Parameter[(idx)]", "struct" : pluriarray, "incr" : 1, "nbmax" : 0x200, "need" : False, "values" :
                [{"name" : "Highest SubIndex Supported", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID used by PDO", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Transmission Type", "type" : 0x02, "access" : 'rw', "pdo" : False},
                 {"name" : "Inhibit Time", "type" : 0x03, "access" : 'rw', "pdo" : False},
                 {"name" : "Compatibility Entry", "type" : 0x03, "access" : 'rw', "pdo" : False},
                 {"name" : "Event Timer", "type" : 0x03, "access" : 'rw', "pdo" : False}]},
    0x1600 : {"name" : "Receive PDO %d Mapping[(idx)]", "struct" : plurirec, "incr" : 1, "nbmax" : 0x200, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "PDO %d Mapping for an application object %d[(idx,sub)]", "type" : 0x04, "access" : 'rw', "pdo" : False, "nbmax" : 0x40}]},
    0x1800 : {"name" : "Transmit PDO %d Parameter[(idx)]", "struct" : pluriarray, "incr" : 1, "nbmax" : 0x200, "need" : False, "values" :
                [{"name" : "Highest SubIndex Supported", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "COB ID used by PDO", "type" : 0x04, "access" : 'rw', "pdo" : False},
                 {"name" : "Transmission Type", "type" : 0x02, "access" : 'rw', "pdo" : False},
                 {"name" : "Inhibit Time", "type" : 0x03, "access" : 'rw', "pdo" : False},
                 {"name" : "Compatibility Entry", "type" : 0x03, "access" : 'rw', "pdo" : False},
                 {"name" : "Event Timer", "type" : 0x03, "access" : 'rw', "pdo" : False}]},
    0x1A00 : {"name" : "Transmit PDO %d Mapping[(idx)]", "struct" : plurirec, "incr" : 1, "nbmax" : 0x200, "need" : False, "values" :
                [{"name" : "Number of Entries", "type" : 0x02, "access" : 'ro', "pdo" : False},
                 {"name" : "PDO %d Mapping for a process data variable %d[(idx,sub)]", "type" : 0x04, "access" : 'rw', "pdo" : False, "nbmax" : 0x40}]},
}

#-------------------------------------------------------------------------------
#                          Definition of Node Object
#-------------------------------------------------------------------------------

"""
Class recording the Object Dictionary entries. It checks at each modification
that the structure of the Object Dictionary stay coherent
"""

class Node:
    
    def __init__(self, name = "", type = "slave", id = 0, profilename = "DS-301", profile = {}, specificmenu = []):
        self.Name = name
        self.Type = type
        self.ID = id
        self.ProfileName = profilename
        self.Profile = profile
        self.SpecificMenu = specificmenu
        self.Dictionary = {}
        self.ParamsDictionary = {}
        self.DS302 = {}
        self.UserMapping = {}
    
    """
    Return the node name
    """
    def GetNodeName(self):
        return self.Name
    
    """
    Define the node name
    """
    def SetNodeName(self, name):
        self.Name = name

    """
    Return the node type ("master" or "slave")
    """
    def GetNodeType(self):
        return self.Type
    
    """
    Define the node type ("master" or "slave")
    """
    def SetNodeType(self, type):
        self.Type = type

    """
    Return the node ID
    """
    def GetNodeID(self):
        return self.ID
    
    """
    Define the node ID
    """
    def SetNodeID(self, id):
        self.ID = id

    """
    Return the Specific Profile Name
    """
    def GetProfileName(self):
        return self.ProfileName
    
    """
    Define the Specific Profile Name
    """
    def SetProfileName(self, profilename):
        self.ProfileName = profilename

    """
    Return the Specific Profile
    """
    def GetProfile(self):
        return self.Profile
    
    """
    Define the Specific Profile
    """
    def SetProfile(self, profile):
        self.Profile = profile
    
    """
    Define the DS-302 Profile
    """
    def SetDS302Profile(self, profile):
        self.DS302 = profile
    
    """
    Define the DS-302 Profile
    """
    def GetDS302Profile(self):
        return self.DS302
    
    """
    Return the Specific Menu Entries
    """
    def GetSpecificMenu(self):
        return self.SpecificMenu
    
    """
    Define the Specific Menu Entries
    """
    def SetSpecificMenu(self, specificmenu):
        self.SpecificMenu = specificmenu
    
    """
    Extend the Specific Menu Entries
    """
    
    def ExtendSpecificMenu(self, specificmenu):
        self.SpecificMenu.extend(AddMenuEntries)
    
    """
    Function which return the different Mappings available for this node
    """
    def GetMappings(self, userdefinedtoo = True):
        if userdefinedtoo:
            return [self.Profile, self.DS302, self.UserMapping]
        else:
            return [self.Profile, self.DS302]
    
    """
    Add a new entry in the Object Dictionary
    """
    def AddEntry(self, index, subIndex = None, value = None):
        if index not in self.Dictionary:
            if not subIndex:
                self.Dictionary[index] = value
                return True
            elif subIndex == 1:
                self.Dictionary[index] = [value]
                return True
        elif subIndex > 1 and type(self.Dictionary[index]) == ListType and subIndex == len(self.Dictionary[index]) + 1:
            self.Dictionary[index].append(value)
            return True
        return False

    """
    Warning ! Modifies an existing entry in the Object Dictionary. Can't add a new one.
    """
    def SetEntry(self, index, subIndex = None, value = None):
        if index in self.Dictionary:
            if not subIndex:
                if value != None:
                    self.Dictionary[index] = value
                return True
            elif type(self.Dictionary[index]) == ListType and 0 < subIndex <= len(self.Dictionary[index]):
                if value != None:
                    self.Dictionary[index][subIndex - 1] = value
                return True
        return False
    
    def SetParamsEntry(self, index, subIndex = None, comment = None, save = None, callback = None):
        if not getattr(self, "ParamsDictionary", False):
            self.ParamsDictionary = {}
        if index in self.Dictionary:
            if (comment != None or save != None or callback != None) and index not in self.ParamsDictionary:
                self.ParamsDictionary[index] = {}
            if subIndex == None or type(self.Dictionary[index]) != ListType and subIndex == 0:
                if comment != None:
                    self.ParamsDictionary[index]["comment"] = comment
                if save != None:
                    self.ParamsDictionary[index]["save"] = save
                if callback != None:
                    self.ParamsDictionary[index]["callback"] = callback
                return True
            elif type(self.Dictionary[index]) == ListType and 0 <= subIndex <= len(self.Dictionary[index]):
                if (comment != None or save != None or callback != None) and subIndex not in self.ParamsDictionary[index]:
                    self.ParamsDictionary[index][subIndex] = {}
                if comment != None:
                    self.ParamsDictionary[index][subIndex]["comment"] = comment
                if save != None:
                    self.ParamsDictionary[index][subIndex]["save"] = save
                return True
        return False
    
    """
    Removes an existing entry in the Object Dictionary. If a subIndex is specified
    it will remove this subIndex only if it's the last of the index. If no subIndex
    is specified it removes the whole index and subIndexes from the Object Dictionary.
    """
    def RemoveEntry(self, index, subIndex = None):
        if not getattr(self, "ParamsDictionary", False):
            self.ParamsDictionary = {}
        if index in self.Dictionary:
            if not subIndex:
                self.Dictionary.pop(index)
                if index in self.ParamsDictionary:
                    self.ParamsDictionary.pop(index)
                return True
            elif type(self.Dictionary[index]) == ListType and subIndex == len(self.Dictionary[index]):
                self.Dictionary[index].pop(subIndex - 1)
                if index in self.ParamsDictionary:
                    if subIndex in self.ParamsDictionary[index]:
                        self.ParamsDictionary[index].pop(subIndex)
                    if len(self.ParamsDictionary[index]) == 0:
                        self.ParamsDictionary.pop(index)
                if len(self.Dictionary[index]) == 0:
                    self.Dictionary.pop(index)
                    if index in self.ParamsDictionary:
                        self.ParamsDictionary.pop(index)
                return True
        return False
    
    """
    Check if an entry exists in the Object Dictionary and returns the answer.
    """
    def IsEntry(self, index):
        if index in self.Dictionary:
            return True
        return False
    
    """
    Returns the value of the entry asked. If the entry has the value "count", it
    returns the number of subIndex in the entry except the first.
    """
    def GetEntry(self, index, subIndex = None):
        if index in self.Dictionary:
            if subIndex == None:
                if type(self.Dictionary[index]) == ListType:
                    values = [len(self.Dictionary[index])]
                    values.extend(self.Dictionary[index])
                    return values
                else:
                    return self.Dictionary[index]
            elif subIndex == 0:
                if type(self.Dictionary[index]) == ListType:
                    return len(self.Dictionary[index])
                else:
                    return self.Dictionary[index]
            elif type(self.Dictionary[index]) == ListType and 0 < subIndex <= len(self.Dictionary[index]):
                return self.Dictionary[index][subIndex - 1]
        return None

    """
    Returns the value of the entry asked. If the entry has the value "count", it
    returns the number of subIndex in the entry except the first.
    """
    def GetParamsEntry(self, index, subIndex = None):
        if not getattr(self, "ParamsDictionary", False):
            self.ParamsDictionary = {}
        if index in self.Dictionary:
            if subIndex == None:
                if type(self.Dictionary[index]) == ListType:
                    if index in self.ParamsDictionary:
                        result = []
                        for i in xrange(len(self.Dictionary[index]) + 1):
                            line = DefaultParams.copy()
                            if i in self.ParamsDictionary[index]:
                                line.update(self.ParamsDictionary[index][i])
                            result.append(line)
                        return result
                    else:
                        return [DefaultParams.copy() for i in xrange(len(self.Dictionary[index]) + 1)]
                else:
                    result = DefaultParams.copy()
                    if index in self.ParamsDictionary:
                        result.update(self.ParamsDictionary[index])
                    return result
            elif subIndex == 0 and type(self.Dictionary[index]) != ListType:
                result = DefaultParams.copy()
                if index in self.ParamsDictionary:
                    result.update(self.ParamsDictionary[index])
                return result
            elif type(self.Dictionary[index]) == ListType and 0 <= subIndex <= len(self.Dictionary[index]):
                result = DefaultParams.copy()
                if index in self.ParamsDictionary and subIndex in self.ParamsDictionary[index]:
                    result.update(self.Dictionary[index][subIndex])
                return result
        return None

    def HasEntryCallbacks(self, index):
        if not getattr(self, "ParamsDictionary", False):
            self.ParamsDictionary = {}
        if index in self.Dictionary and index in self.ParamsDictionary and "callback" in self.ParamsDictionary[index]:
            return self.ParamsDictionary[index]["callback"]
        return False

    """
    Add a new entry in the User Mapping Dictionary
    """
    def AddMappingEntry(self, index, subIndex = None, name = "Undefined", struct = 0, size = None, nbmax = None, default = None, values = None):
        if index not in self.UserMapping:
            if values == None:
                values = []
            if subIndex == None:
                self.UserMapping[index] = {"name" : name, "struct" : struct, "need" : False, "values" : values}
                if size != None:
                    self.UserMapping[index]["size"] = size
                if nbmax != None:
                    self.UserMapping[index]["nbmax"] = nbmax
                if default != None:
                    self.UserMapping[index]["default"] = default
                return True
        elif subIndex != None and subIndex == len(self.UserMapping[index]["values"]):
            if values == None:
                values = {}
            self.UserMapping[index]["values"].append(values)
            return True
        return False

    """
    Warning ! Modifies an existing entry in the User Mapping Dictionary. Can't add a new one.
    """
    def SetMappingEntry(self, index, subIndex = None, name = None, struct = None, size = None, nbmax = None, default = None, values = None):
        if index in self.UserMapping:
            if subIndex == None:
                if name != None:
                    self.UserMapping[index]["name"] = name
                    if self.UserMapping[index]["struct"] & OD_IdenticalSubindexes:
                        self.UserMapping[index]["values"][1]["name"] = name + " %d[(sub)]"
                    elif not self.UserMapping[index]["struct"] & OD_MultipleSubindexes:
                        self.UserMapping[index]["values"][0]["name"] = name
                if struct != None:
                    self.UserMapping[index]["struct"] = struct
                if size != None:
                    self.UserMapping[index]["size"] = size
                if nbmax != None:
                    self.UserMapping[index]["nbmax"] = nbmax
                if default != None:
                    self.UserMapping[index]["default"] = default
                if values != None:
                    self.UserMapping[index]["values"] = values
                return True
            elif 0 <= subIndex < len(self.UserMapping[index]["values"]) and values != None:
                self.UserMapping[index]["values"][subIndex].update(values)
                return True
        return False
    
    """
    Removes an existing entry in the User Mapping Dictionary. If a subIndex is specified
    it will remove this subIndex only if it's the last of the index. If no subIndex
    is specified it removes the whole index and subIndexes from the User Mapping Dictionary.
    """
    def RemoveMappingEntry(self, index, subIndex = None):
        if index in self.UserMapping:
            if subIndex == None:
                self.UserMapping.pop(index)
                return True
            elif subIndex == len(self.UserMapping[index]["values"]) - 1:
                self.UserMapping[index]["values"].pop(subIndex)
                return True
        return False

    def RemoveMapVariable(self, index, subIndex):
        model = index << 16
        mask = 0xFFFF << 16
        if subIndex:
            model += subIndex << 8
            mask = 0xFF << 8
        for i in self.Dictionary.iterkeys():
            if 0x1600 <= i <= 0x17FF or 0x1A00 <= i <= 0x1BFF:
                for j,value in enumerate(self.Dictionary[i]):
                    if (value & mask) == model:
                        self.Dictionary[i][j] = 0
    
    def RemoveLine(self, index, max, incr = 1):
        i = index
        while i < max and self.IsEntry(i + incr):
            self.Dictionary[i] = self.Dictionary[i + incr]
            i += incr
        self.Dictionary.pop(i)

    def RemoveUserType(self, index):
        type = self.GetEntry(index, 1)
        for i in self.UserMapping:
            for value in self.UserMapping[i]["values"]:
                if value["type"] == index:
                    value["type"] = type
        self.RemoveMappingEntry(index)
        self.RemoveEntry(index)

    """
    Return a copy of the node
    """
    def Copy(self):
        return cPickle.loads(cPickle.dumps(self))

    """
    Return a sorted list of indexes in Object Dictionary
    """
    def GetIndexes(self):
        listindex = self.Dictionary.keys()
        listindex.sort()
        return listindex

    """
    Print the Dictionary values
    """
    def Print(self):
        listindex = self.Dictionary.keys()
        listindex.sort()
        for index in listindex:
            print "%04X : %s"%(index, self.Dictionary[index])    
