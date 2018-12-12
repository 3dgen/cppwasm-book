mergeInto(LibraryManager.library, {
    WSNew: function (url, conn) {
        return JS_WSNew(Pointer_stringify(url), conn);
    },

    WSSend: function (ws, data) {
        return JS_WSSend(ws, Pointer_stringify(data));
    },
	
	WSDelete: function (ws) {
		return JS_WSDelete(ws);
	}
})