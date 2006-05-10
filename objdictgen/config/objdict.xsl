<?xml version="1.0" encoding='ISO-8859-1' ?>
<!--
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" indent="yes"
              doctype-public="-//W3C//DTD HTML 4.01 Transitional//EN"/>

  <xsl:template match="node">
    <html>
      <head><title>CANOpen object dictionary Configuration</title>
      <style type="text/css">
         table {
	  border: 1px solid #200088;
          
        }
	  td {
	  border: 1px solid #200088;
        }
	
         h1 {
	   background: #FFDD66;
	   border: 3px solid #AA0000;
           margin: 2em;
           padding: 1em;
	 }

         h2, h2.dico, h2.pdo {
	   background: #5577FF;
	   border: 1px solid #AA0000;
           margin: 1em;
	 }

	  .entree {
	  color: #AA0000;
	  }
         
	  .title {
	  text-decoration: underline;
	  background: #BBCCEE;
          }

	  .default {
	  font-size: smaller;
	  }

	</style>

      </head>
      <body  bgcolor="#ffffff" text="#000000">
	
	<h1>Object dictionary for <em class="entree"><xsl:apply-templates select="@type_node"/></em> Node : <em class="entree"> <xsl:apply-templates select="@name" /> </em> </h1>
	<h2>Node identity </h2>
	<ul>
	  <li>Device type (index 0x1000) :
	    <xsl:choose>
	      <xsl:when test="@device_type_1000"> 
		<em class="entree">  <xsl:apply-templates select="@device_type_1000"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li>Manufacturer device name (index 0x1008) :
	    <xsl:choose>
	      <xsl:when test="@manufacturer_device_name_1008"> 
		<em class="entree">  <xsl:apply-templates select="@manufacturer_device_name_1008"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li>Manufacturer hardware version (index 0x1009) :
	    <xsl:choose>
	      <xsl:when test="@manufacturer_hardware_version_1009"> 
		<em class="entree">  <xsl:apply-templates select="@manufacturer_hardware_version_1009"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined. Default is the compilation date of objdict.c</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li> Manufacturer software version (index 0x100A) :
	    <xsl:choose>
	      <xsl:when test="@manufacturer_software_version_100A"> 
		<em class="entree">  <xsl:apply-templates select="@manufacturer_software_version_100A"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined. Default is the compilation time of objdict.c</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li>Vendor id (index 0x1018, subindex 1) :
	    <xsl:choose>
	      <xsl:when test="@vendor_id_1018"> 
		<em class="entree">  <xsl:apply-templates select="@vendor_id_1018"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li>Product code (index 0x1018, subindex 2) :
	    <xsl:choose>
	      <xsl:when test="@product_code_1018"> 
		<em class="entree">  <xsl:apply-templates select="@product_code_1018"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li>Revision number (index 0x1018, subindex 3) :
	    <xsl:choose>
	      <xsl:when test="@revision_number_1018"> 
		<em class="entree">  <xsl:apply-templates select="@revision_number_1018"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>
	  <li>Serial number (index 0x1018, subindex 4) :
	    <xsl:choose>
	      <xsl:when test="@serial_number_1018"> 
		<em class="entree">  <xsl:apply-templates select="@serial_number_1018"/> </em>
	      </xsl:when>
	      <xsl:otherwise>
		<em class="default">Undefined</em>
	      </xsl:otherwise>
	    </xsl:choose>
	  </li>	  	  
	</ul>
	<xsl:apply-templates select="heartbeat_consumers"/>
	<xsl:apply-templates select="sdo_clients"/>
	<xsl:apply-templates select="pdo"/>
	<h2>Mapped variables and tables</h2>	
	<p>
	  The access of the variable is by default "rw". A read-only variable cannot be mapped in a PDO receive.
	</p>
	<p>Here are some others rules for the mapping : </p>
	<ul>
	  <li>At an Index, you can map a variable at subindex 0x0 or 0x1, as you like.</li>
	  <li>To map several variables at the same Index, you must start at subindex 0x1,
	  because in this case, the subindex 0x0 contains the number of subindex.</li>
	  <li>You cannot map variables and tables at the same index.</li>
	  <li>The mapping of a table is always starting at subindex 0x1.</li>
	</ul>
        <table class="mapping">
          <tr class="title">
             <td>Var Name</td><td>Bits</td><td>Index</td><td>Sub-index</td><td>access</td><td>Type (UNS/REAL)</td><td>Min value</td><td>Max value</td>
          </tr>
	<xsl:apply-templates select="mapped_variable"/>
	<tr class="title">
	  <td>Table Name</td><td>Bits</td><td>Index</td><td>Sub-index</td><td>access</td><td>Type (UNS/REAL)</td><td>Min value</td><td>Max value</td>
	</tr>
	<xsl:apply-templates select="mapped_table"/>
	<tr class="title">
	  <td>String Name</td><td>Bytes</td><td>Index</td><td>Sub-index</td><td>access</td>
	</tr>
	<xsl:apply-templates select="mapped_string_variable"/>
	</table>
      </body>
    </html>
  </xsl:template>


  <xsl:template match="heartbeat_consumers">
    <h2>Number of heartbeat consumers : 
    <em class="entree"><xsl:apply-templates select="@nombre" /></em>
    </h2>
    <p>This means that the node can expect heartbeats sent by <xsl:apply-templates select="@nombre" /> nodes. Canfestival limitation : You must at least have one.</p>
  </xsl:template>

  <xsl:template match="sdo_clients">
    <h2>Number of SDO clients : 
    <em class="entree"><xsl:apply-templates select="@nombre" /></em>
    </h2>
    <p>Usualy, a slave node does not have the use of SDO clients, but
    today - it is a Canfestival limitation -  you must define at least one. 
    </p>
    <p>
      The Master, which can
    send SDO to "n" slaves nodes, must define here "n" SDO
    clients. Remember that in most cases, the SDO protocol is used by
    the master to configure a slave dictionary or read a value. In
    this use, the master is the client, and the slave is the server.</p>
    <h2>Number of SDO servers : 
    <em class="default">1 (cannot be changed)</em>
    </h2>
    <p>A Canfestival node must have exactly one SDO server, defined at index 0x1200. The user cannot change this.</p> 
  </xsl:template>


  <xsl:template match="pdo">
    <h2>PDO 
    <xsl:choose>
      <xsl:when test="@type_rx_tx='rx'"> 
	<em class="entree"> receive. </em>
      </xsl:when>
     <xsl:when test="@type_rx_tx='tx'"> 
	<em class="entree">  transmit. </em>
      </xsl:when>
      <xsl:otherwise>
	<em class="default">Undefined. Not normal !!!!</em>
      </xsl:otherwise>
    </xsl:choose>
	CobId : 
	<xsl:choose>
	  <xsl:when test="@cob_id"> 
	    <em class="entree">  <xsl:apply-templates select="@cob_id"/> </em>
	  </xsl:when>
	  <xsl:otherwise>
	    <em class="default">Undefined (Using default).</em>
	  </xsl:otherwise>
	</xsl:choose>
	Index communication parameter : 
	<xsl:choose>
	  <xsl:when test="@index_communication_parameter"> 
	    <em class="entree">  <xsl:apply-templates select="@index_communication_parameter"/> </em>
	  </xsl:when>
	  <xsl:otherwise>
	    <em class="default">Undefined (Using default).</em>
	  </xsl:otherwise>
	</xsl:choose>
    </h2>
    <ul>
      <li>
	Maximum of objects (ie variables) that can be embeded : 
	<xsl:choose>
	  <xsl:when test="@max_objects_in_pdo"> 
	    <em class="entree">  <xsl:apply-templates select="@max_objects_in_pdo"/> </em>
	  </xsl:when>
	  <xsl:otherwise>
	    <em class="default">Undefined (Using default : 8).</em>
	  </xsl:otherwise>
	</xsl:choose>
      </li>
      <li>
	Transmission type :
	<xsl:choose>
	  <xsl:when test="@transmission_type"> 
	    <em class="entree">  <xsl:apply-templates select="@transmission_type"/> </em>
	  </xsl:when>
	  <xsl:otherwise>
	    <em class="default">Undefined (Using default : 253).</em>
	  </xsl:otherwise>
	</xsl:choose>
      </li>
    </ul>
    <h3>Default mapped objects :</h3>
    <table class="mapping">
      <tr class="title">
	<td>Var Name</td><td>Bits</td><td>Index</td><td>Sub-index</td>
      </tr>
      <xsl:apply-templates select="mapped_object"/>
    </table>


    <p>About the cobId : The 4th first PDO receive defined at index 0x1400 to 0x1403, or transmit (0x1800 to 0x1803) are changed at runtime according to the DS 301, whatever the value you put here.</p>
    <p>If the Index communication parameter is not defined, have a look at objdict.c to know what index have beed created.</p>
    <p>It is not required to map some objects in a PDO. It can also be done by the node itself at runtime, or by an other node whith the SDO functionalitie.</p>
  </xsl:template>
  



  <xsl:template match="mapped_object">
    <tr>
      <xsl:choose>
	<xsl:when test="@name"> 
	  <td><em class="entree">  <xsl:apply-templates select="@name"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <td><em class="entree"><xsl:apply-templates select="@size_in_bits" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@index" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@sub_index" /></em></td>     
    </tr>
  </xsl:template>

  <xsl:template match="mapped_variable">
    <tr>
      <td><em><xsl:apply-templates select="@name" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@size_in_bits" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@index" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@sub_index" /></em></td>     
      <xsl:choose>
	<xsl:when test="@access"> 
	  <td><em class="entree">  <xsl:apply-templates select="@access"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined. Default is RW</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
	<xsl:when test="@type"> 
	  <td><em class="entree">  <xsl:apply-templates select="@type"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined. Default is UNS</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
	<xsl:when test="@min_value"> 
	  <td><em class="entree">  <xsl:apply-templates select="@min_value"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
	<xsl:when test="@max_value"> 
	  <td><em class="entree">  <xsl:apply-templates select="@max_value"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined</em></td>
	</xsl:otherwise>
      </xsl:choose>
    </tr>
  </xsl:template>

  <xsl:template match="mapped_table">
    <tr>
      <td><em><xsl:apply-templates select="@name" /></em>[<em class="entree"><xsl:apply-templates select="@number_elements" /></em>]</td>
      <td><em class="entree"><xsl:apply-templates select="@size_in_bits" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@index" /></em></td>
      <td>1 to <em class="entree"><xsl:apply-templates select="@number_elements" /></em></td>    
      <xsl:choose>
	<xsl:when test="@access"> 
	  <td><em class="entree">  <xsl:apply-templates select="@access"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined. Default is RW</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
	<xsl:when test="@type"> 
	  <td><em class="entree">  <xsl:apply-templates select="@type"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined. Default is UNS</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
	<xsl:when test="@min_value"> 
	  <td><em class="entree">  <xsl:apply-templates select="@min_value"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined</em></td>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:choose>
	<xsl:when test="@max_value"> 
	  <td><em class="entree">  <xsl:apply-templates select="@max_value"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined</em></td>
	</xsl:otherwise>
      </xsl:choose>
    </tr>
  </xsl:template>

  <xsl:template match="mapped_string_variable">
    <tr>
      <td><em><xsl:apply-templates select="@name" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@size_in_byte" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@index" /></em></td>
      <td><em class="entree"><xsl:apply-templates select="@sub_index" /></em></td>     
      <xsl:choose>
	<xsl:when test="@access"> 
	  <td><em class="entree">  <xsl:apply-templates select="@access"/> </em></td>
	</xsl:when>
	<xsl:otherwise>
	  <td><em class="default">Undefined. Default is RW</em></td>
	</xsl:otherwise>
      </xsl:choose>
    </tr>
  </xsl:template>

</xsl:stylesheet>
