const heatshrink = require("node-heatshrink");
let deasync = require("deasync");

deasync.promise = function(fn) {
	return function () {
		var done = false
		var args = Array.prototype.slice.apply(arguments)
		var err
		var res

        fn.apply(this, args).then(resolve).catch(reject);

		deasync.loopWhile(function () {
			return !done
		})
		if (err)
			throw err

		return res
		function resolve(r) {
			res = r
			done = true
		}

		function reject(e) {
			err = e
			done = true
		}
	}
};

module.exports = {
    compress: function compress(data) {
        return heatshrink.encode(data, 8, 6);
    }
};
