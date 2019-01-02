Module = {};
Module.onRuntimeInitialized = function() {
  postMessage("Worker Ready.");
}

onmessage = function(e){
  console.log("Worker: message from mainThread:" + e.data);
  console.log("Worker: mission start.");
  var p = Module._Pi(e.data);
  postMessage(p);
  console.log("Worker: mission finished.");
}