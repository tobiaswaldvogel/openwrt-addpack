L.ui.view.extend({
//	title: L.tr('Mount Points'),
//	description: L.tr('The following rules are currently active on this system.'),

	YesNoValue: L.cbi.DummyValue.extend({
		ucivalue: function(sid) {
			val = this.callSuper('ucivalue', sid)
			return val == '1' ? L.tr('yes') : L.tr('no')
		}
	}),

	DevValue: L.cbi.DummyValue.extend({
		ucivalue: function(sid) {
			var uci = this.ucipath(sid);
			var val = this.ownerMap.get(uci.config, uci.section, 'uuid');
			if (typeof(val) != 'undefined')
				return 'UUID: ' + val;
			var val = this.ownerMap.get(uci.config, uci.section, 'label');
			if (typeof(val) != 'undefined')
				return 'Label: ' + val;
			return this.callSuper('ucivalue', sid);
                }
	}),

	TargetValue: L.cbi.DummyValue.extend({
		ucivalue: function(sid) {
			var uci = this.ucipath(sid);
			if ('1' == this.ownerMap.get(uci.config, uci.section, 'is_rootfs'))
				return  '/overlay';
			return this.callSuper('ucivalue', sid);
		}
	}),

	extEdit: function(ev) {
		var self = ev.data.self;

		var m = new L.cbi.Modal('fstab', {
			caption:	L.tr('Mount entry'),
			ownerMap:	self.ownerMap,
		});

		var s = m.section(L.cbi.SingleSection, ev.data.sid, {
			anonymous:	true
		});

		s.option(L.cbi.CheckboxValue, 'enabled', {
			caption:        L.tr('Enable this mount'),
		});

		s.option(L.cbi.InputValue, 'target', {
       	                caption:        L.tr('Mount Point'),
			description:	L.tr('Specifies the directory the device is attached to'),
               	        placeholder:    '?',
                });

		s.option(L.cbi.InputValue, 'options', {
			caption:        L.tr('Mount options'),
			description:	L.tr('See "mount" manpage for details'),
			optional:	true,
			placeholder:    'defaults'
		});				

		s.option(L.cbi.CheckboxValue, 'is_rootfs', {
			caption:        L.tr('Use as root filesystem'),
			description:	L.tr('Configures this mount as overlay storage for block-extroot')
		});

		s.option(L.cbi.CheckboxValue, 'enabled_fsck', {
			caption:        L.tr('Run filesystem check'),
			description:	L.tr('Run a filesystem check before mounting the device')
		});
		

		m.on('save', function(ev) {
			ev.data.self.options.ownerMap.redraw();
		});
		m.on('apply', function(ev) {
			ev.data.self.options.ownerMap.redraw();
		});

		m.show();
	},

	execute: function() {
		var self = this;
		var m = new L.cbi.Map('fstab', {
			caption:	L.tr('Mount Points'),
			readonly:	!this.options.acls.fstab
		});

		var mount = m.section(L.cbi.TableSection, 'mount', {
			caption:	L.tr('Mount Points define at which point a memory device will be attached to the filesystem'),
			anonymous:	true,
			addremove:	true,
			extedit:	true,
			add_caption:	L.tr('Add mount point')
		});

		mount.on('extEdit', this.extEdit);

		mount.option(L.cbi.CheckboxValue, 'enabled', {
			caption:	L.tr('Enabled')
		});

		mount.option(self.DevValue, 'device', {
			caption:	L.tr('Device')
		});

		mount.option(self.TargetValue, 'target', {
			caption:        L.tr('Mount Point'),
			placeholder:	'?',
		});

		mount.option(L.cbi.DummyValue, 'fstype', {
			caption:	L.tr('FileSystem'),
			placeholder:	'?',
		});

		mount.option(L.cbi.DummyValue, 'options', {
			caption:        L.tr('Options'),
			placeholder:	'defaults'
		});

		mount.option(self.YesNoValue, 'is_rootfs', {
			caption:	L.tr('Root')
		});

		mount.option(self.YesNoValue, 'enabled_fsck', {
			caption:	L.tr('Check')
		});

		return m.insertInto('#map');
	}
});
