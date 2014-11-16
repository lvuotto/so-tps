var map = function () {
	var key = this.subreddit;
	var value = {
    score_sum: this.score,
    count: 1
  };
	emit(key, value);
};
