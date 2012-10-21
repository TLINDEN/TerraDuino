<% extend base.tpl %>

<% block title %>
Temperatur
<% endblock title %>

<% block content %>
<ul>
  <li>Temperatur: <% float temp %>&deg; C</li>
  <li>Luftfeuchte: <% float humidity %>%</li>
</ul>
<% endblock content %>




