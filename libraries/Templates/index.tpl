<% extend base.tpl %>

<% block title %>
Home
<% endblock title %>

<% block content %>
<h4>&Uuml;bersicht</h4>
<table border="0">
<tr>
  <td align="right">Temperatur:</td><td><% float temp %>&deg; C</td>
</tr>

<tr>
  <td align="right">Luftfeuchte:</td><td><% float humidity %>%</li>
</tr>

<tr>
  <td align="right">Zeit:</td><td><% int day %>.<% int month %>.<% int year %> <% int hour %>:<% int minute %>:<% int second %></td>
</tr>

<tr>
  <td align="right">Steuermodus:</td><td><% String modus %>%</li>
</tr>

<tr>
  <td align="right">Sonnenaufgang:</td><td><% int sunrisehour %>:<% int sunriseminute %></td>
</tr>

<tr>
  <td align="right">Sonnenuntergang:</td><td><% int sunsethour %>:<% int sunsetminute %></td>
</tr>

<tr>
  <td align="right">Status Steuerkan&auml;le:</td>
  <td>
<% for channel in ChannelList Channels %>
     <li><% String channel.name %>: <% int channel.operate %></li>
<% endfor %>
  </td>
</tr>

  
</table>
<% endblock content %>




