mergeInto(LibraryManager.library, {
    XHRGet: function (url, cb) {
        return JS_XHRGet(Pointer_stringify(url), cb);
    },
})