<% extend base.tpl %>

<% block title %>
Programm Einstellen
<% endblock title %>

<% block content %>
<h4>Programm Einstellen</h4>

<p>
</p>

<span style="color: green"><% String message %></span>

<form name="editprogram" action="editprogram.html" method="POST">

Program: <% id %><br/>

<input type="hidden" name="id" value="<% int id %>"/>

<p>Einstellungen:</p>

<table border="0">
  <tr>
    <td align="right">Typ</td>
    <select name="type">
<% for type in TypeList Types %>
   <option value="<% int type.value %>" <% String type.current %>><% String type.description %></option>
<% endfor %>
    </select>
  </tr><tr>
    <td align="right">Start Stunde</td>
    <td><input type="text" name="start_hour" size="2" maxlength="2" value="<% int starthour %>"/></td>
  </tr><tr>
    <td align="right">Start Minute</td>
    <td><input type="text" name="start_min" size="2" maxlength="2" value="<% int startmin %>"/></td>
  </tr><tr>
    <td align="right">Stop Stunde</td>
    <td><input type="text" name="stop_hour" size="2" maxlength="2" value="<% int stophour %>"/></td>
  </tr><tr>
    <td align="right">Stop Minute</td>
    <td><input type="text" name="stop_min" size="2" maxlength="2" value="<% int stopmin %>"/></td>
  </tr><tr>
    <td align="right">Start Verz&ouml;gerung (Minuten)</td>
    <td><input type="text" name="start_delay" size="2" maxlength="2" value="<% int startdelay %>"/></td>
  </tr><tr>
    <td align="right">Stop Verz&ouml;gerung (Minuten)</td>
    <td><input type="text" name="stop_delay" size="2" maxlength="2" value="<% int stopdelay %>"/></td>
  </tr><tr>
    <td align="right">Einschalt Verz&ouml;gerung (Cooldown Zeit) (Sekunden)</td>
    <td><input type="text" name="cooldown" size="2" maxlength="2" value="<% int cooldown %>"/></td>
  </tr>
</table>

<input type="submit" name="submit" value="Einstellen"/>

</form>

<% endblock content %>




