<% extend base.tpl %>

<% block title %>
Programme
<% endblock title %>

<% block content %>
<h4>Programme &Uuml;bersicht</h4>
<table border="0">
<tr>
  <th>Program Nummer</th>
  <th>Programm Modus</th>
  <th>Programm</th>
</tr>
<% for program in ProgramList Programs %>
  <tr>
     <td><a href="editprogram.html?program=<% int program.id %>"><% int program.id %></a></td>
     <td><% String program.modus %></td>
     <td><% String program.description %></td>
  </tr>
<% endfor %>
  
</table>
<% endblock content %>




