var map = function () {
  var key = this.subreddit;
  var value = {
    score_sum: this.score,
    count: 1
  };
  emit(key, value);
};

var reduce = function (k, v) {
  var reduced_v = { score_sum: 0, count: 0 }; 

  for (var i = 0; i < v.length; i++){
    reduced_v.score_sum += v[i].score_sum;
    reduced_v.count += v[i].count;
  }

  return reduced_v;
};

var finalize = function (k, reduced_v) {
  reduced_v.avg = reduced_v.score_sum / reduced_v.count;
  return reduced_v;
};

var options = {
  "out": "ej1",
  "finalize": finalize
};


conn = new Mongo();
db = conn.getDB("reddit");


db.posts.mapReduce(map, reduce, options);
var r = db.ej1.find().sort({ "value.avg": -1 })[0]
printjson(r);
