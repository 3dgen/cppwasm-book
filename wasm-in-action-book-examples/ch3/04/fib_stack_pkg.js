mergeInto(LibraryManager.library, {
    js_print_fib: function (ptr, count) {
        var str = 'js_print_fib: ';
        for (var i = 0; i < count; i++){
          str += Module.HEAP32[(ptr >> 2) + i];
          str += ' ';
        }
        console.log(str);
    }
})