from django.contrib import messages
from django.http import HttpResponse, HttpResponseRedirect
from datetime import datetime, timedelta
from django.template.loader import render_to_string
from django.core.exceptions import ObjectDoesNotExist
from django.template import RequestContext
from django.contrib.auth.decorators import login_required
from django.core.paginator import Paginator, InvalidPage, EmptyPage
from django.core.urlresolvers import reverse
from django.shortcuts import render_to_response
from terraduino.models import *
from django.db.models import Q
from settings import MEDIA_ROOT, RRD_DIR

from operator import itemgetter

import time, sys

def public(request):
	channels = Channel.objects.all().order_by("number")
	programs = Program.objects.all().order_by("number")
	log = Log.objects.order_by("-timestamp")[0]
	return render_to_response("terraduino/public.html", dict(channels=channels, programs=programs, log=log), context_instance=RequestContext(request))


def modified(request):
	"""
	returns channels and programs which have been modified by admin
	so that terraduino can update its eeprom db and context
	"""
	channels = Channel.objects.filter(modified=True)
	programs = Program.objects.filter(modified=True)
	for channel in channels:
		channel.modified = False
		channel.save()
	for program in programs:
		program.modified = False
		program.save()
	return render_to_response("terraduino/modified.html", dict(channels=channels, programs=programs), context_instance=RequestContext(request))


def report(request, c1, c2, c3, c4, c5, c6, c7, c8, t, h, ts, mem, uptime):
	"""
	receives reporting and status data from terraduino by get request
	and save everything into log(watt) objects
	"""
	channels = (c1, c2, c3, c4, c5, c6, c7, c8)
	log = Log(
		temperature = float(t),
		humidity    = float(h),
		uptime      = float(uptime),
		freememory  = int(mem),
		devicedate  = datetime.fromtimestamp(int(ts)),
	)
	log.save()

	for N in range(0, 8):
		op = False
		if int(channels[N]) == 1:
			op = True
		c = Channel.objects.get(number=N)
		watt = LogWatt(channel = c, operates=op)
		watt.save()
		c.operates = op
		c.save()

	return HttpResponse('ok')

def val(value):
	"""
	simple helper to save typing, outputs rrd compliant number
	"""
	return HttpResponse("%d\n0" % value)

def five():
	"""
	returns the datetime object of the time 5 minutes before now
	"""
	return datetime.now() - timedelta(minutes=5)

def rrdchannel(request, number=0):
	"""
	outputs watts per last 5 minutes sinked by given channel in rrd format
	if there is no log entry younger than 5 minutes, return 0
	"""
	try:
		if number > 0:
			c = Channel.objects.get(number=int(number))
			log = LogWatt.objects.exclude(timestamp__lt=five()).filter(channel=c).order_by("-timestamp")[0]
			if log.operates == 1:
				return val(c.watt * 3600)
			else:
				return val(0)
		else:
			watts = 0
			for c in Channel.objects.all():
				try:
					log = LogWatt.objects.exclude(timestamp__lt=five()).filter(channel=c).order_by("-timestamp")[0]
					if log.operates == 1:
						watts = watts + (c.watt * 3600)
				except:
					pass
			return val(watts)
	except:
		return val(0)

def rrdtemperature(request):
	"""
	outputs temperature in rrd format
	if there is no log entry younger than 5 minutes, return 0
	"""
	try:
		log = Log.objects.exclude(timestamp__lt=five()).order_by("-timestamp")[0]
		return val(log.temperature)
	except:
		return val(0)

def rrdhumidity(request):
	"""
	outputs humidity in rrd format
	if there is no log entry younger than 5 minutes, return 0
	"""
	try:
		log = Log.objects.exclude(timestamp__lt=five()).order_by("-timestamp")[0]
		return val(log.humidity)
	except:
		return val(0)

def rrdmemory(request):
	"""
	outputs free memory in rrd format
	if there is no log entry younger than 5 minutes, return 0
	"""
	try:
		log = Log.objects.exclude(timestamp__lt=five()).order_by("-timestamp")[0]
		return val(log.freememory)
	except:
		return val(0)

def rrduptime(request):
	"""
	outputs uptime in rrd format
	if there is no log entry younger than 5 minutes, return 0
	"""
	try:
		log = Log.objects.exclude(timestamp__lt=five()).order_by("-timestamp")[0]
		return val(log.uptime)
	except:
		return val(0)

