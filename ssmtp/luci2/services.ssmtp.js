L.ui.view.extend({
	execute: function() {
		var self = this;
		var m = new L.cbi.Map('ssmtp', {
			caption:	L.tr('Email notifications'),
			description:	L.tr('Email notifications settings for ssmtp.'),
		});

		var s = m.section(L.cbi.TypedSection, 'ssmtp', {
			caption:	L.tr('Mail options'),
			readonly:	!this.options.acls.ssmtp
		});

		s.option(L.cbi.InputValue, 'email', {
			caption:	L.tr('Email addresss to notify')
		});

		s.option(L.cbi.InputValue, 'mx', {
			caption:	L.tr('Mail server')
		});

		s.option(L.cbi.InputValue, 'sender', {
			caption:	L.tr('Sender address')
		});

		s.option(L.cbi.InputValue, 'user', {
			caption:	L.tr('User ID'),
			optional:	true
		});

		s.option(L.cbi.PasswordValue, 'pwd', {
			caption:	L.tr('Password'),
			optional:	true
		});

		s.option(L.cbi.CheckboxValue, 'TLS', {
			caption:	L.tr('Use TLS'),
			optional:	true
		});

		return m.insertInto('#map');
	}
});
