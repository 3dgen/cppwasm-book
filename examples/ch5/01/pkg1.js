mergeInto(LibraryManager.library, {
    XHRGet: function (url) {
        return JS_XHRGet(Pointer_stringify(url));
    },
})