from terraduino.models import *
from django.contrib import admin

class ChannelAdmin(admin.ModelAdmin):
	list_display = ('number', 'name', 'watt', 'operates', 'modified', 'program')
	list_editable = ('name', 'watt', 'program',)
	readonly_fields = ('modified',)
	ordering = ['name']

	def save_model(self, request, obj, form, change):
		obj.modified = True
		obj.save()

class ProgramAdmin(admin.ModelAdmin):
	list_display = ('__unicode__', 'number', 'modified', 'channel')
	readonly_fields = ('number', 'modified')
	ordering = ['number']
	fieldsets = (
		(None, {
			'fields': (('number', 'type'), ('start_delay', 'stop_delay'), ('start', 'stop'), ('cooldown'), ('sleep', 'sleep_increment')),
			'classes': ('wide',)
		}),
	)
	def save_model(self, request, obj, form, change):
		obj.modified = True
		obj.save()

class LogAdmin(admin.ModelAdmin):
	list_display = ('logtime', '__unicode__',)

class LogWattAdmin(admin.ModelAdmin):
	list_display = ('timestamp', 'operates', '__unicode__',)

admin.site.register(Channel, ChannelAdmin)
admin.site.register(Program, ProgramAdmin)
admin.site.register(Log, LogAdmin)
admin.site.register(LogWatt, LogWattAdmin)

