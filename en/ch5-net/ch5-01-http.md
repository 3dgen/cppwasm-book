# 5.1 XMLHttpRequest

The HTTP protocol is the most commonly used transport protocol in web page. This section describes the use of the `XMLHttpRequest` object in Emscripten for data transfer.

## 5.1.1 Introduction to `XMLHttpRequest` object

The following JavaScript code is an example of getting HTTP data using the `XMLHttpRequest` object:

```js
	var request = new XMLHttpRequest();
	request.open("GET", "t1.txt", true);
	request.responseType = "text";	
	request.onreadystatechange = function(){
		if (request.readyState == 4) {
			if (request.status == 200) {
				console.log(request.responseText);
			}
			else{
				console.log(request.statusText);
			}
		}
	};
	request.send();
```

The code above creates an `XMLHttpRequest` object `request`, uses the asynchronous `GET` method to get the data of the remote *t1.txt*, and prints the obtained string by setting the `onreadystatechange` callback event. Browse the page, the console output is as follows:

![](images/01-xhr-js.png)

> **info**  For more details about `XMLHttpRequest`, please refer to[https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest](https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest)

## 5.1.2 Wrapping `XMLHttpRequest` object using C API

In order to avoid the UI hang, we usually use the asynchronous mode to initiate the HTTP request, so the C API of the `XMLHttpRequest` object is divided into two parts:

1. Functions that called by C, implemented by JavaScript. Including the creation of the `XMLHttpRequest` object, the initiation of HTTP requests etc.
1. Functions that called by JavaScript, implemented by C. Includes callback functions for various events.

Let's look at the C code first:

```c
//xhr_wrap1.cpp

//imp by JavaScript, call by C:
EM_PORT_API(void) XHRGet(const char* url);

//imp by C, call by JavaScript:
EM_PORT_API(void) XHROnMessage(const char* url, const char* data){
	printf("http request succeeded. URL: %s  contex: %s\n", url, data);
}

EM_PORT_API(void) XHROnError(const char* url, const int error_code){
	printf("http request failed. URL: %s  error code:%d\n", url, error_code);
}

int main(){
	XHRGet("t1.txt");
	XHRGet("t2.txt");
	XHRGet("t3.txt");
}
```

`XHRGet()` is implemented in JavaScript. The code of the import library is as follows:

```js
//pkg1.js
mergeInto(LibraryManager.library, {
    XHRGet: function (url) {
        return JS_XHRGet(Pointer_stringify(url));
    },
})
```

`XHRGet()` calls the `JS_XHRGet()` function, which is located in "xhr_wrap1.html":

```js
//xhr_wrap1.html
	function JS_XHRGet(url) {
		var request = new XMLHttpRequest();
		request.open("GET", url, true);
		request.responseType = "text";	
		request.url = url;
		request.onreadystatechange = function(){
			if (request.readyState == 4) {
				if (request.status == 200) {
					Module.ccall('XHROnMessage', 'null',
						['string', 'string'], [request.url, request.responseText]);
				}
				else{
					Module.ccall('XHROnError', 'null',
						['string', 'number'], [request.url, request.status]);
				}
			}
		};
		request.send();
	}
```

Compile with the following command:

```
emcc  xhr_wrap1.cpp --js-library pkg1.js -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" -o xhr_wrap1.js
```

Browse the page, the console outputs:

![](images/01-xhr-wrap1.png)

It can be seen that the program correctly processed the return value, and **because the asynchronous HTTP request is used, the order of return is not consistent with the order of the request**.

## 5.1.3 Extending callback

Sometimes we have multiple objects in C++ that need to initiate HTTP requests, and each object handles return value in differently way. At this time, the wrap method in Section 5.1.2 will not meet the needs. Therefore, we need to improve it by referring to the method in Section 4.3. The C code is as follows:

```c
//xhr_wrap2.cpp
struct XHR_CB;

//imp by JavaScript, call by C:
EM_PORT_API(void) XHRGet(const char* url, XHR_CB* cb);

//XHR callback interface:
class CXHRCallbackInterface{
public:
	CXHRCallbackInterface(){}
	virtual ~CXHRCallbackInterface(){}
	
	virtual void OnMessage(const char* url, const char* data) = 0;
	virtual void OnError(const char* url, const int code) = 0;
};

//XHR callback1:
class CXHRCallback1 : public CXHRCallbackInterface{
public:
	CXHRCallback1(){}
	virtual ~CXHRCallback1(){}
	
	void OnMessage(const char* url, const char* data) {
		printf("CXHRCallback1::OnMessage(); URL: %s  contex: %s\n", url, data);
	}
	void OnError(const char* url, const int code) {
		printf("CXHRCallback1::OnError(); URL: %s  error code:%d\n", url, code);
	}
};

//XHR callback2:
class CXHRCallback2 : public CXHRCallbackInterface{
public:
	CXHRCallback2(){}
	virtual ~CXHRCallback2(){}
	
	void OnMessage(const char* url, const char* data) {
		printf("CXHRCallback2::OnMessage(); URL: %s  contex: %s\n", url, data);
	}
	void OnError(const char* url, const int code) {
		printf("CXHRCallback2::OnError(); URL: %s  error code:%d\n", url, code);
	}
};

//imp by C, call by JavaScript:
EM_PORT_API(void) XHROnMessage(const char* url, const char* data, XHR_CB* cb){
	CXHRCallbackInterface* ci = (CXHRCallbackInterface*)cb;
	ci->OnMessage(url, data);
}

EM_PORT_API(void) XHROnError(const char* url, const int code, XHR_CB* cb){
	CXHRCallbackInterface* ci = (CXHRCallbackInterface*)cb;
	ci->OnError(url, code);
}

CXHRCallback1 cb1;
CXHRCallback2 cb2;

int main(){
	XHRGet("t1.txt", (XHR_CB*)&cb1);
	XHRGet("t2.txt", (XHR_CB*)&cb2);
	XHRGet("t3.txt", (XHR_CB*)&cb2);
}
```

The JavaScript import library is as follows:

```js
//xhr_wrap2.pkg
mergeInto(LibraryManager.library, {
    XHRGet: function (url, cb) {
        return JS_XHRGet(Pointer_stringify(url), cb);
    },
})
```

JavaScript code:

```js
//xhr_wrap2.html
	function JS_XHRGet(url, cb) {
		var request = new XMLHttpRequest();
		request.open("GET", url, true);
		request.responseType = "text";	
		request.url = url;
		request.wrap_cb = cb;
		request.onreadystatechange = function(){
			if (request.readyState == 4) {
				if (request.status == 200) {
					Module.ccall('XHROnMessage', 'null',
						['string', 'string', 'number'],
						[request.url, request.responseText, request.wrap_cb]);
				}
				else{
					Module.ccall('XHROnError', 'null',
						['string', 'number', 'number'],
						[request.url, request.status, request.wrap_cb]);
				}
			}
		};
		request.send();
	}
```

The core idea of the above code is that we bind a callback handler object `cb` for each `XHRGet()` request. When the HTTP request is completed, the bound `cb` object will be called to handle the event.

Compile with the following command:

```
emcc xhr_wrap2.cpp --js-library pkg2.js -s "EXTRA_EXPORTED_RUNTIME_METHODS=['ccall']" -o xhr_wrap2.js
```

Browse the page, the console outputs:

![](images/01-xhr-wrap2.png)