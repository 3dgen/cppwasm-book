mergeInto(LibraryManager.library, {
	ButtonCreate: function () {
		return js_ButtonCreate();
    },
	
	ButtonAddRef: function (button_id) {
		js_ButtonAddRef(button_id);
    },
	
	ButtonRelease: function (button_id) {
		return js_ButtonRelease(button_id);
	},

	ButtonSetInnerHtml: function(button_id, str) {
		js_ButtonSetInnerHtml(button_id, Pointer_stringify(str));
	}
})