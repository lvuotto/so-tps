var map = function () {
  var k = this.subreddit;
  var v = {
    "puntaje": this.score,
    "cantidad": 1
  };
  emit(k, v);
};

var reduce = function (k, v) {
  var r = { "puntaje": 0, "cantidad": 0 }; 
  for (var i = 0; i < v.length; i++){
    r.puntaje += v[i].puntaje;
    r.cantidad += v[i].cantidad;
  }

  return r;
};

var finalize = function (k, rv) {
  return rv.puntaje / rv.cantidad;
};

var options = {
  "out": "ej1",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej1.find();
if (r.hasNext()) {
  var m = r.next();
  while (r.hasNext()) {
    var v = r.next();
    m = v.value > m.value ? v : m;
  }
  print(m._id);
}
