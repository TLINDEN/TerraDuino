<% extend base.tpl %>

<% block title %>
Zuordnung Steuerkanal zu Programm Einstellen
<% endblock title %>

<% block content %>
<h4>Zuordnung Steuerkanal zu Programm Einstellen</h4>

<p>
</p>

<span style="color: green"><% String message %></span>

<form name="setprogram" action="setprogram.html" method="POST">

Steuerkanal: <% name %><br/>

<input type="hidden" name="id" value="<% int id %>"/>

Zugeordnetes Programm:
<select name="program">
<% for program in ProgramList Programs %>
  <option value="<% int program.id %>" <% String program.current %>><% String program.description %></option>
<% endfor %>
</select>

<input type="submit" name="submit" value="Einstellen"/>

</form>

<% endblock content %>




