<?xml version="1.0"?><!--
	Copyright 2022 MicroEJ Corp. All rights reserved.
	Use of this source code is governed by a BSD-style license that can be found with this software.
-->
<frontpanel 
	xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
	xmlns="https://developer.microej.com" 
	xsi:schemaLocation="https://developer.microej.com .widget.xsd">
	
	<device name="AMAP4PEVB" skin="images/board.png">
		<ej.fp.widget.Display x="433" y="215" width="454" height="454" filter="mask_454.png"/>
		<ej.fp.widget.Pointer x="433" y="215" width="454" height="454" touch="true"/>
		<ej.fp.widget.Button label="0" x="1119" y="722" skin="images/button0.png" pushedSkin="images/button0Pushed.png" listenerClass="com.is2t.microej.fp.ButtonListener"/>
	</device>
</frontpanel>