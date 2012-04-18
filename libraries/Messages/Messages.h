FLASH_STRING(f_failed_dht, "Failed to read from DHT");

FLASH_STRING(f_sht_temp, "Temperature: ");
FLASH_STRING(f_grad, " *C");
FLASH_STRING(f_percent, " %");
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
	     "  h            - show help\n");

FLASH_STRING(f_sh_reset, "Resetting Config");
FLASH_STRING(f_sh_initres, "Initiating a soft reset");
FLASH_STRING(f_sh_inv, "Invalid Command (try 'h' to get help) ");

FLASH_STRING(f_shp_tit, "Current PIN states:");
FLASH_STRING(f_shp_head, "Switch | MAIN | Ch1 | Ch2 | Ch3 | Ch4 | Ch5 | Ch6"); 
FLASH_STRING(f_shp_line, "-------+------+-----+-----+-----+-----+-----+-----");
FLASH_STRING(f_shp_pin, "  PIN  |  ");
FLASH_STRING(f_shp_pipe, "|  ");
FLASH_STRING(f_shp_mode, " Mode  |   ");
FLASH_STRING(f_shp_rel, " Relay |      ");
FLASH_STRING(f_shp_2sp, "  ");

FLASH_STRING(f_sherr_char, "Error: unallowed char in: ");
FLASH_STRING(f_sherr_form, "Error: invalid format entered, expected: ");
FLASH_STRING(f_sherr_ex3dig, "3 digits separated by :");
FLASH_STRING(f_sherr_ex3digdot, "3 digits separated by .");
FLASH_STRING(f_sherr_exip, "4 octets separated by .");
FLASH_STRING(f_sherr_exhour, "hour not within 0-23");
FLASH_STRING(f_sherr_exmin, "minute not within 0-59");
FLASH_STRING(f_sherr_exsec, "second not within 0-59");
FLASH_STRING(f_sherr_exoct, "octet not within 0-255");

FLASH_STRING(f_sherr_exday, "day not within 1-31");
FLASH_STRING(f_sherr_exmon, "month not within 1-12");
FLASH_STRING(f_sherr_exyea, "year not within 2010-3600");

FLASH_STRING(f_sherr_hhmmmis, "Error: Parameter in the form HH:MM:SS missing!");
FLASH_STRING(f_sherr_ddmmmis, "Error: Parameter in the form DD.MM.YYYY missing!");

FLASH_STRING(f_sht_timesav, "Time successfully changed to ");
FLASH_STRING(f_sht_datesav, "Date successfully changed to ");
FLASH_STRING(f_sht_ipsav, "IP address successfully changed to ");

FLASH_STRING(f_rtc_ok, "RTC has set the system time");
FLASH_STRING(f_rtc_fail, "Unable to sync with the RTC");

