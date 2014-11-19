var map = function () {
  emit(this.username, this.number_of_upvotes);
};

var reduce = function (k, v) {
  return v.length <= 5 ? Array.sum(v) : 0;
};

var finalize = function (k, v) {
  return v;
};

var options = {
  "out": "ej4",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej4.find().sort({ "value": -1 })[0]
printjson(r);
