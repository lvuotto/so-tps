var map = function () {
  var k = this.username;
  var v = {
    votos: this.number_of_upvotes,
    cantidad: 1
  };
  emit(k, v);
};

var reduce = function (k, v) {
  var r = { votos: 0, cantidad: 0 };
  for (var i = 0; i < v.length; i++){
    r.votos += v[i].votos;
    r.cantidad += v[i].cantidad;
  }
  if (r.cantidad > 5) r.votos = -1;

  return r;
};

var options = {
  "out": "ej4"
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej4.find().sort({ "value.votos": -1 });
if (r.hasNext())
  print(r.next()._id);
