# -*- coding: utf-8 -*-

import os
import re
from settings import MEDIA_ROOT, RRD_DIR
from django.core.files import File
from os.path import join as pjoin
from tempfile import *

from string import join
from django.db import models
from django.core.urlresolvers import reverse
from django import forms
from django.db.models import Q



class Program (models.Model):
	TypeChoices      = ((0, 'automatic'), (1, 'static'), (2, 'manual'))
	number           = models.IntegerField()
	type             = models.IntegerField(choices = TypeChoices, default=0, verbose_name='Programm Typ')
	start_delay      = models.IntegerField(default=0, verbose_name='Einschaltverzögerung Minuten', blank=True)
	stop_delay       = models.IntegerField(default=0, verbose_name='Abschaltverzögerung Minuten', blank=True)
	start            = models.TimeField(blank=True, null=True, verbose_name='Manuelle Startzeit')
	stop             = models.TimeField(blank=True, null=True, verbose_name='Manuelle Stoppzeit')
	cooldown         = models.IntegerField(default=0, verbose_name='Abkühlungszeit bei Wiedereinschalten Minuten', blank=True)
	sleep            = models.DateTimeField(blank=True, null=True, verbose_name='Start Winterruhe')
	sleep_increment  = models.IntegerField(default=0, verbose_name='Winterruhe Verzögerungsreduktion Minuten pro Tag', blank=True)
	modified         = models.BooleanField(default=False)

	def __unicode__(self):
		cap = u""
		if self.type == 0:
			cap = u"Astro %s Einschaltverzögerung %d, Abschaltverzögerung %d, Cooldown %d" % (self.number, self.start_delay, self.stop_delay, self.cooldown)
		else:
			cap = u"Manual %s Start: %d:%d, Stopp: %d:%d" % ( self.number, self.start.hour, self.start.minute, self.stop.hour, self.stop.minute)
		if self.sleep:
			cap = cap + u" Winterruhe Begin %d.%d, %d Minuten Increment" % (self.sleep.hour, self.sleep.minute, self.sleep_increment)
		return cap

	def channel(self):
		ch = ""
		for channel in Channel.objects.filter(program = self):
			ch = "%s, %s" % (ch, channel)
		return ch

class Channel (models.Model):
	name     = models.CharField(max_length=256)
	number   = models.IntegerField()
	watt     = models.IntegerField()
	operates = models.BooleanField(default=False)
	program  = models.ForeignKey(Program, blank=True, null=True)
	modified = models.BooleanField(default=False)

	def __unicode__(self):
		return "%s %d W" % (self.name, self.watt)

	ordering = ['number']

class Log (models.Model):
	temperature = models.FloatField(default=0)
	humidity    = models.FloatField(default=0)
	uptime      = models.IntegerField(default=0)
	freememory   = models.IntegerField(default=0)
	devicedate  = models.DateTimeField(auto_now_add=True)
	timestamp   = models.DateTimeField(auto_now_add=True)

	def __unicode__(self):
		return u"%5.2f °C, %5.2f %% H, uptime counter: %d, %d bytes free memory, device date: %s" % (self.temperature, self.humidity, self.uptime, self.freememory, self.devicedate)

	def logtime(self):
		return self.timestamp.strftime('%d.%m.%Y %H:%M:%S')

	logtime.admin_order_field = 'timestamp'

class LogWatt (models.Model):
	channel   = models.ForeignKey(Channel)
	operates  = models.BooleanField(default=False)
	timestamp = models.DateTimeField(auto_now_add=True)

	def __unicode__(self):
		return "%s %d W/s" % (self.channel.name, self.channel.watt)
