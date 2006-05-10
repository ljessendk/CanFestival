#!/bin/sh

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

# Xbootclasspath option added to workaround bugs in Java 1.4
java -Xbootclasspath/p:jaxe/lib/xml-apis.jar:jaxe/lib/xerces.jar:jaxe/lib/xalan.jar -classpath jaxe/lib/Jaxe.jar:jaxe/lib/xml-apis.jar:jaxe/lib/xerces.jar:jaxe/lib/xalan.jar:jaxe/lib/jakarta-oro-2.0.8.jar:jaxe/lib/jazzy.jar jaxe/Jaxe config/objdict_Jaxe_cfg.xml 
