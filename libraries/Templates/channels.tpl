<% extend base.tpl %>

<% block title %>
Kan&auml;le
<% endblock title %>

<% block content %>
<h4>Steuer Kan&auml;e &Uuml;bersicht</h4>
<table border="0">
<tr>
  <th>Kanal</th>
  <th>Programm Nummer</th>
  <th>Programm Modus</th>
  <th>Programm</th>
</tr>
<% for channel in ChannelList Channels %>
  <tr>
     <td><a href="setprogram.html?channel=<% channel.id %>"><% String channel.name %></a></td>
     <td><% int channel.programid %></td>
     <td><% String channel.modus %></td>
     <td><% String channel.program %></td>
  </tr>
<% endfor %>
  
</table>
<% endblock content %>




