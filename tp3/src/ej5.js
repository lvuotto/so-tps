var map = function () {
  var k = this.subreddit;
  var v = {
    "puntaje": this.score,
    "palabras": this.title.trim().split(/\s+/).length
  };
  emit(k, v);
};

var reduce = function (k, v) {
  var r = { "puntaje": 0, "palabras": 0 };
  for (var i = 0; i < v.length; i++){
    r.puntaje += v[i].puntaje;
    r.palabras += v[i].palabras;
  }

  return r;
};

var finalize = function (k, rv) {
  return 280 < rv.puntaje && rv.puntaje < 300 ? rv.palabras : -1;
};

var options = {
  "out": "ej5",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej5.find({ "value": { $gt: -1 } }).sort({ "value": -1 });
r.forEach(function (v) { print(v.value); });
