<% extend base.tpl %>

<% block title %>
Datum und Uhrzeit Einstellen
<% endblock title %>

<% block content %>
<h4>Datum und Uhrzeit Einstellen</h4>

<p>Geben Sie die Werte ein und Dr&uuml;cken Sie "Einstellen".
Es ist insbesondere bei der Uhrzeit ratsam, sich zu beeilen.</p>

<span style="color: green"><% String message %></span>

<form name="setdate" action="setdate.html" method="POST">

<table border="0">
 <tr>
   <td align="right">Tag:</td>
   <td><input name="day" type="text" size="2" maxlength="2" value="<% int day %>"/></td>
 </tr>

 <tr>
   <td align="right">Monat:</td>
   <td><input name="month" type="text" size="2" maxlength="2" value="<% int month %>"/></td>
 </tr>

 <tr>
   <td align="right">Jahr:</td>
   <td><input name="year" type="text" size="4" maxlength="4" value="<% int year %>"/></td>
 </tr>

 <tr>
   <td align="right">Stunde:</td>
   <td><input name="hour" type="text" size="2" maxlength="2" value="<% int hour %>"/></td>
 </tr>

 <tr>
   <td align="right">Minute:</td>
   <td><input name="minute" type="text" size="2" maxlength="2" value="<% int minute %>"/></td>
 </tr>

 <tr>
   <td align="right">Sekunde:</td>
   <td><input name="second" type="text" size="2" maxlength="2" value="<% int second %>"/></td>
 </tr>

 <tr>
   <td align="right">Sommerzeit:</td>
   <td><input name="dst" type="text" size="1" maxlength="1" value="<% int dst %>"/>
     (0 = Winterzeit, 1 = Sommerzeit)
   </td>
 </tr>

<tr>
  <td colspan="2">
    <input type="submit" name="submit" value="Einstellen"/>
  </td>
</tr>
  
</table>

</form>

<% endblock content %>




