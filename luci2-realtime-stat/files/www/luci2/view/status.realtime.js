L.ui.view.extend({
	PortValue: L.cbi.InputValue.extend({
		ucivalue: function(sid)
		{
			var v = this.callSuper('ucivalue', sid);
			if (typeof v == 'undefined')
				return '';

			return 9100 + parseInt(v);
		},

		formvalue: function(sid)
		{
			var v = $('#' + this.id(sid)).val();

			return parseInt(v) - 9100;
		}
	}),

	execute: function() {
		var self = this;

		var m = new L.cbi.Map('p910nd', {
			caption:     L.tr('p910nd printer daemon')
		});

		var s = m.section(L.cbi.TypedSection, 'p910nd', {
			caption:      L.tr('Printers'),
			addremove:    true,
			add_caption:  L.tr('Add shared printer …'),
		});

		s.option(L.cbi.CheckboxValue, 'enabled', {
			caption:     L.tr('Enabled'),
			initial:     0,
			enabled:     '1',
			disabled:    '0'
		});

		s.option(self.PortValue, 'port', {
			caption:     L.tr('Port'),
			description: L.tr('Specifies the listening port'),
			datatype:    'range(9100, 65535)'
		});

		s.option(L.cbi.InputValue, 'device', {
			caption:     L.tr('Device'),
			description: L.tr('Printer device path in the filesystem')
		});

		s.option(L.cbi.CheckboxValue, 'bidirectional', {
			caption:     L.tr('Bidirectional'),
			description: L.tr('Bidirectional copying'),
			initial:     1,
			enabled:     '1',
			disabled:    '0'
		});

		return m.insertInto('#map');
	}
});
