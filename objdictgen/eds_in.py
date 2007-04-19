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

from xml.parsers import expat

import node
from node import *

maxObjects = 8

currentPDOIndex = 0
currentBitsMapped = 0
currentMaxObjects = 0
currentNbMappedObjects = 0

nextPdoIndex = {"rx":0x1400,"tx":0x1800}

valid_elements = ["node","heartbeat_consumers","sdo_clients","pdo","mapped_object",
    "pdo_param","pdo_receive","pdo_transmit","mapped_variable","mapped_table",
    "mapped_string_variable","mapped_string_table"]

#-------------------------------------------------------------------------------
#                       Callback method of parse
#-------------------------------------------------------------------------------

def StartElement(name, attrs):
    if name in valid_elements:
        if name == "node":
            startNode(attrs)
        elif name == "heartbeat_consumers":
            startHeartBeatConsumers(attrs)
        elif name == "sdo_clients":
            startSdoClients(attrs)
        elif name in ["pdo_param","pdo_receive","pdo_transmit"]:
            raise ValueError, """!!! The XML grammar has changed.
Please, open your xml file, delete the tags pdo_param, pdo_receive and pdo_transmit.
Use instead the tag pdo for each pdo to create, and (optional) use the tag mapped_object (menu pdo/map and object ...)."""
        elif name == "pdo":
            startPdo(attrs)
        elif name == "mapped_object":
            startMappedObject(attrs)
        elif name == "mapped_variable":
            startMappedVariable(attrs)
        elif name == "mapped_table":
            startMappedTable(attrs)
        elif name == "mapped_string_variable":
            startMappedVariable(attrs)
        elif name == "mapped_string_table":
            startMappedTable(attrs)

def EndElement(name):
    if name in valid_elements:
       if name == "node":
           stopNode()

def CharacterData(data):
    pass

#-------------------------------------------------------------------------------
#                          Creation of Node
#-------------------------------------------------------------------------------

def startNode(attrs):
    name = attrs["name"]
    Node.SetNodeName(name)
    
    if "node_id" in attrs and len(attrs["node_id"]) > 0:
        node_id = eval(attrs["node_id"])
    else:
        node_id = 0x01  # We define here a default node_id. 
    Node.SetNodeID(node_id)
    
    typeNode = attrs["type_node"]
    Node.SetNodeType(typeNode)
    
    if "device_type_1000" in attrs:
        device_type = eval(attrs["device_type_1000"])
    else:
        device_type = 0
    Node.AddEntry(0x1000, 0, device_type)
    Node.AddEntry(0x1001, 0, 0)
    Node.AddEntry(0x1005, 0, 0x00000080)
    Node.AddEntry(0x1006, 0, 0)
    Node.AddEntry(0x1007, 0, 0)

    if "manufacturer_device_name_1008" in attrs:
        manufacturer_device_name = attrs["manufacturer_device_name_1008"]
    else:
        manufacturer_device_name = ""
    Node.AddEntry(0x1008, 0, manufacturer_device_name)
    
    if "manufacturer_hardware_version_1009" in attrs:
        manufacturer_hardware_version = attrs["manufacturer_hardware_version_1009"]
    else:
        manufacturer_hardware_version = "__DATE__"
    Node.AddEntry(0x1009, 0, manufacturer_hardware_version)

    if "manufacturer_software_version_100A" in attrs:
        manufacturer_software_version = attrs["manufacturer_software_version_100A"]
    else:
        manufacturer_software_version = 0
    Node.AddEntry(0x100A, 0,  manufacturer_software_version)

    if "vendor_id_1018" in attrs:
        vendor_id = eval(attrs["vendor_id_1018"])
    else:
        vendor_id = 0
    if "product_code_1018" in attrs:
        product_code = eval(attrs["product_code_1018"])
    else:
        product_code = 0
    if "revision_number_1018" in attrs:
        revision_number = eval(attrs["revision_number_1018"])
    else:
        revision_number = 0
    if "serial_number_1018" in attrs:
        serial_number = eval(attrs["serial_number_1018"])
    else:
        serial_number = 0
    Node.AddEntry(0x1018, 1, vendor_id)
    Node.AddEntry(0x1018, 2, product_code)
    Node.AddEntry(0x1018, 3, revision_number)
    Node.AddEntry(0x1018, 4, serial_number)

def stopNode():
    heartBeatProducer()
    sdoServer()

#-------------------------------------------------------------------------------
#                      Creation of PDO in Object Dictionary
#-------------------------------------------------------------------------------

def startPdo(attrs):
    global currentPdoIndex
    global currentMaxObjects
    global currentNbMappedObjects
    global currentBitsMapped
    global maxObjects
    
    cobId = 0
    transmissionType = 253 # Default is on request. Why not ?

    # Find the type of the PDO and search the index of the last added
    type = attrs["type_rx_tx"]
    index = nextPdoIndex[type]
    
    # If the index of the PDO is define, verify that it has a good index
    if "index_communication_parameter" in attrs:
        index = eval(attrs["index_communication_parameter"])
    if type == "rx" and not 0x1400 <= index <= 0x15FF:
        raise ValueError, """!!! Abort because Index PDO receive : 0x%04X not valid.
Valid index is 0x1400 ... 0x15FF"""%index
    if type == "tx" and not 0x1800 <= index <= 0x19FF:
        raise ValueError, """!!! Abort because Index PDO transmit : 0x%04X not valid.
Valid index is 0x1800 ... 0x19FF"""%index
    
    # Extract the PDO communication parameters
    if "cob_id" == attrs:
        cobId = eval(attrs["cob_id"])
    if "max_objects_in_pdo" == attrs:
        maxObjects = eval(attrs["max_objects_in_pdo"])
    if "transmission_type" in attrs:
        transmissionType = eval(attrs["transmission_type"])

    if Node.IsEntry(index):
        raise ValueError, """!!! Abort because the PDO at index : 0x%04X have been already defined."""%index

    # Communication parameters
    Node.AddEntry(index, 1, cobId)
    Node.AddEntry(index, 2, transmissionType)
    
    # Mapping parameters
    mapping_index = index + 0x200
    for i in xrange(1, maxObjects + 1):
        Node.AddEntry(mapping_index, i, 0x0)

    currentPdoIndex = index
    currentMaxObjects = maxObjects
    currentBitsMapped = 0
    currentNbMappedObjects = 0
    
    nextPdoIndex[type] = index + 1

def startMappedObject(attrs):
    global currentPdoIndex
    global currentMaxObjects
    global currentNbMappedObjects
    global currentBitsMapped
    
    index = currentPdoIndex
    mapping_index = index + 0x200
    
    indexObject = eval(attrs["index"])
    subIndexObject = eval(attrs["sub_index"])
    sizeInBitsObject = eval(attrs["size_in_bits"])

    if currentMaxObjects == 0:
        raise ValueError, """!!! Abort because of a bogue for mapped object (defined at index 0x%04X, subIndex 0x%025X)
in PDO. index : 0x%04X is undefined."""%(indexObject,subindexObject,mapping_index)
    if currentNbMappedObjects >= currentMaxObjects:
        raise ValueError, """!!! Abort mapping object (defined at index 0x%04X, subIndex 0x%02X)
in PDO index 0x%04X. max objects (%d) reached."""%(IndexObject,subIndexObject,mapping_index,pdo[mapping_index]["maxObjects"])
    if currentBitsMapped + sizeInBitsObject > 64:
        raise ValueError, """!!! Abort mapping object (defined at index 0x%04X, subIndex 0x%02X)
in PDO index 0x%04X. No room to put %d bits in the PDO."""%(IndexObject,subIndexObject,mapping_index,sizeInBitsObject)

    value = eval("0x%04X%02X%02X"%(indexObject,subIndexObject,sizeInBitsObject))
    Node.SetEntry(mapping_index, currentNbMappedObjects + 1, value)
    
    currentNbMappedObjects += 1
    currentBitsMapped += sizeInBitsObject

#-------------------------------------------------------------------------------
#                    Creation of mapped variable and table 
#-------------------------------------------------------------------------------

def startMappedVariable(attrs):
    name = attrs["name"]
    index = eval(attrs["index"])
    subIndex = eval(attrs["sub_index"])
    
    if "size_in_bits" in attrs:
        size = eval(attrs["size_in_bits"]) # Numeric variable
        if "type" in attrs:
            type = attrs["type"]
            if (type == "UNS"):
                type = "UNSIGNED"
        else: # Default type
            type = "UNSIGNED"
        typename = "%s%d"%(type,size)

        type_index = Manager.GetTypeIndex(typename, False)
        if type_index == None:
            raise ValueError, """!!! ERROR : For variable "%s" at index 0x%04X, subindex 0x%02X : Unrecognized type : %s"""%(name,index,subIndex,typename)
        
        # Begin ValueRange support
        if "min_value" in attrs or "max_value" in attrs:
            if "min_value" in attrs and "max_value" in attrs:
                minValue = eval(attrs["min_value"])
                maxValue = eval(attrs["max_value"])
                if (minValue > maxValue):
                    raise ValueError, """!!! ERROR : For variable "%s" at index 0x%04X, subindex 0x%02X : error in value-range : min > max"""%(name,index,subIndex)
            else:
                raise ValueError, """!!! ERROR : For variable "%s" at index 0x%04X, subindex 0x%02X : You have defined only a min or a max value. \nIf you define one, you must define both."""%(name,index,subIndex)
        
            type_index = findRangeType(type_index, minValue, maxValue)
            if type_index == None:
                raise ValueError, """!!! Sorry, too many different value range have been defined"""
        # End ValueRange support    

    if "size_in_byte" in attrs:
        size = eval(attrs["size_in_byte"]) # String variable
        type_index = findStringType(Manager.GetTypeIndex("VISIBLE_STRING", False), size)
        if type_index == None:
            raise ValueError, """!!! Sorry, too many different string length have been defined"""
          
    if "access" in attrs:
        access = attrs["access"].lower()
    else: 
        access = "rw" # default value
 
    if index < 0x2000 or index > 0xBFFF:
        raise ValueError, """!!! ERROR : For variable "%s" at index 0x%04X, subindex 0x%02X : Variable can't be defined using this index-subindex."""%(name,index,subIndex)

    if subIndex == 0:
        Node.AddMappingEntry(index, name = name, struct = 1)
    elif subIndex == 1:
        Node.AddMappingEntry(index, struct = 3)
        Node.AddMappingEntry(index, 0, values = {"name" : "Number of Entries", "type" : 0x02, "access" : "ro", "pdo" : False})    
    result = Node.AddMappingEntry(index, subIndex, values = {"name" : name, "type" : type_index, "access" : access, "pdo" : True})
    
    if result:
        Node.AddEntry(index, subIndex, 0)
    else:
        raise ValueError, """!!! ERROR : For variable "%s" at index 0x%04X, subindex 0x%02X : Unable to map"""%(name,index,subIndex)

def startMappedTable(attrs):
    name = attrs["name"]
    number_elements = eval(attrs["number_elements"])
    index = eval(attrs["index"])

    if "size_in_bits" in attrs:
        size = eval(attrs["size_in_bits"]) # Numeric variable
        if "type" in attrs:
            type = attrs["type"]
            if (type == "UNS"):
                type = "UNSIGNED"
        else: # Default type
            type = "UNSIGNED"
        typename = "%s%d"%(type,size)

        type_index = Manager.GetTypeIndex(typename, False)
        if type_index == None:
            raise ValueError, """!!! ERROR : For table \"%s\" at index 0x%04X : Unrecognized type : %s"""%(name,index,typename)
        
        # Begin ValueRange support
        if "min_value" in attrs or "max_value" in attrs:
            if "min_value" in attrs and "max_value" in attrs:
                minValue = eval(attrs["min_value"])
                maxValue = eval(attrs["max_value"])
                if (minValue > maxValue):
                    raise ValueError, """!!! ERROR : For table \"%s\" at index 0x%04X : error in value-range : min > max"""%(name,index)
            else:
                raise ValueError, """!!! ERROR : For table \"%s\" at index 0x%04X : You have defined only a min or a max value. \nIf you define one, you must define both."""%(name,index)
        
            type_index = findRangeType(type_index, minValue, maxValue)
            if type_index == None:
                raise ValueError, """!!! Sorry, too many different value range have been defined"""
        # End ValueRange support

    if "size_in_byte" in attrs:
        size = eval(attrs["size_in_byte"]) # String variable
        type_index = findStringType(Manager.GetTypeIndex("VISIBLE_STRING", False), size)
        if type_index == None:
            raise ValueError, """!!! Sorry, too many different string length have been defined"""
	      
    if "access" in attrs:
        access = attrs["access"].lower()
    else:
        access = "rw" # default value    

    if index < 0x2000 or index > 0xBFFF:
        raise ValueError, """!!! ERROR : For table \"%s\" at index 0x%04X : Variable can't be defined using this index-subindex."""%(name,index)

    result = Node.AddMappingEntry(index, name = name, struct = 7)
    if not result:
        raise ValueError, """!!! ERROR : For table \"%s\" at index 0x%04X : Unable to map because a variable or a table is using this index"""%(name,index)
    Node.AddMappingEntry(index, 0, values = {"name" : "Number of Entries", "type" : 0x02, "access" : "ro", "pdo" : False})
    Node.AddMappingEntry(index, 1, values = {"name" : name, "type" : type_index, "access" : access, "pdo" : True, "nbmax" : number_elements})
    
    for subIndex in xrange(1,number_elements+1):
        Node.AddEntry(index, subIndex, 0)

def findRangeType(type, minValue, maxValue):
    index = 0xA0
    while index < 0x100 and Node.IsEntry(index):
        current_type = Node.GetEntry(index, 1)
        if current_type == type:
            current_minValue = Node.GetEntry(index, 2)
            current_maxValue = Node.GetEntry(index, 3)
            if current_minValue == minValue and current_maxValue == maxValue:
                return index
        index += 1
    if index < 0x100:
        infos = Manager.GetEntryInfos(type, False)
        name = "%s[%d-%d]"%(infos["name"], minValue, maxValue)
        Node.AddMappingEntry(index, name = name, struct = 3, size = infos["size"], default = infos["default"])
        Node.AddMappingEntry(index, 0, values = {"name" : "Number of Entries", "type" : 0x02, "access" : "ro", "pdo" : False})
        Node.AddMappingEntry(index, 1, values = {"name" : "Type", "type" : 0x02, "access" : "ro", "pdo" : False})
        Node.AddMappingEntry(index, 2, values = {"name" : "Minimum Value", "type" : type, "access" : "ro", "pdo" : False})
        Node.AddMappingEntry(index, 3, values = {"name" : "Maximum Value", "type" : type, "access" : "ro", "pdo" : False})
        Node.AddEntry(index, 1, type)
        Node.AddEntry(index, 2, minValue)
        Node.AddEntry(index, 3, maxValue)
        return index
    return None

def findStringType(type, length):
    index = 0xA0
    while index < 0x100 and Node.IsEntry(index):
        current_type = Node.GetEntry(index, 1)
        if current_type == type:
            current_length = Node.GetEntry(index, 2)
            if current_length == length:
                return index
        index += 1
    if index < 0x100:
        infos = Manager.GetEntryInfos(type, False)
        name = "%s%d"%(Manager.GetTypeName(type), length)
        Node.AddMappingEntry(index, name = name, struct = 3, size = infos["size"], default = infos["default"])
        Node.AddMappingEntry(index, 0, values = {"name" : "Number of Entries", "type" : 0x02, "access" : "ro", "pdo" : False})
        Node.AddMappingEntry(index, 1, values = {"name" : "Type", "type" : 0x02, "access" : "ro", "pdo" : False})
        Node.AddMappingEntry(index, 2, values = {"name" : "Length", "type" : 0x02, "access" : "ro", "pdo" : False})
        Node.AddEntry(index, 1, type)
        Node.AddEntry(index, 2, length)
        return index
    return None

#-------------------------------------------------------------------------------
#                   Creation HeartBeat Producer & Consumers
#-------------------------------------------------------------------------------

def heartBeatProducer():
    Node.AddEntry(0x1017, 0, 0)
    
def startHeartBeatConsumers(attrs):
    nombre = eval(attrs["nombre"])
    for i in xrange(nombre):
        Node.AddEntry(0x1016, i + 1, 0)		    

#-------------------------------------------------------------------------------
#                       Creation of SDO Server & Clients
#-------------------------------------------------------------------------------

def sdoServer():
    Node.AddEntry(0x1200, 1, 0x600 + Node.GetNodeID())
    Node.AddEntry(0x1200, 2, 0x580 + Node.GetNodeID())
    
def startSdoClients(attrs):
    nombre = eval(attrs["nombre"])
    for i in xrange(nombre):
        Node.AddEntry(0x1280 + i, 1, 0x600)
        Node.AddEntry(0x1280 + i, 2, 0x580)
        Node.AddEntry(0x1280 + i, 3, 0)
    
#-------------------------------------------------------------------------------
#                           Parse file with Saxe
#-------------------------------------------------------------------------------

def ParseFile(filepath):
    return dict([(                          # Dict containing whole Eds
              blocktuple[0],                # EntryName : Assignements dict
              dict([assignment.split('=')   #  Name : Value
                for assignment in           # Assignements are
                blocktuple[-1].splitlines()    # all the lines
                if assignment.find('=') > 0 ]  # that contains '='
               )) for blocktuple in [       # Split the eds files into
                 block.split("]")           # (EntryName,Assignements) tuple
                 for block in               # for each blocks staring with '['
                 open(filepath,'r').read().split("[")]
               if blocktuple[0].isalnum()]) # if EntryName exists

def GenerateNode(filepath, manager):
    global Node
    global Manager
    Manager = manager
    Node = node.Node()
    eds_dict = ParseFile(filepath)
    ProfileNb=int(eds_dict["1000"]["DefaultValue"],16) & 0x0000ffff
    if ProfileNb not in [301, 302]:
        execfile("config/DS-%d.prf"%ProfileNb)
        Node.SetProfileName("DS-%d"%ProfileNb)
        Node.SetProfile(Mapping)
        Node.SetSpecificMenu(AddMenuEntries)
    for Entry in ParseFile(filepath):
        pass
        
    return Node

#-------------------------------------------------------------------------------
#                             Main Function
#-------------------------------------------------------------------------------

if __name__ == '__main__':
    ParseFile("test.xml")

