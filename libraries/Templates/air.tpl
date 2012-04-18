<% extend base.tpl %>

<% block title %>
Klima Einstellung
<% endblock title %>

<% block content %>
<h4>Klima Einstellung</h4>

<p>
Stellen Sie ein, ob bei Temperatur&auml;nderungen der L&uuml;fter
eingeschaltet werden soll und bei welchen Grenzwerten.
</p>

<span style="color: green"><% String message %></span>

<form name="setdate" action="air.html" method="POST">

<table border="0">
 <tr>
   <td align="right">Aktivieren</td>
   <td>
     <select name="active">
<% for air in AirList Air %>
     <option value="<% int air.id %>" <% String air.current %>><% String air.description %></option>
<% endfor %>
     </select>
   </td>
 </tr>

 <tr>
   <td align="right">Tmin (schaltet L&uuml;fter aus):</td>
   <td><input name="tmin" type="text" size="2" maxlength="2" value="<% int tmin %>"/></td>
 </tr>

 <tr>
   <td align="right">Tmax (schaltet L&uuml;fter ein):</td>
   <td><input name="tmax" type="text" size="2" maxlength="2" value="<% int tmax %>"/></td>
 </tr>

<tr>
  <td colspan="2">
    <input type="submit" name="submit" value="Einstellen"/>
  </td>
</tr>
  
</table>

</form>

<% endblock content %>




