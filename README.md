#### Introduction

This is the source code and hardware plan for a terrarium controller I am building.

It uses an Arduino(TM) microcontroller for the processing of signals and relays.

Features include:

* turn on/off relay channel based on real sunrise/sunset time
* user can add delays for each channel, so that the channels turn on one after one
* channels can be programmed statically as well (static start and stop time)
* a cooldown time can be specified per channel for lamps which need to cooldown bevore turned on again
* an air condition which gets triggered by a temperature sensor
* controllable via web interface
* controllable via serial console

#### Einfuehrung

Hier wird der Sourcecode f�r Terraduino veroeffentlicht, mit dem ich mein Terrarium steuere.

Der Controller basiert auf einem Arduino Board, der die Signale und Relaisschaltungen verarbeitet.

Features:

* Relais basierend auf Sonnenauf- und Untergang schalten
* man kann pro Kanal (Relais) eine Start- und Stopverzoegerung einstellen, so dass die Kanaele nacheinander schalten
* statische Programme auch moeglich (feste Start- und Stopzeit)
* pro Kanal kann eine Einschaltverzoegerung (cooldown) angegeben werden fuer Lampen, die sich nach erneutem Einschalten erst abkuehlen muessen
* Lueftersteuerung basierend auf Temperatursensor
* alles einstellbar ueber ein Webinterface
* alles einstellbar ueber die serielle Console

Project Home: http://www.daemon.de/TerraDuino
