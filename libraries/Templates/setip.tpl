<% extend base.tpl %>

<% block title %>
IP Adresse Einstellen
<% endblock title %>

<% block content %>
<h4>IP Adresse Einstellen</h4>

<p>
Geben Sie eine statische IP Adresse ein, die in Ihrem LAN
noch nicht in Verwendung ist. Booten Sie den Terraduino
anschliessend.<br/>
Wenn Sie einen Fehler gemacht haben sollten und Sie den
Terraduino nicht mehr erreichen k&ouml;nnen, m&uuml;ssen
Sie ihn resetten: Serielle Schnittstelle (9600,1,N), booten,
"R" eingeben (Ohne Anf&uuml;hrungszeichen!). Der Terraduino
bootet dann mit der default Adresse: <b>10.0.0.1</b>.
</p>

<span style="color: green"><% String message %></span>

<form name="setip" action="setip.html" method="POST">

IP Adresse:
<input type="text" name="octet1" value="<% uint8_t octet1 %>" maglength="3" size="3"/>.
<input type="text" name="octet2" value="<% uint8_t octet2 %>" maglength="3" size="3"/>.
<input type="text" name="octet3" value="<% uint8_t octet3 %>" maglength="3" size="3"/>.
<input type="text" name="octet4" value="<% uint8_t octet4 %>" maglength="3" size="3"/>
<br/>

<input type="submit" name="submit" value="Einstellen"/>

</form>

<% endblock content %>




