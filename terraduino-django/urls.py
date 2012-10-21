from django.conf.urls.defaults import *

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

from django.contrib.auth.decorators import login_required
from django.contrib import databrowse

urlpatterns = patterns('',
    (r'^(?P<c1>\d+)/(?P<c2>\d+)/(?P<c3>\d+)/(?P<c4>\d+)/(?P<c5>\d+)/(?P<c6>\d+)/(?P<c7>\d+)/(?P<c8>\d+)/(?P<t>[\.\d]+)/(?P<h>[\.\d]+)/(?P<ts>\d+)/(?P<mem>\d+)/(?P<uptime>[\.\d]+)/$', 'terraduino.views.report'),
    (r'^modified/$', 'terraduino.views.modified'),
    (r'^rrd/channel/$', 'terraduino.views.rrdchannel'),
    (r'^rrd/channel/(?P<number>\d+)/$', 'terraduino.views.rrdchannel'),
    (r'^rrd/t/$', 'terraduino.views.rrdtemperature'),
    (r'^rrd/h/$', 'terraduino.views.rrdhumidity'),
    (r'^rrd/mem/$', 'terraduino.views.rrdmemory'),
    (r'^rrd/uptime/$', 'terraduino.views.rrduptime'),
    (r'^$', 'terraduino.views.public'),
)
