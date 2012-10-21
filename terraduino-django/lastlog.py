#!/usr/local/bin/python

import sys, os, getopt, syslog, re, os
import ConfigParser, time, socket
from datetime import datetime as datefn

os.environ['DJANGO_SETTINGS_MODULE'] = 'settings'

sys.path.append('/home/scip/www/daemon.de/blog')
sys.path.append('/home/scip/www/daemon.de/blog/python')

from django.core.management import execute_manager

try:
    import settings # Assumed to be in the same directory.
except ImportError:
    import sys
    sys.stderr.write("Error: Can't find the file 'settings.py'")
    sys.exit(1)

from terraduino.models import *
from django.db.models import Q

print Log.objects.all().order_by("-timestamp")[0]

sys.exit(0)

