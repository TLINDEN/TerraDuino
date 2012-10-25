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
	     "Available commands: \r\n"
	     "  r                   - reset EEPROM configuration to defaults and do a\r\n"
	     "                        soft reset\r\n"
	     "  b                   - do a soft reset\r\n"
	     "  s                   - display T and H sensor data and date+time\r\n"
	     "  i                   - show ip address\r\n"
	     "  i x.x.x.x y.y.y.y   - set ip address (x) and default-gw (y)\r\n"
	     "  d DD.MM.YYYY        - set date\r\n"
	     "  t HH:MM:SS          - set time (enter wintertime!)\r\n"
	     "  a Tmin Tmax         - set aircondition, leave params to turn it off\r\n"
	     "  c                   - dump channel config\r\n"
	     "  p                   - dump program config\r\n"
	     "  P                   - dump PIN states\r\n"
	     "  m                   - show free memory\r\n"
	     "  h                   - show help\r\n");

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
FLASH_STRING(f_sherr_exip, "2 x 4 octets separated by .");
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

FLASH_STRING(f_rtc_ok, "DS1307 RTC clock")
FLASH_STRING(f_rtc_fail, "Unable to sync with the DS1307 RTC");

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

FLASH_STRING(f_prompt, "enter command (h for help)> ");

FLASH_STRING(f_ship, "Current ip address: ");
FLASH_STRING(f_mem, "Free memory: ");

FLASH_STRING(err_hour1,"Fehler: Start Stunde ! 0-23!");
FLASH_STRING(err_min1, "Fehler: Start Minute ! 0-59!");
FLASH_STRING(err_hour2, "Fehler: Stop Stunde ! 0-23!");
FLASH_STRING(err_min2, "Fehler: Stop Minute ! 0-59!");
FLASH_STRING(err_delay1, "Fehler: Start Verz&ouml;gerung ! 0-65536!");
FLASH_STRING(err_delay2, "Fehler: Stop Verz&ouml;gerung ! 0-65536!");
FLASH_STRING(err_cooldown, "Fehler: Einschalt Verz&ouml;gerung ! 0-65536!");
FLASH_STRING(err_ip, "Fehler: IP Adresse ! (1-255).(0-255).(0-255).(0-255)!");
FLASH_STRING(err_hour, "Fehler: Stunde ! 0-23");
FLASH_STRING(err_min, "Fehler: Minute ! 0-59");
FLASH_STRING(err_sec, "Fehler: Sekunde ! 0-59");
FLASH_STRING(err_day, "Fehler: Tag ! 1-31");
FLASH_STRING(err_mon, "Fehler: Monat ! 1-12");
FLASH_STRING(err_yea, "Fehler: Jahr ! 2010-3600");
FLASH_STRING(err_sleepmon, "Fehler: Start Winterruhe Monat ! 1-12!");
FLASH_STRING(err_sleepincr, "Fehler: Inkrement ! 0-120!");

FLASH_STRING(f_init_dberror, "  DB version does not match! Re-initializing DB, Existing settings will be overwritten!");
FLASH_STRING(f_init, "Initializing ");
FLASH_STRING(f_init_dht, "DHT22 Temperature and Humidity sensor");
FLASH_STRING(f_init_wire, "Wire SPI/ISP communication protocol");
FLASH_STRING(f_init_eep, "EEProm");
FLASH_STRING(f_init_dbread, "  Reading DB from EEProm");
FLASH_STRING(f_init_dbok, "  Fetched matching DB from EEprom with version ");
FLASH_STRING(f_init_speak, "PC 8ohm speaker");
FLASH_STRING(f_init_air, "Relay channel 8 air fan");
FLASH_STRING(f_init_sw, "Switches");
FLASH_STRING(f_init_relay, "220V/1A relays ");
FLASH_STRING(f_init_ok, " ok");
FLASH_STRING(f_init_timers, "Program timers");
FLASH_STRING(f_init_eth, "Ethernet Wiznet W5100 with ip address ");
FLASH_STRING(f_init_www, "Webserver on tcp port 80");
FLASH_STRING(f_init_wdt, "Watchdog timer to ");
FLASH_STRING(f_init_done, "Startup complete");

FLASH_STRING(ipgw, " Default Gateway:");
