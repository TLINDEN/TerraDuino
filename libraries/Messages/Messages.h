FLASH_STRING(f_failed_dht, "Failed to read from DHT");

FLASH_STRING(f_sht_temp, "Temperature: ");
FLASH_STRING(f_grad, " *C");
FLASH_STRING(f_percent, " %");
FLASH_STRING(f_colon, ": ");
FLASH_STRING(f_sht_hum, "   Humidity: ");
FLASH_STRING(f_sht_aircond, "   Air Cond: ");
FLASH_STRING(f_sht_run, "running");
FLASH_STRING(f_sht_tmin, "       Tmin: ");
FLASH_STRING(f_sht_tmax, "       Tmax: ");
FLASH_STRING(f_sht_off, "turned off");
FLASH_STRING(f_sht_som, "        DST: Sommerzeit");
FLASH_STRING(f_sht_win, "        DST: Winterzeit");
FLASH_STRING(f_sht_sunr, "    Sunrise: ");
FLASH_STRING(f_sht_suns, "     Sunset: ");

FLASH_STRING(f_sh_help,
	     "Available commands: \n"
	     "  r            - reset EEPROM configuration to defaults and do a\n"
	     "                 soft reset\n"
	     "  b            - do a soft reset\n"
	     "  s            - display T and H sensor data and date+time\n"
	     "  i            - show ip address\n"
	     "  i x.x.x.x    - set ip address\n"
	     "  d DD.MM.YYYY - set date\n"
	     "  t HH:MM:SS   - set time (enter wintertime!)\n"
	     "  a Tmin Tmax  - set aircondition, leave params to turn it off\n"
	     "  c            - dump channel config\n"
	     "  p            - dump program config\n"
	     "  P            - dump PIN states\n"
	     "  m            - show free memory\n"
	     "  h            - show help\n");

FLASH_STRING(f_sh_reset, "Resetting Config");
FLASH_STRING(f_sh_initres, "Initiating a soft reset");
FLASH_STRING(f_sh_inv, "Invalid Command (try 'h' to get help) ");

FLASH_STRING(f_shp_tit, "Current PIN states:");
FLASH_STRING(f_shp_head, "Switch | MAIN | CH1 | CH2 | CH3 | CH4 | CH5 | CH6 | CH7"); 
FLASH_STRING(f_shp_line, "-------+------+-----+-----+-----+-----+-----+-----+-----");
FLASH_STRING(f_shp_pin, "  PIN  |  ");
FLASH_STRING(f_shp_pipe, "| ");
FLASH_STRING(f_shp_pipe1, "|  ");
FLASH_STRING(f_shp_mode, " Mode  |   ");
FLASH_STRING(f_shp_rel, " Relay |      ");
FLASH_STRING(f_shp_2sp, "  ");

FLASH_STRING(f_sherr_char, "Error: unallowed char in: ");
FLASH_STRING(f_sherr_form, "Error: invalid format entered, expected: ");
FLASH_STRING(f_sherr_ex3dig, "3 digits separated by :");
FLASH_STRING(f_sherr_ex3digdot, "3 digits separated by .");
FLASH_STRING(f_sherr_exip, "4 octets separated by .");
FLASH_STRING(f_sherr_extmp, "2 digits separated by space (0-65 *C)");
FLASH_STRING(f_sherr_exhour, "hour not within 0-23");
FLASH_STRING(f_sherr_exmin, "minute not within 0-59");
FLASH_STRING(f_sherr_exsec, "second not within 0-59");
FLASH_STRING(f_sherr_exoct, "octet not within 0-255");
FLASH_STRING(f_sherr_exstmp, "temp not within 0-65");

FLASH_STRING(f_sherr_exday, "day not within 1-31");
FLASH_STRING(f_sherr_exmon, "month not within 1-12");
FLASH_STRING(f_sherr_exyea, "year not within 2010-3600");

FLASH_STRING(f_sherr_hhmmmis, "Error: Parameter in the form HH:MM:SS missing!");
FLASH_STRING(f_sherr_ddmmmis, "Error: Parameter in the form DD.MM.YYYY missing!");

FLASH_STRING(f_sherr, "Warning: Temperature too low for too long!");

FLASH_STRING(f_sht_timesav, "Time successfully changed to ");
FLASH_STRING(f_sht_datesav, "Date successfully changed to ");
FLASH_STRING(f_sht_ipsav, "IP address successfully changed to ");
FLASH_STRING(f_sht_tmpsav, "Air condition successfully set to ");
FLASH_STRING(f_sht_airoff, "Air condition successfully turned OFF");

FLASH_STRING(f_rtc_ok, "RTC has set the system time");
FLASH_STRING(f_rtc_fail, "Unable to sync with the RTC");

FLASH_STRING(f_shc_cn, "Channel #");
FLASH_STRING(f_shc_pn, "Program #");
FLASH_STRING(f_shc_pr, "        Program: ");
FLASH_STRING(f_shc_typ, "            Typ: ");
FLASH_STRING(f_shc_cfg, " config: ");
FLASH_STRING(f_shc_st, "Statisch");
FLASH_STRING(f_shc_start, "          Start: ");
FLASH_STRING(f_shc_stop, "           Stop: ");
FLASH_STRING(f_shc_ast, "Astronomisch");
FLASH_STRING(f_shc_stdel, "    Start Delay: ");
FLASH_STRING(f_shc_stodel, "     Stop Delay: ");
FLASH_STRING(f_shc_min, " Minuten");
FLASH_STRING(f_shc_sec, " Sekunden");
FLASH_STRING(f_shc_1min, " Minute");
FLASH_STRING(f_shc_1sec, " Sekunde");
FLASH_STRING(f_shc_clock, " Uhr");
FLASH_STRING(f_shc_delay, " Einschalt Verzoegerung: ");
FLASH_STRING(f_shc_man, "Manuell");

FLASH_STRING(f_prompt, "> ");

FLASH_STRING(f_ship, "Current ip address: ");
FLASH_STRING(f_mem, "Free memory: ");

FLASH_STRING(thome, "Home");
FLASH_STRING(tchannels, "Steuerkan&auml;le");
FLASH_STRING(tdate, "Datum und Uhrzeit Einstellen");
FLASH_STRING(tsetp, "Zuordnung Steuerkanal zu Programm Einstellen");
FLASH_STRING(tprog, "Programme");
FLASH_STRING(tprogset, "Programm Einstellen");
FLASH_STRING(tip, "IP Adresse Einstellen");
FLASH_STRING(tair, "Klima Einstellung");


FLASH_STRING(hhead,
	     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">"
	     "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"de-de\" lang=\"de-de\" dir=\"ltr\">"
	     "<head><title>");

FLASH_STRING(hmenu,
	     "</title>"
	     "</head>"
	     "<body>"
	     "<p>"
	     "<a href=\"index.html\">Home</a> | "
	     "<a href=\"setdate.html\">Zeit Einstellen</a> | "
	     "<a href=\"setip.html\">IP Einstellen</a></li> | "
	     "<a href=\"air.html\">Klima Einstellung</a></li> | "
	     "<a href=\"channels.html\">Kan&auml;le</a> | "
	     "<a href=\"programs.html\">Programme</a>"
	     "</p>");

FLASH_STRING(hfoot,
	     "<hr>"
	     "<p>powered by <a href=\"https://github.com/TLINDEN/TerraDuino\">TerraDuino</a></p>"
	     "</body>"
	     "</html>");

FLASH_STRING(ch1, "<h4>&Uuml;bersicht</h4>");
FLASH_STRING(chc, "<h4>Steuerkan&auml;le</h4>");
FLASH_STRING(chd, "<h4>Datum und Uhrzeit Einstellen</h4>"
	     "<p>Geben Sie die Werte ein und Dr&uuml;cken Sie \"Einstellen\". "
	     "Es ist insbesondere bei der Uhrzeit ratsam, sich zu beeilen.</p>");
FLASH_STRING(chs, "<h4>Zuordnung Steuerkanal zu Programm Einstellen</h4>");
FLASH_STRING(chp, "<h4>Programme &Uuml;bersicht</h4>");
FLASH_STRING(chps, "<h4>Programm Einstellen</h4>");
FLASH_STRING(chip,
	     "<h4>IP Adresse Einstellen</h4>"
	     "<p>"
	     "Geben Sie eine statische IP Adresse ein, die in Ihrem LAN "
	     "noch nicht in Verwendung ist. Booten Sie den Terraduino "
	     "anschliessend.<br/>"
	     "Wenn Sie einen Fehler gemacht haben sollten und Sie den "
	     "Terraduino nicht mehr erreichen k&ouml;nnen, m&uuml;ssen "
	     "Sie ihn resetten: Serielle Schnittstelle (9600,1,N), booten, "
	     "\"R\" eingeben (Ohne Anf&uuml;hrungszeichen!). Der Terraduino "
	     "bootet dann mit der default Adresse: <b>10.0.0.1</b>.<br/> "
	     "<b>VORSICHT: Dabei gehen alle Einstellungen verloren!</b>"
	     "</p>"
);
FLASH_STRING(chair,
	     "<h4>Klima Einstellung</h4>"
	     "<p>"
	     "Stellen Sie ein, ob bei Temperatur&auml;nderungen der L&uuml;fter "
	     "eingeschaltet werden soll und bei welchen Grenzwerten."
	     "</p>");


FLASH_STRING(tablechan,
	     "<table border=\"0\">"
	     "<tr>"
	     "  <th>Kanal</th>"
	     "  <th>Zugewiesenes Programm</th>"
	     "</tr>");

FLASH_STRING(tablesd,
	     "<table border=\"0\">"
	     " <tr>"
	     "   <td align=\"center\">TT</td>"
	     "   <td align=\"center\">MM</td>"
	     "   <td align=\"center\">JJJJ</td>"
	     "   <td align=\"center\">HH</td>"
	     "   <td align=\"center\">MM</td>"
	     "   <td align=\"center\">SS</td>"
	     " </tr>");
FLASH_STRING(tableprog,
	     "<table border=\"0\">"
	     "<tr>"
	     "  <th>Nummer</th>"
	     "  <th>Programm</th>"
	     "</tr>");
FLASH_STRING(tableprogset,
	     "<table border=\"0\">"
	     "<tr>"
	     "<td align=\"right\" colspan=\"2\">");

FLASH_STRING(tableair,
	     "<table border=\"0\">"
	     "<tr>"
	     "<td align=\"right\">Aktivieren</td>"
	     "<td>");


FLASH_STRING(chlinkl, "<a href=\"setprogram.html?channel=");
FLASH_STRING(chlinkr, "\">");
FLASH_STRING(chlinke, "</a>");

FLASH_STRING(prlinkl, "<a href=\"editprogram.html?program=");

FLASH_STRING(table, "<table border=\"0\" cellspacing=\"10\">");
FLASH_STRING(tablesm, "<table border=\"0\" cellpadding=\"2\">");
FLASH_STRING(tdr, "<td align=\"right\">");
FLASH_STRING(tdrt, "<td align=\"right\" valign=\"top\">");
FLASH_STRING(td, "<td>");
FLASH_STRING(tde, "</td>");
FLASH_STRING(tra, "<tr>");
FLASH_STRING(tre, "</tr>");
FLASH_STRING(tablee, "</table>");
FLASH_STRING(br, "<br/>");
FLASH_STRING(itemp, "Temperatur:");
FLASH_STRING(igrad, " &deg;C");
FLASH_STRING(ihum, "Luftfeuchte:");
FLASH_STRING(iperc, " %");
FLASH_STRING(itime, "Zeit:");
FLASH_STRING(imode, "Steuermodus:");
FLASH_STRING(isunr, "Sonnenaufgang:");
FLASH_STRING(isuns, "Sonnenuntergang:");
FLASH_STRING(ichan, "Status Steuerkan&auml;le:");
FLASH_STRING(iklim, "Status Klimasteuerung: ");
FLASH_STRING(imm, "Manuell");
FLASH_STRING(ima, "Automatisch");
FLASH_STRING(irun, " in Betrieb ");
FLASH_STRING(ioff, " Abgeschaltet ");

FLASH_STRING(bis, " - ");

FLASH_STRING(msg1, "<span style=\"color: green\">");
FLASH_STRING(msg2, "</span>");

FLASH_STRING(sdform, "<form name=\"setdate\" action=\"setdate.html\" method=\"POST\">");

FLASH_STRING(sdfi2,
	     "   <td align=\"center\">"
	     "<input type=\"text\" size=\"2\" maxlength=\"2\" name=\"");
FLASH_STRING(sdfi4,
	     "   <td align=\"center\">"
	     "<input type=\"text\" size=\"4\" maxlength=\"4\" name=\"");
FLASH_STRING(sdfdst,
	     "   <td align=\"left\" colspan=\"6\">Sommerzeit:"
	     "   <input name=\"6\" type=\"text\" size=\"1\" maxlength=\"1\" value=\"");
FLASH_STRING(sdfdsth,    "     (0 = Winterzeit, 1 = Sommerzeit)");

FLASH_STRING(sdfv, "\" value=\"");
FLASH_STRING(sdfe, "\"/>");

FLASH_STRING(submit, "<input type=\"submit\" name=\"submit\" value=\"Einstellen\"/>");

FLASH_STRING(sddone, "Datum und Uhrzeit eingestellt");

FLASH_STRING(spform, "<form name=\"setprogram\" action=\"setprogram.html\" method=\"POST\">");
FLASH_STRING(sdchannel, "Steuerkanal: ");
FLASH_STRING(sdpc, "Zugeordnetes Programm:");
FLASH_STRING(hidden, "<input type=\"hidden\" name=\"0\" value=\"");
FLASH_STRING(spsel,  "<select name=\"1\">");
FLASH_STRING(opt, "<option value=\"");
FLASH_STRING(opte, "</option>");
FLASH_STRING(sele, "</select>");
FLASH_STRING(selected, " selected='selected' ");

FLASH_STRING(forme, "</form>");
FLASH_STRING(err_unknchan, "Fehler: Unbekannter Steuerkanal!");
FLASH_STRING(err_unknid, "Fehler: Channel ID nicht angegeben!");
FLASH_STRING(err_unknpr, "Fehler: Unbekanntes Programm!");

FLASH_STRING(spdone, "Steuerkanal neuem Programm zugeordnet");

FLASH_STRING(spsform,  "<form name=\"editprogram\" action=\"editprogram.html\" method=\"POST\">");

FLASH_STRING(spf2,
	     "<input type=\"text\" size=\"2\" maxlength=\"2\" name=\"");
FLASH_STRING(spf5,
	     "<input type=\"text\" size=\"5\" maxlength=\"5\" name=\"");

FLASH_STRING(spcooldown, "Einschalt Verz&ouml;gerung (Cooldown Zeit) (Minuten)");

FLASH_STRING(err_unknprog, "Fehler: Unbekanntes Programm!");
FLASH_STRING(err_unknprogid, "Fehler: Program ID nicht angegeben!");

FLASH_STRING(err_type, "Fehler: Typ != 0|1|2!")
FLASH_STRING(err_hour1,"Fehler: Start Stunde ! 0-23!");
FLASH_STRING(err_min1, "Fehler: Start Minute ! 0-59!");
FLASH_STRING(err_hour2, "Fehler: Stop Stunde ! 0-23!");
FLASH_STRING(err_min2, "Fehler: Stop Minute ! 0-59!");
FLASH_STRING(err_delay1, "Fehler: Start Verz&ouml;gerung ! 0-65536!");
FLASH_STRING(err_delay2, "Fehler: Stop Verz&ouml;gerung ! 0-65536!");
FLASH_STRING(err_cooldown, "Fehler: Einschalt Verz&ouml;gerung ! 0-65536!");

FLASH_STRING(err_ip, "Fehler: IP Adresse ! (1-255).(0-255).(0-255).(0-255)!");

FLASH_STRING(progdone, "Programm wurde gespeichert");

FLASH_STRING(ipform,
	     "<form name=\"setip\" action=\"setip.html\" method=\"POST\">"
	     "IP Adresse: ");
FLASH_STRING(spf3,
	     "<input type=\"text\" size=\"3\" maxlength=\"3\" name=\"");
FLASH_STRING(ipdone, "Neue IP Adresse eingestellt");

FLASH_STRING(airform, "<form name=\"setdate\" action=\"air.html\" method=\"POST\">");
FLASH_STRING(airactive, "Aktiv");
FLASH_STRING(airinactive, "Inaktiv");
FLASH_STRING(tmin, "Tmin (schaltet L&uuml;fter aus):");
FLASH_STRING(tmax, "Tmax (schaltet L&uuml;fter ein):");
FLASH_STRING(talarm, "<br/>Alarm wenn T weniger als:");
FLASH_STRING(err_air, "Fehler: Temperatur 0-65!");
FLASH_STRING(airdone, "Klimaeinstellungen gespeichert");


FLASH_STRING(err_hour, "Fehler: Stunde ! 0-23");
FLASH_STRING(err_min, "Fehler: Minute ! 0-59");
FLASH_STRING(err_sec, "Fehler: Sekunde ! 0-59");
FLASH_STRING(err_day, "Fehler: Tag ! 1-31");
FLASH_STRING(err_mon, "Fehler: Monat ! 1-12");
FLASH_STRING(err_yea, "Fehler: Jahr ! 2010-3600");
